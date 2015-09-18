////////////////////////////////////////////////////////////////////////
/// \file  GENIEHelper.h
/// \brief Wrapper for generating neutrino interactions with GENIE
///
/// \version $Id: GENIEHelper.cxx,v 1.58 2012-11-28 23:04:03 rhatcher Exp $
/// \author  brebel@fnal.gov  rhatcher@fnal.gov
/// \update 2010-03-04 Sarah Budd added simple_flux
/// \update 2013-04-24 rhatcher adapt to R-2_8_0 interface; subset flux files
////////////////////////////////////////////////////////////////////////

// C/C++ includes
#include <math.h>
#include <map>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <glob.h>
#include <cstdlib>  // for unsetenv()

//ROOT includes
#include "TH1.h"
#include "TH2.h" //used by GAtmoFlux
#include "TFile.h"
#include "TDirectory.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TCollection.h"
#include "TSystem.h"
#include "TString.h"
#include "TRandom.h" //needed for gRandom to be defined
#include "TRegexp.h"
#include "TMath.h"
#include "TStopwatch.h"

//GENIE includes
#include "Conventions/GVersion.h"
#include "Conventions/Units.h"
#include "EVGCore/EventRecord.h"
#include "EVGDrivers/GMCJDriver.h"
#include "GHEP/GHepUtils.h"
#include "FluxDrivers/GCylindTH1Flux.h"
#include "FluxDrivers/GMonoEnergeticFlux.h"
#include "FluxDrivers/GNuMIFlux.h"
#include "FluxDrivers/GSimpleNtpFlux.h"
#include "FluxDrivers/GBartolAtmoFlux.h"  //for atmo nu generation
#include "FluxDrivers/GFlukaAtmo3DFlux.h" //for atmo nu generation
#include "FluxDrivers/GAtmoFlux.h"        //for atmo nu generation
#include "Conventions/Constants.h" //for calculating event kinematics
#ifndef GENIE_USE_ENVVAR
#include "Utils/AppInit.h"
#include "Utils/RunOpt.h"
#endif

#include "Geo/ROOTGeomAnalyzer.h"
#include "Geo/GeomVolSelectorFiducial.h"
#include "Geo/GeomVolSelectorRockBox.h"
#include "Utils/StringUtils.h"
#include "Utils/XmlParserUtils.h"
#include "Interaction/InitialState.h"
#include "Interaction/Interaction.h"
#include "Interaction/Kinematics.h"
#include "Interaction/KPhaseSpace.h"
#include "Interaction/ProcessInfo.h"
#include "Interaction/XclsTag.h"
#include "GHEP/GHepParticle.h"
#include "PDG/PDGCodeList.h"

// assumes in GENIE
#include "FluxDrivers/GFluxBlender.h"
#include "FluxDrivers/GFlavorMixerI.h"
#include "FluxDrivers/GFlavorMap.h"
#include "FluxDrivers/GFlavorMixerFactory.h"

//NuTools includes
#include "EventGeneratorBase/evgenbase.h"
#include "EventGeneratorBase/GENIE/GENIEHelper.h"
#include "SimulationBase/MCTruth.h"
#include "SimulationBase/MCFlux.h"
#include "SimulationBase/GTruth.h"
#include "SimulationBase/MCParticle.h"
#include "SimulationBase/MCNeutrino.h"

// Framework includes
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "cetlib/search_path.h"
#include "cetlib/getenv.h"
#include "cetlib/split_path.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#ifndef NO_IFDH_LIB
  // IFDHC 
  #include "ifdh.h"
#else
  // nothing doing ... use ifdef to hide any referece that might need header
  #include <cassert>
#endif

namespace evgb {

  static const int kNue      = 0;
  static const int kNueBar   = 1;
  static const int kNuMu     = 2;
  static const int kNuMuBar  = 3;
  static const int kNuTau    = 4;
  static const int kNuTauBar = 5;

  //--------------------------------------------------
  GENIEHelper::GENIEHelper(fhicl::ParameterSet const& pset,
			   TGeoManager*               geoManager,
			   std::string         const& rootFile,
			   double              const& detectorMass)
    : fGeoManager        (geoManager)
    , fGeoFile           (rootFile) 
    , fGenieEventRecord  (0)
    , fGeomD             (0)
    , fFluxD             (0)
    , fFluxD2GMCJD       (0)
    , fDriver            (0)
    , fIFDH              (0)
    , fHelperRandom      (0)
    , fUseHelperRndGen4GENIE(pset.get< bool                  >("UseHelperRndGen4GENIE",true))
    , fFluxType          (pset.get< std::string              >("FluxType")               )
    , fFluxSearchPaths   (pset.get< std::string              >("FluxSearchPaths","")     )
    , fFluxFilePatterns  (pset.get< std::vector<std::string> >("FluxFiles")              )
    , fMaxFluxFileMB     (pset.get< int                      >("MaxFluxFileMB",    2000) ) // 2GB max default
    , fFluxCopyMethod    (pset.get< std::string              >("FluxCopyMethod","DIRECT")) // "DIRECT" = old direct access method
    , fFluxCleanup       (pset.get< std::string              >("FluxCleanup","/var/tmp") ) // "ALWAYS", "NEVER", "/var/tmp"
    , fBeamName          (pset.get< std::string              >("BeamName")               )
    , fTopVolume         (pset.get< std::string              >("TopVolume")              )
    , fWorldVolume       ("volWorld")         
    , fDetLocation       (pset.get< std::string              >("DetectorLocation")       )
    , fFluxUpstreamZ     (pset.get< double                   >("FluxUpstreamZ",  -2.e30) )
    , fEventsPerSpill    (pset.get< double                   >("EventsPerSpill",      0) )
    , fPOTPerSpill       (pset.get< double                   >("POTPerSpill",     5.e13) )
    , fHistEventsPerSpill(0.)
    , fSpillEvents       (0)
    , fSpillExposure     (0.)
    , fTotalExposure     (0.)
    , fMonoEnergy        (pset.get< double                   >("MonoEnergy",        2.0) )
    , fBeamRadius        (pset.get< double                   >("BeamRadius",        3.0) )
    , fDetectorMass      (detectorMass)
    , fSurroundingMass   (pset.get< double                   >("SurroundingMass",    0.) )
    , fGlobalTimeOffset  (pset.get< double                   >("GlobalTimeOffset", 1.e4) )
    , fRandomTimeOffset  (pset.get< double                   >("RandomTimeOffset", 1.e4) )
    , fGenFlavors        (pset.get< std::vector<int>         >("GenFlavors")             )
    , fAtmoEmin          (pset.get< double                   >("AtmoEmin",          0.1) )
    , fAtmoEmax          (pset.get< double                   >("AtmoEmax",         10.0) )
    , fAtmoRl            (pset.get< double                   >("Rl",               20.0) )
    , fAtmoRt            (pset.get< double                   >("Rt",               20.0) )
    , fEnvironment       (pset.get< std::vector<std::string> >("Environment")            )
    , fXSecTable         (pset.get< std::string              >("XSecTable",          "") ) //e.g. "gxspl-NuMIsmall.xml"
    , fEventGeneratorList(pset.get< std::string              >("EventGeneratorList", "") ) // "Default"
    , fGXMLPATH          (pset.get< std::string              >("GXMLPATH",           "") )
    , fGMSGLAYOUT        (pset.get< std::string              >("GMSGLAYOUT",         "") ) // [BASIC] or SIMPLE
    , fGENIEMsgThresholds(pset.get< std::string              >("GENIEMsgThresholds", "") ) // : separate list of files
    , fGHepPrintLevel    (pset.get< int                      >("GHepPrintLevel",     -1) ) // see GHepRecord::SetPrintLevel() -1=no-print
    , fMixerConfig       (pset.get< std::string              >("MixerConfig",    "none") )
    , fMixerBaseline     (pset.get< double                   >("MixerBaseline",      0.) )
    , fFiducialCut       (pset.get< std::string              >("FiducialCut",    "none") )
    , fGeomScan          (pset.get< std::string              >("GeomScan",    "default") )
    , fDebugFlags        (pset.get< unsigned int             >("DebugFlags",          0) ) 
  {

    std::vector<double> beamCenter   (pset.get< std::vector<double> >("BeamCenter")   );
    std::vector<double> beamDirection(pset.get< std::vector<double> >("BeamDirection"));
    fBeamCenter.SetXYZ(beamCenter[0], beamCenter[1], beamCenter[2]);
    fBeamDirection.SetXYZ(beamDirection[0], beamDirection[1], beamDirection[2]);

    // special processing of GSEED (GENIE's random seed)... priority:
    //    if set in .fcl file RandomSeed variable, use that
    //    else if already set in environment use that
    //    else use evgb::GetRandomNumberSeed()
    int dfltseed;
    const char* gseedstr = std::getenv("GSEED");
    if ( gseedstr ) {
      dfltseed = strtol(gseedstr,NULL,0);
    } else {
      dfltseed = evgb::GetRandomNumberSeed();
    }
    int seedval = pset.get< int >("RandomSeed", dfltseed);
    // initialize random # generator for use within GENIEHelper
    mf::LogInfo("GENIEHelper") << "Init HelperRandom with seed " << seedval; 
    fHelperRandom = new TRandom3(seedval);

    /// Determine which flux files to use
    /// Do this after random number seed initialization for stability

    // for "ntuple" and "simple_flux" squeeze the patterns so there 
    // are no duplicates; for the others we want to preserve order
    if ( fFluxType.compare("ntuple")      == 0 ||
         fFluxType.compare("simple_flux") == 0 ||
         fFluxType.compare("dk2nu")       == 0    ) {
      // convert vector<> to a set<> and back to vector<>
      // to avoid having duplicate patterns in the list
      std::set<std::string> fluxpattset(fFluxFilePatterns.begin(),fFluxFilePatterns.end());
      //// if we weren't initializing from ctor we could do
      //std::copy(fFluxFilePatterns.begin(),fFluxFilePatterns.end(),std::inserter(fluxpattset,fluxpattset.begin()));
      fFluxFilePatterns.clear(); // clear vector, copy unique set back
      std::copy(fluxpattset.begin(),fluxpattset.end(),
                std::back_inserter(fFluxFilePatterns));
    }
    ExpandFluxPaths();
    if (fFluxCopyMethod == "DIRECT") ExpandFluxFilePatternsDirect();
    else                             ExpandFluxFilePatternsIFDH();

    /// Set the GENIE environment
    /// if using entries in the fEnvironment vector
    //    they should come in pairs of variable name key, then value

    // Process GXMLPATH extensions first, so they are available
    // when GENIE starts to get initialized; these might be
    // alternative locations for configurations (including
    // the GENIE Messenger system).
    SetGXMLPATH();

    // Also set GENIE log4cpp Messenger layout format before 
    // initializing GENIE (can't be changed after singleton is created)
    SetGMSGLAYOUT();

    // Now initialize GENIE Messenger service
    StartGENIEMessenger(pset.get<std::string>("ProductionMode","false"));

    // Determine EventGeneratorList to use
    FindEventGeneratorList();

    // Figure out which cross section file to use
    // post R-2_8_0 this actually triggers reading the file
    ReadXSecTable();

#ifndef GENIE_USE_ENVVAR
    // In case we're printing the event record, how verbose should it be
    genie::GHepRecord::SetPrintLevel(fGHepPrintLevel);

    // Set GENIE's random # seed
    mf::LogInfo("GENIEHelper") << "Init genie::utils::app_init::RandGen() with seed " << seedval; 
    genie::utils::app_init::RandGen(seedval);
#else
    // pre GENIE R-2_8_0 needs random # seed GSEED set in the environment
    // determined the seed to use above, now make sure it is set externally
    std::string seedstr = std::to_string(seedval); // part of C++11 <string>
    mf::LogInfo("GENIEHelper") << "Init GSEED env with seed " << seedval; 
    fEnvironment.push_back("GSEED");
    fEnvironment.push_back(seedstr);

    // pre R-2_8_0 uses environment variables to configure how GENIE runs
    std::ostringstream envlisttext;
    envlisttext << "setting GENIE environment: "; 
    for (size_t i = 0; i < fEnvironment.size(); i += 2) {
      std::string& key = fEnvironment[i];
      std::string& val = fEnvironment[i+1];
      gSystem->Setenv(key.c_str(), val.c_str());
      envlisttext << "\n   " << key  << " to \"" << val <<"\"";                                 
    }
    mf::LogInfo("GENIEHelper") << envlisttext.str(); 
#endif

    if ( fFluxType.compare("atmo") == 0 ) {
      
      if(fGenFlavors.size() != fSelectedFluxFiles.size()){
        mf::LogInfo("GENIEHelper") <<  "ERROR: The number of generated neutrino flavors (" 
                                   << fGenFlavors.size() << ") doesn't correspond to the number of files (" 
                                   << fSelectedFluxFiles.size() << ")!!!";
        exit(1);
      } else {
        for (size_t indx=0; indx < fGenFlavors.size(); ++indx ) {
          mf::LogInfo("GENIEHelper")
            <<  "atmo flux assignment : " << fGenFlavors[indx] << " " << fSelectedFluxFiles[indx];
        }
      }

      if(fEventsPerSpill !=1){
        mf::LogInfo("GENIEHelper") 
          <<  "ERROR: For Atmosphric Neutrino generation, EventPerSpill need to be 1!!";
        exit(1);
      }

      if (fFluxType.compare("atmo_FLUKA") == 0 ){
        mf::LogInfo("GENIEHelper") << "The sims are from FLUKA";
      }
      
      else if (fFluxType.compare("atmo_BARTOL") == 0 ){
        mf::LogInfo("GENIEHelper") << "The sims are from BARTOL";
      }      
      else {
        mf::LogInfo("GENIEHelper") << "Uknonwn flux simulation: " << fFluxType;
        exit(1);
      }
      
      mf::LogInfo("GENIEHelper") << "The energy range is between:  " << fAtmoEmin << " GeV and " 
                                 << fAtmoEmax << " GeV.";
  
      mf::LogInfo("GENIEHelper") << "Generation surface of: (" << fAtmoRl << "," 
                                 << fAtmoRt << ")";

    }// end if atmospheric fluxes
    
    // make the histograms
    if(fFluxType.compare("histogram") == 0){
      mf::LogInfo("GENIEHelper") << "setting beam direction and center at "
                                 << fBeamDirection.X() << " " << fBeamDirection.Y() << " " << fBeamDirection.Z()
                                 << " (" << fBeamCenter.X() << "," << fBeamCenter.Y() << "," << fBeamCenter.Z()
                                 << ") with radius " << fBeamRadius;

      TDirectory *savedir = gDirectory;
    
      fFluxHistograms.clear();

      TFile tf((*fSelectedFluxFiles.begin()).c_str());
      tf.ls();

      for(std::vector<int>::iterator flvitr = fGenFlavors.begin(); flvitr != fGenFlavors.end(); flvitr++){
        if(*flvitr ==  12) fFluxHistograms.push_back(dynamic_cast<TH1D *>(tf.Get("nue")));
        if(*flvitr == -12) fFluxHistograms.push_back(dynamic_cast<TH1D *>(tf.Get("nuebar")));
        if(*flvitr ==  14) fFluxHistograms.push_back(dynamic_cast<TH1D *>(tf.Get("numu")));
        if(*flvitr == -14) fFluxHistograms.push_back(dynamic_cast<TH1D *>(tf.Get("numubar")));
        if(*flvitr ==  16) fFluxHistograms.push_back(dynamic_cast<TH1D *>(tf.Get("nutau")));
        if(*flvitr == -16) fFluxHistograms.push_back(dynamic_cast<TH1D *>(tf.Get("nutaubar")));
      }

      for(unsigned int i = 0; i < fFluxHistograms.size(); ++i){
        fFluxHistograms[i]->SetDirectory(savedir);
        fTotalHistFlux += fFluxHistograms[i]->Integral();
      }

      mf::LogInfo("GENIEHelper") << "total histogram flux over desired flavors = " 
                                 << fTotalHistFlux;

    }//end if getting fluxes from histograms

    std::string flvlist;
    for ( std::vector<int>::iterator itr = fGenFlavors.begin(); itr != fGenFlavors.end(); itr++ )
      flvlist += Form(" %d",*itr);

    if(fFluxType.compare("mono")==0){
      fEventsPerSpill = 1;
      mf::LogInfo("GENIEHelper") 
        << "Generating monoenergetic (" << fMonoEnergy 
        << " GeV) neutrinos with the following flavors: " 
        << flvlist;
    }
    else{

      std::string fileliststr;
      if ( fSelectedFluxFiles.empty() ) {
        fileliststr = "NO FLUX FILES FOUND!";
        mf::LogWarning("GENIEHelper")  << fileliststr;
      }
      else {
        std::vector<std::string>::iterator sitr = fSelectedFluxFiles.begin();
        for ( ; sitr != fSelectedFluxFiles.end(); sitr++) {
          fileliststr +=  "\n\t";
          fileliststr += *sitr;
        }
      }
      mf::LogInfo("GENIEHelper") 
        << "Generating flux with the following flavors: " << flvlist
        << "\nand these file patterns: " << fileliststr;

    }

    if(fEventsPerSpill != 0)
      mf::LogInfo("GENIEHelper") << "Generating " << fEventsPerSpill 
                                 << " events for each spill";
    else
      mf::LogInfo("GENIEHelper") << "Using " << fPOTPerSpill << " pot for each spill";

    return;
  }

