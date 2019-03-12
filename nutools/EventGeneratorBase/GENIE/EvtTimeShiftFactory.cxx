////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeShiftFactory.cxx
/// \brief factory for generating evgb::EvtTimeShiftI class objects
///
/// \version
/// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
/// \update  2015-06-22 initial version
////////////////////////////////////////////////////////////////////////

#include "EvtTimeShiftFactory.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib_except/exception.h"

namespace evgb {

// Define static variable which holds the one-and-only instance
EvtTimeShiftFactory* EvtTimeShiftFactory::fgTheInstance;

EvtTimeShiftFactory::EvtTimeShiftFactory()
{
  fgTheInstance = this;   // record created self in static pointer
}

EvtTimeShiftFactory::~EvtTimeShiftFactory()
{
  fgTheInstance = 0;
}

EvtTimeShiftFactory& EvtTimeShiftFactory::Instance()
{
  // Cleaner dtor calls EvtTimeShiftFactory dtor at job end
  static Cleaner cleaner;

  if ( ! fgTheInstance ) {
    // need to create one
    cleaner.UseMe();   // dummy call to quiet compiler warnings
    fgTheInstance = new EvtTimeShiftFactory();
  }

  return *fgTheInstance;
}

evgb::EvtTimeShiftI*
EvtTimeShiftFactory::GetEvtTimeShift(const std::string& name,
                                     const std::string& config) const
{
  evgb::EvtTimeShiftI* p = 0;

  mf::LogDebug("EvtTime")
    << "EvtTimeShiftFactory::GetEvtTimeShift rwh name --->"
    << name << "<--- \n config -->" << config << "<---" << std::endl;

  // trim any leading whitespace
  std::string nameLocal = name;
  std::string configLocal = "";
  if( nameLocal.find_first_not_of(" \t\n") != 0 )
      nameLocal.erase( 0, nameLocal.find_first_not_of(" \t\n")  );

  // in case "name" actually includes the config string
  size_t iws = nameLocal.find_first_of(" \t\n");
  if ( iws != std::string::npos ) {
    configLocal = nameLocal.substr(iws,std::string::npos);
    configLocal += " ";
    nameLocal.erase(iws,std::string::npos);
  }
  configLocal += config; // append any addition config string

  mf::LogDebug("EvtTime")
    << "EvtTimeShiftFactory::GetEvtTimeShift rwh name --->"
    << nameLocal << "<--- \n config -->" << configLocal << "<---" << std::endl;

  // we don't want map creating an entry if it doesn't exist
  // so use map::find() not map::operator[]
  std::map<std::string, EvtTimeShiftICtorFuncPtr_t>::const_iterator itr
    = fFunctionMap.find(nameLocal);
  if ( fFunctionMap.end() != itr ) {
    // found an appropriate entry in the list
    EvtTimeShiftICtorFuncPtr_t foo = itr->second;  // this is the function
    p = (*foo)(configLocal);  // use function to create the EvtTimeShiftI
  }
  if ( ! p ) {
    mf::LogInfo("EvtTime")
      << "### EvtTimeShiftFactory WARNING: "
      << "EvtTimeShiftI class \"" << nameLocal << "\" is not known" << std::endl;
    Print();
    throw cet::exception("NoEvtTimeShiftClass")
      << "EvtTimeShiftI class \"" << nameLocal << "\" is not known" << std::endl;
  }
  return p;
}

bool EvtTimeShiftFactory::IsKnownEvtTimeShift(const std::string& name)
{
  //  check if we know the name
  bool res = false;
  std::map<std::string, EvtTimeShiftICtorFuncPtr_t>::iterator itr
    = fFunctionMap.find(name);
  if ( fFunctionMap.end() != itr ) res = true;
  return res;
}

const std::vector<std::string>&
EvtTimeShiftFactory::AvailableEvtTimeShift() const
{
  // list of names might be out of date due to new registrations
  // rescan the std::map on each call (which won't be frequent)
  listnames.clear();

  // scan map for registered names
  std::map<std::string, EvtTimeShiftICtorFuncPtr_t>::const_iterator itr;
  for ( itr = fFunctionMap.begin(); itr != fFunctionMap.end(); ++itr )
    listnames.push_back(itr->first);

  return listnames;
}

void EvtTimeShiftFactory::Print() const
{
  std::ostringstream msg;
  msg << "EvtTimeShiftFactory list of known EvtTimeShiftI classes: \n";

  const std::vector<std::string>& known = AvailableEvtTimeShift();
  for (size_t i=0; i < known.size(); ++i) {
    msg << "   [" << std::setw(2) << i << "] " << known[i] << std::endl;
  }
  mf::LogInfo("EvtTime") << msg.str();
}

bool EvtTimeShiftFactory::RegisterCreator(std::string name,
                                         EvtTimeShiftICtorFuncPtr_t foo,
                                         bool* boolptr)
{
  // record new functions for creating processes
  fFunctionMap[name] = foo;
  fBoolPtrMap[name]  = boolptr;
  return true;
}

} // namespace evgb
