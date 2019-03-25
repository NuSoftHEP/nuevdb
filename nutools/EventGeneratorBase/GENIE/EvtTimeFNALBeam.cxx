////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeFNALBeam.cxx
/// \brief configurable FNAL Beam time distribution
///
/// \version $Id: EvtTimeFNALBeam.cxx,v 1.1 2015/06/30 18:01:24 rhatcher Exp $
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \update  2015-06-22 initial version
/// \update  2019-03-12 updated version w/ configurability for Booster
///
///  This routine is based on a "theoretical" description of how
///  the Fermilab accelerator system works.
///
///  For the Booster there are 84 RF "buckets" or 84 "bunches" of protons
///  in the system at a time;  a "notch" (3) is taken out
///  leaving 81 filled buckets / bunches === "batch"
///
///  NuMI take 2 sets of 6 batches and stacks them.
///  In actual practice doesn't have stacking so exact with a
///  1-2 bucket offset, so inter-batch separation isn't as deep
///  If we ever desire a more data driven time profile we can
///  get wall monitor time structure histograms from Phil A.
///
///  A note about "bucket" or "bunch" width (essentially the same thing)
///  per Phil A. private conversation (2010-03-25):
///     0.75 ns sigma for NuMI comes from MINOS Time-of-Flight paper
///        though it could be currently ~ 1ns
///     2.0 - 2.5 ns width for Booster is reasonable
///        it is expected that the Booster width >> NuMI width
///        due to higher electric fields / deeper buckets
///
////////////////////////////////////////////////////////////////////////

#include "EvtTimeFNALBeam.h"
#include "EvtTimeShiftFactory.h"
TIMESHIFTREG3(evgb,EvtTimeFNALBeam,evgb::EvtTimeFNALBeam)

#include <iostream>
#include <iomanip>
#include <algorithm>

//GENIE includes
#ifdef GENIE_PRE_R3
  #include "GENIE/Utils/StringUtils.h"
#else
  #include "GENIE/Framework/Utils/StringUtils.h"
#endif
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib_except/exception.h"

const double ksigma2fwhm = 2.0*std::sqrt(2.0*std::log(2.0));

namespace evgb {

  EvtTimeFNALBeam::EvtTimeFNALBeam(const std::string& config)
    // default to a NuMI config
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
    if ( config == "" ) return;
    // GENIEHelper does a PrintConfig() when it gets this object ...

    // not the most sophisticated of parsing ... but FHICL would be overkill

    std::string configLocal = config;
    // convert string to lowercase
    std::transform(configLocal.begin(),configLocal.end(),configLocal.begin(),::tolower);

    // for now make use of GENIE utilities
    std::vector<std::string> strs =
      genie::utils::str::Split(configLocal,"\t\n ,;=(){}[]");
    // weed out blank ones
    strs.erase(std::remove_if(strs.begin(), strs.end(),
                              [](const std::string& x) {
                                return ( x == "") ; // put your condition here
                              }), strs.end());

    // debugging info
    std::ostringstream msgx;
    msgx << "Config elements:" << std::endl;
    for (size_t j=0; j<strs.size(); ++j) {
      msgx << " [" << std::setw(3) << j << "] -->" << strs[j] << "<--\n";
    }
    // this should end up as LogDebug
    mf::LogDebug("EvtTime") << msgx.str() << std::flush;

