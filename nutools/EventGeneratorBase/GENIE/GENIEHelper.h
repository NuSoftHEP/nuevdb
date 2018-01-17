////////////////////////////////////////////////////////////////////////
/// \file  GENIEHelper.h
/// \brief Wrapper for generating neutrino interactions with GENIE
///
/// \version $Id: GENIEHelper.h,v 1.25 2012-09-07 21:35:26 brebel Exp $
/// \author  brebel@fnal.gov rhatcher@fnal.gov
////////////////////////////////////////////////////////////////////////
#ifndef EVGB_GENIEHELPER_H
#define EVGB_GENIEHELPER_H

#include <vector>
#include <set>

#include "TGeoManager.h"

#include "EVGDrivers/GFluxI.h"
#include "EVGDrivers/GeomAnalyzerI.h"
#include "EVGDrivers/GMCJDriver.h"

class TH1D;
class TH2D;
class TF1;
class TRandom3;

///parameter set interface
namespace fhicl {
  class ParameterSet;
}

/// IFDH interface (data handling) ... if using bare interface
namespace ifdh_ns {
  class ifdh;
}

namespace simb {
  class MCTruth;
  class MCFlux;
  class GTruth;
}

///GENIE neutrino interaction simulation
namespace genie { class EventRecord; }

namespace evgb {

  class EvtTimeShiftI;   // for shifting time within a spill

  class GENIEHelper {

  public:

    explicit GENIEHelper(fhicl::ParameterSet const& pset,
                         TGeoManager*               rootGeom,
                         std::string         const& rootFile,
                         double              const& detectorMass);
    ~GENIEHelper();

    void                   Initialize();
    bool                   Stop();
    bool                   Sample(simb::MCTruth &truth,
                                  simb::MCFlux  &flux,
                                  simb::GTruth  &gtruth);

    double                 TotalHistFlux();
    double                 TotalExposure()    const { return fTotalExposure;  }

    // Call the following method before calling Stop, otherwise fSpillExposure
    // will be reset to 0
    double                 SpillExposure()    const { return fSpillExposure;  }
    std::string            FluxType()         const { return fFluxType;       }
    std::string            DetectorLocation() const { return fDetLocation;    }

    // methods for checking the various algorithms in GENIEHelper - please
    // do not use these in your code!!!!!
    std::vector<TH1D*>     FluxHistograms()   const { return fFluxHistograms; }
    double                 TotalMass()        const { return fDetectorMass+fSurroundingMass; }

    genie::EventRecord *  GetGenieEventRecord() { return fGenieEventRecord; }

    // access the random number generator that is supplying additional values for helper
    TRandom3*             GetHelperRandom() { return fHelperRandom; }

    // direct access to flux driver ... no ownership handover
    // base is the "real" flux driver, might be wrapped by a flavor mixer
    genie::GFluxI*        GetFluxDriver(bool base = true )
      { return ( (base) ? fFluxD : fFluxD2GMCJD ); }

  private:

    void InitializeGeometry();
    void InitializeFiducialSelection();
    void InitializeRockBoxSelection();
    void InitializeFluxDriver();
    void ConfigGeomScan();
    void SetMaxPathOutInfo();
    void PackNuMIFlux(simb::MCFlux &flux);
    void PackSimpleFlux(simb::MCFlux &flux);
    void PackMCTruth(genie::EventRecord *record, simb::MCTruth &truth);
    void PackGTruth(genie::EventRecord *record, simb::GTruth &truth);

    void ExpandFluxPaths();
    void ExpandFluxFilePatternsDirect();
    void ExpandFluxFilePatternsIFDH();
    bool StringToBool(std::string v);

    void SetGXMLPATH();
    void SetGMSGLAYOUT();
    void StartGENIEMessenger(std::string prodmode);
    void FindEventGeneratorList();
    void ReadXSecTable();

    TGeoManager*             fGeoManager;        ///< pointer to ROOT TGeoManager
    std::string              fGeoFile;           ///< name of file containing the Geometry description

    genie::EventRecord*      fGenieEventRecord;  ///< last generated event
    genie::GeomAnalyzerI*    fGeomD;
    genie::GFluxI*           fFluxD;             ///< real flux driver
    genie::GFluxI*           fFluxD2GMCJD;       ///< flux driver passed to genie GMCJDriver, might be GFluxBlender
    genie::GMCJDriver*       fDriver;

    // for now leave this here ... but not necessary when using IFDH_service
    ifdh_ns::ifdh*           fIFDH;              ///< (optional) flux file handling

    TRandom3*                fHelperRandom;      ///< random # generator for GENIEHelper
    bool                     fUseHelperRndGen4GENIE;   ///< use fHelperRandom for gRandom during Sample()
    evgb::EvtTimeShiftI*     fTimeShifter;       ///< generator for time offset within a spill