  //--------------------------------------------------
  GENIEHelper::~GENIEHelper()
  {
    // user request writing out the scan of the geometry
    if ( fGeomD && fMaxPathOutInfo != "" ) {
      genie::geometry::ROOTGeomAnalyzer* rgeom = 
        dynamic_cast<genie::geometry::ROOTGeomAnalyzer*>(fGeomD);

      string filename = "maxpathlength.xml";
      mf::LogInfo("GENIEHelper") 
        << "Saving MaxPathLengths as: \"" << filename << "\"";

      const genie::PathLengthList& maxpath = rgeom->GetMaxPathLengths();

      maxpath.SaveAsXml(filename);
      // append extra info to file
      std::ofstream mpfile(filename.c_str(), std::ios_base::app);
      mpfile
        << std::endl
        << "<!-- this file is only relevant for a setup compatible with:" 
        << std::endl
        << fMaxPathOutInfo 
        << std::endl
        << "-->" 
        << std::endl;
      mpfile.close();
    }  // finished writing max path length XML file (if requested)

    // protect against lack of driver due to not getting to Initialize()
    // (called from module's beginRun() method)
    if ( ! fDriver || ! fFluxD ) {
      mf::LogInfo("GENIEHelper") 
        << "~GENIEHelper called, but previously failed to construct "
        << ( (fDriver) ? " genie::GMCJDriver":"" )
        << ( (fFluxD)  ? " genie::GFluxI":"" );
    } else {
      double probscale = fDriver->GlobProbScale();
      double rawpots   = 0;
      if      ( fFluxType.compare("ntuple")==0 ) {
        genie::flux::GNuMIFlux* numiFlux = dynamic_cast<genie::flux::GNuMIFlux *>(fFluxD);
        rawpots = numiFlux->UsedPOTs();
        numiFlux->PrintConfig();
      }
      else if ( fFluxType.compare("simple_flux")==0 ) {
        genie::flux::GSimpleNtpFlux* simpleFlux = dynamic_cast<genie::flux::GSimpleNtpFlux *>(fFluxD);
        rawpots = simpleFlux->UsedPOTs();
        simpleFlux->PrintConfig();
      }
      mf::LogInfo("GENIEHelper") 
        << " Total Exposure " << fTotalExposure
        << " GMCJDriver GlobProbScale " << probscale 
        << " FluxDriver base pots " << rawpots
        << " corrected POTS " << rawpots/TMath::Max(probscale,1.0e-100);
    }

    // clean up owned genie object (other genie obj are ref ptrs)
    delete fGenieEventRecord;
    delete fDriver;
    delete fHelperRandom;

#ifndef NO_IFDH_LIB
    if ( fIFDH ) {
      if        ( fFluxCleanup.find("ALWAYS")   == 0 ) {
        fIFDH->cleanup();
      } else if ( fFluxCleanup.find("/var/tmp") == 0 ) {
        auto ffitr = fSelectedFluxFiles.begin();
        for ( ; ffitr != fSelectedFluxFiles.end(); ++ffitr ) {
          std::string ff = *ffitr;
          if ( ff.find("/var/tmp") == 0 ) {
            mf::LogDebug("GENIEHelper") << "delete " << ff;
            fIFDH->rm(ff);
          }
        }
      }
      delete fIFDH;
      fIFDH = 0;
    }
#endif

  }

  //--------------------------------------------------
  double GENIEHelper::TotalHistFlux() 
  {
    if ( fFluxType.compare("mono")         == 0 ||
         fFluxType.compare("ntuple")       == 0 ||
         fFluxType.compare("simple_flux" ) == 0 ||
         fFluxType.compare("dk2nu")        == 0    ) return -999.;

    return fTotalHistFlux;
  }

  //--------------------------------------------------
  void GENIEHelper::Initialize()
  {
    fDriver = new genie::GMCJDriver(); // needs to be before ConfigGeomScan
#ifndef GENIE_USE_ENVVAR
    // this configuration happened via $GEVGL beore R-2_8_0
    fDriver->SetEventGeneratorList(fEventGeneratorList);
#endif

    // initialize the Geometry and Flux drivers
    InitializeGeometry();
    InitializeFluxDriver();

    fDriver->UseFluxDriver(fFluxD2GMCJD);
    fDriver->UseGeomAnalyzer(fGeomD);

    // must come after creation of Geom, Flux and GMCJDriver
    ConfigGeomScan();  // could trigger fDriver->UseMaxPathLengths(*xmlfile*)

    fDriver->Configure();  // trigger GeomDriver::ComputeMaxPathLengths() 
    fDriver->UseSplines();
    fDriver->ForceSingleProbScale();

    if ( fFluxType.compare("histogram") == 0 && fEventsPerSpill < 0.01 ) {
      // fluxes are assumed to be given in units of neutrinos/cm^2/1e20POT/energy 
      // integral over all fluxes removes energy dependence
      // histograms should have bin width that reflects the value of the /energy bit
      // ie if /energy = /50MeV then the bin width should be 50 MeV
      
      // determine product of pot/spill, mass, and cross section
      // events = flux * pot * 10^-38 cm^2 (xsec) * (mass detector (in kg) / nucleon mass (in kg))
      fXSecMassPOT  = 1.e-38*1.e-20;
      fXSecMassPOT *= fPOTPerSpill*(fDetectorMass+fSurroundingMass)/(1.67262158e-27); 

      mf::LogInfo("GENIEHelper") << "Number of events per spill will be based on poisson mean of "
                                 << fXSecMassPOT*fTotalHistFlux;

      fHistEventsPerSpill = fHelperRandom->Poisson(fXSecMassPOT*fTotalHistFlux);
    }

    // set the pot/event counters to zero
    fSpillEvents   = 0;
    fSpillExposure = 0.;
    fTotalExposure = 0.;

    // If the flux driver knows how to keep track of exposure (time,pots)
    // reset it now as some might have been used in determining
    // the geometry maxpathlength or internally scanning for weights.
    // This should not be necessary (GENIE should do it automagically)
    // but the current version (as of 3665) doesn't.

    double preUsedFluxPOTs = 0;
    bool   wasCleared = true;
    bool   doprintpre = false;
    if( fFluxType.compare("ntuple") == 0 ) {
      genie::flux::GNuMIFlux* gnumiflux = 
        dynamic_cast<genie::flux::GNuMIFlux *>(fFluxD);
      preUsedFluxPOTs = gnumiflux->UsedPOTs();
      if ( preUsedFluxPOTs > 0 ) {
        doprintpre = true;
        gnumiflux->Clear("CycleHistory");
        if ( gnumiflux->UsedPOTs() != 0 ) wasCleared = false;
      }
    } else if ( fFluxType.compare("simple_flux") == 0 ) {
      genie::flux::GSimpleNtpFlux* gsimpleflux = 
        dynamic_cast<genie::flux::GSimpleNtpFlux *>(fFluxD);
      preUsedFluxPOTs = gsimpleflux->UsedPOTs();
      if ( preUsedFluxPOTs > 0 ) {
        doprintpre = true;
        gsimpleflux->Clear("CycleHistory");
        if ( gsimpleflux->UsedPOTs() != 0 ) wasCleared = false;
      }
    }
    if ( doprintpre ) {
      double probscale = fDriver->GlobProbScale();
      mf::LogInfo("GENIEHelper") 
        << "Pre-Event Generation: " 
        << " FluxDriver base " << preUsedFluxPOTs
        << " / GMCJDriver GlobProbScale " << probscale 
        << " = used POTS " << preUsedFluxPOTs/TMath::Max(probscale,1.0e-100)
        << " "
        << (wasCleared?"successfully":"failed to") << " cleared count for "
        << fFluxType;
    }

    return;
  }

  //--------------------------------------------------
  void GENIEHelper::InitializeGeometry()
  {
    genie::geometry::ROOTGeomAnalyzer *rgeom = 
      new genie::geometry::ROOTGeomAnalyzer(fGeoManager);

    // pass some of the debug flag bits on to the geometry manager
    int geomFlags = ( fDebugFlags >> 16 ) & 0xFF ;
    if ( geomFlags ) {
      int keep = ( geomFlags >> 7 );
      mf::LogInfo("GENIEHelper")
        << "InitializeGeometry set debug 0x"
        << std::hex << geomFlags << std::dec
        << " keepSegPath " << keep;
      rgeom->SetDebugFlags(geomFlags);
      if ( keep ) rgeom->SetKeepSegPath(true);
    }

    // get the world volume name from the geometry
    fWorldVolume = fGeoManager->GetTopVolume()->GetName();

    // the detector geometry uses cgs units.
    rgeom->SetLengthUnits(genie::units::centimeter);
    rgeom->SetDensityUnits(genie::units::gram_centimeter3);
    rgeom->SetTopVolName(fTopVolume.c_str());
    rgeom->SetMixtureWeightsSum(1.);

    //  casting to the GENIE geometry driver interface
    fGeomD        = rgeom; // dynamic_cast<genie::GeomAnalyzerI *>(rgeom);
    InitializeFiducialSelection();

    return;
  }

