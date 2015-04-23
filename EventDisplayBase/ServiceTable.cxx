///
/// \file   ServiceTable.h
/// \brief  Interface to services and their configurations
/// \author messier@indiana.edu
///
#include "EventDisplayBase/ServiceTable.h"

#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "EventDisplayBase/ParameterSetEdit.h"
#include "EventDisplayBase/ParameterSetEditDialog.h"

#include <iostream>

using namespace evdb;

std::map<std::string, int> ServiceTable::fgCategoryOverrides;

bool ServiceTable::IsNoneService(const std::string& s) 
{
  if(fgCategoryOverrides.count(s))
    return fgCategoryOverrides[s] == kNONE_SERVICE;

  return (s.find("none")!=std::string::npos);
}

//......................................................................

bool ServiceTable::IsARTService(const std::string& s) 
{
  if(fgCategoryOverrides.count(s))
    return fgCategoryOverrides[s] == kART_SERVICE;

  //
  // This is the list of ART services I know about. Add ones you know about.
  //
  const char* artService[] = {
    "Timing",
    "TFileService",
    "SimpleMemoryCheck",
    "message",
    "scheduler",
    "RandomNumberGenerator",
    "FileTransfer",
    "CatalogInterface",
    "FileCatalogInterface",
    0
  };
  for (unsigned int j=0; artService[j]!=0; ++j) {
    if (s.find(artService[j])!=std::string::npos) return true;
  }
  return false;
}

//......................................................................

bool ServiceTable::IsDrawingService(const std::string& s) 
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
  std::vector< fhicl::ParameterSet > psets;
  art::ServiceRegistry& inst = art::ServiceRegistry::instance();
  inst.presentToken().getParameterSets(psets);

  //
  // Make a table of services with their categories and parameter
  // sets, if any
  //
  fServices.clear();
  for (size_t i=0; i<psets.size(); ++i) {

    std::string stype = psets[i].get<std::string>("service_type","none");
    
    bool isnone       = this->IsNoneService(stype);
    bool isdrawing    = !isnone && this->IsDrawingService(stype);
    bool isart        = !isnone && this->IsARTService(stype);
    bool isexperiment = !(isnone||isdrawing||isart);
    
    ServiceTableEntry s;
    s.fName     = stype;
    s.fParamSet = "";

    s.fCategory = kNONE_SERVICE;
    if (isdrawing)    s.fCategory = kDRAWING_SERVICE;
    if (isart)        s.fCategory = kART_SERVICE;
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

void ServiceTable::Edit(unsigned int i) 
{
  //
  // Get the list of parameters sets "in play" and find the one that
  // matches the requested edit
  //
  std::vector< fhicl::ParameterSet > psets;
  art::ServiceRegistry& inst = art::ServiceRegistry::instance();
  inst.presentToken().getParameterSets(psets);
  
  for (size_t j=0; j<psets.size(); ++j){
    bool ismatch = 
      (psets[j].get<std::string>("service_type", "none").
       compare(fServices[i].fName)==0);
    if (ismatch) {
      new ParameterSetEditDialog(i);
      /*
      new ParameterSetEdit(0,
			   "Service",
			   fServices[i].fName,
			   psets[j].to_string(),
			   &fServices[i].fParamSet);
      */
    }
  }
}

//......................................................................

void ServiceTable::ApplyEdits()
{
  //
  // Look to see if we have any new service configurations to apply
  //
  art::ServiceRegistry& inst = art::ServiceRegistry::instance();
  std::vector< fhicl::ParameterSet > psets;
  inst.presentToken().getParameterSets(psets);
  for(size_t ps = 0; ps < psets.size(); ++ps){    
    for (unsigned int i=0; i<fServices.size(); ++i) {
      if (fServices[i].fParamSet.empty()) continue;

      bool ismatch = 
	(fServices[i].fName.
	 compare(psets[ps].get<std::string>("service_type","none"))==0);

      
      if (ismatch) {
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
	catch (fhicl::exception& e) {
	  LOG_ERROR("ServiceTable") << "Error parsing the new configuration:\n"
				    << e
				    << "\nRe-configuration has been ignored for service: "
				    << fServices[i].fName;
	}
      }
    }
  }
  inst.presentToken().putParameterSets(psets);
}

//......................................................................

void ServiceTable::OverrideCategory(const std::string& s, int cat)
{
  fgCategoryOverrides[s] = cat;
}

//......................................................................

const fhicl::ParameterSet ServiceTable::GetParameterSet(unsigned int id) const
{
  unsigned int i;
  art::ServiceRegistry& sr = art::ServiceRegistry::instance();
  
  std::vector< fhicl::ParameterSet > pset;
  sr.presentToken().getParameterSets(pset);
  
  for (i=0; i<pset.size(); ++i) {
    std::string t = pset[i].get<std::string>("service_type","none");
    if (t==fServices[id].fName) return pset[i];
  }
  //
  // Fall through to here only on errors
  //
  LOG_ERROR("ServiceTable") << " Parameter set " 
			    << fServices[i].fName
			    << " not found ";
  static fhicl::ParameterSet empty;
  return empty;
}

//......................................................................

ServiceTable::ServiceTable() {}

////////////////////////////////////////////////////////////////////////
