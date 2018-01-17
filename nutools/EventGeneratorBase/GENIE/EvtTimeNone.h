////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeNone.h
/// \class evgb::EvtTimeNone
/// \brief time distribution that is delta of 0 (no shift)
///
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \created 2015-06-22
/// \version $Id: EvtTimeNone.h,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
////////////////////////////////////////////////////////////////////////

#ifndef SIMB_EVTTIMENONE_H
#define SIMB_EVTTIMENONE_H

#include "EvtTimeShiftI.h"
#include <string>
#include <vector>

namespace evgb {

  class EvtTimeNone : public evgb::EvtTimeShiftI {
    
  public:
  
    EvtTimeNone(const std::string& config);
    virtual ~EvtTimeNone();

    //
    // complete the EvtTimeShiftI interface:
    //

    /// each schema must take a string that configures it
    /// it is up to the individual model to parse said string
    /// and extract parameters 
    virtual void      Config(const std::string& config );

    /// return time within a 'record' in nanoseconds
    /// version taking array might be used for relative batch fractions
    /// that vary on a record-by-record basis
    virtual double    TimeOffset();
    virtual double    TimeOffset(std::vector<double> v);

    /// provide a means of printing the configuration
    virtual void     PrintConfig(bool verbose=true);

  private:

  };

} // namespace evgb

#endif //SIMB_EVTTIMENONE_H