  //--------------------------------------------------
  void GENIEHelper::InitializeFiducialSelection()
  {
    genie::GeomAnalyzerI* geom_driver = fGeomD; // GENIEHelper name -> gNuMIExptEvGen name
    std::string fidcut = fFiducialCut;   // ditto

    if( fidcut.find_first_not_of(" \t\n") != 0) // trim any leading whitespace
      fidcut.erase( 0, fidcut.find_first_not_of(" \t\n")  );

    // convert string to lowercase
    std::transform(fidcut.begin(),fidcut.end(),fidcut.begin(),::tolower);

    if ( "" == fidcut || "none" == fidcut ) return;

    if ( fidcut.find("rock") != string::npos ) {
      // deal with RockBox separately than basic shapes
      InitializeRockBoxSelection();
      return;
    }

    // below is as it is in $GENIE/src/support/numi/EvGen/gNuMIExptEvGen
    // except the change in message logger from log4cpp (GENIE) to cet's MessageLogger used by art

    ///
    /// User defined fiducial volume cut
    ///      [0][M]<SHAPE>:val1,val2,...
    ///   "0" means reverse the cut (i.e. exclude the volume)
    ///   "M" means the coordinates are given in the ROOT geometry
    ///       "master" system and need to be transformed to "top vol" system
    ///   <SHAPE> can be any of "zcyl" "box" "zpoly" "sphere"
    ///       [each takes different # of args]
    ///   This must be followed by a ":" and a list of values separated by punctuation
    ///       (allowed separators: commas , parentheses () braces {} or brackets [] )
    ///   Value mapping:
    ///      zcly:x0,y0,radius,zmin,zmax           - cylinder along z at (x0,y0) capped at z's
    ///      box:xmin,ymin,zmin,xmax,ymax,zmax     - box w/ upper & lower extremes
    ///      zpoly:nfaces,x0,y0,r_in,phi,zmin,zmax - nfaces sided polygon in x-y plane
    //       sphere:x0,y0,z0,radius                - sphere of fixed radius at (x0,y0,z0)
    ///   Examples:    
    ///      1) 0mbox:0,0,0.25,1,1,8.75
    ///         exclude (i.e. reverse) a box in master coordinates w/ corners (0,0,0.25) (1,1,8.75)
    ///      2) mzpoly:6,(2,-1),1.75,0,{0.25,8.75}
    ///         six sided polygon in x-y plane, centered at x,y=(2,-1) w/ inscribed radius 1.75
    ///         no rotation (so first face is in y-z plane +r from center, i.e. hex sits on point)
    ///         limited to the z range of {0.25,8.75} in the master ROOT geom coordinates
    ///      3) zcly:(3,4),5.5,-2,10
    ///         a cylinder oriented parallel to the z axis in the "top vol" coordinates
    ///         at x,y=(3,4) with radius 5.5 and z range of {-2,10}
    ///
    genie::geometry::ROOTGeomAnalyzer * rgeom = 
      dynamic_cast<genie::geometry::ROOTGeomAnalyzer *>(geom_driver);
    if ( ! rgeom ) {
      mf::LogWarning("GENIEHelpler")
        << "Can not create GeomVolSelectorFiduction,"
        << " geometry driver is not ROOTGeomAnalyzer";
      return;
    }

    mf::LogInfo("GENIEHelper") << "fiducial cut: " << fidcut;

    // for now, only fiducial no "rock box"
    genie::geometry::GeomVolSelectorFiducial* fidsel =
      new genie::geometry::GeomVolSelectorFiducial();

    fidsel->SetRemoveEntries(true);  // drop segments that won't be considered

    vector<string> strtok = genie::utils::str::Split(fidcut,":");
    if ( strtok.size() != 2 ) {
      mf::LogWarning("GENIEHelper")
        << "Can not create GeomVolSelectorFiduction,"
        << " no \":\" separating type from values.  nsplit=" << strtok.size();
      for ( unsigned int i=0; i < strtok.size(); ++i )
        mf::LogWarning("GENIEHelper")
          << "strtok[" << i << "] = \"" << strtok[i] << "\"";
      return;
    }

    // parse out optional "x" and "m"
    string stype = strtok[0];
    bool reverse = ( stype.find("0") != string::npos );
    bool master  = ( stype.find("m") != string::npos );  // action after values are set

    // parse out values
    vector<double> vals;
    vector<string> valstrs = genie::utils::str::Split(strtok[1]," ,;(){}[]");
    vector<string>::const_iterator iter = valstrs.begin();
    for ( ; iter != valstrs.end(); ++iter ) {
      const string& valstr1 = *iter;
      if ( valstr1 != "" ) vals.push_back(atof(valstr1.c_str()));
    }
    size_t nvals = vals.size();
    // pad it out to at least 7 entries to avoid index issues if used
    for ( size_t nadd = 0; nadd < 7-nvals; ++nadd ) vals.push_back(0);
    
    //std::cout << "ivals = [";
    //for (unsigned int i=0; i < nvals; ++i) {
    //  if (i>0) cout << ",";
    //  std::cout << vals[i];
    //}
    //std::cout << "]" << std::endl;
    
    // std::vector elements are required to be adjacent so we can treat address as ptr
    
    if        ( stype.find("zcyl")   != string::npos ) {
      // cylinder along z direction at (x0,y0) radius zmin zmax
      if ( nvals < 5 ) 
        mf::LogError("GENIEHelper") << "MakeZCylinder needs 5 values, not " << nvals
                                    << " fidcut=\"" << fidcut << "\"";
      fidsel->MakeZCylinder(vals[0],vals[1],vals[2],vals[3],vals[4]);

    } else if ( stype.find("box")    != string::npos ) {
      // box (xmin,ymin,zmin) (xmax,ymax,zmax)
      if ( nvals < 6 ) 
        mf::LogError("GENIEHelper") << "MakeBox needs 6 values, not " << nvals
                                    << " fidcut=\"" << fidcut << "\"";
      double xyzmin[3] = { vals[0], vals[1], vals[2] };
      double xyzmax[3] = { vals[3], vals[4], vals[5] };
      fidsel->MakeBox(xyzmin,xyzmax);

    } else if ( stype.find("zpoly")  != string::npos ) {
      // polygon along z direction nfaces at (x0,y0) radius phi zmin zmax
      if ( nvals < 7 ) 
        mf::LogError("GENIEHelper") << "MakeZPolygon needs 7 values, not " << nvals
                                    << " fidcut=\"" << fidcut << "\"";
      int nfaces = (int)vals[0];
      if ( nfaces < 3 ) 
        mf::LogError("GENIEHelper") << "MakeZPolygon needs nfaces>=3, not " << nfaces
                                    << " fidcut=\"" << fidcut << "\"";
      fidsel->MakeZPolygon(nfaces,vals[1],vals[2],vals[3],vals[4],vals[5],vals[6]);

    } else if ( stype.find("sphere") != string::npos ) {
      // sphere at (x0,y0,z0) radius 
      if ( nvals < 4 ) 
        mf::LogError("GENIEHelper") << "MakeZSphere needs 4 values, not " << nvals
                                    << " fidcut=\"" << fidcut << "\"";
      fidsel->MakeSphere(vals[0],vals[1],vals[2],vals[3]);

    } else {
      mf::LogError("GENIEHelper")
        << "Can not create GeomVolSelectorFiduction for shape \"" << stype << "\"";
    }

    if ( master  ) {
      fidsel->ConvertShapeMaster2Top(rgeom);
      mf::LogInfo("GENIEHelper") << "Convert fiducial volume from master to topvol coords";
    }
    if ( reverse ) {
      fidsel->SetReverseFiducial(true);
      mf::LogInfo("GENIEHelper") << "Reverse sense of fiducial volume cut";
    }
    
    rgeom->AdoptGeomVolSelector(fidsel);

  }

  //--------------------------------------------------
  void GENIEHelper::InitializeRockBoxSelection()
  {
    genie::GeomAnalyzerI* geom_driver = fGeomD; // GENIEHelper name -> gNuMIExptEvGen name
    std::string fidcut = fFiducialCut;   // ditto

    if( fidcut.find_first_not_of(" \t\n") != 0) // trim any leading whitespace
      fidcut.erase( 0, fidcut.find_first_not_of(" \t\n")  );

    // convert string to lowercase
    std::transform(fidcut.begin(),fidcut.end(),fidcut.begin(),::tolower);

    genie::geometry::ROOTGeomAnalyzer * rgeom = 
      dynamic_cast<genie::geometry::ROOTGeomAnalyzer *>(geom_driver);
    if ( ! rgeom ) {
      mf::LogWarning("GENIEHelpler")
        << "Can not create GeomVolSelectorRockBox,"
        << " geometry driver is not ROOTGeomAnalyzer";
      return;
    }

    mf::LogInfo("GENIEHelper") << "fiducial (rock) cut: " << fidcut;

    // for now, only fiducial no "rock box"
    genie::geometry::GeomVolSelectorRockBox* rocksel =
      new genie::geometry::GeomVolSelectorRockBox();

    vector<string> strtok = genie::utils::str::Split(fidcut,":");
    if ( strtok.size() != 2 ) {
      mf::LogWarning("GENIEHelper")
        << "Can not create GeomVolSelectorRockBox,"
        << " no \":\" separating type from values.  nsplit=" << strtok.size();
      for ( unsigned int i=0; i < strtok.size(); ++i )
        mf::LogWarning("GENIEHelper")
          << "strtok[" << i << "] = \"" << strtok[i] << "\"";
      return;
    }

    string stype = strtok[0];

    // parse out values
    vector<double> vals;
    vector<string> valstrs = genie::utils::str::Split(strtok[1]," ,;(){}[]\t\n\r");
    vector<string>::const_iterator iter = valstrs.begin();
    for ( ; iter != valstrs.end(); ++iter ) {
      const string& valstr1 = *iter;
      if ( valstr1 != "" ) {
        double aval = atof(valstr1.c_str());
        mf::LogDebug("GENIEHelper") << "rock value [" << vals.size() << "] "
                                    << aval;
        vals.push_back(aval);
      }
    }
    size_t nvals = vals.size();

    rocksel->SetRemoveEntries(true);  // drop segments that won't be considered

    // assume coordinates are in the *master* (not "top volume") system
    // need to set fTopVolume to fWorldVolume as Sample() will keep setting it
    fTopVolume = fWorldVolume;
    rgeom->SetTopVolName(fTopVolume.c_str());

    if ( nvals < 6 ) {
      throw cet::exception("GENIEHelper") << "rockbox needs at "
					  << "least 6 values, found " 
					  << nvals << "in \"" 
					  << strtok[1] << "\"";
      
    }
    double xyzmin[3] = { vals[0], vals[1], vals[2] };
    double xyzmax[3] = { vals[3], vals[4], vals[5] };

    bool   rockonly  = true;
    double wallmin   = 800.;   // geometry in cm, ( 8 meter buffer)
    double dedx      = 2.5 * 1.7e-3; // GeV/cm, rho=2.5, 1.7e-3 ~ rock like loss
    double fudge     = 1.05;

    if ( nvals >=  7 ) rockonly = vals[6];
    if ( nvals >=  8 ) wallmin  = vals[7];
    if ( nvals >=  9 ) dedx     = vals[8];
    if ( nvals >= 10 ) fudge    = vals[9];

    rocksel->SetRockBoxMinimal(xyzmin,xyzmax);
    rocksel->SetMinimumWall(wallmin);
    rocksel->SetDeDx(dedx/fudge);

    // if not rock-only then make a tiny exclusion bubble
    // call to MakeBox shouldn't be necessary
    //  should be done by SetRockBoxMinimal but for some GENIE versions isn't
    if ( ! rockonly ) rocksel->MakeSphere(0,0,0,1.0e-10);
    else              rocksel->MakeBox(xyzmin,xyzmax); 

    rgeom->AdoptGeomVolSelector(rocksel);
  }

