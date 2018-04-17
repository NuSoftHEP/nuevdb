////////////////////////////////////////////////////////////////////////
//
// brebel@fnal.gov
//
////////////////////////////////////////////////////////////////////////
#ifndef EVGEN_TEST_H
#define EVGEN_TEST_H

#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <unistd.h>

// Framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "cetlib_except/exception.h"
#include "cetlib/search_path.h"

#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "nusimdata/SimulationBase/MCNeutrino.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nutools/EventGeneratorBase/evgenbase.h"

#include "nutools/EventGeneratorBase/CRY/CRYHelper.h"
#include "nutools/EventGeneratorBase/GENIE/GENIEHelper.h"

#include "TStopwatch.h"
#include "TGeoManager.h"

namespace art  { class Event; }
namespace simb { class MCParticle; }

///Monte Carlo event generation
namespace evgen {

  /// A module to check the results from the Monte Carlo generator
  class EventGeneratorTest : public art::EDAnalyzer {

  public:

    explicit EventGeneratorTest(fhicl::ParameterSet const &pset);
    virtual ~EventGeneratorTest();                        

    void analyze(art::Event const& evt);  
    void beginJob();

  private:

    fhicl::ParameterSet GENIEParameterSet(std::string fluxType, 
					  bool usePOTPerSpill);

    void                GENIETest(fhicl::ParameterSet const& pset);
    void                GENIEHistogramFluxTest();
    void                GENIESimpleFluxTest();
    void                GENIEMonoFluxTest();
    void                GENIEAtmoFluxTest();
    void                GENIENtupleFluxTest();

    fhicl::ParameterSet  CRYParameterSet();
    void                 CRYTest();
    bool                 IntersectsDetector(simb::MCParticle const& part);
    void                 ProjectToSurface(TLorentzVector pos,
					  TLorentzVector mom,
					  int axis,
					  double surfaceLoc,
					  double* xyz);


    double      fTotalGENIEPOT;          ///< number of POT to generate with GENIE when 
           	                         ///< in total POT mode			 
    double 	fTotalGENIEInteractions; ///< number of interactions to generate with 
           	                         ///< GENIE when in EventsPerSpill mode	 
    double 	fTotalCRYSpills;         ///< number of spills to use when testing CRY
    std::string fTopVolume;              ///< Top Volume used by GENIE
    std::string fGeometryFile;           ///< location of Geometry GDML file to test
    double      fCryDetLength;           ///< length of detector to test CRY, units of cm
    double      fCryDetWidth;            ///< width of detector to test CRY, units of cm
    double      fCryDetHeight;           ///< height of detector to test CRY, units of cm
  };
}

namespace evgen {

  //____________________________________________________________________________
  EventGeneratorTest::EventGeneratorTest(fhicl::ParameterSet const& pset)
    : EDAnalyzer             (pset)
    , fTotalGENIEPOT         ( pset.get< double      >("TotalGENIEPOT",          5e18))
    , fTotalGENIEInteractions( pset.get< double      >("TotalGENIEInteractions", 100) )
    , fTotalCRYSpills        ( pset.get< double      >("TotalCRYSpills",         1000))
    , fTopVolume             ( pset.get< std::string >("TopVolume"                   ))
    , fGeometryFile          ( pset.get< std::string >("GeometryFile"                ))
    , fCryDetLength          (1000.)
    , fCryDetWidth           (500.)
    , fCryDetHeight          (500.)
  {  
    /// Create a Art Random Number engine
    int seed = (pset.get< int >("Seed", evgb::GetRandomNumberSeed()));
    createEngine(seed);
  }

  //____________________________________________________________________________
  EventGeneratorTest::~EventGeneratorTest()
  {  
  }

  //____________________________________________________________________________
  void EventGeneratorTest::beginJob()
  {  
  }

  //____________________________________________________________________________
  void EventGeneratorTest::analyze(art::Event const& evt)
  {
    mf::LogWarning("EventGeneratorTest") << "testing GENIE...";
    mf::LogWarning("EventGeneratorTest") << "\t histogram flux...";
    this->GENIEHistogramFluxTest();
    mf::LogWarning("EventGeneratorTest") << "\t \t done."
					 << "\t simple flux...";
    this->GENIESimpleFluxTest();
    mf::LogWarning("EventGeneratorTest") << "\t \t done."
					 << "\t atmo flux...";
    this->GENIEAtmoFluxTest();
    mf::LogWarning("EventGeneratorTest") << "\t \t done."
					 << "\t mono flux...";
    this->GENIEMonoFluxTest();
    mf::LogWarning("EventGeneratorTest") << "\t \t done.\n"
					 << "GENIE tests done";

    mf::LogWarning("EventGeneratorTest") << "testing CRY...";
    this->CRYTest();
    mf::LogWarning("EventGeneratorTest") << "\t CRY test done.";
  }

