////////////////////////////////////////////////////////////////////////
/// \file  NuReweight.h
/// \brief Wrapper for reweightings neutrino interactions within the ART framework
///
/// \author  nathan.mayer@tufts.edu
////////////////////////////////////////////////////////////////////////

#include "nutools/NuReweight/GENIEReweight.h"

namespace simb  { class MCTruth;      }
namespace simb  { class GTruth;       }

namespace rwgt{

  class NuReweight : public GENIEReweight {

  public:
    NuReweight();
    ~NuReweight();
    
    double CalcWeight(const simb::MCTruth & truth, const simb::GTruth & gtruth) const;
    
  private:
    genie::EventRecord RetrieveGHEP(const simb::MCTruth & truth, const simb::GTruth & gtruth) const;
    
  };


}