  //--------------------------------------------------
  void GENIEHelper::InitializeFluxDriver()
  {

    if(fFluxType.compare("ntuple") == 0){

      genie::flux::GNuMIFlux* numiFlux = new genie::flux::GNuMIFlux();

#ifndef GFLUX_MISSING_SETORVECTOR
      mf::LogDebug("GENIEHelper") << "LoadBeamSimData w/ vector of size " << fSelectedFluxFiles.size();
      numiFlux->LoadBeamSimData(fSelectedFluxFiles,fDetLocation);
#else
      // older code can only take one file name (wildcard pattern)
      if ( fSelectedFluxFiles.empty() ) fSelectedFluxFiles.push_back("empty-fluxfile-set");
      if ( fSelectedFluxFiles.size() > 1 )
        mf::LogWarning("GENIEHelper")
          << "LoadBeamSimData could use only first of " 
          << fSelectedFluxFiles.size() << " patterns";
      numiFlux->LoadBeamSimData(fSelectedFluxFiles[0], fDetLocation);
#endif

      // initialize to only use neutrino flavors requested by user
      genie::PDGCodeList probes;
      for ( std::vector<int>::iterator flvitr = fGenFlavors.begin(); flvitr != fGenFlavors.end(); flvitr++ )
        probes.push_back(*flvitr);
      numiFlux->SetFluxParticles(probes);

      if ( TMath::Abs(fFluxUpstreamZ) < 1.0e30 ) numiFlux->SetUpstreamZ(fFluxUpstreamZ);

      // set the number of cycles to run
      // +++++++++this is stupid - to really set it i have to get a 
      // value from the MCJDriver and i am not even sure what i have 
      // below is approximately correct.
      // for now just run on a set number of events that is kept track of 
      // in the sample method
      //  numiFlux->SetNumOfCycles(int(fPOT/fFluxNormalization));
    
      fFluxD = numiFlux; // dynamic_cast<genie::GFluxI *>(numiFlux);
    } //end if using ntuple flux files
    else if(fFluxType.compare("simple_flux")==0){

      genie::flux::GSimpleNtpFlux* simpleFlux = 
        new genie::flux::GSimpleNtpFlux();

#ifndef GFLUX_MISSING_SETORVECTOR
      mf::LogDebug("GENIEHelper") << "LoadBeamSimData w/ vector of size " << fSelectedFluxFiles.size();
      simpleFlux->LoadBeamSimData(fSelectedFluxFiles,fDetLocation);
#else
      // older code can only take one file name (wildcard pattern)
      if ( fSelectedFluxFiles.empty() ) fSelectedFluxFiles.push_back("empty-fluxfile-set");
      if ( fSelectedFluxFiles.size() > 1 )
        mf::LogWarning("GENIEHelper")
          << "LoadBeamSimData could use only first of " 
          << fSelectedFluxFiles.size() << " patterns";
      simpleFlux->LoadBeamSimData(fSelectedFluxFiles[0], fDetLocation);
#endif

      // initialize to only use neutrino flavors requested by user
      genie::PDGCodeList probes;
      for ( std::vector<int>::iterator flvitr = fGenFlavors.begin(); flvitr != fGenFlavors.end(); flvitr++ )
        probes.push_back(*flvitr);
      simpleFlux->SetFluxParticles(probes);

      if ( TMath::Abs(fFluxUpstreamZ) < 1.0e30 ) simpleFlux->SetUpstreamZ(fFluxUpstreamZ);

      fFluxD = simpleFlux; // dynamic_cast<genie::GFluxI *>(simpleFlux);
    
    } //end if using simple_flux flux files
    else if(fFluxType.compare("histogram") == 0){

      genie::flux::GCylindTH1Flux* histFlux = new genie::flux::GCylindTH1Flux();
    
      // now add the different fluxes - fluxes were added to the vector in the same 
      // order that the flavors appear in fGenFlavors
      int ctr = 0;
      for ( std::vector<int>::iterator i = fGenFlavors.begin(); i != fGenFlavors.end(); i++ ) {
        histFlux->AddEnergySpectrum(*i, fFluxHistograms[ctr]);
        ++ctr;
      } //end loop to add flux histograms to driver

      histFlux->SetNuDirection(fBeamDirection);
      histFlux->SetBeamSpot(fBeamCenter);
      histFlux->SetTransverseRadius(fBeamRadius);
    
      fFluxD = histFlux; // dynamic_cast<genie::GFluxI *>(histFlux);
    } //end if using a histogram
    else if(fFluxType.compare("mono") == 0){

      // weight each species equally in the generation
      double weight = 1./(1.*fGenFlavors.size());
      //make a map of pdg to weight codes
      std::map<int, double> pdgwmap;
      for ( std::vector<int>::iterator i = fGenFlavors.begin(); i != fGenFlavors.end(); i++ )
        pdgwmap[*i] = weight;

      genie::flux::GMonoEnergeticFlux *monoflux = new genie::flux::GMonoEnergeticFlux(fMonoEnergy, pdgwmap);
      monoflux->SetDirectionCos(fBeamDirection.X(), fBeamDirection.Y(), fBeamDirection.Z());
      monoflux->SetRayOrigin(fBeamCenter.X(), fBeamCenter.Y(), fBeamCenter.Z());
      fFluxD = monoflux; // dynamic_cast<genie::GFluxI *>(monoflux);
    } //end if using monoenergetic beam


    //Using the atmospheric fluxes
    else if(fFluxType.compare("atmo_FLUKA") == 0 || fFluxType.compare("atmo_BARTOL") == 0){

      // Instantiate appropriate concrete flux driver
      genie::flux::GAtmoFlux *atmo_flux_driver = 0;
      
      if(fFluxType.compare("atmo_FLUKA") == 0) {
        genie::flux::GFlukaAtmo3DFlux * fluka_flux = new genie::flux::GFlukaAtmo3DFlux;
        atmo_flux_driver = dynamic_cast<genie::flux::GAtmoFlux *>(fluka_flux);
      }
      if(fFluxType.compare("atmo_BARTOL") == 0) {
        genie::flux::GBartolAtmoFlux * bartol_flux = new genie::flux::GBartolAtmoFlux;
        atmo_flux_driver = dynamic_cast<genie::flux::GAtmoFlux *>(bartol_flux);
      } 
      
      atmo_flux_driver->ForceMinEnergy(fAtmoEmin);
      atmo_flux_driver->ForceMaxEnergy(fAtmoEmax);
      
      std::ostringstream atmoCfgText;
      atmoCfgText << "Configuration for " << fFluxType
                  << ", Rl " << fAtmoRl << " Rt " << fAtmoRt;
      for ( size_t j = 0; j < fGenFlavors.size(); ++j ) {
        int         flavor  = fGenFlavors[j];
        std::string flxfile = fSelectedFluxFiles[j];
        atmo_flux_driver->SetFluxFile(flavor,flxfile);
        atmoCfgText << "\n  FLAVOR: " << std::setw(3) << flavor 
                    << "  FLUX FILE: " <<  flxfile;      
      }
      mf::LogInfo("GENIEHelper") << atmoCfgText.str();

      atmo_flux_driver->LoadFluxData();
      
      // configure flux generation surface:
      atmo_flux_driver->SetRadii(fAtmoRl, fAtmoRt);
            
      fFluxD = atmo_flux_driver;//dynamic_cast<genie::GFluxI *>(atmo_flux_driver);
    } //end if using atmospheric fluxes


    //
    // Is the user asking to do flavor mixing?
    //
    fFluxD2GMCJD = fFluxD;  // default: genie's GMCJDriver uses the bare flux generator
    if( fMixerConfig.find_first_not_of(" \t\n") != 0) // trim any leading whitespace
      fMixerConfig.erase( 0, fMixerConfig.find_first_not_of(" \t\n")  );
    std::string keyword = fMixerConfig.substr(0,fMixerConfig.find_first_of(" \t\n"));
    if ( keyword != "none" ) {
      // Wrap the true flux driver up in the adapter to allow flavor mixing
      genie::flux::GFlavorMixerI* mixer = 0;
      // here is where we map MixerConfig string keyword to actual class
      // first is a special case that is part of GENIE proper
      if ( keyword == "map" || keyword == "swap" || keyword == "fixedfrac" )
        mixer = new genie::flux::GFlavorMap();
      // if it wasn't one of the predefined known mixers then
      // see if the factory knows about it and can create one
      // assuming the keyword (first token) is the class name
      if ( ! mixer ) {
        genie::flux::GFlavorMixerFactory& mixerFactory = 
          genie::flux::GFlavorMixerFactory::Instance();
        mixer = mixerFactory.GetFlavorMixer(keyword);
        if ( mixer ) {
          // remove class name from config string
          fMixerConfig.erase(0,keyword.size()); 
          // trim any leading whitespace
          if ( fMixerConfig.find_first_not_of(" \t\n") != 0 )
            fMixerConfig.erase( 0, fMixerConfig.find_first_not_of(" \t\n")  );
        } else {
          const std::vector<std::string>& knownMixers = 
            mixerFactory.AvailableFlavorMixers();
          mf::LogWarning("GENIEHelper")
            << " GFlavorMixerFactory known mixers: ";
          for (unsigned int j=0; j < knownMixers.size(); ++j ) {
            mf::LogWarning("GENIEHelper")
              << "   [" << std::setw(2) << j << "]  " << knownMixers[j];
          }
        }
      }
      // configure the mixer
      if ( mixer ) mixer->Config(fMixerConfig);
      else {
        mf::LogWarning("GENIEHelper") 
          << "GENIEHelper MixerConfig keyword was \"" << keyword
          << "\" but that did not map to a class; " << std::endl
          << "GFluxBlender in use, but no mixer";
      }

      genie::GFluxI* realFluxD = fFluxD;
      genie::flux::GFluxBlender* blender = new genie::flux::GFluxBlender();
      blender->SetBaselineDist(fMixerBaseline);
      blender->AdoptFluxGenerator(realFluxD);
      blender->AdoptFlavorMixer(mixer);
      fFluxD2GMCJD = blender;
      if ( fDebugFlags & 0x01 ) {
        if ( mixer ) mixer->PrintConfig();
        blender->PrintConfig();
        std::cout << std::flush;
      }
    }

    return;
  }

  //--------------------------------------------------
  void GENIEHelper::ConfigGeomScan()
  {

    // trim any leading whitespace
    if( fGeomScan.find_first_not_of(" \t\n") != 0) 
      fGeomScan.erase( 0, fGeomScan.find_first_not_of(" \t\n")  );

    if ( fGeomScan.find("default") != std::string::npos ) return;

    genie::geometry::ROOTGeomAnalyzer* rgeom = 
      dynamic_cast<genie::geometry::ROOTGeomAnalyzer*>(fGeomD);

    if ( !rgeom ) {
      throw cet::exception("GENIEHelper") << "fGeomD wasn't of type "
					  << "genie::geometry::ROOTGeomAnalyzer*";
    }

    // convert string to lowercase

    // parse out string
    vector<string> strtok = genie::utils::str::Split(fGeomScan," ");
    // first value is a string, others should be numbers unless "file:"
    string scanmethod = strtok[0];

    // convert key string to lowercase (but not potential file name)
    std::transform(scanmethod.begin(),scanmethod.end(),scanmethod.begin(),::tolower);

    if ( scanmethod.find("file") != std::string::npos ) {
      // xml expand path before passing in
      string filename = strtok[1];
      string fullname = genie::utils::xml::GetXMLFilePath(filename);
      mf::LogInfo("GENIEHelper") 
        << "ConfigGeomScan getting MaxPathLengths from \"" << fullname << "\"";
      fDriver->UseMaxPathLengths(fullname);
      return;
    }

    vector<double> vals;
    for ( size_t indx=1; indx < strtok.size(); ++indx ) {
      const string& valstr1 = strtok[indx];
      if ( valstr1 != "" ) vals.push_back(atof(valstr1.c_str()));
    }
    size_t nvals = vals.size();
    // pad it out to at least 4 entries to avoid index issues
    for ( size_t nadd = 0; nadd < 4-nvals; ++nadd ) vals.push_back(0);

    double safetyfactor = 0;
    int    writeout = 0;
    if (        scanmethod.find("box") != std::string::npos ) {
      // use box method
      int np = (int)vals[0];
      int nr = (int)vals[1];
      if ( nvals >= 3 ) safetyfactor = vals[2];
      if ( nvals >= 4 ) writeout     = vals[3];
      // protect against too small values
      if ( np <= 10 ) np = rgeom->ScannerNPoints();
      if ( nr <= 10 ) nr = rgeom->ScannerNRays();
      mf::LogInfo("GENIEHelper") 
        << "ConfigGeomScan scan using box " << np << " points, "
        << nr << " rays";
      rgeom->SetScannerNPoints(np);
      rgeom->SetScannerNRays(nr);
    } else if ( scanmethod.find("flux") != std::string::npos ) {
      // use flux method
      int np = (int)vals[0];
      if ( nvals >= 2 ) safetyfactor = vals[1];
      if ( nvals >= 3 ) writeout     = vals[2];
      // sanity check, need some number of rays to explore the geometry
      // negative now means adjust rays to enu_max (GENIE svn 3676)
      if ( abs(np) <= 100 ) {
        int npnew = rgeom->ScannerNParticles(); 
        if ( np < 0 ) npnew = -abs(npnew);
        mf::LogWarning("GENIEHelper") 
          << "Too few rays requested for geometry scan: " << np
          << ", use: " << npnew << "instead";
        np = npnew;
      }
      mf::LogInfo("GENIEHelper") 
        << "ConfigGeomScan scan using " << np << " flux particles"
        << ( (np>0) ? "" : " with ray energy pushed to flux driver maximum" );
      rgeom->SetScannerFlux(fFluxD);
      rgeom->SetScannerNParticles(np);
    } 
    else{
      // unknown
      throw cet::exception("GENIEHelper") << "fGeomScan unknown method: \"" 
					  << fGeomScan << "\"";
    }
    if ( safetyfactor > 0 ) {
      mf::LogInfo("GENIEHelper") 
        << "ConfigGeomScan setting safety factor to " << safetyfactor;
      rgeom->SetMaxPlSafetyFactor(safetyfactor);
    }
    if ( writeout != 0 ) SetMaxPathOutInfo();
  }

  //--------------------------------------------------
  void GENIEHelper::SetMaxPathOutInfo()
  {
    // create an info string based on:
    // ROOT geometry, TopVolume, FiducialCut, GeomScan, Flux

    mf::LogInfo("GENIEHelper") << "about to create MaxPathOutInfo";

    fMaxPathOutInfo = "\n";
    fMaxPathOutInfo += "   FluxType:     " + fFluxType + "\n";
    fMaxPathOutInfo += "   BeamName:     " + fBeamName + "\n";
    fMaxPathOutInfo += "   FluxFiles:    ";
    std::vector<string>::iterator ffitr = fSelectedFluxFiles.begin();
    for ( ; ffitr != fSelectedFluxFiles.end() ; ++ffitr )
      fMaxPathOutInfo += "\n         " + *ffitr;
    fMaxPathOutInfo += "\n";
    fMaxPathOutInfo += "   DetLocation:  " + fDetLocation + "\n";
    fMaxPathOutInfo += "   ROOTFile:     " + fGeoFile     + "\n";
    fMaxPathOutInfo += "   WorldVolume:  " + fWorldVolume + "\n";
    fMaxPathOutInfo += "   TopVolume:    " + fTopVolume   + "\n";
    fMaxPathOutInfo += "   FiducialCut:  " + fFiducialCut + "\n";
    fMaxPathOutInfo += "   GeomScan:     " + fGeomScan    + "\n";

    mf::LogInfo("GENIEHelper") << "MaxPathOutInfo: \"" 
			       << fMaxPathOutInfo << "\"";

  }

