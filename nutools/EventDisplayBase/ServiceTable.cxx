///
/// \file   EventDisplayBase/ServiceTable.h
/// \brief  Interface to services and their configurations
/// \author messier@indiana.edu
///
#include "nutools/EventDisplayBase/ServiceTable.h"

#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "nutools/EventDisplayBase/ParameterSetEdit.h"
#include "nutools/EventDisplayBase/ParameterSetEditDialog.h"

#include <iostream>

using namespace evdb;

std::map<std::string, int> ServiceTable::fgCategoryOverrides;

bool ServiceTable::IsNoneService(std::string const& s)
{
  if(fgCategoryOverrides.count(s))
    return fgCategoryOverrides[s] == kNONE_SERVICE;

  return (s.find("none")!=std::string::npos);
}

//......................................................................

bool ServiceTable::IsDrawingService(std::string const& s)
{
  if(fgCategoryOverrides.count(s))
    return fgCategoryOverrides[s] == kDRAWING_SERVICE;

  return (s.find("DrawingOptions")!=std::string::npos);
}

//......................................................................

void ServiceTable::Discover()
{
  //
  // Find all the parameter sets that go with services
  //
  std::vector<fhicl::ParameterSet> psets;
  auto& inst = art::ServiceRegistry::instance();
  inst.presentToken().getParameterSets(psets);

  //
  // Make a table of services with their categories and parameter
  // sets, if any
  //
  fServices.clear();
  for (size_t i=0; i<psets.size(); ++i) {

    auto const& stype = psets[i].get<std::string>("service_type","none");

    bool const isnone       = this->IsNoneService(stype);
    bool const isdrawing    = !isnone && this->IsDrawingService(stype);
    bool const isexperiment = !(isnone||isdrawing);

    ServiceTableEntry s;
    s.fName     = stype;
    s.fParamSet = "";

    s.fCategory = kNONE_SERVICE;
    if (isdrawing)    s.fCategory = kDRAWING_SERVICE;
    if (isexperiment) s.fCategory = kEXPERIMENT_SERVICE;

    fServices.push_back(s);
  }
}

//......................................................................

ServiceTable& ServiceTable::Instance()
{
  static ServiceTable s;
  return s;
}

//......................................................................

void ServiceTable::Edit(unsigned int const i)
{
  //
  // Get the list of parameters sets "in play" and find the one that
  // matches the requested edit
  //
  auto& inst = art::ServiceRegistry::instance();

  std::vector<fhicl::ParameterSet> psets;
  inst.presentToken().getParameterSets(psets);

  for (size_t j=0; j<psets.size(); ++j){
    if (psets[j].get<std::string>("service_type", "none") == fServices[i].fName) {
      new ParameterSetEditDialog(i);
    }
  }
}

//......................................................................

void ServiceTable::ApplyEdits()
{
  //
  // Look to see if we have any new service configurations to apply
  //
  auto& inst = art::ServiceRegistry::instance();
  //
  // auto& mgr = inst.manager_.factory_;
  // for (auto& service :
  std::vector<fhicl::ParameterSet> psets;
  inst.presentToken().getParameterSets(psets);
  for (size_t ps = 0; ps < psets.size(); ++ps) {
    for (unsigned int i=0; i < fServices.size(); ++i) {
      if (fServices[i].fParamSet.empty()) continue;

      if (psets[ps].get<std::string>("service_type","none") != fServices[i].fName)
        continue;

      LOG_DEBUG("ServiceTable") << "Applying edits for "
                                << fServices[i].fName
                                << "\n"
                                << fServices[i].fParamSet;

      try {
        fhicl::ParameterSet pset;
        fhicl::intermediate_table itable;
        //
        // Each of the next 2 lines may throw on error: should check.
        //
        fhicl::parse_document(fServices[i].fParamSet, itable);
        fhicl::make_ParameterSet(itable, pset);
        fServices[i].fParamSet = "";
        psets[ps] = pset;
      }
      catch (fhicl::exception const& e) {
        LOG_ERROR("ServiceTable") << "Error parsing the new configuration:\n"
                                  << e
                                  << "\nRe-configuration has been ignored for service: "
                                  << fServices[i].fName;
      }
    }
  }
  inst.presentToken().putParameterSets(psets);
}

//......................................................................

void ServiceTable::OverrideCategory(std::string const& s, int const cat)
{
  fgCategoryOverrides[s] = cat;
}

//......................................................................

fhicl::ParameterSet const& ServiceTable::GetParameterSet(unsigned int const id) const
{
  auto& sr = art::ServiceRegistry::instance();

  std::vector<fhicl::ParameterSet> pset;
  sr.presentToken().getParameterSets(pset);

  unsigned int i{};
  for (; i<pset.size(); ++i) {
    auto const& t = pset[i].get<std::string>("service_type","none");
    if (t==fServices[id].fName) return pset[i];
  }
  //
  // Fall through to here only on errors
  //
  LOG_ERROR("ServiceTable") << " Parameter set "
                            << fServices[i].fName
                            << " not found ";
  static fhicl::ParameterSet const empty;
  return empty;
}

//......................................................................

ServiceTable::ServiceTable() {}

////////////////////////////////////////////////////////////////////////
