#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include "fhiclcpp/ParameterSet.h"

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
// for sim::GetRandomNumberSeed()
#include "nutools/EventGeneratorBase/evgenbase.h"

#include "nutools/EventGeneratorBase/GENIE/GENIE2ART.h"
#include "nutools/EventGeneratorBase/GENIE/EvtTimeShiftI.h"
#include "nutools/EventGeneratorBase/GENIE/EvtTimeShiftFactory.h"

// GENIE includes
#ifdef GENIE_PRE_R3
  #include "Ntuple/NtpMCEventRecord.h"
  #include "Ntuple/NtpMCTreeHeader.h"
  #include "PDG/PDGLibrary.h"
  // -- GENIE Messenger conflict LOG_INFO w/ ART messagefacility
  //#include "Messenger/Messenger.h"
  #include "GHEP/GHepRecord.h"

  #include "FluxDrivers/GNuMIFlux.h"
  #include "FluxDrivers/GSimpleNtpFlux.h"
#else
  #include "GENIE/Framework/ParticleData/PDGLibrary.h"
  #include "GENIE/Framework/GHEP/GHepRecord.h"
  #include "GENIE/Framework/Ntuple/NtpMCFormat.h"
  #include "GENIE/Framework/Ntuple/NtpWriter.h"
  #include "GENIE/Framework/Ntuple/NtpMCEventRecord.h"
  // #include "GENIE/Framework/Ntuple/NtpMCTreeHeader.h"
  // #include "GENIE/Framework/Messenger/Messenger.h" -- conflict LOG_INFO w/ messagefacility

  #include "GENIE/Tools/Flux/GNuMIFlux.h"
  #include "GENIE/Tools/Flux/GSimpleNtpFlux.h"
#endif

#include "dk2nu/tree/dk2nu.h"
#include "dk2nu/tree/NuChoice.h"

// ROOT includes
#include "TChain.h"
#include "TBranchElement.h"
#include "TBranchObject.h"

// CLHEP
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoissonT.h"
#include "CLHEP/Random/RandGauss.h"

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>

#include <fstream>
#include <cstdio>
#include <iomanip>

#include "nutools/EventGeneratorBase/GENIE/EVGBAssociationUtil.h"

namespace evg {
  class AddGenieEventsToArt;

  struct AddGenieEventsToArtParams {
    // hold/document fcl parameters
    template<class T> using Atom     = fhicl::Atom<T>;
    template<class T> using Sequence = fhicl::Sequence<T>;
    template<class T> using Table    = fhicl::Table<T>;
    using Comment = fhicl::Comment;
    using Name    = fhicl::Name;

    Sequence<std::string> fileList {
      Name("fileList"),
      Comment("list of input gntp.*.ghep.root files"),
        // no default { " " } // no default
    };
    Atom<std::string> countConfig {
      Name("countConfig"),
      Comment("how many events to pull \"<form>: <value> [<value>]\""
              "  known functional forms:\n"
              "  \"fixed: <n>\"\n"
              "  \"flat: <nmin> <nmax>\"\n"
              "  \"poisson: <mean>\"\n"
              "  \"poisson-1: <mean>\"  use Poisson, then subtract 1 (floor 0)\n"
              "  \"gauss: <mean> <rms>\" (floor 0)"),
      "fixed: 1" // default
    };
    Atom<double>      globalTimeOffset {
      Name("globalTimeOffset"),
      Comment("fixed offset to add (in ns)"),
      0.0
    };
    Atom<std::string>  timeConfig {
      Name("timeConfig"),
      Comment("time distribution beyond globalTimeOffset (in ns)\n"
              "  e.g.  \"flat: 1000\"\n"
              "        \"numi: \"\n"
              "currently does not support modified numi parameters"),
      "numi:"
    };
    struct VtxOffsets {
      Atom<double> xlo { Name("xlo"), Comment("min x addition"), 0.0 };
      Atom<double> ylo { Name("ylo"), Comment("min y addition"), 0.0 };
      Atom<double> zlo { Name("zlo"), Comment("min z addition"), 0.0 };
      Atom<double> xhi { Name("xhi"), Comment("max x addition"), 0.0 };
      Atom<double> yhi { Name("yhi"), Comment("max y addition"), 0.0 };
      Atom<double> zhi { Name("zhi"), Comment("max z addition"), 0.0 };
    };
    Table<VtxOffsets> vtxOffsets {
      Name("vtxOffsets"),
      Comment("allow module to offset global vertex (genie vtx units = m)")
    };
    Atom<bool>        addMCFlux {
      Name("addMCFlux"),
      Comment("attempt to fetch and fill MCFlux for each genie::EventRecord"),
      true
    };
    Atom<bool>        randomEntries {
      Name("randomEntries"),
      Comment("use random sets of entries from input files\n"
              "rather than go through the files sequentially"),
      true
    };
    Atom<int>         outputPrintLevel {
      Name("outputPrintLevel"),
      Comment("print fetched genie::EventRecord -1=no, 13=max info\n"
              "see GENIE manual for legal values"),
      -1
    };
    Atom<std::string> outputDumpFileName {
      Name("outputDumpFileName"),
      Comment("name of file to print to (if outputPrintLevel >= 0)\n"
              "\"std::cout\" for standard out\n"
              "otherwise string with %l replaced by module_label"),
      "AddGenieEventsToArt_%l.txt"
    };
      // want this to be optional ...
    Atom<int>         seed {
      Name("seed"),
      Comment("random number seed"),
      0
    };
  }; // AddGenieEventsToArtParams
}

