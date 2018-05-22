////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeFNALBeam.cxx
/// \brief configurable FNAL Beam time distribution
///
/// \version $Id: EvtTimeFNALBeam.cxx,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \update  2015-06-22 initial version
////////////////////////////////////////////////////////////////////////

#include "EvtTimeFNALBeam.h"
#include "EvtTimeShiftFactory.h"
TIMESHIFTREG3(evgb,EvtTimeFNALBeam,evgb::EvtTimeFNALBeam)

#include <iostream>

namespace evgb {

  EvtTimeFNALBeam::EvtTimeFNALBeam(const std::string& config)
    : EvtTimeShiftI(config)
    , fTimeBetweenBuckets(1e9/53.103e6)
    , fBucketTimeSigma(0.750)
    , fNBucketsPerBatch(84)        // NOvA-era 81+3, MINOS-era 81+5
    , fNFilledBucketsPerBatch(81)  // 81 for both eras
    , fDisallowedBatchMask(6,0)    // don't disallow any
    , fGlobalOffset(0)
  {
    std::vector<double> bi(6,1.0); // 6 equal batches
    SetBatchIntensities(bi);
    Config(config);
  }

  EvtTimeFNALBeam::~EvtTimeFNALBeam() { ; }

  void EvtTimeFNALBeam::Config(const std::string& config)
  {
    // parse config string
    if ( config != "" ) {
      std::cerr
        << "!!!!! EvtTimeFNALBeam - not yet up to parsing Config string "
        << ", ignoring:"
        << std::endl
        << "\"" << config << "\""
        << std::endl
        << "Starting with: "
        << std::endl;
      PrintConfig();
    }
  }

  double EvtTimeFNALBeam::TimeOffset()
  {
    // calculate in small to large

    // pick a time within a bucket
    double offset = fRndmGen->Gaus(0.0,fBucketTimeSigma);

    // pick a bucket within a batch
    // assume all ~ buckets constant in batch until we have another model
    offset +=  fTimeBetweenBuckets *
               (double)fRndmGen->Integer(fNFilledBucketsPerBatch);

    // pick a bucket
    bool   disallowed = true;
    size_t ibatch = 0;
    size_t nbatch = fCummulativeBatchPDF.size();
    double r = 2;
    while ( disallowed ) {
      r = fRndmGen->Uniform();
      for (ibatch=0; ibatch<nbatch; ++ibatch) {
        if ( r <= fCummulativeBatchPDF[ibatch] ) break;
      }
      disallowed = ( fDisallowedBatchMask[ibatch] != 0 );
    }
    offset += fTimeBetweenBuckets*(double)fNBucketsPerBatch*(double)ibatch;

    // finally the global offset
    return offset + fGlobalOffset;
  }

  double EvtTimeFNALBeam::TimeOffset(std::vector<double> bi)
  {
    CalculateCPDF(bi);
    return TimeOffset();
  }

  void EvtTimeFNALBeam::PrintConfig(bool /* verbose */)
  {
    std::cout << "EvtTimeFNALBeam config: " << std::endl
              << "  TimeBetweenBuckets:     " << fTimeBetweenBuckets << " ns" << std::endl
              << "  BucketTimeSigma:        " << fBucketTimeSigma << " ns" << std::endl
              << "  NBucketsPerBatch:       " << fNBucketsPerBatch << std::endl
              << "  NFilledBucketsPerBatch: " << fNFilledBucketsPerBatch << std::endl
              << "  Relative Fractions:    ";
    double prev=0;
    for (size_t i=0; i < fCummulativeBatchPDF.size(); ++i) {
      double frac = fCummulativeBatchPDF[i] - prev;
      bool skip = fDisallowedBatchMask[i];
      std::cout << " ";
      if (skip) std::cout << "{{";
      std::cout << frac;
      if (skip) std::cout << "}}";
      prev = fCummulativeBatchPDF[i];
    }
    std::cout << std::endl
              << "  GlobalOffset:           " << fGlobalOffset << " ns" << std::endl;
  }


  void EvtTimeFNALBeam::SetBatchIntensities(std::vector<double> bi)
  {
    CalculateCPDF(bi);
  }

  void EvtTimeFNALBeam::SetDisallowedBatchMask(std::vector<int> disallow)
  {
    size_t ndis = disallow.size();
    size_t nbi  = fCummulativeBatchPDF.size();
    fDisallowedBatchMask = disallow;
    // expand it so it's mirrors # of batch intensities
    // but allow all that haven't been set
    if ( nbi > ndis ) fDisallowedBatchMask.resize(nbi,0);
  }

  void EvtTimeFNALBeam::CalculateCPDF(std::vector<double> bi)
  {
    fCummulativeBatchPDF.clear();
    double sum = 0;
    size_t nbi = bi.size();
    for (size_t i=0; i < nbi; ++i) {
      sum += bi[i];
      fCummulativeBatchPDF.push_back(sum);
    }
    // normalize to unit probability
    for (size_t i=0; i < nbi; ++i) fCummulativeBatchPDF[i] /= sum;
    // make sure the mask vector keeps up (but never make it smaller)
    // allowing all new batches
    if ( nbi > fDisallowedBatchMask.size() )
      fDisallowedBatchMask.resize(nbi,0);

    /*
    for (size_t j=0; j<nbi; ++j) {
      std::cout << " CPDF[" << j << "] " << fCummulativeBatchPDF[j]
                << "  " << ((fDisallowedBatchMask[j])?"dis":"") << "allowed"
                << std::endl;
    }
    */

  }


} // namespace evgb
