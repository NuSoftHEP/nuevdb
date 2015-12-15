////////////////////////////////////////////////////////////////////////
/// \file  GTruth.h
/// \Class to hold the additional information needed to recreate a genie::EventRecord
/// \author  nathan.mayer@tufts.edu
////////////////////////////////////////////////////////////////////////

/// This class stores/retrieves the additional information needed (and not in MCTruth) to recreate a genie::EventRecord
/// for genie based event reweighting.

#include "SimulationBase/GTruth.h"

#include <vector>
#include <iostream>
#include <string>

namespace simb {

  //---------------------------------------------------------------
  GTruth::GTruth() 
    : fGint(-1)
    , fGscatter(-1)
    , fweight(0)
    , fprobability(0)
    , fXsec(0)
    , fDiffXsec(0)
    , fNumPiPlus(-1)
    , fNumPiMinus(-1)
    , fNumPi0(-1)
    , fNumProton(-1)
    , fNumNeutron(-1)
    , fIsCharm(false)
    , fResNum(-1)
    , fgQ2(kUndefinedValue)
    , fgq2(kUndefinedValue)
    , fgW(kUndefinedValue)
    , fgT(kUndefinedValue) 
    , fgX(kUndefinedValue)
    , fgY(kUndefinedValue)
    , fIsSeaQuark(false)
    , ftgtZ(0)
    , ftgtA(0)
    , ftgtPDG(0)
    , fProbePDG(-1)
  {
    fFShadSystP4.SetXYZT(0, 0, 0, 0);
    fHitNucP4.SetXYZT(0, 0, 0, 0);
    fProbeP4.SetXYZT(0, 0, 0, 0);
    fVertex.SetXYZT(0, 0, 0, 0);
  }
  
} // namespace simb
