////////////////////////////////////////////////////////////////////////
/// \brief  GENIE neutrino event generator, loosely based on NOvA's
/// \author rhatcher@fnal.gov
/// \date
////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <unistd.h>

// ROOT includes
#include "TStopwatch.h"
#include "TGeoManager.h"

// Framework includes
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


#include "nutools/EventGeneratorBase/evgenbase.h"
#include "nutools/EventGeneratorBase/GENIE/GENIEHelper.h"

#include "nutools/EventGeneratorBase/GENIE/EVGBAssociationUtil.h"

#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCNeutrino.h"

//--- Dk2Nu additions
//--- BEGIN
#include "dk2nu/tree/dk2nu.h"
#include "dk2nu/tree/NuChoice.h"
#include "dk2nu/genie/GDk2NuFlux.h"
//--- END

//#undef  PUT_DK2NU_ASSN
#define PUT_DK2NU_ASSN 1

///Monte Carlo event generation
namespace evgen {

  /// A module to check the results from the Monte Carlo generator
  class TestGENIEHelper : public art::EDProducer {

  public:

    explicit TestGENIEHelper(fhicl::ParameterSet const &pset);
    virtual ~TestGENIEHelper();

    void produce(art::Event& evt);
    void beginJob();
    void beginRun(art::Run &run);
    void endSubRun(art::SubRun &sr);

  private:

    evgb::GENIEHelper  *fGENIEHelp;        ///< GENIEHelper object
    TStopwatch          fStopwatch;
    int                 fEventsPerSpill;   ///< negative for Poisson()

  };
}

namespace evgen {

  //___________________________________________________________________________
  TestGENIEHelper::TestGENIEHelper(fhicl::ParameterSet const& pset)
    : fGENIEHelp       (0)
      //, fPassEmptySpills (pset.get< bool >("PassEmptySpills"))
      //, fSpillCounter    (0)
      //, fPOTPerSpill     (pset.get< double >("POTPerSpill",    5.0e13))
      , fEventsPerSpill  (pset.get< double >("EventsPerSpill", 1))
      //, fTotalExposure   (0)
      //, fTotalPOTLimit   (pset.get< double >("TotalPOTLimit"))
  {
    fStopwatch.Start();

    produces< std::vector<simb::MCTruth> >();
    produces< std::vector<simb::MCFlux>  >();
    produces< std::vector<simb::GTruth>  >();
    //produces< sumdata::SpillData >();
    //produces< sumdata::POTSum, art::InSubRun  >();
    //produces< sumdata::RunData, art::InRun    >();
    // Associate every truth with the flux it came from
    produces< art::Assns<simb::MCTruth, simb::MCFlux> >();
    produces< art::Assns<simb::MCTruth, simb::GTruth> >();

    //--- Dk2Nu additions
    //--- BEGIN
    produces< std::vector<bsim::Dk2Nu>  >();
    produces< std::vector<bsim::NuChoice>  >();
#ifdef PUT_DK2NU_ASSN
    produces< art::Assns<simb::MCTruth, bsim::Dk2Nu> >();
    produces< art::Assns<simb::MCTruth, bsim::NuChoice> >();
#endif
    //--- END

    //art::ServiceHandle<geo::Geometry> geo;
    std::string geomFileName =
      pset.get<std::string>("GeomFileName");
    mf::LogInfo("TestGENIEHelper") << "using GeomFileName '"
                                   << geomFileName << "'";
    TGeoManager::Import(geomFileName.c_str());
    double detectorMass = 1; // no generally used (except for histogram)

    fGENIEHelp = new evgb::GENIEHelper(pset,
                                       gGeoManager,
                                       geomFileName,
                                       detectorMass);
                                       /*
				       geo->ROOTGeoManager(),
				       geo->ROOTFile(),
				       geo->TotalMass(pset.get< std::string>("TopVolume").c_str()));
                                       */
  }

