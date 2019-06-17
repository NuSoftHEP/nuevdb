////////////////////////////////////////////////////////////////////////
/// \file  MCTruthAndFriendsItr.cxx
/// \brief allow easy iteration over MCTruth and associated GTruth/MCFlux
///
/// \version $Id: $
/// \author  rhatcher@fnal.gov
////////////////////////////////////////////////////////////////////////
#ifndef EVGB_MCTRUTHANDFRIENDSITR_H
#define EVGB_MCTRUTHANDFRIENDSITR_H

#include <vector>
#include <utility>   // for pair<>
#include <set>

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "nusimdata/SimulationBase/GTruth.h"

#include "dk2nu/tree/dk2nu.h"
#include "dk2nu/tree/NuChoice.h"

namespace evgb {

  class MCTruthAndFriendsItr {

  public:
    MCTruthAndFriendsItr(art::Event const & evtIn,
                         std::vector<std::string> const & labels);

    virtual ~MCTruthAndFriendsItr() { ; }

    bool Next();   // move to next
    const simb::MCTruth*   GetMCTruth()  const { return thisMCTruth; }
    const simb::GTruth*    GetGTruth()   const { return thisGTruth;  }
    const simb::MCFlux*    GetMCFlux()   const { return thisMCFlux;  }
    const bsim::Dk2Nu*     GetDk2Nu()    const { return thisDk2Nu;   }
    const bsim::NuChoice*  GetNuChoice() const { return thisNuChoice; }

    std::string            GetLabel()   const { return thisLabel;   } 
    // return associated label???

  private:

    art::Event const &                evt;
    std::vector<std::string> const &  fInputModuleLabels;

    std::vector< art::Handle< std::vector<simb::MCTruth> > > mclists;

    std::set<std::pair<int,int> >                 indices;
    std::set<std::pair<int,int> >::const_iterator indx_itr;
    std::vector<std::string>                      outlabels;

    int                               nmctruth;
    int                               imctruth;

    const simb::MCTruth*              thisMCTruth;
    const simb::GTruth*               thisGTruth;
    const simb::MCFlux*               thisMCFlux;
    const bsim::Dk2Nu*                thisDk2Nu;
    const bsim::NuChoice*             thisNuChoice;
    std::string                       thisLabel;

  }; // end-of-class MCTruthAndFriendsItr

} // end-of-namespace evgb

#endif // EVGB_MCTRUTHANDFRIENDSITR_H