class evg::AddGenieEventsToArt : public art::EDProducer {
public:

  // Allow 'art --print-description' to work
  using Parameters = art::EDProducer::Table<evg::AddGenieEventsToArtParams>;

  //explicit AddGenieEventsToArt(fhicl::ParameterSet const & p);
  explicit AddGenieEventsToArt(const Parameters & p);

  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.
  ~AddGenieEventsToArt();

  // Plugins should not be copied or assigned.
  AddGenieEventsToArt(AddGenieEventsToArt const &) = delete;
  AddGenieEventsToArt(AddGenieEventsToArt &&) = delete;
  AddGenieEventsToArt & operator = (AddGenieEventsToArt const &) = delete;
  AddGenieEventsToArt & operator = (AddGenieEventsToArt &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

  //void reconfigure(const Parameters & params) override;

  // Selected optional functions.
  /*
  void beginJob() override;
  void beginRun(art::Run & r) override;
  void beginSubRun(art::SubRun & sr) override;
  void endJob() override;
  void endRun(art::Run & r) override;
  void endSubRun(art::SubRun & sr) override;
  void respondToCloseInputFile(art::FileBlock const & fb) override;
  void respondToCloseOutputFiles(art::FileBlock const & fb) override;
  void respondToOpenInputFile(art::FileBlock const & fb) override;
  void respondToOpenOutputFiles(art::FileBlock const & fb) override;
  */

private:

  typedef enum EDistrib { kUnknownDist,
                          kFixed,
                          kFlat,
                          kPoisson,
                          kPoissonMinus1,
                          kGaussian } RndDist_t;

  // Private member functions here.
  void         ParseCountConfig();
  size_t       GetNumToAdd() const;
  void         ParseTimeConfig();
  void         ParseVtxOffsetConfig();

  Parameters                       fParams;

  //  member data here.

  std::vector<std::string>         fFileList;
  double                           fGlobalTimeOffset;
  evgb::EvtTimeShiftI*             fTimeShifter;
  double                           fXlo;  // vtx offset ranges
  double                           fYlo;
  double                           fZlo;
  double                           fXhi;
  double                           fYhi;
  double                           fZhi;
  bool                             fAddMCFlux;
  bool                             fRandomEntries;

  std::string                      fMyModuleType;
  std::string                      fMyModuleLabel;
  int                              fOutputPrintLevel;
  std::string                      fOutputDumpFileName;
  //< use %l to substitute in module_label
  //< use "", "--", "cout" or "std::cout" for using that instead of file
  std::ostream*                    fOutputStream;

  // parsed NumToAddString parameters
  RndDist_t                        fRndDist;
  double                           fRndP1;
  double                           fRndP2;

  TChain*                          fGTreeChain;
  genie::NtpMCEventRecord*         fMCRec;
  size_t                           fNumMCRec;
  size_t                           fLastUsedMCRec; // if going sequentially

  // possible flux branches

  genie::flux::GNuMIFluxPassThroughInfo*  fGNuMIFluxPassThroughInfo;

  genie::flux::GSimpleNtpEntry*           fGSimpleNtpEntry;
  genie::flux::GSimpleNtpNuMI*            fGSimpleNtpNuMI;
  genie::flux::GSimpleNtpAux*             fGSimpleNtpAux;

  bsim::Dk2Nu*                            fDk2Nu;
  bsim::NuChoice*                         fNuChoice;


  // selection order - sequential-round, sequential-1time, random-1?

};

evg::AddGenieEventsToArt::AddGenieEventsToArt(const Parameters& params)
  : EDProducer()  // (params)  // but must have EDAnalyzer(params) ...
  , fParams(params)
  , fGlobalTimeOffset(0)
  , fTimeShifter(0)
  , fXlo(0), fYlo(0), fZlo(0)
  , fXhi(0), fYhi(0), fZhi(0)
  , fAddMCFlux(false)
  , fRandomEntries(true)
  , fOutputPrintLevel(-1)
  , fOutputStream(0)
    //
  , fRndDist(kUnknownDist)
  , fRndP1(-1)
  , fRndP2(-1)
  , fGTreeChain(new TChain("gtree"))
  , fMCRec(new genie::NtpMCEventRecord)
  , fNumMCRec(0)
  , fLastUsedMCRec(0)
  , fGNuMIFluxPassThroughInfo(0)
  , fGSimpleNtpEntry(0)
  , fGSimpleNtpNuMI(0)
  , fGSimpleNtpAux(0)
  , fDk2Nu(0)
  , fNuChoice(0)
{
  // trigger early initialization of PDG database & GENIE message service
  // just to get it out of the way and not intermixed with other output
  genie::PDGLibrary::Instance();

  fMyModuleType  = fParams.get_PSet().get<std::string>("module_type");
  fMyModuleLabel = fParams.get_PSet().get<std::string>("module_label");

  mf::LogInfo("AddGenieEventsToArt")
    << " ctor start " << fMyModuleLabel
    << " (" << fMyModuleType << ") " << std::endl << std::flush;

  fFileList = fParams().fileList();

  // get the random number seed, use a random default if not specified
  // in the configuration file.
  unsigned int seed = fParams().seed();
  if ( seed == 0 ) seed = evgb::GetRandomNumberSeed();

  // only need sub-label if using more than one engine for each
  // instance of this module (already tagged by equiv of fMyModuleLabel
  createEngine(seed); // ,"HepJamesRandom",sub-label);

  ParseCountConfig();
  ParseVtxOffsetConfig();
  ParseTimeConfig();
  fGlobalTimeOffset = fParams().globalTimeOffset();
  fAddMCFlux        = fParams().addMCFlux();
  fRandomEntries    = fParams().randomEntries();

  // Call appropriate produces<>() functions here.

  produces< std::vector<simb::MCTruth> >();
  produces< std::vector<simb::GTruth>  >();
  produces< art::Assns<simb::MCTruth, simb::GTruth> >();
  if ( fAddMCFlux ) {
    produces< std::vector<simb::MCFlux>  >();
    // Associate every truth with the flux it came from
    produces< art::Assns<simb::MCTruth, simb::MCFlux> >();
  }

  //produces< sumdata::SpillData >();
  //produces< sumdata::POTSum, art::InSubRun  >();
  //produces< sumdata::RunData, art::InRun    >();

  std::string outFileList = "adding file pattern: ";
  for (size_t i=0; i < fFileList.size(); ++i) {
    outFileList += "\n";
    outFileList += fFileList[i];
    fGTreeChain->Add(fFileList[0].c_str());
  }
  mf::LogInfo("AddGenieEventsToArt") << outFileList;

  fNumMCRec = fGTreeChain->GetEntries();
  fLastUsedMCRec = fNumMCRec;

  // attach flux branches ...
  /**
     gtree->GetBranch("flux")->GetClassName()
     (const char* 0x31f41c0)"genie::flux::GNuMIFluxPassThroughInfo"

     gtree->GetBranch("simple")->GetClassName()
     (const char* 0x2a2a8e0)"genie::flux::GSimpleNtpEntry"
     gtree->GetBranch("numi")->GetClassName()
     (const char* 0x2a33d40)"genie::flux::GSimpleNtpNuMI
     gtree->GetBranch("aux")->GetClassName()
     (const char* 0x2a35320)"genie::flux::GSimpleNtpAux"

     gtree->GetBranch("dk2nu")->GetClassName()
     (const char* 0x3457d59)"bsim::Dk2Nu"
     gtree->GetBranch("nuchoice")->GetClassName()
     (const char* 0x3479329)"bsim::NuChoice"
  **/
  TObjArray* blist = fGTreeChain->GetListOfBranches();
  TIter    next(blist);
  TObject* obj;
  while ( ( obj = next() ) ) {
    std::string bname = obj->GetName();
    //  should be a list of TBranchElement or TBranchObject items
    //  TBranchObject are ancient ... should have been replaced by Elements
    const TBranchElement* belement = dynamic_cast<const TBranchElement*>(obj);
    const TBranchObject*  bobject  = dynamic_cast<const TBranchObject*>(obj);
    if ( ! belement && ! bobject ) {
      std::string reallyIsA = obj->ClassName();
      mf::LogError("AddGenieEventsToArt")
        << "### supposed branch element '" << bname
        << "' wasn't a TBranchElement/TBranchObject but instead a "
        << reallyIsA << std::endl;
      if ( bname == "gmcrec" ) {
        mf::LogError("AddGenieEventsToArt")
          << "### since this is '" << bname
          << "' this is likely to end very badly badly" << std::endl;
      }
      continue;
    }
    std::string bclass = (belement) ? belement->GetClassName()
                                    : bobject->GetClassName();
    if ( bclass == "genie::NtpMCEventRecord" ) {
      fGTreeChain->SetBranchAddress(bname.c_str(),&fMCRec);
    } else if ( bclass == "genie::flux::GNuMIFluxPassThroughInfo" ) {
      fGNuMIFluxPassThroughInfo = new genie::flux::GNuMIFluxPassThroughInfo;
      fGTreeChain->SetBranchAddress(bname.c_str(),&fGNuMIFluxPassThroughInfo);
    } else if ( bclass == "genie::flux::GSimpleNtpEntry" ) {
      fGSimpleNtpEntry = new genie::flux::GSimpleNtpEntry;
      fGTreeChain->SetBranchAddress(bname.c_str(),&fGSimpleNtpEntry);
    } else if ( bclass == "genie::flux::GSimpleNtpNuMI" ) {
      fGSimpleNtpNuMI = new genie::flux::GSimpleNtpNuMI;
      fGTreeChain->SetBranchAddress(bname.c_str(),&fGSimpleNtpNuMI);
    } else if ( bclass == "genie::flux::GSimpleNtpAux" ) {
      fGSimpleNtpAux = new genie::flux::GSimpleNtpAux;
      fGTreeChain->SetBranchAddress(bname.c_str(),&fGSimpleNtpAux);
    } else if ( bclass == "bsim::Dk2Nu" ) {
      fDk2Nu = new bsim::Dk2Nu;
      fGTreeChain->SetBranchAddress(bname.c_str(),&fDk2Nu);
    } else if ( bclass == "bsim::NuChoice" ) {
      fNuChoice = new bsim::NuChoice;
      fGTreeChain->SetBranchAddress(bname.c_str(),&fNuChoice);
    } else {
      mf::LogError("AddGenieEventsToArt")
        << "### branch element '" << bname
        << "' was unhandled '" << bclass << "' class" << std::endl;
    }
  } // while ( next )

  mf::LogInfo("AddGenieEventsToArt")
    << "chain has " << fNumMCRec << " entries"
    << std::endl;

  // setup to write out file, if requested
  if ( fOutputPrintLevel > 0 ) {
    if ( fOutputDumpFileName == ""          ||
         fOutputDumpFileName == "--"        ||
         fOutputDumpFileName == "cout"      ||
         fOutputDumpFileName == "std::cout"     ) {
      // standardize so we don't check all these again
      fOutputDumpFileName = "std::cout";
      fOutputStream       = &(std::cout);
    } else {
      size_t posl = fOutputDumpFileName.find("%l");
      if ( posl != std::string::npos ) {
        fOutputDumpFileName.replace(posl,2,fMyModuleLabel);
      }
      mf::LogInfo("AddGenieEventToArt")
        << "#### AddGenieEventsToArt::ctor open "
        << fOutputDumpFileName
        << std::endl << std::flush;
      fOutputStream =
        new std::ofstream(fOutputDumpFileName.c_str(),
                          std::ios_base::trunc|std::ios_base::out);
    }
  } // if ( fOutputPrintLevel > 0 )

}

evg::AddGenieEventsToArt::~AddGenieEventsToArt()
{
  // release resources
  if ( fGTreeChain ) delete fGTreeChain;
  if ( fOutputStream && ( fOutputDumpFileName != "std::cout" ) ) {
    std::ofstream* ofs = dynamic_cast<std::ofstream*>(fOutputStream);
    if ( ofs ) {
      mf::LogInfo("AddGenieEventToArt")
        << "#### AddGenieEventsToArt::dtor close "
        << fOutputDumpFileName
        << std::endl << std::flush;
      ofs->flush();
      ofs->close();
    }
    delete fOutputStream;
  }
}

void evg::AddGenieEventsToArt::produce(art::Event & evt)
{

  //std::cerr << "AddGenieEventsToArt::produce start" << std::endl << std::flush;

  std::unique_ptr< std::vector<simb::MCTruth> >
    mctruthcol(new std::vector<simb::MCTruth>);

  std::unique_ptr< std::vector<simb::GTruth> >
     gtruthcol(new std::vector<simb::GTruth>);

  std::unique_ptr< std::vector<simb::MCFlux> >
     mcfluxcol(new std::vector<simb::MCFlux>);

  std::unique_ptr< art::Assns<simb::MCTruth, simb::MCFlux> >
        tfassn(new art::Assns<simb::MCTruth, simb::MCFlux>);
  std::unique_ptr< art::Assns<simb::MCTruth, simb::GTruth> >
        tgassn(new art::Assns<simb::MCTruth, simb::GTruth>);

  // number of interactions to add to _this_ record/"event"
  size_t n = GetNumToAdd();
  //std::cerr << "#### AddGenieEventsToArt::produce " << n
  //          << " MCTruth objects" << std::endl << std::flush;

  // make a list of entries in TChain to use for this overlay
  // same entry should never be in the list twice ...
  std::vector<size_t> entries;

  art::ServiceHandle<art::RandomNumberGenerator> rng;
  CLHEP::HepRandomEngine& engine = rng->getEngine();
  CLHEP::RandFlat flat(engine);

  //mf::LogInfo("AddGeniEventsToArt") << "attempt to get " << n << " entries";
  while ( entries.size() != n ) {
    if ( ! fRandomEntries ) {
      // going through file sequentially
      ++fLastUsedMCRec;
      if ( fLastUsedMCRec >= fNumMCRec ) fLastUsedMCRec = 0;
      entries.push_back(fLastUsedMCRec);
      // mf::LogInfo("AddGeniEventsToArt") << "adding " << fLastUsedMCRec;
    } else {
      size_t indx = flat.fireInt(fNumMCRec);
      // ensure it isn't already there ..
      if ( std::find(entries.begin(),entries.end(),indx) != entries.end() ) {
        // mf::LogInfo("AddGeniEventsToArt") << "rejecting "
        //                                   << indx << " as already there";
      } else {
        entries.push_back(indx);
        // mf::LogInfo("AddGeniEventsToArt") << "adding " << indx;
      }
    }
  }
  //mf::LogInfo("AddGeniEventsToArt") << "entries.size " << entries.size();

  for (size_t i=0; i<n; ++i) {
    simb::MCTruth mctruth;
    simb::GTruth  gtruth;
    simb::MCFlux  mcflux;

    size_t ientry = entries[i];
    // fetch a single entry from GENIE input file
    fGTreeChain->GetEntry(ientry);
    genie::EventRecord* grec = fMCRec->event;

    /*
    mf::LogInfo("AddGeniEventsToArt")
      << "#### AddGenieEventsToArt::produce " << i+1 << " of " << n
      << " using entry " << ientry
      << std::endl;
    */

    if ( fOutputStream ) {
      // alas not currently able to get current setting
      // int plevel = genie::GHepRecord::GetPrintLevel(); //
      genie::GHepRecord::SetPrintLevel(fOutputPrintLevel); //
      //std::cerr << "####  AddGenieEventsToArt::produce() writing to "
      //          << fOutputDumpFileName
      //          << std::endl << std::flush;
      *fOutputStream << *fMCRec;
      fOutputStream->flush();
    }

    // generate offset in time
    double evtTimeOffset = fGlobalTimeOffset + fTimeShifter->TimeOffset();

    // offset vertex position
    double xoff = flat.fire(fXlo,fXhi);
    double yoff = flat.fire(fYlo,fYhi);
    double zoff = flat.fire(fZlo,fZhi);

    TLorentzVector vtxOffset(xoff,yoff,zoff,evtTimeOffset);

    // convert to simb:: ART objects using GENIE2ART functions
    evgb::FillMCTruth(grec,vtxOffset,mctruth);
    evgb::FillGTruth(grec,gtruth);

    if (fAddMCFlux) {
      if ( fGNuMIFluxPassThroughInfo ) {
        double dk2gen = -99999.;
        evgb::FillMCFlux(fGNuMIFluxPassThroughInfo,dk2gen,mcflux);
      } else if ( fGSimpleNtpEntry ) {
        // cheat at meta data for now ...
        // should be pulling this from input file ... no sure that
        // it is being copied correctly
        // (TChain isn't made of GSimpleNtpMeta objs ... )
        // we need to know how to interpret the Aux variablees
        static genie::flux::GSimpleNtpMeta* meta = 0;
        if ( ! meta ) {
          meta = new genie::flux::GSimpleNtpMeta;
          // hopefully this is the layout
          // aux ints:     tgen
          // aux doubles:  fgXYWgt nimpwt muparpx muparpy muparpz mupare necm
          meta->auxintname.push_back("tgen");
          meta->auxdblname.push_back("fgXYWgt");
          meta->auxdblname.push_back("nimpwt");
          meta->auxdblname.push_back("muparpx");
          meta->auxdblname.push_back("muparpy");
          meta->auxdblname.push_back("muparpz");
          meta->auxdblname.push_back("mupare");
          meta->auxdblname.push_back("necm");
        }
        evgb::FillMCFlux(fGSimpleNtpEntry,fGSimpleNtpNuMI,
                         fGSimpleNtpAux,meta,mcflux);
      } else if ( fDk2Nu ) {
        evgb::FillMCFlux(fDk2Nu,fNuChoice,mcflux);
      }
    }

    /*
    genie::NtpMCRecHeader rec_header = fMCRec->hdr;
    //LOG("gevdump", pNOTICE)
    std::cout
      << " ** Event: " << rec_header.ievent // implicit newline in print
      << *grec;
    // add to our collections
    */

    mctruthcol->push_back(mctruth);
    gtruthcol->push_back(gtruth);
    if (fAddMCFlux) {
      // deterimine if there is something to fetch
      mcfluxcol->push_back(mcflux);
    }

    // LArSoft #include "lardata/Utilities/AssociationUtil.h"
    // NOVA    #include "Utilities/AssociationUtil.h"
    // these util::CreateAssn are taken from LArSoft's GENIEGen_module
    //   ~/Work/DUNE/code/larsim/larsim/EventGenerator/GENIE/GENIEGen_module.cc
    // this seems to be MARK CreateAssn_07 ??  but that's one-to-many
    //    implicit in this is a indx=UNIT_MAX arg so this is only acting
    //    on the last element of truthcol

    evgb::util::CreateAssn(*this, evt, *mctruthcol, *gtruthcol, *tgassn,
                           gtruthcol->size()-1, gtruthcol->size());

    if (fAddMCFlux) {
      evgb::util::CreateAssn(*this, evt, *mctruthcol, *mcfluxcol, *tfassn,
                             mcfluxcol->size()-1, mcfluxcol->size());
    }
    //

  } // done collecting input

  //std::cerr << "AddGenieEventsToArt::produce put into event"
  //          << std::endl << std::flush;

  // put the collections in the event
  evt.put(std::move(mctruthcol));
  evt.put(std::move(gtruthcol));
  evt.put(std::move(tgassn));
  if ( fAddMCFlux ) {
    evt.put(std::move(mcfluxcol));
    evt.put(std::move(tfassn));
  }

}

//-------------------------------------------------------------------------
void evg::AddGenieEventsToArt::ParseCountConfig()
{
  // Parse countConfig to get parameters on how many to add
  // Should be one of these:
  //    "fixed:"     <N>
  //    "flat:       <Nmin> <Nmax>"
  //    "poisson:    <Nmean>"
  //    "poisson-1:  <Nmean>"
  //    "gauss:      <mean> <rms>"

  std::string str = fParams().countConfig();

  // let user use whatever case they like
  std::transform(str.begin(),str.end(),str.begin(),::tolower);
  // trim any leading whitespace
  if( str.find_first_not_of(" \t\n") != 0)
      str.erase( 0, str.find_first_not_of(" \t\n")  );

  size_t i = str.find_first_of(" \t\n");
  std::string distName = str.substr(0,i);
  str.erase(0,i);

  // now 'str' should have 1 or 2 numerical values

  int nf = sscanf(str.c_str(),"%lf %lf",&fRndP1,&fRndP2);

  if ( nf == 0 ) {
    mf::LogError("AddGenieEventsToArt")
      << "ParseCountConfig " << str
      << " had " << nf << " args, expected something '"
      << str << "'"<< std::endl;
    throw cet::exception("badDist countConfig")
      << __FILE__ << ":" << __LINE__
      << " badDist '" << str << "'";
  }

  if ( distName.find("fix") == 0 || distName == "n" || distName == "n:" ) {
    fRndDist = kFixed;
    if ( nf != 1 ) {
      mf::LogError("AddGenieEventsToArt")
        << "ParseCountConfig " << distName
        << " had 2 args, expected 1: '"
        << str << "', ignoring 2nd" << std::endl;
    }
  }
  else if ( distName.find("flat") == 0 ) {
    fRndDist = kFlat;
    if ( nf == 1 ) fRndP2 = fRndP1;
    // make sure they're ordered
    if ( fRndP2 < fRndP1 ) std::swap(fRndP1,fRndP2);
  }
  else if ( distName.find("poiss") == 0 ) {
    fRndDist = kPoisson;
    if ( distName.find("-1") != std::string::npos ) fRndDist = kPoissonMinus1;
    if ( nf != 1 ) {
      mf::LogError("AddGenieEventsToArt")
        << "ParseCountConfig " << distName
        << " had 2 args, expected 1: '"
        << str << "', ignoring 2nd" << std::endl;
    }
  }
  else if ( distName.find("gaus") == 0 ) {
    fRndDist = kGaussian;
    if ( nf != 2 ) {
      mf::LogError("AddGenieEventsToArt")
        << "ParseCountConfig " << distName
        << " had " << nf << " args, expected 2: '"
        << str << "'"<< std::endl;
      throw cet::exception("badDist countConfig")
        << __FILE__ << ":" << __LINE__
        << " badDist '" << distName << "'";
    }
  } else {
      mf::LogError("AddGenieEventsToArt")
        << "ParseCountConfig unknown distName " << distName
        << " had' " << nf << " args '"
        << str << "'"<< std::endl;
    throw cet::exception("unknownDist countConfig")
      << __FILE__ << ":" << __LINE__
      << " unknown '" << distName << "'";
  }

  mf::LogInfo("AddGenieEventsToArt")
    << "ParseCountConfig label='" << fMyModuleLabel << "'"
    << " distName='" << distName  << "' (int)"
    << (int)fRndDist << "; cfgstr '" << str << "' --> "
    << " p1 " << fRndP1 << " p2 " << fRndP2 << " nf " << nf
    << std::endl;

#if 0
  // test how fireInt() works ...
  static bool first = true;
  if ( first ) {
    art::ServiceHandle<art::RandomNumberGenerator> rng;
    CLHEP::HepRandomEngine& engine = rng->getEngine();

    // can't do servicehandle in ctor (or ctor callees)
    CLHEP::RandFlat flatTest(engine);   // pass by ref, doesn't own
    first = false;
    for (int rtest = 0; rtest < 5; ++rtest ) {
      std::cout << " ======= testing CLHEP::RandFlat::fireInt("
                << rtest << ") =======" << std::endl;
      for (int i=0; i<100; ++i)
        std::cout << "  " << flatTest.fireInt(rtest);
      std::cout << std::endl;
    }
  }
#endif
}

size_t evg::AddGenieEventsToArt::GetNumToAdd() const
{

  art::ServiceHandle<art::RandomNumberGenerator> rng;
  CLHEP::HepRandomEngine& engine = rng->getEngine();

  size_t nchosen = 0;

  switch ( fRndDist ) {
  case kFixed:
    {
      nchosen = fRndP1; // nothing random about it
    }
  case kFlat:
    {
      CLHEP::RandFlat flat(engine);   // pass by ref, doesn't own
      // couldn't find good documentation on how this worked ... empirically
      //   fireInt(0) & fireInt(1) give 0 always
      //   fireInt(2) gives 0 or 1
      //   fireInt(3) gives 0, 1, 2
      //   fireInt(4) gives 0, 1, 2, 3
      // ...
      // so for p1=5, p2=7 we want 5 + [0:2] i.e 5+0=5, 5+1=6, 5+2=7
      //    and thus range should be 3
      int range = (int)(fRndP2-fRndP1) + 1;
      nchosen = fRndP1 + flat.fireInt(range);
    }
    break;
  case kPoisson:
  case kPoissonMinus1:
    {
      CLHEP::RandPoisson poisson(engine);
      nchosen = poisson.fire(fRndP1);
      if ( fRndDist == kPoissonMinus1 ) {
        if ( nchosen > 0 ) --nchosen;
        else {
          mf::LogError("AddGenieEventsToArt")
            << "fRndDist[type=" << (int)fRndDist
            << "] '" << fParams().timeConfig() << "' "
            << " nchosen " << nchosen
            << " can't subtract 1 for kPoissonMinus1"
            << std::endl;
        }
      }
    }
    break;
  case kGaussian:
    {
      CLHEP::RandGauss gauss(engine);
      double tmp = gauss.fire(fRndP1,fRndP2);
      if ( tmp > 0 ) nchosen = (size_t)(tmp);
      else {
        nchosen = 0;
          mf::LogError("AddGenieEventsToArt")
            << "fRndDist[type=" << (int)fRndDist
            << "] '" << fParams().timeConfig() << "' "
            << " tmp " << tmp
            << "; can't return < 0 for kGaussian, return 0"
            << std::endl;
      }
    }
    break;
  default:
    {
      nchosen = 0;
      mf::LogError("AddGenieEventsToArt")
        << "fRndDist[type=" << (int)fRndDist
        << "] '" << fParams().timeConfig() << "' not handled"
        << std::endl;
    }
  }

  /*
  mf::LogInfo("AddGenieEventsToArt")
    << "fRndDist[type=" << (int)fRndDist
    << "] '" << fParams().timeConfig() << "' "
    << " nchosen " << nchosen << std::endl;
  */

  return nchosen;
}

//-------------------------------------------------------------------------
void evg::AddGenieEventsToArt::ParseTimeConfig()
{
  std::string timeConfig = fParams().timeConfig();
  // trim any leading whitespace
  if( timeConfig.find_first_not_of(" \t\n") != 0)
      timeConfig.erase( 0, timeConfig.find_first_not_of(" \t\n")  );

  size_t i = timeConfig.find_first_of(": \t\n");
  std::string timeName   = timeConfig.substr(0,i);
  timeConfig.erase(0,i);

  mf::LogInfo("AddGenieEventsToArt")
    << "ParseTimeConfig label='" << fMyModuleLabel << "'"
    << " name='" << timeName << "'"
    << " cfg='" << timeConfig << "'" << std::endl;
  if ( timeName == "none" ) timeName = "evgb::EvtTimeNone";
  if ( timeName == "flat" ) timeName = "evgb::EvtTimeFlat";
  if ( timeName == "numi" || timeName == "NuMI"||
       timeName == "fnal" || timeName == "FNAL"   )
    timeName = "evgb::EvtTimeFNALBeam";

  evgb::EvtTimeShiftFactory& timeFactory =
    evgb::EvtTimeShiftFactory::Instance();
  fTimeShifter = timeFactory.GetEvtTimeShift(timeName,timeConfig);

  if ( ! fTimeShifter ) {
    timeFactory.Print();
    throw cet::exception("BAD TimeShifter")
      << __FILE__ << ":" << __LINE__
      << " unknown '" << timeName << "'";
  }
}

//-------------------------------------------------------------------------
void evg::AddGenieEventsToArt::ParseVtxOffsetConfig()
{
  fXlo = fParams().vtxOffsets().xlo();
  fYlo = fParams().vtxOffsets().ylo();
  fZlo = fParams().vtxOffsets().zlo();
  fXhi = fParams().vtxOffsets().xhi();
  fYhi = fParams().vtxOffsets().yhi();
  fZhi = fParams().vtxOffsets().zhi();

  if ( fXlo != 0 && fYlo != 0 && fZlo != 0 &&
       fXhi != 0 && fYhi != 0 && fZhi != 0    ) {
    mf::LogInfo("AddGenieEventsToArt")
      << "ParseVtxOffsetConfig label='" << fMyModuleLabel << "' \n"
      << " x [" << std::setw(11) << fXlo << " "
      <<           std::setw(11) << fXhi << " ]\n"
      << " y [" << std::setw(11) << fYlo << " "
      <<           std::setw(11) << fYhi << " ]\n"
      << " z [" << std::setw(11) << fZlo << " "
      <<           std::setw(11) << fZhi << " ]";
  }

}
//-------------------------------------------------------------------------

/*
void evg::AddGenieEventsToArt::reconfigure(const Parameters & params)
{
  fParams = params;
}
*/

/*
void evg::AddGenieEventsToArt::beginJob()
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::beginRun(art::Run & r)
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::beginSubRun(art::SubRun & sr)
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::endJob()
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::endRun(art::Run & r)
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::endSubRun(art::SubRun & sr)
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::reconfigure(fhicl::ParameterSet const & p)
{
  // Implementation of optional member function here.
  std::cerr << "evg::AddGenieEventsToArt::reconfigure() not implemented"
            << std::endl;
  abort();
}

void evg::AddGenieEventsToArt::respondToCloseInputFile(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::respondToCloseOutputFiles(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::respondToOpenInputFile(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void evg::AddGenieEventsToArt::respondToOpenOutputFiles(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}
*/

DEFINE_ART_MODULE(evg::AddGenieEventsToArt)
