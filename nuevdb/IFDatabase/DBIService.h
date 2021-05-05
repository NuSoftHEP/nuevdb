///////////////////////////////////////////////////////////////////////////
/// \brief  Simple service to provide a configurable Database table object
/// \author Christopher Backhouse - bckhouse@caltech.edu
//////////////////////////////////////////////////////////////////////////

#ifndef IFDBISERVICE_H
#define IFDBISERVICE_H

#include <string>

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "nuevdb/EventDisplayBase/Reconfigurable.h"
#include "nuevdb/IFDatabase/Table.h"


namespace nutools
{
  namespace dbi
  {

    /// \brief Simple service to provide a RunHistory configured to the right run
    class DBIService : public evdb::Reconfigurable
    {
    public:
      // Get a RunHistoryService instance here
      DBIService(const fhicl::ParameterSet& pset);

      void reconfigure(const fhicl::ParameterSet& pset);

      Table* CreateTable(std::string tableName="",
                         std::string schemaName="",
                         int tableType=nutools::dbi::kConditionsTable,
                         int dataSource=nutools::dbi::kOffline);

    protected:
      int fVerbosity;
      bool fTimeQueries;
      bool fTimeParsing;

      std::string fWebServiceURL;
      std::string fQueryEngineURL;
      std::string fDBUser;

    };

  }
}

DECLARE_ART_SERVICE(nutools::dbi::DBIService, LEGACY)

#endif
////////////////////////////////////////////////////////////////////////
