///
/// \file   EventDisplayBase/ServiceTable.h
/// \brief  Interface to services and their configurations
/// \author messier@indiana.edu
///
#ifndef EVDB_SERVICETABLE_H
#define EVDB_SERVICETABLE_H
#include <vector>
#include <map>
#include <string>
namespace fhicl { class ParameterSet; }

namespace evdb {
  static const int kDRAWING_SERVICE    = 1;
  static const int kEXPERIMENT_SERVICE = 2;
  static const int kART_SERVICE        = 3;
  static const int kNONE_SERVICE       = 4;

  ///
  /// \brief Information about a service required by the event display
  ///
  class ServiceTableEntry {
  public:
    std::string fName;
    std::string fParamSet;
    int         fCategory;
  };

  ///
  /// \brief Collection of Services used in the event display
  ///
  class ServiceTable {
  public:
    static ServiceTable& Instance();

    void Discover();
    
    static bool IsNoneService   (const std::string& s);
    static bool IsARTService    (const std::string& s);
    static bool IsDrawingService(const std::string& s);
    
    void Edit(unsigned int i);
    void ApplyEdits();

    static void OverrideCategory(const std::string& s, int cat);
    
    const fhicl::ParameterSet GetParameterSet(unsigned int i) const;

  public:
    std::vector<ServiceTableEntry> fServices;
    
  private:
    static std::map<std::string, int> fgCategoryOverrides;

    ServiceTable();
  };
}

#endif
////////////////////////////////////////////////////////////////////////