  //____________________________________________________________________________
  fhicl::ParameterSet EventGeneratorTest::GENIEParameterSet(std::string fluxType,
							    bool usePOTPerSpill)
  {
    // make a parameter set first so that we can pass it to the GENIEHelper
    // object we are going to make
    std::vector<double> beamCenter; 
    beamCenter.push_back(0.0); beamCenter.push_back(0.); beamCenter.push_back(0.0);

    std::vector<double> beamDir; 
    beamDir.push_back(0.); beamDir.push_back(0.); beamDir.push_back(1.);

    std::vector<int> flavors;
    if(fluxType.compare("atmo_FLUKA") == 0){
      flavors.push_back(14);
    }
    else{
      flavors.push_back(12); flavors.push_back(14); flavors.push_back(-12); flavors.push_back(-14);
    }
    

    std::vector<std::string> env;
    env.push_back("GPRODMODE"); env.push_back("YES");
    env.push_back("GEVGL");     env.push_back("Default");

    double potPerSpill = 5.e13;
    double eventsPerSpill = 0;
    if(!usePOTPerSpill) eventsPerSpill = 1;

    std::vector<std::string> fluxFiles;
    fluxFiles.push_back("samples_for_geniehelper/L010z185i_lowthr_ipndshed.root");
    if(fluxType.compare("simple_flux") == 0){
      fluxFiles.clear();
      fluxFiles.push_back("samples_for_geniehelper/gsimple_NOvA-NDOS_le010z185i_20100521_RHC_lowth_s_00001.root");
    }
    else if(fluxType.compare("atmo_FLUKA") == 0){
      fluxFiles.clear();
      // at FNAL this is installed relative to in /nusoft/data/flux
      fluxFiles.push_back("atmospheric/battistoni/sdave_numu07.dat");
    }

    else if(fluxType.compare("ntuple") == 0){
      throw cet::exception("EventGeneratorTest") <<"No ntuple flux file "
						 << "exists, bail ungracefully";
    }

    fhicl::ParameterSet pset;
    pset.put("FluxType",         fluxType);
    pset.put("FluxFiles",        fluxFiles);
    pset.put("BeamName",         "numi");
    pset.put("TopVolume",        fTopVolume);
    pset.put("EventsPerSpill",   eventsPerSpill);
    pset.put("POTPerSpill",      potPerSpill);
    pset.put("BeamCenter",       beamCenter);
    pset.put("BeamDirection",    beamDir);
    pset.put("GenFlavors",       flavors);    
    pset.put("Environment",      env);
    pset.put("DetectorLocation", "NOvA-ND");

    mf::LogWarning("EventGeneratorTest") << pset.to_string();

    return pset;
  }
  
  //____________________________________________________________________________
  void EventGeneratorTest::GENIETest(fhicl::ParameterSet const& pset)
  {
    // use cet::search_path to get the Geometry file path
    cet::search_path sp("FW_SEARCH_PATH");
    std::string geometryFile = fGeometryFile;
    if( !sp.find_file(geometryFile, fGeometryFile) )
      throw cet::exception("EventGeneratorTest") << "cannot find geometry file:\n " 
						 << geometryFile
						 << "\n to test GENIE";

    TGeoManager::Import(geometryFile.c_str());

    // make the GENIEHelper object
    evgb::GENIEHelper help(pset,
			   gGeoManager,
			   geometryFile,
			   gGeoManager->FindVolumeFast(pset.get< std::string>("TopVolume").c_str())->Weight());
    help.Initialize();

    int interactionCount = 0;

    int nspill = 0;
    int spillLimit = 0;

    // decide if we are in POT/Spill or Events/Spill mode
    double eps = pset.get<double>("EventsPerSpill");
    if(eps > 0.) spillLimit = TMath::Nint(fTotalGENIEInteractions/eps);
    else         spillLimit = 1000;

    while(nspill < spillLimit){
      ++nspill;
      while( !help.Stop() ){

	simb::MCTruth truth;
	simb::MCFlux  flux;
	simb::GTruth  gTruth;

	if( help.Sample(truth, flux, gTruth) )
	  ++interactionCount;

      } // end creation loop for this spill

    } // end loop over spills

    // count the POT used and the number of events made
    mf::LogWarning("EventGeneratorTest") << "made " << interactionCount << " interactions with " 
				      << help.TotalExposure() << " POTs";

    // compare to a simple expectation
    double totalExp = 0.;
    if(help.FluxType().compare("histogram") == 0 && pset.get<double>("EventsPerSpill") == 0){
      std::vector<TH1D*> fluxhist = help.FluxHistograms();

      if(fluxhist.size() < 1){
	throw cet::exception("EventGeneratorTest") << "using histogram fluxes but no histograms provided!";
      }
      
      // see comments in GENIEHelper::Initialize() for how this calculation was done.
      totalExp = 1.e-38*1.e-20*help.TotalHistFlux();
      totalExp *= help.TotalExposure()*help.TotalMass()/(1.67262158e-27);

      mf::LogWarning("EventGeneratorTest") << "expected " << totalExp << " interactions";
      if(std::abs(interactionCount - totalExp) > 3.*std::sqrt(totalExp) ){
	throw cet::exception("EventGeneratorTest") << "generated count is more than "
						   << "3 sigma off expectation";
      }

    }// end if histogram fluxes


    return;

  }

