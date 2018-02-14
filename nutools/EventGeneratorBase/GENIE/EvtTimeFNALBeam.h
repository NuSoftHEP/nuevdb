////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeFNALBeam.h
/// \class evgb::EvtTimeFNALBeam
/// \brief configurable FNAL Beam time distribution
///
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \created 2015-06-22
/// \version $Id: EvtTimeFNALBeam.h,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
////////////////////////////////////////////////////////////////////////

#ifndef SIMB_EVTTIMEFNALBEAM_H
#define SIMB_EVTTIMEFNALBEAM_H

#include "EvtTimeShiftI.h"
#include <string>
#include <vector>

namespace evgb {

  class EvtTimeFNALBeam : public evgb::EvtTimeShiftI {
    
  public:
  
    EvtTimeFNALBeam(const std::string& config);
    virtual ~EvtTimeFNALBeam();

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
    virtual double    TimeOffset(std::vector<double> bi);

    /// provide a means of printing the configuration
    virtual void     PrintConfig(bool verbose=true);

    /// specific methods for this variant
    void   SetTimeBetweenBuckets(double val) { fTimeBetweenBuckets=val; }
    double GetTimeBetweenBuckets() const { return fTimeBetweenBuckets; }
    void   SetBucketTimeSigma(double val) { fBucketTimeSigma=val; }
    double GetBucketTimeSigma() const { return fBucketTimeSigma; }
    void   SetNBucketsPerBatch(int ival) { fNBucketsPerBatch=ival; }
    int    GetNBucketsPerBatch() const { return fNBucketsPerBatch; }
    void   SetNFilledBucketsPerBatch(int ival) { fNFilledBucketsPerBatch=ival; }
    int    GetNFilledBucketsPerBatch() const { return fNFilledBucketsPerBatch; }

    void   SetBatchIntensities(std::vector<double> bi);
    void   SetDisallowedBatchMask(std::vector<int> disallow);

    void   SetGlobalOffset(double val) { fGlobalOffset=val; }
    double GetGlobalOffset() const { return fGlobalOffset; }

  private:

    void CalculateCPDF(std::vector<double> batchi);

    double fTimeBetweenBuckets;     ///< time between buckets
    double fBucketTimeSigma;        ///< how wide is distribution in bucket
    int    fNBucketsPerBatch;       ///<
    int    fNFilledBucketsPerBatch; ///<
    std::vector<double> fCummulativeBatchPDF;  ///< summed prob for batches
    std::vector<int>    fDisallowedBatchMask;  ///< disallow individual batches
    double fGlobalOffset;           ///< always displaced by this (in ns)

  };

} // namespace evgb

#endif //SIMB_EVTTIMEFNALBEAM_H