    // blindly reduced UPPER -> lower case above to make this easier
    size_t nstrs = strs.size();
    for (size_t i=0; i<nstrs; ++i) {
      if ( strs[i] == "numi" ) {
        fTimeBetweenBuckets     = 1e9/53.103e6;
        fBucketTimeSigma        = 0.750;
        fNBucketsPerBatch       = 84;  // NOvA-era 81+3, MINOS-era 81+5
        fNFilledBucketsPerBatch = 81;  // 81 for both eras
        fDisallowedBatchMask    = std::vector<int>(6,0); // don't disallow any
        fGlobalOffset           = 0;
        std::vector<double> bi(6,1.0); // 6 equal batches
        SetBatchIntensities(bi);
      } else
      if ( strs[i] == "booster" ) {
        fTimeBetweenBuckets     = 1e9/53.103e6;
        fBucketTimeSigma        = 2.0;
        fNBucketsPerBatch       = 84;  //
        fNFilledBucketsPerBatch = 81;  //
        fDisallowedBatchMask    = std::vector<int>(1,0); // don't disallow any
        fGlobalOffset           = 0;
        std::vector<double> bi(1,1.0); // 1 batch
        SetBatchIntensities(bi);
      } else
      if ( strs[i].find("intensity") != std::string::npos ) {
        // a list of batch intensities ... sets # of batches
        // eat values up until we see the end, or a word
        std::vector<double> bi;
        // count how many of next tokens are numbers (crude)
        for (size_t jj=i+1; jj<nstrs; ++jj) {
          // very crude check of being a number
          // can be  fooled by strange text
          size_t pos = strs[jj].find_first_not_of("0123456789.-+eE");
          if ( pos != std::string::npos ) break;
          // looks like a numeric value
          double val = atof(strs[jj].c_str());
          if ( val < 0 ) {
            mf::LogError("EvtTime")
              << "EvtTimeFNALBeam 'intensity' value [" << (jj-i-1)
              << "]=" << val << " '" << strs[jj] << "' "
              << "can't be less than zero, setting to zero";
            val = 0;
          }
          bi.push_back(val);
        }
        // ate up some strings ... move loop index
        i += bi.size();
        if ( bi.empty() ) {
          mf::LogError("EvtTime")
            << "EvtTimeFNALBeam error 'intensity' option didn't seem to have values";
        } else {
          SetBatchIntensities(bi);
        }
      } else
      if ( strs[i] == "bdisallowed" ) {
        mf::LogError("EvtTime")
          << "EvtTimeFNALBeam sorry 'bdisallowed' option not yet implemented";
      } else {
        // all the rest take one numeric value
        if ( i+1 >= nstrs ) {
          mf::LogError("EvtTime")
            << "EvtTimeFNALBeam sorry too few values for '" << strs[i] << "'";
          continue;
        }
        const char* arg = strs[i+1].c_str();
        if      ( strs[i] == "sigma"     ) fBucketTimeSigma        = atof(arg);
        else if ( strs[i] == "fwhm"      ) fBucketTimeSigma        = atof(arg)/ksigma2fwhm;
        else if ( strs[i] == "dtbucket"  ) fTimeBetweenBuckets     = atof(arg);
        else if ( strs[i] == "nperbatch" ) fNBucketsPerBatch       = atoi(arg);
        else if ( strs[i] == "nfilled"   ) fNFilledBucketsPerBatch = atoi(arg);
        else if ( strs[i] == "global"    ) fGlobalOffset           = atof(arg);
        else {
          mf::LogError("EvtTime")
            << "unknown EvtTimeFNALBeam config key '" << strs[i] << "'";
          continue;
        }
        ++i; // used up an argument
      }
    }
    // consistency check
    if ( fNFilledBucketsPerBatch > fNBucketsPerBatch ) {
      mf::LogError("EvtTime")
        << "EvtTimeFNALBeam nfilled " << fNFilledBucketsPerBatch
        << " of " << fNBucketsPerBatch << " buckets per batch,\n"
        << "set nfilled to match buckets per batch";
      fNFilledBucketsPerBatch = fNBucketsPerBatch;
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

    std::ostringstream msg;
    msg  << "EvtTimeFNALBeam config: \n"
         << "  TimeBetweenBuckets:     " << fTimeBetweenBuckets << " ns\n"
         << "  BucketTimeSigma:        " << fBucketTimeSigma << " ns "
         << "[FWHM " << fBucketTimeSigma*ksigma2fwhm << "]\n"
         << "  NBucketsPerBatch:       " << fNBucketsPerBatch << "\n"
         << "  NFilledBucketsPerBatch: " << fNFilledBucketsPerBatch << "\n"
         << "  Relative Fractions:    ";
    double prev=0;
    for (size_t i=0; i < fCummulativeBatchPDF.size(); ++i) {
      double frac = fCummulativeBatchPDF[i] - prev;
      bool skip = fDisallowedBatchMask[i];
      msg << " ";
      if (skip) msg << "{{";
      msg << frac;
      if (skip) msg << "}}";
      prev = fCummulativeBatchPDF[i];
    }
    msg << "\n"
        << "  GlobalOffset:           " << fGlobalOffset << " ns\n";

    mf::LogInfo("EvtTime") << msg.str();
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