  //___________________________________________________________________________
  TestGENIEHelper::~TestGENIEHelper()
  {
    fStopwatch.Stop();
    mf::LogInfo("TestGENIEHelper") << "real time to produce file: "
			    << fStopwatch.RealTime();
    delete fGENIEHelp; // clean up, and let dtor do its thing
  }

  //___________________________________________________________________________
  void TestGENIEHelper::beginJob()
  {
  }

  //___________________________________________________________________________
  void TestGENIEHelper::beginRun(art::Run& run)
  {
    // grab the geometry object to see what geometry we are using
    // art::ServiceHandle<geo::Geometry> geo;

    /*
    std::unique_ptr<sumdata::RunData>
      runcol(new sumdata::RunData(geo->DetId(),
                                  geo->FileBaseName(),
                                  geo->ExtractGDML()));

    run.put(std::move(runcol));
    */

    std::cerr << " *** TestGENIEHelper::beginRun() begin "
              << std::endl << std::flush;


    // initialize the GENIEHelper here rather than in beginJob to
    // avoid problems with the Geometry reloading at a run boundary.
    // If we ever make more than one run in a single job we will have
    // to re-evaluate
    fGENIEHelp->Initialize();
    //fTotalExposure = 0.0;

    std::cerr << " *** TestGENIEHelper::beginRun() done "
              << std::endl << std::flush;

    return;
  }

  //___________________________________________________________________________
  void TestGENIEHelper::endSubRun(art::SubRun &sr)
  {
    /*
    std::unique_ptr< sumdata::POTSum > p(new sumdata::POTSum);

    // p->totpot     = fGENIEHelp->TotalExposure();
    // p->totgoodpot = fGENIEHelp->TotalExposure();
    p->totpot     = fTotalExposure;
    p->totgoodpot = fTotalExposure;
    p->totspills  = fSpillCounter;
    p->goodspills = fSpillCounter;
    p->Print(std::cout);

    sr.put(std::move(p));
    */

    mf::LogInfo("TestGENIEHelper") << "Total Exposure was "
                                   << fGENIEHelp->TotalExposure();

  }

