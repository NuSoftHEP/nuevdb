////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeFlat.h
/// \class evgb::EvtTimeFlat
/// \brief Flat time distribution
///
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \created 2015-06-22
/// \version $Id: EvtTimeFlat.h,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
////////////////////////////////////////////////////////////////////////

#ifndef SIMB_EVTTIMEFLAT_H
#define SIMB_EVTTIMEFLAT_H

#include "EvtTimeShiftI.h"
#include <string>
#include <vector>

namespace evgb {

  class EvtTimeFlat : public evgb::EvtTimeShiftI {
    
  public:
  
    EvtTimeFlat(const std::string& config);
    virtual ~EvtTimeFlat();

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

    /// specific methods for this variant
    void   SetDuration(double val) { fDuration=val; }
    double GetDuration() const { return fDuration; }
    void   SetGlobalOffset(double val) { fGlobalOffset=val; }
    double GetGlobalOffset() const { return fGlobalOffset; }

  private:

    double fDuration;      ///< duration (in ns)
    double fGlobalOffset;  ///< always displaced by this (in ns)

  };

} // namespace evgb

#endif //SIMB_EVTTIMEFLAT_H