  //--------------------------------------------------
  bool GENIEHelper::Stop()
  {
    //   std::cout << "in GENIEHelper::Stop(), fEventsPerSpill = " << fEventsPerSpill
    //      << " fPOTPerSpill = " << fPOTPerSpill << " fSpillExposure = " << fSpillExposure 
    //      << " fSpillEvents = " << fSpillEvents
    //      << " fHistEventsPerSpill = " << fHistEventsPerSpill << std::endl;

    // determine if we should keep throwing neutrinos for 
    // this spill or move on

    if(fFluxType.compare("atmo_FLUKA") == 0 || fFluxType.compare("atmo_BARTOL") == 0){
      if((fEventsPerSpill > 0) && (fSpillEvents < fEventsPerSpill)){
        return false;
      }
    }

    else if(fEventsPerSpill > 0){
      if(fSpillEvents < fEventsPerSpill) 
        return false;
    }
    else{
      if( ( fFluxType.compare("ntuple")      == 0 || 
            fFluxType.compare("simple_flux") == 0 ||
            fFluxType.compare("dk2nu")       == 0    ) && 
          fSpillExposure < fPOTPerSpill ) return false;
      else if(fFluxType.compare("histogram") == 0){
        if(fSpillEvents < fHistEventsPerSpill) return false;
        else fSpillExposure = fPOTPerSpill;
      }
    }

    // made it to here, means need to reset the counters

    if(fFluxType.compare("atmo_FLUKA") == 0 || fFluxType.compare("atmo_BARTOL") == 0){
      //the exposure for atmo is in SECONDS. In order to get seconds, it needs to 
      //be normalized by 1e4 to take into account the units discrepency between 
      //AtmoFluxDriver(/m2) and Generate(/cm2) and it need to be normalized by 
      //the generation surface area since it's not taken into accoutn in the flux driver
      fTotalExposure = (1e4 * (dynamic_cast<genie::flux::GAtmoFlux *>(fFluxD)->NFluxNeutrinos())) / (TMath::Pi() * fAtmoRt*fAtmoRt);
      
      LOG_DEBUG("GENIEHelper") << "===> Atmo EXPOSURE = " << fTotalExposure << " seconds";
    }

    else{
      fTotalExposure += fSpillExposure;
    }

    fSpillEvents   = 0;
    fSpillExposure = 0.;
    fHistEventsPerSpill = fHelperRandom->Poisson(fXSecMassPOT*fTotalHistFlux);
    return true;
  }

  //--------------------------------------------------
  bool GENIEHelper::Sample(simb::MCTruth &truth, simb::MCFlux  &flux, simb::GTruth &gtruth)
  {
    // set the top volume for the geometry
    fGeoManager->SetTopVolume(fGeoManager->FindVolumeFast(fTopVolume.c_str()));
    
    if ( fGenieEventRecord ) delete fGenieEventRecord;

    // ART Framework plays games with gRandom, undo that if requested
    TRandom* old_gRandom = gRandom;
    if (fUseHelperRndGen4GENIE) gRandom = fHelperRandom;

    fGenieEventRecord = fDriver->GenerateEvent();

    if (fUseHelperRndGen4GENIE) gRandom = old_gRandom;

    // now check if we produced a viable event record
    bool viableInteraction = true;
    if ( ! fGenieEventRecord ) viableInteraction = false;

    // update the spill total information, then check to see 
    // if we got an event record that was valid

    // pack the flux information
    if(fFluxType.compare("ntuple") == 0){
      fSpillExposure = (dynamic_cast<genie::flux::GNuMIFlux *>(fFluxD)->UsedPOTs()/fDriver->GlobProbScale() - fTotalExposure);
      flux.fFluxType = simb::kNtuple;
      PackNuMIFlux(flux);
    }
    else if ( fFluxType.compare("simple_flux")==0 ) { 
      // pack the flux information
      fSpillExposure = (dynamic_cast<genie::flux::GSimpleNtpFlux *>(fFluxD)->UsedPOTs()/fDriver->GlobProbScale() - fTotalExposure);
      flux.fFluxType = simb::kSimple_Flux;
      PackSimpleFlux(flux);
    }

    // if no interaction generated return false
    if(!viableInteraction) return false;
    
    // fill the MC truth information as we have a good interaction
    PackMCTruth(fGenieEventRecord,truth); 
    // fill the Generator (genie) truth information
    PackGTruth(fGenieEventRecord, gtruth);
    
    // check to see if we are using flux ntuples but want to 
    // make n events per spill
    if(fEventsPerSpill > 0 &&
       (fFluxType.compare("ntuple")      == 0 ||
        fFluxType.compare("simple_flux") == 0 ||
        fFluxType.compare("dk2nu")       == 0    )
       ) ++fSpillEvents;

    // now check if using either histogram or mono fluxes, using
    // either n events per spill or basing events on POT per spill for the
    // histogram case
    if(fFluxType.compare("histogram") == 0){
      // set the flag in the parent object that says the 
      // fluxes came from histograms and fill related values
      flux.fFluxType = simb::kHistPlusFocus;

      // save the fluxes - fluxes were added to the vector in the same 
      // order that the flavors appear in fGenFlavors
      int ctr = 0;
      int bin = fFluxHistograms[0]->FindBin(truth.GetNeutrino().Nu().E());
      std::vector<double> fluxes(6, 0.);
      for(std::vector<int>::iterator i = fGenFlavors.begin(); i != fGenFlavors.end(); i++){
        if(*i ==  12) fluxes[kNue]      = fFluxHistograms[ctr]->GetBinContent(bin);
        if(*i == -12) fluxes[kNueBar]   = fFluxHistograms[ctr]->GetBinContent(bin);
        if(*i ==  14) fluxes[kNuMu]     = fFluxHistograms[ctr]->GetBinContent(bin);
        if(*i == -14) fluxes[kNuMuBar]  = fFluxHistograms[ctr]->GetBinContent(bin);
        if(*i ==  16) fluxes[kNuTau]    = fFluxHistograms[ctr]->GetBinContent(bin);
        if(*i == -16) fluxes[kNuTauBar] = fFluxHistograms[ctr]->GetBinContent(bin);
        ++ctr;
      }

      // get the flux for each neutrino flavor of this energy
      flux.SetFluxGen(fluxes[kNue],   fluxes[kNueBar],
                      fluxes[kNuMu],  fluxes[kNuMuBar],
                      fluxes[kNuTau], fluxes[kNuTauBar]);
    
      ++fSpillEvents;
    }
    else if(fFluxType.compare("mono") == 0){
      ++fSpillEvents;
    }

    else if(fFluxType.compare("atmo_FLUKA") == 0 || fFluxType.compare("atmo_BARTOL") == 0){
      if(fEventsPerSpill > 0) ++fSpillEvents;
      flux.fFluxType = simb::kHistPlusFocus;
    }


    // fill these after the Pack[NuMI|Simple]Flux because those
    // will Reset() the values at the start
    TLorentzVector *vertex = fGenieEventRecord->Vertex();
    TLorentzVector nuray_pos = fFluxD->Position();
    TVector3 ray2vtx = nuray_pos.Vect() - vertex->Vect();
    flux.fgenx    = nuray_pos.X();
    flux.fgeny    = nuray_pos.Y();
    flux.fgenz    = nuray_pos.Z();
    flux.fgen2vtx = ray2vtx.Mag();

    genie::flux::GFluxBlender* blender = 
      dynamic_cast<genie::flux::GFluxBlender*>(fFluxD2GMCJD);
    if ( blender ) { 
      flux.fdk2gen = blender->TravelDist();
      // / if mixing flavors print the state of the blender
      if ( fDebugFlags & 0x02 ) blender->PrintState();
    }

    if ( fDebugFlags & 0x04 ) {
      mf::LogInfo("GENIEHelper") << "vertex loc " << vertex->X() << "," 
                                 << vertex->Y() << "," << vertex->Z() << std::endl 
                                 << " flux ray start " << nuray_pos.X() << ","
                                 << nuray_pos.Y() << "," << nuray_pos.Z() << std::endl
                                 << " ray2vtx = " << flux.fgen2vtx
                                 << " dk2ray = " << flux.fdk2gen;
    }
    if ( fGHepPrintLevel >= 0 ) {
      std::cout << *fGenieEventRecord;
    }

    // set the top volume of the geometry back to the world volume
    fGeoManager->SetTopVolume(fGeoManager->FindVolumeFast(fWorldVolume.c_str()));

    return true;
  }

  //--------------------------------------------------
  void GENIEHelper::PackNuMIFlux(simb::MCFlux &flux)
  {
    flux.Reset();

    // cast the fFluxD pointer to be of the right type
    genie::flux::GNuMIFlux *gnf = dynamic_cast<genie::flux::GNuMIFlux *>(fFluxD);
    const genie::flux::GNuMIFluxPassThroughInfo& nflux = gnf->PassThroughInfo();

    // check the particle codes and the units passed through
    //  nflux.pcodes: 0=original GEANT particle codes, 1=converted to PDG
    //  nflux.units:  0=original GEANT cm, 1=meters
    if(nflux.pcodes != 1 && nflux.units != 0)
      mf::LogWarning("GENIEHelper") << "either wrong particle codes or units "
                                    << "from flux object - beware!!";

    // maintained variable names from gnumi ntuples
    // see http://www.hep.utexas.edu/~zarko/wwwgnumi/v19/[/v19/output_gnumi.html]

    flux.frun      = nflux.run;
    flux.fevtno    = nflux.evtno;
    flux.fndxdz    = nflux.ndxdz;
    flux.fndydz    = nflux.ndydz;
    flux.fnpz      = nflux.npz;
    flux.fnenergy  = nflux.nenergy;
    flux.fndxdznea = nflux.ndxdznea;
    flux.fndydznea = nflux.ndydznea;
    flux.fnenergyn = nflux.nenergyn;
    flux.fnwtnear  = nflux.nwtnear;
    flux.fndxdzfar = nflux.ndxdzfar;
    flux.fndydzfar = nflux.ndydzfar;
    flux.fnenergyf = nflux.nenergyf;
    flux.fnwtfar   = nflux.nwtfar;
    flux.fnorig    = nflux.norig;
    flux.fndecay   = nflux.ndecay;
    flux.fntype    = nflux.ntype;
    flux.fvx       = nflux.vx;
    flux.fvy       = nflux.vy;
    flux.fvz       = nflux.vz;
    flux.fpdpx     = nflux.pdpx;
    flux.fpdpy     = nflux.pdpy;
    flux.fpdpz     = nflux.pdpz;
    flux.fppdxdz   = nflux.ppdxdz;
    flux.fppdydz   = nflux.ppdydz;
    flux.fpppz     = nflux.pppz;
    flux.fppenergy = nflux.ppenergy;
    flux.fppmedium = nflux.ppmedium;
    flux.fptype    = nflux.ptype;     // converted to PDG
    flux.fppvx     = nflux.ppvx;
    flux.fppvy     = nflux.ppvy;
    flux.fppvz     = nflux.ppvz;
    flux.fmuparpx  = nflux.muparpx;
    flux.fmuparpy  = nflux.muparpy;
    flux.fmuparpz  = nflux.muparpz;
    flux.fmupare   = nflux.mupare;
    flux.fnecm     = nflux.necm;
    flux.fnimpwt   = nflux.nimpwt;
    flux.fxpoint   = nflux.xpoint;
    flux.fypoint   = nflux.ypoint;
    flux.fzpoint   = nflux.zpoint;
    flux.ftvx      = nflux.tvx;
    flux.ftvy      = nflux.tvy;
    flux.ftvz      = nflux.tvz;
    flux.ftpx      = nflux.tpx;
    flux.ftpy      = nflux.tpy;
    flux.ftpz      = nflux.tpz;
    flux.ftptype   = nflux.tptype;   // converted to PDG
    flux.ftgen     = nflux.tgen;
    flux.ftgptype  = nflux.tgptype;  // converted to PDG
    flux.ftgppx    = nflux.tgppx;
    flux.ftgppy    = nflux.tgppy;
    flux.ftgppz    = nflux.tgppz;
    flux.ftprivx   = nflux.tprivx;
    flux.ftprivy   = nflux.tprivy;
    flux.ftprivz   = nflux.tprivz;
    flux.fbeamx    = nflux.beamx;
    flux.fbeamy    = nflux.beamy;
    flux.fbeamz    = nflux.beamz;
    flux.fbeampx   = nflux.beampx;
    flux.fbeampy   = nflux.beampy;
    flux.fbeampz   = nflux.beampz;    

    flux.fdk2gen   = gnf->GetDecayDist();

    return;
  }

