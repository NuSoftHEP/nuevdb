////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeNone.cxx
/// \brief Flat time distribution
///
/// \version $Id: EvtTimeNone.cxx,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \update  2015-06-22 initial version
////////////////////////////////////////////////////////////////////////

#include "EvtTimeNone.h"
#include "EvtTimeShiftFactory.h"
TIMESHIFTREG3(evgb,EvtTimeNone,evgb::EvtTimeNone)

#include <iostream>

namespace evgb {

  EvtTimeNone::EvtTimeNone(const std::string& config) 
    : EvtTimeShiftI(config)
  { Config(config); }

  EvtTimeNone::~EvtTimeNone() { ; }

  void EvtTimeNone::Config(const std::string& config)
  {
  }

  double EvtTimeNone::TimeOffset()
  {
    return 0;
  }

  double EvtTimeNone::TimeOffset(std::vector<double> /* v */)
  {
    return TimeOffset();
  }

  void EvtTimeNone::PrintConfig(bool /* verbose */)
  {
  }

} // namespace evgb
