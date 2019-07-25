////////////////////////////////////////////////////////////////////////
/// \file  GiBUUHelper.h
/// \brief Wrapper for generating neutrino interactions with GIBUU
///
/// \version $Id: GiBUUHelper.h,v 1.25 2012-09-07 21:35:26 brebel Exp $
/// \author  jpaley@fnal.gov, brebel@fnal.gov rhatcher@fnal.gov
////////////////////////////////////////////////////////////////////////
#ifndef EVGB_GIBUUHELPER_H
#define EVGB_GIBUUHELPER_H

#include <vector>
#include <set>

#include "TGeoManager.h"

#include "nutools/EventGeneratorBase/GENIE/GENIEHelper.h"

namespace evgb {

  class GiBUUHelper : public GENIEHelper {

  public:

    explicit GiBUUHelper(fhicl::ParameterSet const& pset,
                         TGeoManager*               rootGeom,
                         std::string         const& rootFile,
                         double              const& detectorMass);
    ~GiBUUHelper();

    genie::EventRecord* GetGiBUUEventRecord() { return fGiBUUEventRecord; }

  private:

    genie::EventRecord* fGiBUUEventRecord;

  };
}
#endif //EVGB_GENIEHELPER_H
