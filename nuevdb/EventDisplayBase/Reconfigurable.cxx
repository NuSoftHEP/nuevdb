#include "nutools/EventDisplayBase/Reconfigurable.h"
#include "nutools/EventDisplayBase/ServiceTable.h"

#include <iostream>

evdb::Reconfigurable::Reconfigurable(fhicl::ParameterSet const& ps)
{
  ServiceTable::Instance().RegisterService(ps, cet::exempt_ptr<Reconfigurable>{this});
}
