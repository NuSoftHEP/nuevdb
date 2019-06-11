////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeFlat.cxx
/// \brief Flat time distribution
///
/// \version $Id: EvtTimeFlat.cxx,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \update  2015-06-22 initial version
////////////////////////////////////////////////////////////////////////

#include "EvtTimeFlat.h"
#include "EvtTimeShiftFactory.h"
TIMESHIFTREG3(evgb,EvtTimeFlat,evgb::EvtTimeFlat)

#include <iostream>

namespace evgb {

  EvtTimeFlat::EvtTimeFlat(const std::string& config) 
    : EvtTimeShiftI(config)
    , fDuration(6 * 84 * 1e9/53.103e6)
    , fGlobalOffset(0)
  { Config(config); }

  EvtTimeFlat::~EvtTimeFlat() { ; }

  void EvtTimeFlat::Config(const std::string& config)
  {
    // parse config string
    if ( config != "" ) {
      // for now just assume a single number is the duration
      // optional 2nd arg is global offset
      int nf = sscanf(config.c_str(),"%lf %lf",&fDuration,&fGlobalOffset);
      std::cout << "EvtTimeFlat::Config() read " << nf 
                << " values" << std::endl;
    }
    PrintConfig();
  }

  double EvtTimeFlat::TimeOffset()
  {
    return fRndmGen->Uniform(fDuration);
  }

  double EvtTimeFlat::TimeOffset(std::vector<double> /* v */)
  {
    // flat ... doesn't need additional parameter so ignore them
    return TimeOffset();
  }

  void EvtTimeFlat::PrintConfig(bool /* verbose */)
  {
    std::cout << "EvtTimeFlat config: "
              << "  GlobalOffset " << fGlobalOffset << " ns"
              << ", Duration " << fDuration << " ns"
              << std::endl;
  }

} // namespace evgb