    std::string              fFluxType;          ///< histogram or ntuple or atmo_FLUKA or atmo_BARTOL
    std::string              fFluxSearchPaths;   ///< colon separated set of path stems
    std::vector<std::string> fFluxFilePatterns;  ///< wildcard patterns files containing histograms or ntuples, or txt
    std::vector<std::string> fSelectedFluxFiles; ///< flux files selected after wildcard expansion and subset selection
    int                      fMaxFluxFileMB;     ///< maximum size of flux files (MB)
    int                      fMaxFluxFileNumber; ///< maximum # of flux files
    std::string              fFluxCopyMethod;    ///< "DIRECT" = old direct access method, otherwise = ifdh approach schema ("" okay)
    std::string              fFluxCleanup;       ///< "ALWAYS", "/var/tmp", "NEVER"
    std::string              fBeamName;          ///< name of the beam we are simulating
    std::string              fTopVolume;         ///< top volume in the ROOT geometry in which to generate events
    std::string              fWorldVolume;       ///< name of the world volume in the ROOT geometry
    std::string              fDetLocation;       ///< name of flux window location
    std::vector<TH1D *>      fFluxHistograms;    ///< histograms for each nu species

    double                   fFluxUpstreamZ;     ///< z where flux starts from (if non-default, simple/ntuple only)
    double                   fEventsPerSpill;    ///< number of events to generate in each spill if not using POT/spill.
                                                 ///< If using Atmo, set to 1
    double                   fPOTPerSpill;       ///< number of pot per spill
    double                   fHistEventsPerSpill;///< number of events per spill for histogram fluxes - changes each spill
    int                      fSpillEvents;       ///< total events for this spill
    double                   fSpillExposure;     ///< total exposure (i.e. pot) for this spill
    double                   fTotalExposure;     ///< pot used from flux ntuple
    double                   fMonoEnergy;        ///< energy of monoenergetic neutrinos
    std::string              fFunctionalFlux;
    int                      fFunctionalBinning;
    double                   fEmin;
    double                   fEmax;
    double                   fXSecMassPOT;       ///< product of cross section, mass and POT/spill for histogram fluxes
    double                   fTotalHistFlux;     ///< total flux of neutrinos from flux histograms for used flavors
    TVector3                 fBeamDirection;     ///< direction of the beam for histogram fluxes
    TVector3                 fBeamCenter;        ///< center of beam for histogram fluxes - must be in meters
    double                   fBeamRadius;        ///< radius of cylindar for histogram fluxes - must be in meters
    double                   fDetectorMass;      ///< mass of the detector in kg
    double                   fSurroundingMass;   ///< mass of material surrounding the detector that is intercepted by
                                                 ///< the cylinder for the histogram flux in kg
    double                   fGlobalTimeOffset;  ///< overall time shift (ns) added to every particle time
    double                   fRandomTimeOffset;  ///< additional random time shift (ns) added to every particle time
    std::string              fSpillTimeConfig;   ///< alternative to flat spill distribution
    std::vector<int>         fGenFlavors;        ///< pdg codes for flavors to generate
    double                   fAtmoEmin;          ///< atmo: Minimum energy of neutrinos in GeV
    double                   fAtmoEmax;          ///< atmo: Maximum energy of neutrinos in GeV
    double                   fAtmoRl;            ///< atmo: radius of the sphere on where the neutrinos are generated
    double                   fAtmoRt;            ///< atmo: radius of the transvere (perpendicular) area on the sphere
                                                 ///< where the neutrinos are generated
    std::vector<std::string> fEnvironment;       ///< environmental variables and settings used by genie
    std::string              fXSecTable;         ///< cross section file (was $GSPLOAD)
    std::string              fEventGeneratorList;///< control over event topologies, was $GEVGL [Default]
    std::string              fGXMLPATH;          ///< locations for GENIE XML files
    std::string              fGMSGLAYOUT;        ///< format for GENIE log message [BASIC]|SIMPLE (SIMPLE=no timestamps)
    std::string              fGENIEMsgThresholds;///< additional XML file setting Messager level thresholds (":" separated)
    int                      fGHepPrintLevel;    ///< GHepRecord::SetPrintLevel(), -1=no-print
    std::string              fMixerConfig;       ///< configuration string for genie GFlavorMixerI
    double                   fMixerBaseline;     ///< baseline distance if genie flux can't calculate it
    std::string              fFiducialCut;       ///< configuration for geometry selector
    std::string              fGeomScan;          ///< configuration for geometry scan to determine max pathlengths
    std::string              fMaxPathOutInfo;    ///< output info if writing PathLengthList from GeomScan
    unsigned int             fDebugFlags;        ///< set bits to enable debug info
  };
}
#endif //EVGB_GENIEHELPER_H
