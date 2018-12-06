///////////////////////////////////////////////////////////////////////
/// \file  GENIEHelper.h
/// \brief Wrapper for generating neutrino interactions with GENIE
///
/// \version $Id: GENIEHelper.cxx,v 1.58 2012-11-28 23:04:03 rhatcher Exp $
/// \author  brebel@fnal.gov  rhatcher@fnal.gov
/// update 2010-03-04 Sarah Budd added simple_flux
/// update 2013-04-24 rhatcher adapt to R-2_8_0 interface; subset flux files
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
#include "TF1.h"
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
#include "TRotation.h"

//GENIE includes
#ifdef GENIE_PRE_R3
  #include "GENIE/Conventions/GVersion.h"
  #include "GENIE/Conventions/Units.h"
  #include "GENIE/EVGCore/EventRecord.h"
  #include "GENIE/EVGDrivers/GMCJDriver.h"
  #include "GENIE/GHEP/GHepUtils.h"
  #include "GENIE/FluxDrivers/GCylindTH1Flux.h"
  #include "GENIE/FluxDrivers/GMonoEnergeticFlux.h"
  #include "GENIE/FluxDrivers/GNuMIFlux.h"
  #include "GENIE/FluxDrivers/GSimpleNtpFlux.h"
  #include "GENIE/FluxDrivers/GFluxDriverFactory.h"
  #if __GENIE_RELEASE_CODE__ >= GRELCODE(2,11,0)
    #include "GENIE/FluxDrivers/GBGLRSAtmoFlux.h"  //for atmo nu generation
    #include "GENIE/FluxDrivers/GFLUKAAtmoFlux.h"  //for atmo nu generation
  #else
    #include "GENIE/FluxDrivers/GBartolAtmoFlux.h"  //for atmo nu generation
    #include "GENIE/FluxDrivers/GFlukaAtmo3DFlux.h" //for atmo nu generation
  #endif
  #if __GENIE_RELEASE_CODE__ >= GRELCODE(2,12,2)
    #include "GENIE/FluxDrivers/GHAKKMAtmoFlux.h" // for atmo nu generation
  #endif
  #include "GENIE/FluxDrivers/GAtmoFlux.h"        //for atmo nu generation

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-variable"
  #include "GENIE/Conventions/Constants.h" //for calculating event kinematics
  #pragma GCC diagnostic pop

  #include "GENIE/PDG/PDGLibrary.h"
  #include "GENIE/PDG/PDGCodes.h"
  #include "GENIE/Utils/AppInit.h"
  #include "GENIE/Utils/RunOpt.h"

  #include "GENIE/Geo/ROOTGeomAnalyzer.h"
  #include "GENIE/Geo/GeomVolSelectorFiducial.h"
  #include "GENIE/Geo/GeomVolSelectorRockBox.h"
  #include "GENIE/Utils/StringUtils.h"
  #include "GENIE/Utils/XmlParserUtils.h"
  #include "GENIE/Interaction/InitialState.h"
  #include "GENIE/Interaction/Interaction.h"
  #include "GENIE/Interaction/Kinematics.h"
  #include "GENIE/Interaction/KPhaseSpace.h"
  #include "GENIE/Interaction/ProcessInfo.h"
  #include "GENIE/Interaction/XclsTag.h"
  #include "GENIE/GHEP/GHepParticle.h"
  #include "GENIE/PDG/PDGCodeList.h"

  #include "GENIE/FluxDrivers/GFluxBlender.h"
  #include "GENIE/FluxDrivers/GFlavorMixerI.h"
  #include "GENIE/FluxDrivers/GFlavorMap.h"
  #include "GENIE/FluxDrivers/GFlavorMixerFactory.h"

#else
  // GENIE R-3 reorganized headers
  #include "GENIE/Framework/Conventions/GVersion.h"
  #include "GENIE/Framework/Utils/StringUtils.h"
  #include "GENIE/Framework/Utils/XmlParserUtils.h"

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-variable"
  // constants for calculating event kinematics
  #include "GENIE/Framework/Conventions/Constants.h"
  #pragma GCC diagnostic pop

  #include "GENIE/Framework/Conventions/Units.h"
  #include "GENIE/Framework/ParticleData/PDGCodes.h"
  #include "GENIE/Framework/ParticleData/PDGCodeList.h"
  #include "GENIE/Framework/ParticleData/PDGLibrary.h"

  #include "GENIE/Framework/GHEP/GHepParticle.h"
  #include "GENIE/Framework/GHEP/GHepUtils.h"

  #include "GENIE/Framework/Interaction/InitialState.h"
  #include "GENIE/Framework/Interaction/Interaction.h"
  #include "GENIE/Framework/Interaction/Kinematics.h"
  #include "GENIE/Framework/Interaction/KPhaseSpace.h"
  #include "GENIE/Framework/Interaction/ProcessInfo.h"
  #include "GENIE/Framework/Interaction/XclsTag.h"

  #include "GENIE/Framework/EventGen/GFluxI.h"
  #include "GENIE/Framework/EventGen/EventRecord.h"
  #include "GENIE/Framework/EventGen/GMCJDriver.h"

  #include "GENIE/Framework/Utils/AppInit.h"
  #include "GENIE/Framework/Utils/RunOpt.h"

  #include "GENIE/Tools/Geometry/ROOTGeomAnalyzer.h"
  #include "GENIE/Tools/Geometry/GeomVolSelectorFiducial.h"
  #include "GENIE/Tools/Geometry/GeomVolSelectorRockBox.h"

  #include "GENIE/Tools/Flux/GFluxBlender.h"
  #include "GENIE/Tools/Flux/GFlavorMixerI.h"
  #include "GENIE/Tools/Flux/GFlavorMap.h"
  #include "GENIE/Tools/Flux/GFlavorMixerFactory.h"
  #include "GENIE/Tools/Flux/GFluxDriverFactory.h"

  #include "GENIE/Tools/Flux/GCylindTH1Flux.h"
  #include "GENIE/Tools/Flux/GMonoEnergeticFlux.h"
  #include "GENIE/Tools/Flux/GNuMIFlux.h"
  #include "GENIE/Tools/Flux/GSimpleNtpFlux.h"
  #include "GENIE/Tools/Flux/GAtmoFlux.h"
  #include "GENIE/Tools/Flux/GBGLRSAtmoFlux.h"  // was GBartolAtmoFlux.h
  #include "GENIE/Tools/Flux/GFLUKAAtmoFlux.h"  // was GFlukaAtmo3DFlux.h
  #include "GENIE/Tools/Flux/GHAKKMAtmoFlux.h"  //

#endif


// dk2nu flux
#include "dk2nu/tree/dk2nu.h"
#include "dk2nu/tree/dkmeta.h"
#include "dk2nu/tree/NuChoice.h"
#include "dk2nu/genie/GDk2NuFlux.h"

// NuTools includes
#include "nutools/EventGeneratorBase/evgenbase.h"
#include "nutools/EventGeneratorBase/GENIE/GENIEHelper.h"

#include "nutools/EventGeneratorBase/GENIE/GENIE2ART.h"
#include "nutools/EventGeneratorBase/GENIE/EvtTimeShiftFactory.h"
#include "nutools/EventGeneratorBase/GENIE/EvtTimeShiftI.h"