  //____________________________________________________________________________
  void EventGeneratorTest::GENIEHistogramFluxTest()
  {

    mf::LogWarning("EventGeneratorTest") << "\t\t\t 1 event per spill...\n";

    // make the parameter set
    fhicl::ParameterSet pset1(this->GENIEParameterSet("histogram", false));

    this->GENIETest(pset1);

    mf::LogWarning("EventGeneratorTest") <<"\t\t\t events based on POT per spill...\n";

    fhicl::ParameterSet pset2(this->GENIEParameterSet("histogram", true));
    this->GENIETest(pset2);

    return;
  } 

  //____________________________________________________________________________
  void EventGeneratorTest::GENIESimpleFluxTest()
  {

    // make the parameter set
    mf::LogWarning("EventGeneratorTest") << "testing GENIEHelper in simple_flux mode with \n"
	      << "\t 1 event per spill...\n";

    fhicl::ParameterSet pset1 = this->GENIEParameterSet("simple_flux", false);
    this->GENIETest(pset1);

    mf::LogWarning("EventGeneratorTest") <<"\t events based on POT per spill...\n";

    fhicl::ParameterSet pset2 = this->GENIEParameterSet("simple_flux", true);
    this->GENIETest(pset2);

    return;
  } 

  //____________________________________________________________________________
  void EventGeneratorTest::GENIEMonoFluxTest()
  {

    // make the parameter set
    fhicl::ParameterSet pset1 = this->GENIEParameterSet("mono", false);

    mf::LogWarning("EventGeneratorTest") << "\t\t 1 event per spill...\n";

    this->GENIETest(pset1);

    return;

  } 

  //____________________________________________________________________________
  void EventGeneratorTest::GENIEAtmoFluxTest()
  {
    
    // make the parameter set
    fhicl::ParameterSet pset1 = this->GENIEParameterSet("atmo_FLUKA", false);
    
    mf::LogWarning("EventGeneratorTest") << "\t\t 1 event per spill...\n";
    
    this->GENIETest(pset1);
    
    return;
    
  } 


  //____________________________________________________________________________


  fhicl::ParameterSet EventGeneratorTest::CRYParameterSet()
  {
    fhicl::ParameterSet pset;
    pset.put("SampleTime",       600e-6            );
    pset.put("TimeOffset",      -30e-6             );
    pset.put("EnergyThreshold",  50e-3             );
    pset.put("Latitude",         "latitude 41.8 "  );
    pset.put("Altitude",         "altitude 0 "     );
    pset.put("SubBoxLength",     "subboxLength 75 ");

    mf::LogWarning("EventGeneratorTest") << pset.to_string();

    return pset;
  }