  //--------------------------------------------------
  void GENIEHelper::PackMCTruth(genie::EventRecord *record,
                                simb::MCTruth &truth)
  {
    
    TLorentzVector *vertex = record->Vertex();

    // get the Interaction object from the record - this is the object
    // that talks to the event information objects and is in m
    genie::Interaction *inter = record->Summary();
  
    // get the different components making up the interaction
    const genie::InitialState &initState  = inter->InitState();
    const genie::ProcessInfo  &procInfo   = inter->ProcInfo();
    //const genie::Kinematics   &kine       = inter->Kine();
    //const genie::XclsTag      &exclTag    = inter->ExclTag();
    //const genie::KPhaseSpace  &phaseSpace = inter->PhaseSpace();

    //choose a spill time (ns) to shift the vertex times by:

    double spillTime = fGlobalTimeOffset + fHelperRandom->Uniform()*fRandomTimeOffset;

    // add the particles from the interaction
    TIter partitr(record);
    genie::GHepParticle *part = 0;
    // GHepParticles return units of GeV/c for p.  the V_i are all in fermis
    // and are relative to the center of the struck nucleus.
    // add the vertex X/Y/Z to the V_i for status codes 0 and 1
    int trackid = 0;
    std::string primary("primary");

    while( (part = dynamic_cast<genie::GHepParticle *>(partitr.Next())) ){
    
      simb::MCParticle tpart(trackid, 
                             part->Pdg(),
                             primary,
                             part->FirstMother(),
                             part->Mass(), 
                             part->Status());
      double vtx[4] = {part->Vx(), part->Vy(), part->Vz(), part->Vt()};
      tpart.SetGvtx(vtx);
      tpart.SetRescatter(part->RescatterCode());

      // set the vertex location for the neutrino, nucleus and everything
      // that is to be tracked.  vertex returns values in meters.
      if(part->Status() == 0 || part->Status() == 1){
        vtx[0] = 100.*(part->Vx()*1.e-15 + vertex->X());
        vtx[1] = 100.*(part->Vy()*1.e-15 + vertex->Y());
        vtx[2] = 100.*(part->Vz()*1.e-15 + vertex->Z());
        vtx[3] = part->Vt() + spillTime;
      }
      TLorentzVector pos(vtx[0], vtx[1], vtx[2], vtx[3]);
      TLorentzVector mom(part->Px(), part->Py(), part->Pz(), part->E());
      tpart.AddTrajectoryPoint(pos,mom);
      if(part->PolzIsSet()) {
        TVector3 polz;
        part->GetPolarization(polz);
        tpart.SetPolarization(polz);
      }
      truth.Add(tpart);

      ++trackid;        
    }// end loop to convert GHepParticles to MCParticles

    // is the interaction NC or CC
    int CCNC = simb::kCC;
    if(procInfo.IsWeakNC()) CCNC = simb::kNC;

    // what is the interaction type
    int mode = simb::kUnknownInteraction;

    if     (procInfo.IsQuasiElastic()       ) mode = simb::kQE;
    else if(procInfo.IsDeepInelastic()      ) mode = simb::kDIS;
    else if(procInfo.IsResonant()           ) mode = simb::kRes;
    else if(procInfo.IsCoherent()           ) mode = simb::kCoh;
    else if(procInfo.IsCoherentElas()       ) mode = simb::kCohElastic;
    else if(procInfo.IsElectronScattering() ) mode = simb::kElectronScattering;
    else if(procInfo.IsNuElectronElastic()  ) mode = simb::kNuElectronElastic;
    else if(procInfo.IsInverseMuDecay()     ) mode = simb::kInverseMuDecay;
    else if(procInfo.IsIMDAnnihilation()    ) mode = simb::kIMDAnnihilation;
    else if(procInfo.IsInverseBetaDecay()   ) mode = simb::kInverseBetaDecay;
    else if(procInfo.IsGlashowResonance()   ) mode = simb::kGlashowResonance;
    else if(procInfo.IsAMNuGamma()          ) mode = simb::kAMNuGamma;
    else if(procInfo.IsMEC()                ) mode = simb::kMEC;
    else if(procInfo.IsDiffractive()        ) mode = simb::kDiffractive;
    else if(procInfo.IsEM()                 ) mode = simb::kEM;
    else if(procInfo.IsWeakMix()            ) mode = simb::kWeakMix;
    
    int itype = simb::kNuanceOffset + genie::utils::ghep::NuanceReactionCode(record);
   
    // set the neutrino information in MCTruth
    truth.SetOrigin(simb::kBeamNeutrino);
    
    // The genie event kinematics are subtle different from the event 
    // kinematics that a experimentalist would calculate
    // Instead of retriving the genie values for these kinematic variables 
    // calcuate them from the the final state particles 
    // while ingnoring the fermi momentum and the off-shellness of the bound nucleon.
    genie::GHepParticle * hitnucl = record->HitNucleon();
    TLorentzVector pdummy(0, 0, 0, 0);
    const TLorentzVector & k1 = *((record->Probe())->P4());
    const TLorentzVector & k2 = *((record->FinalStatePrimaryLepton())->P4());
    //const TLorentzVector & p1 = (hitnucl) ? *(hitnucl->P4()) : pdummy;

    double M  = genie::constants::kNucleonMass; 
    TLorentzVector q  = k1-k2;                     // q=k1-k2, 4-p transfer
    double Q2 = -1 * q.M2();                       // momemtum transfer
    double v  = (hitnucl) ? q.Energy()       : -1; // v (E transfer to the nucleus)
    double x  = (hitnucl) ? 0.5*Q2/(M*v)     : -1; // Bjorken x
    double y  = (hitnucl) ? v/k1.Energy()    : -1; // Inelasticity, y = q*P1/k1*P1
    double W2 = (hitnucl) ? M*M + 2*M*v - Q2 : -1; // Hadronic Invariant mass ^ 2
    double W  = (hitnucl) ? std::sqrt(W2)    : -1;
    
    truth.SetNeutrino(CCNC,
                      mode,
                      itype,
                      initState.Tgt().Pdg(),
                      initState.Tgt().HitNucPdg(),
                      initState.Tgt().HitQrkPdg(),
                      W,
                      x,
                      y,
                      Q2);
    return;
  }
  
  //--------------------------------------------------
  void GENIEHelper::PackGTruth(genie::EventRecord *record, 
                               simb::GTruth &truth) {
    
    //interactions info
    genie::Interaction *inter = record->Summary();
    const genie::ProcessInfo  &procInfo = inter->ProcInfo();
    truth.fGint = (int)procInfo.InteractionTypeId(); 
    truth.fGscatter = (int)procInfo.ScatteringTypeId(); 
     
    //Event info
    truth.fweight = record->Weight(); 
    truth.fprobability = record->Probability(); 
    truth.fXsec = record->XSec(); 
    truth.fDiffXsec = record->DiffXSec(); 

    TLorentzVector vtx;
    TLorentzVector *erVtx = record->Vertex();
    vtx.SetXYZT(erVtx->X(), erVtx->Y(), erVtx->Z(), erVtx->T() );
    truth.fVertex = vtx;
    
    //genie::XclsTag info
    const genie::XclsTag &exclTag = inter->ExclTag();
    truth.fNumPiPlus = exclTag.NPiPlus(); 
    truth.fNumPiMinus = exclTag.NPiMinus();
    truth.fNumPi0 = exclTag.NPi0();    
    truth.fNumProton = exclTag.NProtons(); 
    truth.fNumNeutron = exclTag.NNucleons();
    truth.fIsCharm = exclTag.IsCharmEvent();   
    truth.fResNum = (int)exclTag.Resonance();

    //kinematics info 
    const genie::Kinematics &kine = inter->Kine();
    
    truth.fgQ2 = kine.Q2(true);
    truth.fgq2 = kine.q2(true);
    truth.fgW = kine.W(true);
    if ( kine.KVSet(genie::kKVSelt) ) {
      // only get this if it is set in the Kinematics class
      // to avoid a warning message
      truth.fgT = kine.t(true);
    }
    truth.fgX = kine.x(true);
    truth.fgY = kine.y(true);
    
    /*
    truth.fgQ2 = kine.Q2(false); 
    truth.fgW = kine.W(false);
    truth.fgT = kine.t(false);
    truth.fgX = kine.x(false);
    truth.fgY = kine.y(false);
    */
    truth.fFShadSystP4 = kine.HadSystP4();
    
    //Initial State info
    const genie::InitialState &initState  = inter->InitState();
    truth.fProbePDG = initState.ProbePdg();
    truth.fProbeP4 = *initState.GetProbeP4();
    
    //Target info
    const genie::Target &tgt = initState.Tgt();
    truth.fIsSeaQuark = tgt.HitSeaQrk();
    truth.fHitNucP4 = tgt.HitNucP4();
    truth.ftgtZ = tgt.Z();
    truth.ftgtA = tgt.A();
    truth.ftgtPDG = tgt.Pdg();

    return;

  }

  //----------------------------------------------------------------------
  void GENIEHelper::PackSimpleFlux(simb::MCFlux &flux)
  {
    flux.Reset();

    // cast the fFluxD pointer to be of the right type
    genie::flux::GSimpleNtpFlux *gsf = dynamic_cast<genie::flux::GSimpleNtpFlux *>(fFluxD);
    
    // maintained variable names from gnumi ntuples
    // see http://www.hep.utexas.edu/~zarko/wwwgnumi/v19/[/v19/output_gnumi.html]
    
    const genie::flux::GSimpleNtpEntry* nflux_entry = gsf->GetCurrentEntry();
    const genie::flux::GSimpleNtpNuMI*  nflux_numi  = gsf->GetCurrentNuMI();
  
    flux.fntype  = nflux_entry->pdg;
    flux.fnimpwt = nflux_entry->wgt;
    flux.fdk2gen = nflux_entry->dist;
    flux.fnenergyn = flux.fnenergyf = nflux_entry->E;

    if ( nflux_numi ) {
      flux.frun      = nflux_numi->run;
      flux.fevtno    = nflux_numi->evtno;
      flux.ftpx      = nflux_numi->tpx;
      flux.ftpy      = nflux_numi->tpy;
      flux.ftpz      = nflux_numi->tpz;
      flux.ftptype   = nflux_numi->tptype;   // converted to PDG
      flux.fvx       = nflux_numi->vx;
      flux.fvy       = nflux_numi->vy;
      flux.fvz       = nflux_numi->vz;

      flux.fndecay   = nflux_numi->ndecay;
      flux.fppmedium = nflux_numi->ppmedium;

      flux.fpdpx     = nflux_numi->pdpx;
      flux.fpdpy     = nflux_numi->pdpy;
      flux.fpdpz     = nflux_numi->pdpz;

      double apppz = nflux_numi->pppz;
      if ( TMath::Abs(nflux_numi->pppz) < 1.0e-30 ) apppz = 1.0e-30;
      flux.fppdxdz   = nflux_numi->pppx / apppz;
      flux.fppdydz   = nflux_numi->pppy / apppz;
      flux.fpppz     = nflux_numi->pppz;

      flux.fptype    = nflux_numi->ptype;

    }
    
    // anything useful stuffed into vdbl or vint?
    // need to check the metadata  auxintname, auxdblname

    const genie::flux::GSimpleNtpAux*   nflux_aux  = gsf->GetCurrentAux();
    const genie::flux::GSimpleNtpMeta*  nflux_meta  = gsf->GetCurrentMeta();
    if ( nflux_aux && nflux_meta ) {

      // references just for reducing complexity
      const std::vector<std::string>& auxdblname = nflux_meta->auxdblname;
      const std::vector<std::string>& auxintname = nflux_meta->auxintname;
      const std::vector<int>&    auxint = nflux_aux->auxint;
      const std::vector<double>& auxdbl = nflux_aux->auxdbl;

      for (size_t id=0; id<auxdblname.size(); ++id) {
        if ("muparpx"   == auxdblname[id]) flux.fmuparpx  = auxdbl[id];
        if ("muparpy"   == auxdblname[id]) flux.fmuparpy  = auxdbl[id];
        if ("muparpz"   == auxdblname[id]) flux.fmuparpz  = auxdbl[id];
        if ("mupare"    == auxdblname[id]) flux.fmupare   = auxdbl[id];
        if ("necm"      == auxdblname[id]) flux.fnecm     = auxdbl[id];
        if ("nimpwt"    == auxdblname[id]) flux.fnimpwt   = auxdbl[id];
        if ("fgXYWgt"   == auxdblname[id]) {
          flux.fnwtnear = flux.fnwtfar = auxdbl[id]; 
        }
      }
      for (size_t ii=0; ii<auxintname.size(); ++ii) {
        if ("tgen"      == auxintname[ii]) flux.ftgen     = auxint[ii];
        if ("tgptype"   == auxintname[ii]) flux.ftgptype  = auxint[ii];
      }

    }

#define RWH_TEST
#ifdef RWH_TEST
    static bool first = true;
    if (first) {
      first = false;
      mf::LogDebug("GENIEHelper")
        << "GSimpleNtpMeta:\n"
        << *nflux_meta << "\n";
    }
    mf::LogDebug("GENIEHelper")
      << "simb::MCFlux:\n"
      << flux << "\n"
      << "GSimpleNtpFlux:\n"
      << *nflux_entry << "\n"
      << *nflux_numi << "\n"
      << *nflux_aux << "\n";
#endif

    //   flux.fndxdz    = nflux.ndxdz;
    //   flux.fndydz    = nflux.ndydz;
    //   flux.fnpz      = nflux.npz;
    //   flux.fnenergy  = nflux.nenergy;
    //   flux.fndxdznea = nflux.ndxdznea;
    //   flux.fndydznea = nflux.ndydznea;
    //   flux.fnenergyn = nflux.nenergyn;
    //   flux.fnwtnear  = nflux.nwtnear;
    //   flux.fndxdzfar = nflux.ndxdzfar;
    //   flux.fndydzfar = nflux.ndydzfar;
    //   flux.fnenergyf = nflux.nenergyf;
    //   flux.fnwtfar   = nflux.nwtfar;
    //   flux.fnorig    = nflux.norig;
    // in numi //   flux.fndecay   = nflux.ndecay;
    //   flux.fntype    = nflux.ntype;
    // in numi //   flux.fvx       = nflux.vx;
    // in numi //  flux.fvy       = nflux.vy;
    // in numi //  flux.fvz       = nflux.vz;
    //   flux.fppenergy = nflux.ppenergy;
    // in numi //   flux.fppmedium = nflux.ppmedium;
    //   flux.fppvx     = nflux.ppvx;
    //   flux.fppvy     = nflux.ppvy;
    //   flux.fppvz     = nflux.ppvz;
    // see above //   flux.fmuparpx  = nflux.muparpx;
    // see above //   flux.fmuparpy  = nflux.muparpy;
    // see above //   flux.fmuparpz  = nflux.muparpz;
    // see above //   flux.fmupare   = nflux.mupare;
    // see above //   flux.fnecm     = nflux.necm;
    // see above //   flux.fnimpwt   = nflux.nimpwt;
    //   flux.fxpoint   = nflux.xpoint;
    //   flux.fypoint   = nflux.ypoint;
    //   flux.fzpoint   = nflux.zpoint;
    //   flux.ftvx      = nflux.tvx;
    //   flux.ftvy      = nflux.tvy;
    //   flux.ftvz      = nflux.tvz;
    // see above //   flux.ftgen     = nflux.tgen;
    // see above //   flux.ftgptype  = nflux.tgptype;  // converted to PDG
    //   flux.ftgppx    = nflux.tgppx;
    //   flux.ftgppy    = nflux.tgppy;
    //   flux.ftgppz    = nflux.tgppz;
    //   flux.ftprivx   = nflux.tprivx;
    //   flux.ftprivy   = nflux.tprivy;
    //   flux.ftprivz   = nflux.tprivz;
    //   flux.fbeamx    = nflux.beamx;
    //   flux.fbeamy    = nflux.beamy;
    //   flux.fbeamz    = nflux.beamz;
    //   flux.fbeampx   = nflux.beampx;
    //   flux.fbeampy   = nflux.beampy;
    //   flux.fbeampz   = nflux.beampz; 
    
    flux.fdk2gen   = gsf->GetDecayDist();

    return;
  }