// nusimdata includes
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCNeutrino.h"

// Framework includes
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "cetlib/search_path.h"
#include "cetlib/getenv.h"
#include "cetlib/split_path.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "cetlib_except/exception.h"

// can't find IFDH_service.h header ... unless ups depends on ifdh_art
//
//  #undef USE_IFDH_SERVICE

#ifndef NO_IFDH_LIB
  #define USE_IFDH_SERVICE 1
  // IFDHC
  #ifdef USE_IFDH_SERVICE
    #include "IFDH_service.h"
  #else
    // bare IFDHC
    #include "ifdh.h"
  #endif
#else
  #undef USE_IFDH_SERVICE
  // nothing doing ... use ifdef to hide any reference that might need header
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
    , fTimeShifter       (0)
    , fFluxType          (pset.get< std::string              >("FluxType")               )
    , fFluxSearchPaths   (pset.get< std::string              >("FluxSearchPaths","")     )
    , fFluxFilePatterns  (pset.get< std::vector<std::string> >("FluxFiles")              )
    , fMaxFluxFileMB     (pset.get< int                      >("MaxFluxFileMB",    2000) ) // 2GB max default
    , fMaxFluxFileNumber (pset.get< int                      >("MaxFluxFileNumber",9999) ) // at most 9999 files
    , fFluxCopyMethod    (pset.get< std::string              >("FluxCopyMethod","DIRECT")) // "DIRECT" = old direct access method
    , fFluxCleanup       (pset.get< std::string              >("FluxCleanup","/var/tmp") ) // "ALWAYS", "NEVER", "/var/tmp"
    , fBeamName          (pset.get< std::string              >("BeamName")               )
    , fFluxRotCfg        (pset.get< std::string              >("FluxRotCfg","none")      )
    , fFluxRotValues     (pset.get< std::vector<double>      >("FluxRotValues", {} )     ) // default empty vector
    , fFluxRotation      (0)
    , fTopVolume         (pset.get< std::string              >("TopVolume")              )
    , fWorldVolume       ("volWorld")
    , fDetLocation       (pset.get< std::string              >("DetectorLocation")       )
    , fFluxUpstreamZ     (pset.get< double                   >("FluxUpstreamZ",  -2.e30) )
    , fEventsPerSpill    (pset.get< double                   >("EventsPerSpill",      0) )
    , fPOTPerSpill       (pset.get< double                   >("POTPerSpill",     0.0) )
    , fHistEventsPerSpill(0.)
    , fSpillEvents       (0)
    , fSpillExposure     (0.)
    , fTotalExposure     (0.)
    , fMonoEnergy        (pset.get< double                   >("MonoEnergy",        2.0) )
    , fFunctionalFlux    (pset.get< std::string              >("FunctionalFlux", "x") )
    , fFunctionalBinning (pset.get< int                      >("FunctionalBinning", 10000) )
    , fEmin              (pset.get< double                   >("FluxEmin", 0) )
    , fEmax              (pset.get< double                   >("FluxEmax", 10) )
    , fBeamRadius        (pset.get< double                   >("BeamRadius",        3.0) )
    , fDetectorMass      (detectorMass)
    , fSurroundingMass   (pset.get< double                   >("SurroundingMass",    0.) )
    , fGlobalTimeOffset  (pset.get< double                   >("GlobalTimeOffset", 1.e4) )
    , fRandomTimeOffset  (pset.get< double                   >("RandomTimeOffset", 1.e4) )
    , fSpillTimeConfig   (pset.get< std::string              >("SpillTimeConfig",    "") )
    , fGenFlavors        (pset.get< std::vector<int>         >("GenFlavors")             )
    , fAtmoEmin          (pset.get< double                   >("AtmoEmin",          0.1) )
    , fAtmoEmax          (pset.get< double                   >("AtmoEmax",         10.0) )
    , fAtmoRl            (pset.get< double                   >("Rl",               20.0) )
    , fAtmoRt            (pset.get< double                   >("Rt",               20.0) )
    , fEnvironment       (pset.get< std::vector<std::string> >("Environment")            )
    , fXSecTable         (pset.get< std::string              >("XSecTable",          "") ) //e.g. "gxspl-FNALsmall.xml"
    , fTuneName          (pset.get< std::string              >("TuneName","${GENIE_XSEC_TUNE}") )
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

    // fEnvironment is (generally) deprecated ... print out any settings
    if ( fEnvironment.size() > 0 ) {
      std::ostringstream fenvout;
      fenvout << " fEnviroment.size() = " << fEnvironment.size();
      for (size_t i = 0; i < fEnvironment.size(); i += 2) {
        fenvout << std::endl << " [" << std::setw(20)
                << fEnvironment[i] << "] ==> "
                << fEnvironment[i+1] << std::endl;
      }
      mf::LogInfo("GENIEHelper") << " Use of fEnvironment parameters is deprecated:\n"
                                 << fenvout.str();
    }

    // for histogram, mono-energetic, functional form fluxes ...
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

    // clean up user input
    // also classifies flux type to simplify tests
    //  e.g.  atmo_  tree_
    RegularizeFluxType();

    /// Determine which flux files to use
    /// Do this after random number seed initialization for stability

    // for tree-based fluxes
    // (e.g. "tree_numi" (nee "ntuple"), "tree_simple" and "tree_dk2nu")
    // we don't care about order and don't want duplicates
    // for others order might matter
    if ( fFluxType.find("tree_") == 0 ) SqueezeFilePatterns();

    ExpandFluxPaths();
    if ( fFluxCopyMethod == "DIRECT" ) ExpandFluxFilePatternsDirect();
    else                               ExpandFluxFilePatternsIFDH();

    /// For atmos_ / astro_ fluxes we might need to set a
    /// coordinate system rotation
    if ( fFluxType.find("atmo_")  == 0 ||
         fFluxType.find("astro_") == 0    ) {
      BuildFluxRotation();
    }

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

    // In case we're printing the event record, how verbose should it be
    genie::GHepRecord::SetPrintLevel(fGHepPrintLevel);

    // Set GENIE's random # seed
    mf::LogInfo("GENIEHelper")
      << "Init genie::utils::app_init::RandGen() with seed " << seedval;
    genie::utils::app_init::RandGen(seedval);

    // special things for atmos fluxes
    if ( fFluxType.find("atmo_") == 0 ) AtmoFluxCheck();

    // make the histogram associations
    if ( fFluxType.find("histogram") == 0 ) HistogramFluxCheck();

    std::string flvlist;
    for ( std::vector<int>::iterator itr = fGenFlavors.begin(); itr != fGenFlavors.end(); itr++ )
      flvlist += Form(" %d",*itr);

    if ( fFluxType.find("mono") == 0 ) {
      fEventsPerSpill = 1;
      mf::LogInfo("GENIEHelper")
        << "Generating monoenergetic (" << fMonoEnergy
        << " GeV) neutrinos with the following flavors: "
        << flvlist;
    } else if ( fFluxType.find("function") == 0    ) {
      fEventsPerSpill = 1;
      mf::LogInfo("GENIEHelper")
        << "Generating neutrinos using the functional form: "
        << fFunctionalFlux << " E [" << fEmin << ":" << fEmax
        << "] GeV with " << fFunctionalBinning << " bins "
        << "with the following flavors: " << flvlist;
    } else {

      // flux methods other than "mono" and "function" require files
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

    // disallow conflicting settings here
    if ( ( fEventsPerSpill != 0 && fPOTPerSpill != 0 ) ||
         ( fEventsPerSpill == 0 && fPOTPerSpill == 0 )    ) {
      throw cet::exception("GENIEHelper")
        << "one or the other of EventsPerSpill ("
        << fEventsPerSpill << ") or "
        << "POTPerSpill ("
        << fPOTPerSpill << ") needs to be zero (but not both)";
    }

    if ( fEventsPerSpill != 0 ) {
      mf::LogInfo("GENIEHelper") << "Generating " << fEventsPerSpill
                                 << " events for each spill";

    } else {
      mf::LogInfo("GENIEHelper") << "Using " << fPOTPerSpill
                                 << " pot for each spill";
    }

    // how to distribute events in time
    if ( fSpillTimeConfig != "" ) {
      evgb::EvtTimeShiftFactory& timeShiftFactory = evgb::EvtTimeShiftFactory::Instance();
      fTimeShifter = timeShiftFactory.GetEvtTimeShift(fSpillTimeConfig);
      if ( fTimeShifter ) {
        fTimeShifter->PrintConfig();
      } else {
        timeShiftFactory.Print();
      }
    }

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
        << ( (fDriver) ? "":" genie::GMCJDriver" )
        << ( (fFluxD)  ? "":" genie::GFluxI" );
    } else {

      double probscale = fDriver->GlobProbScale();
      double rawpots   = 0;

      // rather than ask individual flux drivers for info
      // use the unified interface

      genie::flux::GFluxExposureI* fexposure =
        dynamic_cast<genie::flux::GFluxExposureI*>(fFluxD);
      if ( fexposure ) {
        rawpots = fexposure->GetTotalExposure();
      }
      genie::flux::GFluxFileConfigI* ffileconfig =
        dynamic_cast<genie::flux::GFluxFileConfigI*>(fFluxD);
      if ( ffileconfig ) {
        ffileconfig->PrintConfig();
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
  #ifdef USE_IFDH_SERVICE
    art::ServiceHandle<IFDH> ifdhp;
    if ( fFluxCleanup.find("ALWAYS")   == 0 ) {
      ifdhp->cleanup();
    } else if ( fFluxCleanup.find("/var/tmp") == 0 ) {
      auto ffitr = fSelectedFluxFiles.begin();
      for ( ; ffitr != fSelectedFluxFiles.end(); ++ffitr ) {
        std::string ff = *ffitr;
        if ( ff.find("/var/tmp") == 0 ) {
          mf::LogDebug("GENIEHelper") << "delete " << ff;
          ifdhp->rm(ff);
        }
      }
    }
  #else
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
#endif

  }

  //--------------------------------------------------
  double GENIEHelper::TotalHistFlux()
  {
    if ( fFluxType.find("mono")     == 0 ||
         fFluxType.find("function") == 0 ||
         fFluxType.find("tree_")    == 0 ||
         fFluxType.find("atmo_")    == 0    ) {
      // shouldn't be asking for this for any of the above
      // perhaps not-not "function"?
      return -999.;
    }

    return fTotalHistFlux;
  }

  //--------------------------------------------------
  void GENIEHelper::Initialize()
  {
    // get this out of the way
    genie::PDGLibrary::Instance();

#ifdef GENIE_PRE_R3
    fDriver = new genie::GMCJDriver(); // this needs to be before ConfigGeomScan

    // Determine EventGeneratorList to use; let GMCJDriver know
    FindEventGeneratorList();
    fDriver->SetEventGeneratorList(fEventGeneratorList);

#else
    // Determine Tune and EventGeneratorList to use
    // needs to be before creating GMCJDriver for version R-3 and beyond
    FindTune();

    fDriver = new genie::GMCJDriver(); // this needs to be before ConfigGeomScan
#endif

    // Figure out which cross section file to use
    // post R-2_8_0 this actually triggers reading the file
    ReadXSecTable();

    // initialize the Geometry and Flux drivers
    InitializeGeometry();
    InitializeFluxDriver();

    fDriver->UseFluxDriver(fFluxD2GMCJD);
    fDriver->UseGeomAnalyzer(fGeomD);

    // must come after creation of Geom, Flux and GMCJDriver
    ConfigGeomScan();      // could trigger fDriver->UseMaxPathLengths(*xmlfile*)

    fDriver->Configure();  // could trigger GeomDriver::ComputeMaxPathLengths()
    fDriver->UseSplines();
    fDriver->ForceSingleProbScale();

    if ( fFluxType.find("histogram") == 0 && fEventsPerSpill < 0.01 ) {
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
    // Should be happen for all cases since R-2_8_0 .. but no harm doing it ourselves

    fFluxD->Clear("CycleHistory");

    return;
  }

  //--------------------------------------------------
  void GENIEHelper::RegularizeFluxType()
  {
    // Regularize fFluxType string to sensible setting

    std::string tmpFluxType = fFluxType;

    // remove lead/trailing whitespace
    size_t ftlead  = tmpFluxType.find_first_not_of(" \t\n");
    if ( ftlead )    tmpFluxType.erase( 0, ftlead );
    size_t ftlen   = tmpFluxType.length();
    size_t fttrail = tmpFluxType.find_last_not_of(" \t\n");
    if ( fttrail != ftlen ) tmpFluxType.erase( fttrail+1, ftlen );

    // strip off leading catagories ... we'll put them back later
    // so we don't accidently allow arbitrary strings
    if ( tmpFluxType.find("atmos_") == 0 ) tmpFluxType.erase(0,6);
    if ( tmpFluxType.find("atmo_")  == 0 ) tmpFluxType.erase(0,5);
    if ( tmpFluxType.find("tree_")  == 0 ) tmpFluxType.erase(0,5);

    // make reasonable inferences of what the user intended

    // simple fluxes
    if ( tmpFluxType.find("hist")   != std::string::npos ) tmpFluxType = "histogram";
    if ( tmpFluxType.find("func")   != std::string::npos ) tmpFluxType = "function";
    if ( tmpFluxType.find("mono")   != std::string::npos ) tmpFluxType = "mono";
    // Atmospheric fluxes
    // prior to R-2_11_0 BGLRS was "BARTOL" and HAKKM was "HONDA"
    if ( tmpFluxType.find("FLUKA")  != std::string::npos ) tmpFluxType = "atmo_FLUKA";
    if ( tmpFluxType.find("BARTOL") != std::string::npos ) tmpFluxType = "atmo_BGRLS";
    if ( tmpFluxType.find("BGLRS")  != std::string::npos ) tmpFluxType = "atmo_BGLRS";
    if ( tmpFluxType.find("HONDA")  != std::string::npos ) tmpFluxType = "atmo_HAKKM";
    if ( tmpFluxType.find("HAKKM")  != std::string::npos ) tmpFluxType = "atmo_HAKKM";
    // TTree-based fluxes (old "ntuple" is really "numi")
    //    we're allowed to randomize the order here, and squeeze out duplicates
    if ( tmpFluxType.find("simple") != std::string::npos ) tmpFluxType = "tree_simple";
    if ( tmpFluxType.find("ntuple") != std::string::npos ) tmpFluxType = "tree_numi";
    if ( tmpFluxType.find("numi")   != std::string::npos ) tmpFluxType = "tree_numi";
    if ( tmpFluxType.find("dk2nu")  != std::string::npos ) tmpFluxType = "tree_dk2nu";

    fFluxType = tmpFluxType;
  }

  //--------------------------------------------------
  void GENIEHelper::SqueezeFilePatterns()
  {
    // for "numi" (nee "ntuple"), "simple" and "dk2nu" squeeze the patterns
    // so there are no duplicates; for the others we want to preserve order

    // convert vector<> to a set<> and back to vector<>
    // to avoid having duplicate patterns in the list
    std::set<std::string> fluxpattset(fFluxFilePatterns.begin(),
                                      fFluxFilePatterns.end());
    //// if we weren't initializing from ctor we could do
    //std::copy(fFluxFilePatterns.begin(),fFluxFilePatterns.end(),
    //          std::inserter(fluxpattset,fluxpattset.begin()));
    fFluxFilePatterns.clear(); // clear vector, copy unique set back
    std::copy(fluxpattset.begin(),fluxpattset.end(),
              std::back_inserter(fFluxFilePatterns));
  }

  //--------------------------------------------------
  void GENIEHelper::AtmoFluxCheck()
  {
    /// Speical pre-checks for atmo_ fluxes

    if ( fGenFlavors.size() != fSelectedFluxFiles.size() ) {
      mf::LogInfo("GENIEHelper")
        <<  "ERROR: The number of generated neutrino flavors ("
        << fGenFlavors.size()
        << ") doesn't correspond to the number of files ("
        << fSelectedFluxFiles.size() << ")!!!";
      throw cet::exception("GENIEHelper")
        << "ERROR: atmo_ flavors != files";
    } else {
      std::ostringstream atmofluxmatch;
      for (size_t indx=0; indx < fGenFlavors.size(); ++indx ) {
        atmofluxmatch << "   " << std::setw(3) << fGenFlavors[indx]
                      << " " << fSelectedFluxFiles[indx] << "\n";
      }
      mf::LogInfo("GENIEHelper")
        <<  "atmo flux assignment : \n" << atmofluxmatch.str();
    }

    if ( fEventsPerSpill != 1 ) {
      mf::LogInfo("GENIEHelper")
        <<  "ERROR: For Atmospheric Neutrino generation,"
        << " EventPerSpill need to be 1!!";
      throw cet::exception("GENIEHelper")
        << "ERROR: " << fFluxType << " EventsPerSpill wasn't 1 ("
        << fEventsPerSpill << ")";
    }

    std::ostringstream atmofluxinfo;

    if (fFluxType.find("FLUKA") != std::string::npos ){
      atmofluxinfo << "  The fluxes are from FLUKA";
    }
    else if (fFluxType.find("BARTOL") != std::string::npos ||
             fFluxType.find("BGLRS")  != std::string::npos    ){
      atmofluxinfo << "  The fluxes are from BARTOL/BGLRS";
    }
    else if (fFluxType.find("HONDA") != std::string::npos ||
             fFluxType.find("HAKKM") != std::string::npos    ){
      atmofluxinfo << "  The fluxes are from HONDA/HAKKM";
    }
    else {
      mf::LogInfo("GENIEHelper")
        << "Unknown atmo_ flux simulation: " << fFluxType;
      throw cet::exception("GENIEHelper")
        << "ERROR: bad atmo_ flux type " << fFluxType;
    }

    atmofluxinfo
      << '\n'
      << "  The energy range is between:  " << fAtmoEmin << " GeV and "
      << fAtmoEmax << " GeV."
      << '\n'
      << "  Generation surface of: (" << fAtmoRl << ","
      << fAtmoRt << ")";

    mf::LogInfo("GENIEHelper") << atmofluxinfo.str();

  }

  //--------------------------------------------------
  void GENIEHelper::HistogramFluxCheck()
  {

    mf::LogInfo("GENIEHelper")
      << "setting beam direction and center at "
      << fBeamDirection.X() << " " << fBeamDirection.Y()
      << " " << fBeamDirection.Z()
      << " (" << fBeamCenter.X() << "," << fBeamCenter.Y()
      << "," << fBeamCenter.Z()
      << ") with radius " << fBeamRadius;

    TDirectory *savedir = gDirectory;

    fFluxHistograms.clear();

    TFile tf((*fSelectedFluxFiles.begin()).c_str());
    tf.ls();

    for ( std::vector<int>::iterator flvitr = fGenFlavors.begin(); flvitr != fGenFlavors.end(); flvitr++){
      const char* histname = "none";
      switch ( *flvitr ) {
        case  12: histname = "nue";      break;
        case -12: histname = "nuebar";   break;
        case  14: histname = "numu";     break;
        case -14: histname = "numubar";  break;
        case  16: histname = "nutau";    break;
        case -16: histname = "nutaubar"; break;
        default: {
          throw cet::exception("GENIEHelper")
            <<  "ERROR: no support for histogram flux with flavor PDG="
            << *flvitr;
        }
      }
      fFluxHistograms.push_back(dynamic_cast<TH1D *>(tf.Get(histname)));
    }

    for ( unsigned int i = 0; i < fFluxHistograms.size(); ++i ) {
      fFluxHistograms[i]->SetDirectory(savedir);
      fTotalHistFlux += fFluxHistograms[i]->Integral();
    }

    mf::LogInfo("GENIEHelper")
      << "total histogram flux over desired flavors = "
      << fTotalHistFlux;

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
    mf::LogInfo("GENIEHelper")
      << "GENIEHelper::InitializeGeometry using TopVolume '"
      << fTopVolume << "'";
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

    // simplify a lot of things ...
    // but for now this part only handles the 3 ntuple styles
    // that support the GFluxFileConfig mix-in
    // not the atmos, histo or mono versions

    std::string fluxName = "";

    // what looks like the start of a fully qualified class name
    // or one of our tree_ classes "numi" "simple" "dk2nu"
    // but only know how to configure those that dervie from:
    //    genie::flux::GFluxFileConfigI*
    if ( fFluxType.find("genie::flux::")   != std::string::npos )
      fluxName = fFluxType;
    else if ( fFluxType.find("tree_numi")    == 0 )
      fluxName = "genie::flux::GNuMIFlux";
    else if ( fFluxType.find("tree_simple")  == 0 )
      fluxName = "genie::flux::GSimpleNtpFlux";
    else if ( fFluxType.find("tree_dk2nu")   == 0 )
      fluxName = "genie::flux::GDk2NuFlux";

    if ( fluxName != "" ) {
      // any fall through to hopefully be handled below ...
      genie::flux::GFluxDriverFactory& fluxDFactory =
        genie::flux::GFluxDriverFactory::Instance();
      fFluxD = fluxDFactory.GetFluxDriver(fluxName);
      if ( ! fFluxD ) {
        mf::LogInfo("GENIEHelper")
          << "genie::flux::GFluxDriverFactory had not result for '"
          << fluxName << "' (fFluxType was '" << fFluxType << "'";
        // fall through in hopes someone later picks it up
      } else {
        // got something
        // for the ones that support GFluxFileConfigI (numi,simple,dk2nu)
        // initialize them
        genie::flux::GFluxFileConfigI* ffileconfig =
          dynamic_cast<genie::flux::GFluxFileConfigI*>(fFluxD);
        if ( ffileconfig ) {
          ffileconfig->LoadBeamSimData(fSelectedFluxFiles,fDetLocation);
          ffileconfig->PrintConfig();
          // initialize to only use neutrino flavors requested by user
          genie::PDGCodeList probes;
          for ( std::vector<int>::iterator flvitr = fGenFlavors.begin(); flvitr != fGenFlavors.end(); flvitr++ )
            probes.push_back(*flvitr);
          ffileconfig->SetFluxParticles(probes);
          if ( TMath::Abs(fFluxUpstreamZ) < 1.0e30 ) ffileconfig->SetUpstreamZ(fFluxUpstreamZ);
        }
      }
    } // is genie::flux:: or tree_{numi|simple|dk2nu}

    if ( fFluxType.find("histogram") == 0 ) {

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

    } // end if using a histogram
    else if ( fFluxType.find("mono") == 0 ) {

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
    else if ( fFluxType.find("function") == 0 ) {

      genie::flux::GCylindTH1Flux* histFlux = new genie::flux::GCylindTH1Flux();
      TF1* input_func = new TF1("input_func", fFunctionalFlux.c_str(), fEmin, fEmax);
      TH1D* spectrum = new TH1D("spectrum", "neutrino flux", fFunctionalBinning, fEmin, fEmax);
      spectrum->Add(input_func);

      for ( std::vector<int>::iterator i = fGenFlavors.begin(); i != fGenFlavors.end(); i++ ) {
        histFlux->AddEnergySpectrum(*i, spectrum);
      } //end loop to add flux histograms to driver

      histFlux->SetNuDirection(fBeamDirection);
      histFlux->SetBeamSpot(fBeamCenter);
      histFlux->SetTransverseRadius(fBeamRadius);

      fFluxD = histFlux; // dynamic_cast<genie::GFluxI *>(histFlux);
      delete input_func;
    } //end if using function beam

    // Using the atmospheric fluxes
    else if ( fFluxType.find("atmo_") == 0 ) {

      // Instantiate appropriate concrete flux driver
      genie::flux::GAtmoFlux *atmo_flux_driver = 0;

      if ( fFluxType.find("FLUKA") != std::string::npos ) {
#if __GENIE_RELEASE_CODE__ >= GRELCODE(2,11,0)
        genie::flux::GFLUKAAtmoFlux * fluka_flux =
          new genie::flux::GFLUKAAtmoFlux;
#else
        genie::flux::GFlukaAtmo3DFlux * fluka_flux =
          new genie::flux::GFlukaAtmo3DFlux;
#endif
        atmo_flux_driver = dynamic_cast<genie::flux::GAtmoFlux *>(fluka_flux);
      }
      if ( fFluxType.find("BARTOL") != std::string::npos ||
           fFluxType.find("BGLRS")  != std::string::npos    ) {
#if __GENIE_RELEASE_CODE__ >= GRELCODE(2,11,0)
        genie::flux::GBGLRSAtmoFlux * bartol_flux =
          new genie::flux::GBGLRSAtmoFlux;
#else
        genie::flux::GBartolAtmoFlux * bartol_flux =
          new genie::flux::GBartolAtmoFlux;
#endif
        atmo_flux_driver = dynamic_cast<genie::flux::GAtmoFlux *>(bartol_flux);
      }
#if __GENIE_RELEASE_CODE__ >= GRELCODE(2,12,2)
      if (fFluxType.find("atmo_HONDA") != std::string::npos ||
          fFluxType.find("atmo_HAKKM") != std::string::npos    ) {
        genie::flux::GHAKKMAtmoFlux * honda_flux =
          new genie::flux::GHAKKMAtmoFlux;
        atmo_flux_driver = dynamic_cast<genie::flux::GAtmoFlux *>(honda_flux);
      }
#endif

      atmo_flux_driver->ForceMinEnergy(fAtmoEmin);
      atmo_flux_driver->ForceMaxEnergy(fAtmoEmax);
      if ( fFluxRotation ) atmo_flux_driver->SetUserCoordSystem(*fFluxRotation);

      std::ostringstream atmoCfgText;
      atmoCfgText << "Configuration for " << fFluxType
                  << ", Rl " << fAtmoRl << " Rt " << fAtmoRt;
      for ( size_t j = 0; j < fGenFlavors.size(); ++j ) {
        int         flavor  = fGenFlavors[j];
        std::string flxfile = fSelectedFluxFiles[j];
        atmo_flux_driver->AddFluxFile(flavor,flxfile); // pre-R-2_11_0 was SetFluxFile()
        atmoCfgText << "\n  FLAVOR: " << std::setw(3) << flavor
                    << "  FLUX FILE: " <<  flxfile;
      }
      if ( fFluxRotation ) {
        const int w=13, p=6;
        auto old_p = atmoCfgText.precision(p);
        atmoCfgText << "\n UserCoordSystem rotation:\n"
                    << "  [ "
                    << std::setw(w) << fFluxRotation->XX() << " "
                    << std::setw(w) << fFluxRotation->XY() << " "
                    << std::setw(w) << fFluxRotation->XZ() << " ]\n"
                    << "  [ "
                    << std::setw(w) << fFluxRotation->YX() << " "
                    << std::setw(w) << fFluxRotation->YY() << " "
                    << std::setw(w) << fFluxRotation->YZ() << " ]\n"
                    << "  [ "
                    << std::setw(w) << fFluxRotation->ZX() << " "
                    << std::setw(w) << fFluxRotation->ZY() << " "
                    << std::setw(w) << fFluxRotation->ZZ() << " ]\n";
          atmoCfgText.precision(old_p);
      }
      mf::LogInfo("GENIEHelper") << atmoCfgText.str();

      atmo_flux_driver->LoadFluxData();

      // configure flux generation surface:
      atmo_flux_driver->SetRadii(fAtmoRl, fAtmoRt);

      fFluxD = atmo_flux_driver;//dynamic_cast<genie::GFluxI *>(atmo_flux_driver);

    } //end if using atmospheric fluxes

    if ( ! fFluxD ) {
      mf::LogError("GENIEHelper")
        << "Failed to contruct base flux driver for FluxType '"
        << fFluxType << "'";
      throw cet::exception("GENIEHelper")
        << "Failed to contruct base flux driver for FluxType '"
        << fFluxType << "'\n"
        << __FILE__ << ":" << __LINE__ << "\n";
    }

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

    if ( fGeomScan.find("default") == 0 ) return;

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

    if ( scanmethod.find("file") == 0 ) {
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
    if (        scanmethod.find("box") == 0 ) {
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
    } else if ( scanmethod.find("flux") == 0 ) {
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

    if ( fEventsPerSpill > 0 ) {
      if ( fSpillEvents < fEventsPerSpill ) return false;
    } else {
      // exposure (POT) based
      if ( fFluxType.find("tree_") == 0 ) {
        if ( fSpillExposure < fPOTPerSpill ) {
          return false;
        }
      }
      else if ( fFluxType.find("histogram") == 0 ) {
        if ( fSpillEvents < fHistEventsPerSpill ) return false;
        else fSpillExposure = fPOTPerSpill;
      }
    }

    if ( fFluxType.find("atmo_")  == 0 ) {
      // the exposure for atmo is in SECONDS. In order to get seconds,
      // it needs to be normalized by 1e4 to take into account the units
      // discrepency between AtmoFluxDriver(/m2) and Generate(/cm2)
      // and it need to be normalized by the generation surface area since
      // it's not taken into accoutn in the flux driver
      fTotalExposure =
        (dynamic_cast<genie::flux::GAtmoFlux *>(fFluxD)->NFluxNeutrinos())
        * 1.0e4 / (TMath::Pi() * fAtmoRt*fAtmoRt);

      LOG_DEBUG("GENIEHelper")
        << "===> Atmo EXPOSURE = " << fTotalExposure << " seconds";

    } else {
      fTotalExposure += fSpillExposure;
    }

    // made it to here, means need to reset the counters
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


#if __GENIE_RELEASE_CODE__ >= GRELCODE(2,11,0)
    genie::flux::GFluxExposureI* fexposure =
      dynamic_cast<genie::flux::GFluxExposureI*>(fFluxD);
    if ( fexposure ) {
      fSpillExposure =
        (fexposure->GetTotalExposure()/fDriver->GlobProbScale()) - fTotalExposure;
    }
    // use GENIE2ART code to fill MCFlux
    evgb::FillMCFlux(fFluxD,flux);
#else
    // pack the flux information no support for dk2nu
    if ( fFluxType.find("tree_numi") == 0 ) {
      fSpillExposure = (dynamic_cast<genie::flux::GNuMIFlux *>(fFluxD)->UsedPOTs()/fDriver->GlobProbScale() - fTotalExposure);
      flux.fFluxType = simb::kNtuple;
      PackNuMIFlux(flux);
    }
    else if ( fFluxType.find("tree_simple") == 0 ) {
      // pack the flux information
      fSpillExposure = (dynamic_cast<genie::flux::GSimpleNtpFlux *>(fFluxD)->UsedPOTs()/fDriver->GlobProbScale() - fTotalExposure);
      flux.fFluxType = simb::kSimple_Flux;
      PackSimpleFlux(flux);
    }
#endif

    // if no interaction generated return false
    if ( ! viableInteraction ) return false;

#if __GENIE_RELEASE_CODE__ >= GRELCODE(2,11,0)
    // fill the MCTruth & GTruth information as we have a good interaction
    // these two objects are enough to reconstruct the GENIE record
    // use the new external functions in GENIE2ART

    //choose a spill time (ns) to shift the vertex times by:
    double spilltime = fGlobalTimeOffset;
    if ( ! fTimeShifter ) {
      spilltime += fHelperRandom->Uniform()*fRandomTimeOffset;
    } else {
      spilltime += fTimeShifter->TimeOffset();
    }

    evgb::FillMCTruth(fGenieEventRecord, spilltime, truth);
    evgb::FillGTruth(fGenieEventRecord, gtruth);
#else
    // fill the MC truth information as we have a good interaction
    PackMCTruth(fGenieEventRecord,truth);
    // fill the Generator (genie) truth information
    PackGTruth(fGenieEventRecord, gtruth);
#endif

    // check to see if we are using flux ntuples but want to
    // make n events per spill
    if ( ( fEventsPerSpill > 0 ) &&
         ( fFluxType.find("tree_") == 0 ) ) ++fSpillEvents;

    // now check if using either histogram or mono fluxes, using
    // either n events per spill or basing events on POT per spill for the
    // histogram case
    if ( fFluxType.find("histogram") == 0 ) {
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
    else if ( fFluxType.find("mono")     == 0 ||
              fFluxType.find("function") == 0    ){
      ++fSpillEvents;
    }
    else if ( fFluxType.find("atmo_FLUKA")  == 0 ||
              fFluxType.find("atmo_BARTOL") == 0 ||
              fFluxType.find("atmo_BGLRS")  == 0 ||
              fFluxType.find("atmo_HAKKM")  == 0 ||
              fFluxType.find("atmo_HONDA")  == 0    ) {
      if ( fEventsPerSpill > 0 ) ++fSpillEvents;
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

#ifdef OLD_KINE_CALC
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
#else
    // The internal GENIE event kinematics are subtly different from the event
    // kinematics that a experimentalist would calculate.
    // Instead of retriving the genie values for these kinematic variables ,
    // calculate them from the the final state particles
    // while ignoring the Fermi momentum and the off-shellness of the bound nucleon.
    // (same strategy as in gNtpConv.cxx::ConvertToGST().)
    genie::GHepParticle * hitnucl = record->HitNucleon();
    const TLorentzVector & k1 = *((record->Probe())->P4());
    const TLorentzVector & k2 = *((record->FinalStatePrimaryLepton())->P4());

    // also note that since most of these variables are calculated purely from the leptonic system,
    // they have meaning reactions that didn't strike a nucleon (or even a hadron) as well.
    TLorentzVector q  = k1-k2;      // q=k1-k2, 4-p transfer

    double Q2 = -1 * q.M2();        // momemtum transfer
    double v  = q.Energy();         // v (E transfer to the had system)
    double y  = v/k1.Energy();      // Inelasticity, y = q*P1/k1*P1
    double x, W2, W;
    x = W2 = W = -1;

    if ( hitnucl || procInfo.IsCoherent() ) {
      const double M  = genie::constants::kNucleonMass;
      // Bjorken x.
      // Rein & Sehgal use this same formulation of x even for Coherent
      x  = 0.5*Q2/(M*v);
      // Hadronic Invariant mass ^ 2.
      // ("wrong" for Coherent, but it's "experimental", so ok?)
      W2 = M*M + 2*M*v - Q2;
      W  = std::sqrt(W2);
    }
#endif

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

    // interactions info
    genie::Interaction *inter = record->Summary();
    const genie::ProcessInfo  &procInfo = inter->ProcInfo();
    truth.fGint = (int)procInfo.InteractionTypeId();
    truth.fGscatter = (int)procInfo.ScatteringTypeId();

    // Event info
    truth.fweight = record->Weight();
    truth.fprobability = record->Probability();
    truth.fXsec = record->XSec();
    truth.fDiffXsec = record->DiffXSec();

    TLorentzVector vtx;
    TLorentzVector *erVtx = record->Vertex();
    vtx.SetXYZT(erVtx->X(), erVtx->Y(), erVtx->Z(), erVtx->T() );
    truth.fVertex = vtx;

    // true reaction information and byproducts
    // (PRE FSI)
    const genie::XclsTag &exclTag = inter->ExclTag();
    truth.fIsCharm = exclTag.IsCharmEvent();
    truth.fResNum = (int)exclTag.Resonance();

    // count hadrons from the particle record.
    // note that in principle this information could come from the XclsTag,
    // but that object isn't completely filled for most reactions
    //    truth.fNumPiPlus = exclTag.NPiPlus();
    //    truth.fNumPiMinus = exclTag.NPiMinus();
    //    truth.fNumPi0 = exclTag.NPi0();
    //    truth.fNumProton = exclTag.NProtons();
    //    truth.fNumNeutron = exclTag.NNucleons();
    truth.fNumPiPlus = truth.fNumPiMinus = truth.fNumPi0 = truth.fNumProton = truth.fNumNeutron = 0;
    for (int idx = 0; idx < record->GetEntries(); idx++) {
      // want hadrons that are about to be sent to the FSI model
      const genie::GHepParticle * particle = record->Particle(idx);
      if (particle->Status() != genie::kIStHadronInTheNucleus)
        continue;

      int pdg = particle->Pdg();
      if (pdg == genie::kPdgPi0)
        truth.fNumPi0++;
      else if (pdg == genie::kPdgPiP)
        truth.fNumPiPlus++;
      else if (pdg == genie::kPdgPiM)
        truth.fNumPiMinus++;
      else if (pdg == genie::kPdgNeutron)
        truth.fNumNeutron++;
      else if (pdg == genie::kPdgProton)
        truth.fNumProton++;
    } // for (idx)


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
  void GENIEHelper::BuildFluxRotation()
  {
    // construct fFluxRotation matrix
    //     from fFluxRotCfg + fFluxRotValues
    if ( fFluxRotCfg == "" ||
         ( fFluxRotCfg.find("none") != std::string::npos ) ) return;

    size_t nval = fFluxRotValues.size();

    bool verbose = ( fFluxRotCfg.find("verbose") != std::string::npos );
    if (verbose) {
      std::ostringstream indata;
      indata << "BuildFluxRotation: Cfg \"" << fFluxRotCfg << "\"\n"
             << " " << nval << " values\n";
      for (size_t i=0; i<nval; ++i) {
        indata << "   [" << std::setw(2) << i << "] " << fFluxRotValues[i] << "\n";
      }
      mf::LogInfo("GENIEHelper") << indata.str();
    }

    // interpret as a full 3x3 array
    if ( fFluxRotCfg.find("newxyz") != std::string::npos ||
         fFluxRotCfg.find("3x3")    != std::string::npos    ) {
      if ( nval == 9 ) {
        TRotation fTempRot;
        TVector3 newX = TVector3(fFluxRotValues[0],
                                 fFluxRotValues[1],
                                 fFluxRotValues[2]);
        TVector3 newY = TVector3(fFluxRotValues[3],
                                 fFluxRotValues[4],
                                 fFluxRotValues[5]);
        TVector3 newZ = TVector3(fFluxRotValues[6],
                                 fFluxRotValues[7],
                                 fFluxRotValues[8]);
        fTempRot.RotateAxes(newX,newY,newZ);
        // weirdly necessary; frame vs. obj rotation
        fFluxRotation = new TRotation(fTempRot.Inverse());
        return;
      } else {
        throw cet::exception("BadFluxRotation")
          << "specified: " << fFluxRotCfg << "\n"
          << " but nval=" << nval << ", need 9";
      }
    }

    // another possibility  ... series of rotations around particular axes
    if ( fFluxRotCfg.find("series") != std::string::npos ) {
      TRotation fTempRot;
      // if series then cfg should also have series of rot{X|Y|Z}{deg|rad}
      std::vector<std::string> strs = genie::utils::str::Split(fFluxRotCfg," ,;(){}[]");
      size_t nrot = -1;
      for (size_t j=0; j<strs.size(); ++j) {
        std::string what = strs[j];
        if ( what == ""        ) continue;
        // lower case for convenience
        std::transform(what.begin(),what.end(),what.begin(),::tolower);
        if ( what == "series"  ) continue;
        if ( what == "verbose" ) continue;
        if ( what.find("rot") != 0 ) {
          mf::LogWarning("GENIEHelper")
            << "processing series rotation saw keyword \"" << what << "\" -- ignoring";
          continue;
        }
        char axis = what[3];
        // check that axis is sensibly x, y or z
        if ( axis != 'x' && axis != 'y' && axis != 'z' ) {
          throw cet::exception("BadFluxRotation")
            << "specified: " << fFluxRotCfg << "\n"
            << " keyword '" << what << "': bad axis '" << axis << "'";
        }
        std::string units = what.substr(4);
        // don't worry if written fully as "radians" or "degrees"
        if (units.size() > 3) units.erase(3);
        if ( units != "" && units != "rad" && units != "deg" ) {
          throw cet::exception("BadFluxRotation")
            << "specified: " << fFluxRotCfg << "\n"
            << " keyword '" << what << "': bad units '" << units << "'";
        }
        // no units?  assume degrees
        double scale = (( units == "rad" ) ? 1.0 : TMath::DegToRad() );

        ++nrot;
        if ( nrot >= nval ) {
          // not enough values
          throw cet::exception("BadFluxRotation")
            << "specified: " << fFluxRotCfg << "\n"
            << " asking for rotation [" << nrot << "] "
            << what << " but nval=" << nval;
        }
        double rot = scale * fFluxRotValues[nrot];
        if ( axis == 'x' ) fTempRot.RotateX(rot);
        if ( axis == 'y' ) fTempRot.RotateY(rot);
        if ( axis == 'z' ) fTempRot.RotateZ(rot);

      } // loop over tokens in cfg string

      // weirdly necessary; frame vs. obj rotation
      fFluxRotation = new TRotation(fTempRot.Inverse());

      if ( nrot+1 != nval ) {
        // call out possible user mistake
        mf::LogWarning("GENIEHelper")
          << "BuildFluxRotation only used " << nrot+1 << " of "
          << nval << " FluxRotValues";
      }
      return;
    }

    // could put other interpretations here ...

    throw cet::exception("BadFluxRotation")
      << "specified: " << fFluxRotCfg << "\n"
      << " nval=" << nval << ", but don't know how to interpret that";

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

    bool randomizeFiles = false;
    if ( fFluxType.find("tree_") == 0 ) randomizeFiles = true;

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

      }  // loop over FluxSearchPaths dirs
    }  // loop over user patterns

    std::ostringstream paretext;
    std::ostringstream flisttext;

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

      paretext << "list of " << nfiles
               << " will be randomized and pared down to "
               << fMaxFluxFileMB << " MB or "
               << fMaxFluxFileNumber << " files";

      double* order = new double[nfiles];
      int* indices  = new int[nfiles];
      fHelperRandom->RndmArray(nfiles,order);
      // assign random # for their relative order

      TMath::Sort((int)nfiles,order,indices,false);

      long long int sumBytes = 0; // accumulated size in bytes
      long long int maxBytes = (long long int)fMaxFluxFileMB * 1024ll * 1024ll;

      FileStat_t fstat;
      for (int i=0; i < TMath::Min(nfiles,fMaxFluxFileNumber); ++i) {
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
    if ( fFluxType.find("tree_") == 0 ) {
      size_t nfiles = fSelectedFluxFiles.size();
      if ( nfiles == 0 ) {
        mf::LogError("GENIEHelper")
          << "For \"" << fFluxType <<"\" "
          << "(e.g. \"dk2nu\', \"ntuple\" (\"numi\") or \"simple\") "
          << " specification must resolve to at least one file"
          << "\n  none were found. DIRECT user pattern(s): "
          << patterntext.str()
          << "\n  using FluxSearchPaths of: "
          << dirstext.str();

        throw cet::exception("NoFluxFiles")
          << "no flux files found for: " << patterntext.str() << "\n"
          << " in: " << dirstext.str();

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
    if ( fFluxType.find("tree_") == 0 ) randomizeFiles = true;

  #ifdef USE_IFDH_SERVICE
    art::ServiceHandle<IFDH> ifdhp;
  #else
    if ( ! fIFDH ) fIFDH = new ifdh_ns::ifdh;
  #endif

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

  #ifdef USE_IFDH_SERVICE
      partiallist = ifdhp->findMatchingFiles(spaths,userpattern);
  #else
      partiallist = fIFDH->findMatchingFiles(spaths,userpattern);
  #endif
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

      selectedtext << "list of " << nfiles
                   << " will be randomized and pared down to "
                   << fMaxFluxFileMB << " MB or "
                   << fMaxFluxFileNumber << " files";

      double* order = new double[nfiles];
      int* indices  = new int[nfiles];
      fHelperRandom->RndmArray(nfiles,order);
      // assign random # for their relative order

      TMath::Sort((int)nfiles,order,indices,false);

      long long int sumBytes = 0; // accumulated size in bytes
      long long int maxBytes = (long long int)fMaxFluxFileMB * 1024ll * 1024ll;

      for (size_t i=0; i < TMath::Min(nfiles,size_t(fMaxFluxFileNumber)); ++i) {
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
  #ifdef USE_IFDH_SERVICE
    locallist = ifdhp->fetchSharedFiles(selectedlist,fFluxCopyMethod);
  #else
    locallist = fIFDH->fetchSharedFiles(selectedlist,fFluxCopyMethod);
  #endif

    localtext << "final list of files:";
    size_t i=0;
    for (auto litr = locallist.begin(); litr != locallist.end(); ++litr, ++i) {
        fSelectedFluxFiles.push_back(litr->first);
        localtext << "\n\t[" << std::setw(3) << i << "]\t" << litr->first;
      }

    mf::LogInfo("GENIEHelper")
      << localtext.str();

    // no null path allowed for at least these
    if ( fFluxType.find("tree_") == 0 ) {
      size_t nfiles = fSelectedFluxFiles.size();
      if ( nfiles == 0 ) {
        mf::LogError("GENIEHelper")
          << "For \"" << fFluxType <<"\" "
          << "(e.g. \"dk2nu\', \"ntuple\" (\"numi\") or \"simple\") "
          << "specification must resolve to at least one file"
          << "\n  none were found. IFDH user pattern(s): "
          << patterntext.str()
          << "\n  using FluxSearchPaths of: "
          << spaths;

        throw cet::exception("NoFluxFiles")
          << "no flux files found for: " << patterntext.str() << "\n"
          << " in " << spaths;

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
    int indxGXMLPATH = -1;
    for (size_t i = 0; i < fEnvironment.size(); i += 2) {
      if ( fEnvironment[i].find("GXMLPATH") == 0 ) {
        if ( fGXMLPATH != "" ) fGXMLPATH += ":";
        fGXMLPATH += fEnvironment[i+1];
        indxGXMLPATH = i;
        /*
        throw cet::exception("UsingGXMLPATH")
          << "using Environment fcl parameter GXMLPATH: "
          << fEnvironment[indxGXMLPATH+1]
          << ", use fcl parameter GXMLPATH instead.";
        */
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
      if ( fEnvironment[i].find("GMSGLAYOUT") == 0 ) {
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
      if ( fEnvironment[i].find("GPRODMODE") == 0 ) {
        indxGPRODMODE = i;
        continue;
      }
      if ( fEnvironment[i].find("GMSGCONF") == 0 ) {
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

    genie::utils::app_init::MesgThresholds(fGENIEMsgThresholds);

  }

  //---------------------------------------------------------
  void GENIEHelper::FindTune()
  {
    /// Determine Tune ... initialize as necessary

#ifdef GENIE_PRE_R3
    // Tune isn't relevant pre-R-3
#else
    // this isn't all that useful ... but leave it for the setting of UnphysEventMask
    genie::RunOpt* grunopt = genie::RunOpt::Instance();
    // ctor automatically calls:  grunopt->Init();

    // not sure this is absolutely necessary either
    grunopt->EnableBareXSecPreCalc(true);

    if ( fTuneName.find('$') == 0 ) {
      // need to remove ${}'s
      std::string tuneEnvVar = fTuneName;
      char rmchars[] = "$(){} ";
      for (unsigned int i = 0; i < strlen(rmchars); ++i) {
        // remove moves matching characters in [first,last) to end and
        //   returns a past-the-end iterator for the new end of the range [funky!]
        // erase actually trims the string
        tuneEnvVar.erase( std::remove(tuneEnvVar.begin(), tuneEnvVar.end(), rmchars[i]), tuneEnvVar.end() );
      }

      const char* tune = std::getenv(tuneEnvVar.c_str());
      if ( tune ) {
        mf::LogInfo("GENIEHelper") << "fTuneName started as '" << fTuneName << "' "
                                   << " (env: " << tuneEnvVar << "), "
                                   << " converted to " << tune;
        fTuneName = std::string(tune);
      } else {
        mf::LogError("GENIEHelper") << "fTuneName started as '" << fTuneName << "', "
                                    << " (env: " << tuneEnvVar << "), "
                                    << " but resolved to a empty string";
        throw cet::exception("UnresolvedTuneName")
          << "can't resolve TuneName: " << fTuneName;
      }
    }

    grunopt->SetTuneName(fTuneName);
    FindEventGeneratorList();
    grunopt->SetEventGeneratorList(fEventGeneratorList);
    grunopt->BuildTune();

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
        if ( fEnvironment[i].find("GEVGL") == 0 ) {
          fEventGeneratorList = fEnvironment[i+1];
          throw cet::exception("UsingGEVGL")
            << "using Environment fcl parameter GEVGL: " << fEventGeneratorList
            << ", use fcl parameter EventGeneratorList instead.";
          break;
        }
      }
    }
    if ( fEventGeneratorList == "" ) fEventGeneratorList = "Default";

    mf::LogInfo("GENIEHelper") << "GENIE EventGeneratorList using \""
                               << fEventGeneratorList << "\"";

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
    //    default 'gxspl-FNALsmall.xml'

    if ( fXSecTable == "" ) {
      // stand-alone value is not set
      const char* gspload_alt = std::getenv("GSPLOAD");
      if ( ! gspload_alt ) {
        const char* gspload_dflt = "gxspl-FNALsmall.xml";  // fall back
        gspload_alt = gspload_dflt;
      } else {
        throw cet::exception("$GSPLOAD")
          << "using env variable $GSPLOAD: " << gspload_alt
          << ", use fcl parameter 'XSecTable' instead.";
      }
      fXSecTable = std::string(gspload_alt);
    }

    // find GSPLOAD in the vector, if it exists
    int indxGSPLOAD   = -1;
    for (size_t i = 0; i < fEnvironment.size(); i += 2) {
      if ( fEnvironment[i].find("GSPLOAD") == 0 ) {
        indxGSPLOAD = i;
        throw cet::exception("UsingGSPLOAD")
          << "using Environment fcl parameter GSPLOAD: "
          << fEnvironment[indxGSPLOAD+1]
          << ", use fcl parameter 'XSecTable' instead. "
          << __FILE__ << ":" << __LINE__ << "\n";
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