  //____________________________________________________________________________
  void EventGeneratorTest::CRYTest()
  {
    // make the parameter set
    fhicl::ParameterSet pset = this->CRYParameterSet();

    // get the random number generator service and make some CLHEP generators
    art::ServiceHandle<art::RandomNumberGenerator> rng;
    CLHEP::HepRandomEngine& engine = rng->getEngine();

    // make the CRYHelper
    evgb::CRYHelper help(pset, engine);

    int    nspill         = 0;
    double avPartPerSpill = 0.;
    double avPartIntersectPerSpill = 0.;
    double avMuonIntersectPerSpill = 0.;
    double avEIntersectPerSpill    = 0.;
    while(nspill < TMath::Nint(fTotalCRYSpills) ){

      simb::MCTruth mct;
      
      help.Sample(mct, 
		  1.,
		  100.,
		  0);

      avPartPerSpill += mct.NParticles();

      // now check to see if the particles go through the 
      // detector enclosure
      for(int p = 0; p < mct.NParticles(); ++p){
	if(this->IntersectsDetector(mct.GetParticle(p)) ){
	   avPartIntersectPerSpill += 1.;
	   if(TMath::Abs(mct.GetParticle(p).PdgCode()) == 13)
	     avMuonIntersectPerSpill += 1.;
	   else if(TMath::Abs(mct.GetParticle(p).PdgCode()) == 11)
	     avEIntersectPerSpill += 1.;
	}
      }
    


      ++nspill;
    }

    mf::LogWarning("EventGeneratorTest") << "there are " << avPartPerSpill/(1.*nspill)
					 << " cosmic rays made per spill \n"
					 << avPartIntersectPerSpill/(1.*nspill)
					 << " intersect the detector per spill"
					 << "\n\t " 
					 << avMuonIntersectPerSpill/(1.*nspill)
					 << " muons \n\t"
					 << avEIntersectPerSpill/(1.*nspill)
					 << " electrons";


    return;
  }

  //____________________________________________________________________________
  bool EventGeneratorTest::IntersectsDetector(simb::MCParticle const& part)
  {

    // the particle's initial position and momentum
    TLorentzVector pos = part.Position();
    TLorentzVector mom = part.Momentum();

    if(TMath::Abs(mom.P()) == 0){
      mf::LogWarning("EventGeneratorTest") << "particle has no momentum!!! bail";
      return false;
    }

    double xyz[3] = {0.};

    // Checking intersection with 6 planes

    // 1. Check intersection with the y = +halfheight plane
    this->ProjectToSurface(pos, mom, 1, 0.5*fCryDetHeight, xyz);

    if( TMath::Abs(xyz[0]) <= 0.5*fCryDetWidth 
	&& xyz[2] > 0.
	&& TMath::Abs(xyz[2]) <= fCryDetLength ) return true;
      

    // 2. Check intersection with the +x plane
    this->ProjectToSurface(pos, mom, 0, 0.5*fCryDetWidth, xyz);

    if( TMath::Abs(xyz[1]) <= 0.5*fCryDetHeight 
	&& xyz[2] > 0.
	&& TMath::Abs(xyz[2]) <= fCryDetLength ) return true;

    // 3. Check intersection with the -x plane
    this->ProjectToSurface(pos, mom, 0, -0.5*fCryDetWidth, xyz);

    if( TMath::Abs(xyz[1]) <= 0.5*fCryDetHeight 
	&& xyz[2] > 0.
	&& TMath::Abs(xyz[2]) <= fCryDetLength ) return true;

    // 4. Check intersection with the z=0 plane
    this->ProjectToSurface(pos, mom, 2, 0., xyz);

    if( TMath::Abs(xyz[0]) <= 0.5*fCryDetWidth 
	&& TMath::Abs(xyz[1]) <= 0.5*fCryDetHeight ) return true;

    // 5. Check intersection with the z=detlength plane
    this->ProjectToSurface(pos, mom, 2, fCryDetLength, xyz);

    if( TMath::Abs(xyz[0]) <= 0.5*fCryDetWidth 
	&& TMath::Abs(xyz[1]) <= 0.5*fCryDetHeight ) return true;

    return false;
  }

  //____________________________________________________________________________
  void EventGeneratorTest::ProjectToSurface(TLorentzVector pos,
					    TLorentzVector mom,
					    int axis,
					    double surfaceLoc,
					    double* xyz)
  {
    double momDir = 0.;
    double posDir = 0.;
    if(axis == 0){
      momDir = mom.Px();
      posDir = pos.X();
    }
    else if(axis == 1){
      momDir = mom.Py();
      posDir = pos.X();
    }
    else if(axis == 2){
      momDir = mom.Pz();
      posDir = pos.X();
    }
    
    double ddS        = (momDir/mom.P());
    double length1Dim = (posDir - surfaceLoc);

    if(TMath::Abs(ddS) > 0.){
      length1Dim /= ddS;
      xyz[0] = pos.X() + length1Dim*mom.Px()/mom.P();
      xyz[1] = pos.Y() + length1Dim*mom.Py()/mom.P();
      xyz[2] = pos.Z() + length1Dim*mom.Pz()/mom.P();
    }

    return;
  }

}// namespace

namespace evgen{

  DEFINE_ART_MODULE(EventGeneratorTest)

}

#endif // EVGEN_TEST_H