  //___________________________________________________________________________
  void TestGENIEHelper::produce(art::Event& evt)
  {
    // A temporary value is needed to store the spill exposure that GENIEHelper uses.  TestGENIEHelper
    // needs to remember the number after GENIEHelper has reset it to zero for the purposes of
    // updating fTotalExposure.
    //double SpillExpTemp = 0.0;

    std::unique_ptr< std::vector<simb::MCTruth> > truthcol(new std::vector<simb::MCTruth>);
    std::unique_ptr< std::vector<simb::MCFlux>  > fluxcol (new std::vector<simb::MCFlux >);
    std::unique_ptr< std::vector<simb::GTruth>  > gtruthcol (new std::vector<simb::GTruth >);
    std::unique_ptr< art::Assns<simb::MCTruth, simb::GTruth> > tgtassn(new art::Assns<simb::MCTruth, simb::GTruth>);
    std::unique_ptr< art::Assns<simb::MCTruth, simb::MCFlux> > assns(new art::Assns<simb::MCTruth, simb::MCFlux>);

    std::cerr << " ******************************* TestGENIEHelper::produce() " << std::endl << std::flush;
    std::cout << " stopwatch at produce() ";
    fStopwatch.Print("um"); fStopwatch.Continue();
    std::cout << std::flush;

    //--- Dk2Nu additions
    //--- BEGIN
    std::unique_ptr< std::vector<bsim::Dk2Nu> >
       dk2nucol(new std::vector<bsim::Dk2Nu>);
    std::unique_ptr< std::vector<bsim::NuChoice> >
       nuchoicecol(new std::vector<bsim::NuChoice>);

    std::unique_ptr< art::Assns<simb::MCTruth, bsim::Dk2Nu> >
       dk2nuassn(new art::Assns<simb::MCTruth, bsim::Dk2Nu>);
    std::unique_ptr< art::Assns<simb::MCTruth, bsim::NuChoice> >
       nuchoiceassn(new art::Assns<simb::MCTruth, bsim::NuChoice>);
    //--- END

    while ( ! fGENIEHelp->Stop() ) {

      simb::MCTruth truth;
      simb::MCFlux  flux;
      simb::GTruth  gTruth;

      std::cerr << " *** TestGENIEHelper::produce() about to sample "
                << truthcol->size()
                << std::endl << std::flush;
      std::cout << " stopwatch before Sample() ";
      fStopwatch.Print("um"); fStopwatch.Continue();
      std::cout << std::flush;


      // GENIEHelper returns a false in the sample method if
      // either no neutrino was generated, or the interaction
      // occurred beyond the detector's z extent - ie something we
      // would never see anyway.
      if ( fGENIEHelp->Sample(truth, flux, gTruth ) ) {

        std::cout << " stopwatch after Sample() ";
        fStopwatch.Print("um"); fStopwatch.Continue();
        std::cout << std::flush;

        truthcol ->push_back(truth);
        gtruthcol->push_back(gTruth);
        fluxcol  ->push_back(flux);

        evgb::util::CreateAssn(*this, evt, *truthcol, *fluxcol, *assns,
                         fluxcol->size()-1, fluxcol->size());

        evgb::util::CreateAssn(*this, evt, *truthcol, *gtruthcol, *tgtassn,
                         gtruthcol->size()-1, gtruthcol->size());

        //--- Dk2Nu additions
        //--- BEGIN
        genie::GFluxI* fdriver = fGENIEHelp->GetFluxDriver(true);
        genie::flux::GDk2NuFlux* dk2nuDriver =
          dynamic_cast<genie::flux::GDk2NuFlux*>(fdriver);
        if ( dk2nuDriver ) {
          const bsim::Dk2Nu& dk2nuObj = dk2nuDriver->GetDk2Nu();
          dk2nucol   ->push_back(dk2nuObj);
          const bsim::NuChoice& nuchoiceObj = dk2nuDriver->GetNuChoice();
          nuchoicecol->push_back(nuchoiceObj);

#ifdef PUT_DK2NU_ASSN
          evgb::util::CreateAssn(*this, evt, *truthcol, *dk2nucol, *dk2nuassn,
                                 dk2nucol->size()-1, dk2nucol->size());
          evgb::util::CreateAssn(*this, evt, *truthcol, *nuchoicecol, *nuchoiceassn,
                                 nuchoicecol->size()-1, nuchoicecol->size());
#endif
        }
        //--- END

        std::cerr << " *** TestGENIEHelper::produce() sample success size "
                  << truthcol->size()
                  << std::endl << std::flush;
        //RWH//if ( dk2nuDriver ) { std::cout << dk2nuDriver->GetDk2Nu() << std::endl; }
        std::cout << " stopwatch after push_back + CreateAssn ";
        fStopwatch.Print("um"); fStopwatch.Continue();
        std::cout << std::flush;


      } // end if genie was able to make an event

    } // end event generation loop

    // put the collections in the event
    evt.put(std::move(truthcol));
    evt.put(std::move(fluxcol));
    evt.put(std::move(gtruthcol));
    evt.put(std::move(assns));
    evt.put(std::move(tgtassn));

    std::cerr << " *** TestGENIEHelper::produce() done "
              << " event " << evt.event()
              << std::endl << std::flush;

    //--- Dk2Nu additions
    //--- BEGIN
    // in the constructor we said these were produced ...
    // ... so we have to put them in the record, even if just empty
    evt.put(std::move(dk2nucol));
    evt.put(std::move(nuchoicecol));
    
#ifdef PUT_DK2NU_ASSN
    std::cerr << " *** TestGENIEHelper::produce()"
              << " put dk2nuAssn + nuchoiceAssn ** "
              << " event " << evt.event()
              << std::endl << std::flush;

    evt.put(std::move(dk2nuassn));
    evt.put(std::move(nuchoiceassn));

    std::cerr << " *** TestGENIEHelper::produce() finished put "
              << " event " << evt.event()
              << std::endl << std::flush;

#endif
    //--- END

  } // end of produce method
}// namespace

namespace evgen { DEFINE_ART_MODULE(TestGENIEHelper) }

