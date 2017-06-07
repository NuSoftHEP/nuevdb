///
/// \file   EventDisplayBase/Reconfigurable.h
/// \brief  Interface class to services that are intended to be reconfigurable through the event display
/// \author knoepfel@fnal.gov
///
#ifndef EVDB_RECONFIGURABLE_H
#define EVDB_RECONFIGURABLE_H

namespace fhicl { class ParameterSet; }

namespace evdb {

  class Reconfigurable {
  public:

    explicit Reconfigurable(fhicl::ParameterSet const& ps);
    void do_reconfigure(fhicl::ParameterSet const& pset) { reconfigure(pset); }

  private:
    virtual void reconfigure(fhicl::ParameterSet const&) = 0;
  };

}

#endif
////////////////////////////////////////////////////////////////////////