  //---------------------------------------------------------
  void GENIEHelper::ExpandFluxPaths()
  {
    // expand any wildcards in the paths variable
    // if unset and using the old DIRECT method allow it to fall back
    // to using FW_SEARCH_PATH ... but not for the new ifdhc approach

    std::string initial = fFluxSearchPaths;

    if ( fFluxCopyMethod == "DIRECT" && fFluxSearchPaths == "" ) {
      fFluxSearchPaths = cet::getenv("FW_SEARCH_PATH");
    }
    fFluxSearchPaths = gSystem->ExpandPathName(fFluxSearchPaths.c_str());

    mf::LogInfo("GENIEHelper") 
      << "ExpandFluxPaths initially: \"" << initial << "\"\n"
      << "             final result: \"" << fFluxSearchPaths << "\"\n"
      << "                    using: \"" << fFluxCopyMethod << "\" method";
  }
  //---------------------------------------------------------
  void GENIEHelper::ExpandFluxFilePatternsDirect()
  {
    // Using the the fFluxSearchPaths list of directories, apply the
    // user supplied pattern as a suffix to find the flux files.
    // The userpattern might include simple wildcard globs (in contrast 
    // to proper regexp patterns).

    // After expanding the list to individual files, randomize them
    // and start selecting until a size limit is about to be
    // exceeded (though a minimum there needs to be one file, not
    // matter the limit).

    // older GENIE (pre r3669) can't handle vectors/sets of file names
    // but an only accept a pattern that will resolve to files.  This
    // collection could exceed the desired size threshold, but for
    // pick the collection that expands to the largest list

#ifndef GFLUX_MISSING_SETORVECTOR
    // no extra handling if we can pass a list 
#else
    // need to keep track of patterns that that resolve, and who has most
    std::vector<std::string> patternsWithFiles;
    std::vector<int>         nfilesForPattern;
    int nfilesSoFar = 0;
#endif

    bool randomizeFiles = false;
    if ( fFluxType.compare("ntuple")      == 0 ||
         fFluxType.compare("simple_flux") == 0 ||
         fFluxType.compare("dk2nu")       == 0    ) randomizeFiles = true;

    std::vector<std::string> dirs;
    cet::split_path(fFluxSearchPaths,dirs);
    if ( dirs.empty() ) dirs.push_back(std::string()); // at least null string 

    glob_t g;
    int flags = GLOB_TILDE;   // expand ~ home directories

    std::ostringstream patterntext;  // for info/error messages
    std::ostringstream dirstext;     // for info/error messages

    std::vector<std::string>::const_iterator uitr = fFluxFilePatterns.begin();
    int ipatt = 0;

    for ( ; uitr != fFluxFilePatterns.end(); ++uitr, ++ipatt ) {
      std::string userpattern = *uitr;
      patterntext << "\n\t" << userpattern;

      std::vector<std::string>::const_iterator ditr = dirs.begin();
      for ( ; ditr != dirs.end(); ++ditr ) {
        std::string dalt = *ditr;
        // if non-null, does it end with a "/"?  if not add one
        size_t len = dalt.size();
        if ( len > 0 && dalt.rfind('/') != len-1 ) dalt.append("/");
        if ( uitr == fFluxFilePatterns.begin() ) dirstext << "\n\t" << dalt;

        std::string filepatt = dalt + userpattern;

        glob(filepatt.c_str(),flags,NULL,&g);
        if ( g.gl_pathc > 0 ) flags |= GLOB_APPEND; // next glob() will append to list

#ifndef GFLUX_MISSING_SETORVECTOR
        // nothing special since we can use any files we want
#else
        // keep track of pattern with most files ... we'll use that
        int nresolved = g.gl_pathc - nfilesSoFar;
        nfilesSoFar = g.gl_pathc;
        if ( nresolved > 0 ) {
          patternsWithFiles.push_back(filepatt);
          nfilesForPattern.push_back(nresolved);
        }
#endif

      }  // loop over FluxSearchPaths dirs
    }  // loop over user patterns 

    std::ostringstream paretext;
    std::ostringstream flisttext;

#ifndef GFLUX_MISSING_SETORVECTOR
    int nfiles = g.gl_pathc;

    if ( nfiles == 0 ) {
      paretext << "\n  expansion resulted in a null list for flux files";

    } else if ( ! randomizeFiles ) {
      // some sets of files should be left in order 
      // and no size limitations imposed ... just copy the list

      paretext << "\n  list of files will be processed in order";
      for (int i=0; i<nfiles; ++i) {
        std::string afile(g.gl_pathv[i]);
        fSelectedFluxFiles.push_back(afile);

        flisttext << "[" << setw(3) << i << "] "
                  << afile << "\n";
      }
    } else {

      // now pull from the list randomly
      // do this by assigning a random number to each;
      // ordering that list; and pulling in that order

      paretext << "list of " << nfiles << " will be randomized and pared down to " 
               << fMaxFluxFileMB << " MB";

      double* order = new double[nfiles];
      int* indices  = new int[nfiles];
      fHelperRandom->RndmArray(nfiles,order);
      // assign random # for their relative order
      
      TMath::Sort((int)nfiles,order,indices,false);
      
      long long int sumBytes = 0; // accumulated size in bytes
      long long int maxBytes = (long long int)fMaxFluxFileMB * 1024ll * 1024ll;

      FileStat_t fstat;
      for (int i=0; i<nfiles; ++i) {
        int indx = indices[i];
        std::string afile(g.gl_pathv[indx]);
        bool keep = true;
        
        gSystem->GetPathInfo(afile.c_str(),fstat);
        sumBytes += fstat.fSize;
        // skip those that would push sum above total
        // but always accept at least one (the first)
        if ( sumBytes > maxBytes && i != 0 ) keep = false;

        flisttext << "[" << setw(3) << i << "] "
                  << "=> g[" << setw(3) << indx << "] " 
                  << ((keep)?"keep":"skip") << " " 
                  << setw(6) << (sumBytes/(1024ll*1024ll)) << " "
                  << afile << "\n";

        if ( keep ) fSelectedFluxFiles.push_back(afile);
        else break;  // <voice name=Scotty> Captain, she can't take any more</voice>

      }
      delete [] order;
      delete [] indices;

    }
#else
    // This version of GENIE can't handle a list of files, 
    // so only pass it patterns.  Later code will pick the first 
    // in the list, so list them in decreasing order of # of files
    int  npatt = patternsWithFiles.size();
    if ( npatt > 0 ) {
      flisttext << "ExpandFluxFilePatternsDirect: " << npatt
                << " user patterns resolved to files:\n";
      // std::vector is contiguous, so take address of 0-th element
      const int* nf = &(nfilesForPattern[0]);
      int* indices  = new int[npatt];
      TMath::Sort(npatt,nf,indices,true);  // descending order
      for (int i=0; i<npatt; ++i) {
        int indx = indices[i];
        flisttext  << "[" << i << "] " << nfilesForPattern[indx]
                   << " files in " << patternsWithFiles[indx] << "\n";
        fSelectedFluxFiles.push_back(patternsWithFiles[indx]);
      }
      delete [] indices;
    }
#endif

    mf::LogInfo("GENIEHelper") 
      << "ExpandFluxFilePatternsDirect initially found " << nfiles
      << " files for user patterns:"
      << patterntext.str() << "\n  using FluxSearchPaths of: "
      << dirstext.str() <<  "\n" << paretext.str();
      //<< "\"" << cet::getenv("FW_SEARCH_PATH") << "\"";

    mf::LogDebug("GENIEHelper") << "\n" << flisttext.str();

    // done with glob list
    globfree(&g);

    // no null path allowed for at least these
    if ( fFluxType.compare("ntuple")      == 0 ||
         fFluxType.compare("simple_flux") == 0 ||
         fFluxType.compare("dk2nu")       == 0    ) {
      size_t nfiles = fSelectedFluxFiles.size();
      if ( nfiles == 0 ) {
        mf::LogError("GENIEHelper") 
          << "For \"ntuple\" or \"simple_flux\", specification "
          << "must resolve to at least one file" 
          << "\n  none were found user pattern: " 
          << patterntext.str()
          << "\n  using FluxSearchPaths of: "
          << dirstext.str();
        //\"" << cet::getenv("FW_SEARCH_PATH") << "\"";
        throw cet::exception("NoFluxFiles")
          << "no flux files found for: " << patterntext.str();

      }
    }
    
  } // ExpandFluxFilePatternsDirect

  //---------------------------------------------------------
  void GENIEHelper::ExpandFluxFilePatternsIFDH()
  {
    // Using the the FluxSearchPaths list of directories, apply the
    // user supplied pattern as a suffix to find the flux files.
    // The userpattern might include simple wildcard globs (in contrast 
    // to proper regexp patterns).

    // After expanding the list to individual files, randomize them
    // and start selecting until a size limit is about to be
    // exceeded (though a minimum there needs to be one file, not
    // matter the limit).

    // Use the IFDH interface to get the list of files and sizes;
    // after sorting/selecting use IFDH to make a local copy

#ifdef NO_IFDH_LIB
    std::ostringstream fmesg;
    std::string marker = "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"; 
    fmesg << marker
          << __FILE__ << ":" << __LINE__ 
          << "\nno IFDH implemented on this platform\n"
          << marker;
    // make sure the message goes everywhere
    std::cout << fmesg.str() << std::flush;
    std::cerr << fmesg.str();
    throw cet::exception("Attempt to use ifdh class") << fmesg.str();
    assert(0);
#else
    // if "method" just an identifier and not a scheme then clear it
    if ( fFluxCopyMethod.find("IFDH") == 0 ) fFluxCopyMethod = "";

    bool randomizeFiles = false;
    if ( fFluxType.compare("ntuple")      == 0 ||
         fFluxType.compare("simple_flux") == 0 ||
         fFluxType.compare("dk2nu")       == 0    ) randomizeFiles = true;

    if ( ! fIFDH ) fIFDH = new ifdh_ns::ifdh;

    std::string spaths = fFluxSearchPaths;

    const char* ifdh_debug_env = std::getenv("IFDH_DEBUG_LEVEL");
    if ( ifdh_debug_env ) {
      mf::LogInfo("GENIEHelper") << "IFDH_DEBUG_LEVEL: " << ifdh_debug_env;
      fIFDH->set_debug(ifdh_debug_env);
    }

    // filenames + size 
    std::vector<std::pair<std::string,long>> 
      partiallist, fulllist, selectedlist, locallist;

    std::ostringstream patterntext;  // stringification of vector of patterns
    std::ostringstream fulltext;     // for info on full list of matches
    std::ostringstream selectedtext; // for info on selected files
    std::ostringstream localtext;    // for info on local files
    fulltext << "search paths: " << spaths;

    //std::vector<std::string>::const_iterator uitr = fFluxFilePatterns.begin();

    // loop over possible patterns
    // IFDH handles various stems but done a list of globs
    size_t ipatt=0;
    auto uitr = fFluxFilePatterns.begin();
    for ( ; uitr != fFluxFilePatterns.end(); ++uitr, ++ipatt ) {
      std::string userpattern = *uitr;
      patterntext << "\npattern [" << std::setw(3) << ipatt << "] " << userpattern;
      fulltext    << "\npattern [" << std::setw(3) << ipatt << "] " << userpattern;

      partiallist = fIFDH->findMatchingFiles(spaths,userpattern);
      fulllist.insert(fulllist.end(),partiallist.begin(),partiallist.end());

      // make a complete list ...
      fulltext << " found " << partiallist.size() << " files";
      for (auto pitr = partiallist.begin(); pitr != partiallist.end(); ++pitr) {
        fulltext << "\n  " << std::setw(10) << pitr->second << " " << pitr->first;
      }

      partiallist.clear();      
    }  // loop over user patterns 

    size_t nfiles = fulllist.size();

    mf::LogInfo("GENIEHelper") 
      << "ExpandFluxFilePatternsIFDH initially found " << nfiles << " files";
    mf::LogDebug("GENIEHelper") 
      << fulltext.str();

    if ( nfiles == 0 ) {
      selectedtext << "\n  expansion resulted in a null list for flux files";
    } else if ( ! randomizeFiles ) {
      // some sets of files should be left in order 
      // and no size limitations imposed ... just copy the list

      selectedtext << "\n  list of files will be processed in order";
      selectedlist.insert(selectedlist.end(),fulllist.begin(),fulllist.end());

    } else {

      // for list needing size based trimming and randomization ...
      // pull from the list randomly until a cummulative limit is reached
      // do this by assigning a random number to each;
      // ordering that list; and pulling in that order

      selectedtext << "list of " << nfiles << " will be randomized and pared down to " 
                   << fMaxFluxFileMB << " MB";

      double* order = new double[nfiles];
      int* indices  = new int[nfiles];
      fHelperRandom->RndmArray(nfiles,order);
      // assign random # for their relative order
      
      TMath::Sort((int)nfiles,order,indices,false);
      
      long long int sumBytes = 0; // accumulated size in bytes
      long long int maxBytes = (long long int)fMaxFluxFileMB * 1024ll * 1024ll;

      for (size_t i=0; i<nfiles; ++i) {
        int indx = indices[i];
        bool keep = true;
        
        auto p = fulllist[indx]; // this pair <name,size>
        sumBytes += p.second; 
        // skip those that would push sum above total
        // but always accept at least one (the first)
        if ( sumBytes > maxBytes && i != 0 ) keep = false;

        selectedtext << "\n[" << setw(3) << i << "] "
                     << "=> [" << setw(3) << indx << "] " 
                     << ((keep)?"keep":"SKIP") << " " 
                     << std::setw(6) << (sumBytes/(1024ll*1024ll)) << " MB "
                     << p.first;

        if ( keep ) selectedlist.push_back(p);
        else break;  // <voice name=Scotty> Captain, she can't take any more</voice>

      }
      delete [] order;
      delete [] indices;
    }

    mf::LogInfo("GENIEHelper") 
      << selectedtext.str();

    // have a selected list of remote files
    // get paths to local copies
    locallist = fIFDH->fetchSharedFiles(selectedlist,fFluxCopyMethod);

    localtext << "final list of files:";
    size_t i=0;
    for (auto litr = locallist.begin(); litr != locallist.end(); ++litr, ++i) {
        fSelectedFluxFiles.push_back(litr->first);
        localtext << "\n\t[" << std::setw(3) << i << "]\t" << litr->first;
      }

    mf::LogInfo("GENIEHelper") 
      << localtext.str();

    // no null path allowed for at least these
    if ( fFluxType.compare("ntuple")      == 0 ||
         fFluxType.compare("simple_flux") == 0 ||
         fFluxType.compare("dk2nu")       == 0    ) {
      size_t nfiles = fSelectedFluxFiles.size();
      if ( nfiles == 0 ) {
        mf::LogError("GENIEHelper") 
          << "For \"ntuple\" or \"simple_flux\", specification "
          << "must resolve to at least one file" 
          << "\n  none were found user pattern(s): " 
          << patterntext.str()
          << "\n  using FW_SEARCH_PATH of: "
          << spaths;

        throw cet::exception("NoFluxFiles")
          << "no flux files found for: " << patterntext.str();

      }
    }
#endif  // 'else' code only if NO_IFDH_LIB not defined
  } // ExpandFluxFilePatternsIFDH

