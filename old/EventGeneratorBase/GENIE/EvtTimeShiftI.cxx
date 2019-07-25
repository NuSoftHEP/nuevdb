////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeShiftI.cxx
/// \brief interface for event time distribution
///
/// \version $Id: EvtTimeShiftI.cxx,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \update  2015-06-22 initial version
////////////////////////////////////////////////////////////////////////

#include "EvtTimeShiftI.h"
#include "TRandom3.h"

namespace evgb {

  EvtTimeShiftI::EvtTimeShiftI(const std::string& config)
    : fRndmGen(new TRandom3), fIsOwned(true)
  { 
    // user should call Config(config) in their constructor
  }

  EvtTimeShiftI::~EvtTimeShiftI()
  {
    if (fIsOwned) delete fRndmGen;
    fRndmGen = 0;
  }
  void EvtTimeShiftI::SetRandomGenerator(TRandom* gen, bool isOwned) 
  {
    // deal with what we might already have
    if ( fIsOwned ) { delete fRndmGen; fRndmGen = 0; fIsOwned = false; }

    fRndmGen = gen;
    fIsOwned = isOwned;
  }

} // namespace evgb
