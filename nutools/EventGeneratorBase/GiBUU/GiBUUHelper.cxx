////////////////////////////////////////////////////////////////////////
/// \file  GiBUUHelper.h
/// \brief Wrapper for generating neutrino interactions with GiBUU
///
/// \version $Id: GENIEHelper.cxx,v 1.58 2012-11-28 23:04:03 rhatcher Exp $
/// \author  brebel@fnal.gov  rhatcher@fnal.gov
/// update 2010-03-04 Sarah Budd added simple_flux
/// update 2013-04-24 rhatcher adapt to R-2_8_0 interface; subset flux files
////////////////////////////////////////////////////////////////////////

// GENIE includes
#include "EVGCore/EventRecord.h"

// NuTools includes
#include "nutools/EventGeneratorBase/evgenbase.h"
#include "nutools/EventGeneratorBase/GiBUU/GiBUUHelper.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCNeutrino.h"

// Framework includes
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "cetlib/search_path.h"
#include "cetlib/getenv.h"
#include "cetlib/split_path.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#ifndef NO_IFDH_LIB
  // IFDHC 
  #include "ifdh.h"
#else
  // nothing doing ... use ifdef to hide any referece that might need header
  #include <cassert>
#endif

namespace evgb {

  //static const int kNue      = 0; // unused
  //static const int kNueBar   = 1; // unused
  //static const int kNuMu     = 2; // unused
  //static const int kNuMuBar  = 3; // unused
  //static const int kNuTau    = 4; // unused
  //static const int kNuTauBar = 5; // unused

  //--------------------------------------------------
  GiBUUHelper::GiBUUHelper(fhicl::ParameterSet const& pset,
			   TGeoManager*               geoManager,
			   std::string         const& rootFile,
			   double              const& detectorMass)
    : GENIEHelper(pset, geoManager, rootFile, detectorMass),
      fGiBUUEventRecord(0)
  {
  }

  //--------------------------------------------------
  GiBUUHelper::~GiBUUHelper()
  {
    // clean up owned genie object (other genie obj are ref ptrs)
    delete fGiBUUEventRecord;

  }


} // namespace evgb