  //---------------------------------------------------------
  void GENIEHelper::SetGXMLPATH()
  {
    /// GXMLPATH is where GENIE will look for alternative 
    ///    XML configurations (including message service threshold files)

    // priority order
    //   (fcl file paths):(existing user environment):(FW_SEARCH_PATH)
    //
    //  fcl parameters might be the explicit GXMLPATH value
    //     or a pair in the fEnvironment vector
    //  but the final "official result" should the fGXMLPATH

    // start with fGXMLPATH set from pset "GXMLPATH" value

    // find it in the vector, if it exists
    size_t indxGXMLPATH = -1;
    for (size_t i = 0; i < fEnvironment.size(); i += 2) {
      if ( fEnvironment[i].compare("GXMLPATH") == 0 ) {
        if ( fGXMLPATH != "" ) fGXMLPATH += ":";
        fGXMLPATH += fEnvironment[i+1];
        indxGXMLPATH = i;
        break;
      }
    }

    const char* gxmlpath_env = std::getenv("GXMLPATH");
    if ( gxmlpath_env ) {
      if ( fGXMLPATH != "" ) fGXMLPATH += ":";
      fGXMLPATH += gxmlpath_env;
    }
    const char* fwpath_env = std::getenv("FW_SEARCH_PATH");
    if ( fwpath_env ) {
      if ( fGXMLPATH != "" ) fGXMLPATH += ":";
      fGXMLPATH += fwpath_env;
    }

    // refresh fEnvironment (in case fEnvironment is still being used)
    if ( indxGXMLPATH < 0 ) {
       // nothing in fcl parameters
      indxGXMLPATH=fEnvironment.size();
      fEnvironment.push_back("GXMLPATH");
      fEnvironment.push_back(fGXMLPATH);
    } else {
      // update value
      fEnvironment[indxGXMLPATH+1] = fGXMLPATH;
    }

    // now set it externally for use by GENIE
    gSystem->Setenv("GXMLPATH",fGXMLPATH.c_str());

  }

  //---------------------------------------------------------
  void GENIEHelper::SetGMSGLAYOUT()
  {
    /// GMSGLAYOUT ([BASIC}|SIMPLE) control GENIE's layout of log4cpp message
    ///    SIMPLE lacks the timestamp; this must be set in the environment
    ///    at the time the log4cpp Messenger singleton is created

    // start with GMSGLAYOUT set from pset "GMSGLAYOUT" value

    // find it in the vector, if it exists
    // this will override the top level fcl value
    for (size_t i = 0; i < fEnvironment.size(); i += 2) {
      if ( fEnvironment[i].compare("GMSGLAYOUT") == 0 ) {
        fGMSGLAYOUT = fEnvironment[i+1];
        break;
      }
    }

    // now set it externally, if we have a value
    if ( fGMSGLAYOUT != "" ) {
      gSystem->Setenv("GMSGLAYOUT",fGMSGLAYOUT.c_str());
    }
  }

  //---------------------------------------------------------
  void GENIEHelper::StartGENIEMessenger(std::string prodmodestr)
  {
    /// start with fGENIEMsgThresholds from pset "GENIEMsgThresholds" value
    /// allow fEnvironment $GMSGCONF and $GPRODMODE to expand it
    /// function arg might also trigger addition of 
    /// Messenger_production.xml (pre-R-2_9_0) or Messenger_whisper.xml

    /// $GPRODMODE used to trigger Messenger_production.xml
    /// with R-2_8_0 one must add it explicitly to $GMSGCONF
    
    int indxGPRODMODE = -1;
    int indxGMSGCONF  = -1;
    
    for (size_t i = 0; i < fEnvironment.size(); i += 2) {
      if ( fEnvironment[i].compare("GPRODMODE") == 0 ) {
        indxGPRODMODE = i;
        continue;
      }
      if ( fEnvironment[i].compare("GMSGCONF") == 0 ) {
        indxGMSGCONF = i;
        continue;
      }
    } 
    // GMSGCONF set in fEnvironment tack it on to current setting
    if ( indxGMSGCONF >= 0 ) {
      if ( fGENIEMsgThresholds != "" ) fGENIEMsgThresholds += ":"; 
      fGENIEMsgThresholds += fEnvironment[indxGMSGCONF+1];
    } else {
      // nothing in fcl parameters, make a placeholder
      indxGMSGCONF = fEnvironment.size();
      fEnvironment.push_back("GMSGCONF");
      fEnvironment.push_back("");
    }
    
    bool prodmode = StringToBool(prodmodestr);
    if ( indxGPRODMODE >= 0 ) {
      // user tried to set GPRODMODE via Environment fcl values
      prodmode |= StringToBool(fEnvironment[indxGPRODMODE+1]);
    }

    if ( prodmode ) {
      // okay they really meant it
      // PREpend "Messenger_whisper.xml" to existing value
      // don't forget the colon if necessary
#if __GENIE_RELEASE_CODE__ >= GRELCODE(2,9,0)
      std::string newval = "Messenger_whisper.xml";
#else
      std::string newval = "Messenger_production.xml";
#endif
      if ( fGENIEMsgThresholds != "" ) {
        newval += ":";
        newval += fGENIEMsgThresholds;
      }
      fGENIEMsgThresholds = newval;
    }

    // update fEnvironment value
    if ( indxGMSGCONF >= 0 ) {
      fEnvironment[indxGMSGCONF+1] = fGENIEMsgThresholds;
    }

    mf::LogInfo("GENIEHelper") 
      << "StartGENIEMessenger ProdMode=" << ((prodmode)?"yes":"no")
      << " read from: " << fGENIEMsgThresholds;
#ifndef GENIE_USE_ENVVAR
    genie::utils::app_init::MesgThresholds(fGENIEMsgThresholds);
#else
    gSystem->Setenv("GMSGCONF",fGENIEMsgThresholds.c_str());
    if ( prodmode ) gSystem->Setenv("GPRODMODE","YES");
#endif

  }

  //---------------------------------------------------------
  void GENIEHelper::FindEventGeneratorList()
  {
    /// Determine EventGeneratorList 
    //  new location, fcl parameter "EventGeneratorList"
    //  old location  fEvironment  key="GEVGL"  (if unset by direct pset value)
    //  if neither then use "Default"
    
    if ( fEventGeneratorList == "" ) {
      // find GEVGL in the vector, if it exists
      for (size_t i = 0; i < fEnvironment.size(); i += 2) {
        if ( fEnvironment[i].compare("GEVGL") == 0 ) {
          fEventGeneratorList = fEnvironment[i+1];
          break;
        }
      }
    }
    if ( fEventGeneratorList == "" ) fEventGeneratorList = "Default";

    mf::LogInfo("GENIEHelper") << "GENIE EventGeneratorList using \""
                               << fEventGeneratorList << "\"";
#ifndef GENIE_USE_ENVVAR
    // just save it away because post R-2_8_0 this needs to get
    // passed to the GMCJDriver explicitly
#else
    gSystem->Setenv("GEVGL",fEventGeneratorList.c_str());
#endif

  }

  //---------------------------------------------------------
  void GENIEHelper::ReadXSecTable()
  {
    /// determine which cross section table to use
    /// fully expand the path

    // priority order:
    //    fcl fEnvironment GSPLOAD
    //    fcl XSecTable
    //    $GSPLOAD in environment
    //    default 'gxspl-NuMIsmall.xml'

    if ( fXSecTable == "" ) {
      // stand-alone value is not set
      const char* gspload_alt = std::getenv("GSPLOAD");
      if ( ! gspload_alt ) {
        const char* gspload_dflt = "gxspl-NuMIsmall.xml";  // fall back
        gspload_alt = gspload_dflt;
      }
      fXSecTable = std::string(gspload_alt);
    }

    // find GSPLOAD in the vector, if it exists
    int indxGSPLOAD   = -1;
    for (size_t i = 0; i < fEnvironment.size(); i += 2) {
      if ( fEnvironment[i].compare("GSPLOAD") == 0 ) {
        indxGSPLOAD = i;
        continue;
      }
    }

    if ( indxGSPLOAD < 0 ) {
      // nothing in fcl parameters
      indxGSPLOAD=fEnvironment.size();
      fEnvironment.push_back("GSPLOAD");
      fEnvironment.push_back(fXSecTable);
    } else {
      fXSecTable = fEnvironment[indxGSPLOAD+1];  // get two in sync
    }
    
    // currently GENIE doesn't internally use GXMLPATH when looking for
    // spline files, but instead wants a fully expanded path.
    // Do the expansion here using the extended GXMLPATH list
    // of locations (which included $FW_SEARCH_PATH)
    mf::LogDebug("GENIEHelper") << "GSPLOAD as originally: " << fXSecTable; 

    // RWH cet::search_path returns "" if the input string is actually
    // the full path to the file .. this is not really what one wants,
    // one just wan't the full path to the file; seems to work if
    // "/" is made to be another possible PATH
    cet::search_path spGXML( "/:" + fGXMLPATH );
    std::string fullpath;
    spGXML.find_file(fXSecTable, fullpath);

    if ( fullpath == "" ) {
      mf::LogError("GENIEHelper") 
        << "could not resolve full path for spline file XSecTable/GSPLOAD " 
        << "\"" << fXSecTable << "\" using: " 
        << fGXMLPATH;
      throw cet::exception("UnresolvedGSPLOAD")
        << "can't find XSecTable/GSPLOAD file: " << fXSecTable;
    }
    fXSecTable = fullpath;
    fEnvironment[indxGSPLOAD+1] = fXSecTable;  // get two in sync

    mf::LogInfo("GENIEHelper") 
      << "XSecTable/GSPLOAD full path \"" << fXSecTable << "\"";

#ifndef GENIE_USE_ENVVAR
    TStopwatch xtime;
    xtime.Start();

    // can't use gSystem->Unsetenv() as it is really gSystem->Setenv(name,"")
    unsetenv("GSPLOAD");  // MUST!!! ensure that it isn't set externally
    genie::utils::app_init::XSecTable(fXSecTable,true);

    xtime.Stop();
    mf::LogInfo("GENIEHelper") 
      << "Time to read GENIE XSecTable: " 
      << " Real " << xtime.RealTime() << " s,"
      << " CPU " << xtime.CpuTime() << " s"
      << " from " << fXSecTable;
#else
    // pre R-2_8_0 uses $GSPLOAD to indicate x-sec table
    gSystem->Setenv("GSPLOAD", fXSecTable.c_str());
#endif

  }

  //---------------------------------------------------------
  bool GENIEHelper::StringToBool(std::string v)
  {
    if (v == "true")   return true;  // C++ style
    if (v == "false")  return false;
    if (v == "kTRUE")  return true;  // ROOT style
    if (v == "kFALSE") return false;
    if (v == "TRUE")   return true;  // Some other reasonable variations...
    if (v == "FALSE")  return false;
    if (v == "True")   return true;
    if (v == "False")  return false;
    if (v == "on")     return true;
    if (v == "off")    return false;
    if (v == "On")     return true;
    if (v == "Off")    return false;
    if (v == "ON")     return true;
    if (v == "OFF")    return false;
    if (v == "YES")    return true;
    if (v == "NO")     return false;
    if (v == "Yes")    return true;
    if (v == "No")     return false;
    if (v == "yes")    return true;
    if (v == "no")     return false;
    if (v == "1")      return true;
    if (v == "0")      return false;

    return false;  // by default invalid strings are false
  }

} // namespace evgb

