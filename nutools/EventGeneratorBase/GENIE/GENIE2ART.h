////////////////////////////////////////////////////////////////////////
/// \file  GENIE2ART.h
/// \brief Functions for transforming GENIE objects into ART objects (and back)
///
/// \version $Id: GENIE2ART.h,v 1.0 2016-04-20 18:42:01 rhatcher Exp $
/// \author  rhatcher@fnal.gov
///   Parts taken from GENIEHelper & NuReweight classes
///
////////////////////////////////////////////////////////////////////////
#ifndef EVGB_GENIE2ART_H
#define EVGB_GENIE2ART_H

/// GENIE neutrino interaction simulation objects
namespace genie { 
  class EventRecord; 
  class GFluxI;
  namespace flux {
    class GNuMIFlux;
    class GNuMIFluxPassThroughInfo;
    class GSimpleNtpFlux;
    class GSimpleNtpEntry;
    class GSimpleNtpNuMI;
    class GSimpleNtpAux;
    class GSimpleNtpMeta;
    class GDk2NuFlux;
  }
}
namespace bsim {
  class Dk2Nu;
  class NuChoice;
  class DkMeta;
}


/// ART objects
namespace simb { 
  class MCTruth;
  class GTruth;
  class MCFlux;
}

namespace evgb {

  // adapted from GENIEHelper
  void FillMCTruth(const genie::EventRecord* grec,
                   double spillTime, 
                   simb::MCTruth& mctruth);
  void FillGTruth(const genie::EventRecord* grec,
                  simb::GTruth& gtruth);

  /// return genie::EventRecord pointer; callee takes possession 
  // adapted from NuReweight
  // 
  genie::EventRecord* RetrieveGHEP(const simb::MCTruth& truth,
                                   const simb::GTruth&  gtruth,
                                   bool useFirstTrajPosition = true);

  void FillMCFlux(genie::GFluxI* fdriver, simb::MCFlux& mcflux);

  void FillMCFlux(genie::flux::GNuMIFlux* gnumi, 
                  simb::MCFlux& mcflux);
  void FillMCFlux(const genie::flux::GNuMIFluxPassThroughInfo* nflux, 
                  double dk2gen,
                  simb::MCFlux& flux);

  void FillMCFlux(genie::flux::GSimpleNtpFlux* gsimple,
                  simb::MCFlux& mcflux);
  void FillMCFlux(const genie::flux::GSimpleNtpEntry* nflux_entry,
                  const genie::flux::GSimpleNtpNuMI*  nflux_numi,
                  const genie::flux::GSimpleNtpAux*   nflux_aux,
                  const genie::flux::GSimpleNtpMeta*  nflux_meta,
                  simb::MCFlux& flux);

  void FillMCFlux(genie::flux::GDk2NuFlux* gdk2nu,
                  simb::MCFlux& mcflux);
  void FillMCFlux(const bsim::Dk2Nu*    dk2nu,
                  const bsim::NuChoice* nuchoice,
                  simb::MCFlux& flux);

} // end-of-namespace evgb

#endif  // EVGB_GENIE2ART_H
