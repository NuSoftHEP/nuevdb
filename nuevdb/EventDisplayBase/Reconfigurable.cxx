#include "nuevdb/EventDisplayBase/Reconfigurable.h"
#include "nuevdb/EventDisplayBase/ServiceTable.h"

#include <iostream>

evdb::Reconfigurable::Reconfigurable(fhicl::ParameterSet const& ps)
{
  ServiceTable::Instance().RegisterService(ps, cet::exempt_ptr<Reconfigurable>{this});
}
