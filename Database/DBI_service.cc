///////////////////////////////////////////////////////////////////////////
/// \brief  Simple service to provide a RunHistory configured to the right run
/// \author Christopher Backhouse - bckhouse@caltech.edu
//////////////////////////////////////////////////////////////////////////

// nutools includes
#include "Database/DBIService.h"

// Framework includes
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace nutools
{
namespace dbi
{

  //------------------------------------------------------------
  DBIService::DBIService(const fhicl::ParameterSet& pset,
			 art::ActivityRegistry &reg)
  {
    reconfigure(pset);
  }
  
  //-----------------------------------------------------------
  void DBIService::reconfigure(const fhicl::ParameterSet& pset)
  {
    fVerbosity = pset.get< int >("Verbosity",0);
    fTimeQueries = pset.get< bool >("TimeQueries", false);
    fTimeParsing = pset.get< bool >("TimeParsing", false);
    
    fWebServiceURL = pset.get< std::string >("WebServiceURL");
    fQueryEngineURL = pset.get< std::string >("QueryEngineURL");
    fDBUser = pset.get< std::string >("DBUser");
  }

  //-----------------------------------------------------------
  Table* DBIService::CreateTable(std::string tableName,
				 std::string schemaName,
				 int tableType, int dataSource)
  {
    if (tableName.empty()) return 0;
    
    Table* t = new nutools::dbi::Table();
    t->SetTableName(tableName);
    t->SetDetector(schemaName);
    if (tableType < 0 || tableType >= nutools::dbi::kNTableType)
      t->SetTableType(nutools::dbi::kConditionsTable);
    if (dataSource < 0 || dataSource >= nutools::dbi::kNDataSources)
      t->SetDataSource(nutools::dbi::kOffline);

    t->SetVerbosity(fVerbosity);
    t->SetTimeQueries(fTimeQueries);
    t->SetTimeParsing(fTimeParsing);
    if (!fWebServiceURL.empty())
      t->SetWSURL(fWebServiceURL);
    if (!fQueryEngineURL.empty())
      t->SetQEURL(fQueryEngineURL);

    if (!fDBUser.empty())
      t->SetUser(fDBUser);

    return t;
  }
  
  DEFINE_ART_SERVICE(DBIService)

}
}
////////////////////////////////////////////////////////////////////////
