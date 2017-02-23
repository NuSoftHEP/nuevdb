#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <cinttypes>
#include <cstdio>
//#include <sys/types.h>
//#include <sys/sysinfo.h>

#include <libpq-fe.h>

//#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <nutools/IFDatabase/Table.h>
#include <nutools/IFDatabase/Util.h>

// variable length arrays are not standard c++
#pragma GCC diagnostic ignored "-Wvla"

namespace nutools {
  namespace dbi {

    //************************************************************

    Table::Table() : fHasRecordTime(false),
                     fFlushCache(false),
                     fDisableCache(false),
                     fMaxTSVld(0),fMinTSVld(0),
                     fRecordTime(0)
    {
      fTableName="";
      fConnection=0;
      fHasConnection=false;
      fDetector="";
      fDBHost="";
      fDBName="";
      fDBPort="";
      fUser="";
      fSchema="undef";
      fTableType = kGenericTable;
      fDataTypeMask = 0;
      fDataSource = kUnknownSource;

      fIgnoreEnvVar = false;
      fTestedExists = false;
      fExistsInDB = false;
      addInsertTime = addUpdateTime = false;
      addInsertUser = addUpdateUser = false;
      fIgnoreDB = false;
      fTimeQueries = true;
      fTimeParsing = true;
      fMinChannel = 0;
      fMaxChannel = 0;
      fFolder = "";
      
      Reset();

      srandom(time(0));

      // default total time to attempt to connect to the db/web server will be 
      // ~4 minutes (some randomness is introduced by libwda)      
      fConnectionTimeout = 4*60; 
      // override default timeout if env. variable is set, but must be 
      // greater than 20 seconds	
      char* tmpStr = getenv("DBITIMEOUT");
      if (tmpStr) {
	int tmpTO = atoi(tmpStr);
	if (tmpTO > 20)
	  fConnectionTimeout = tmpTO;
      }

      fTag = "";
      fWSURL = "";
      const char* wsHost = getenv("DBIWSURL");
      if (wsHost) fWSURL = std::string(wsHost);

      fUConDBURL = "";      
      const char* ucondbHost = getenv("DBIUCONDBURL");
      if (ucondbHost) fUConDBURL = std::string(ucondbHost);

      fQEURL = "";
      const char* qeHost = getenv("DBIQEURL");
      if (qeHost) fQEURL = std::string(qeHost);

      fVerbosity=0;
      tmpStr = getenv("DBIVERB");
      if (tmpStr) {
        fVerbosity = atoi(tmpStr);
      }
    }

    //************************************************************

    Table::Table(std::string schemaName, std::string tableName,
                 int ttype,
                 std::string dbhost, std::string dbname,
                 std::string dbport, std::string dbuser)
    {
      fConnection=0;
      fHasConnection=false;
      std::string errStr;
      fIgnoreDB = false;

      fTimeQueries = true;
      fTimeParsing = true;

      fMinChannel = 0;
      fMaxChannel = 0;

      fFolder = "";
      
      fDataSource = kUnknownSource;

      fVerbosity=0;
      char* tmpStr = getenv("DBIVERB");
      if (tmpStr) {
        fVerbosity = atoi(tmpStr);
      }
      
      // default total time to attempt to connect to the db/web server will be 
      // ~4 minutes (some randomness is introduced by libwda)      
      fConnectionTimeout = 4*60; 
      // override default timeout if env. variable is set, but must be 
      // greater than 20 seconds	
      tmpStr = getenv("DBITIMEOUT");
      if (tmpStr) {
	int tmpTO = atoi(tmpStr);
	if (tmpTO > 20)
	  fConnectionTimeout = tmpTO;
      }

      if (!dbname.empty()) SetDBName(dbname);
      /*
      if (DBName() == "") {
        errStr = "Table::Table(): missing database name!";
        throw std::runtime_error(errStr);
      }
      */
      if (!dbhost.empty()) SetDBHost(dbhost);
      /*
      if (DBHost() == "") {
        errStr = "Table::Table(): missing database host!";
        throw std::runtime_error(errStr);
      }
      */

      if (!dbport.empty()) SetDBPort(dbport);
      if (!dbuser.empty()) SetUser(dbuser);
      
      this->SetTableName(tableName);
      fSchema = std::string(schemaName);
      boost::to_lower(fSchema);

      std::string stName = fSchema + std::string(".") + std::string(tableName);
      //      fIgnoreEnvVar = true;

      if (!ExistsInDB()) {
        errStr = "Table::Table(): table \'" + stName + "\' not found in database!";
        throw std::runtime_error(errStr);
      }
      
      Reset();
      fCol.clear();

      bool hasConn = fHasConnection;
      if (! fHasConnection) {
        GetConnection();
        hasConn = false;
      }

      std::vector<std::string> pkeyList;
      // get list of rows that are primary keys
      std::string cmd = "SELECT pg_attribute.attname, format_type(pg_attribute.atttypid, pg_attribute.atttypmod) FROM pg_index, pg_class, pg_attribute WHERE indrelid = pg_class.oid AND pg_attribute.attrelid = pg_class.oid AND pg_attribute.attnum = any(pg_index.indkey) AND indisprimary AND pg_class.oid = '" + stName + "'::regclass";

      PGresult* res = PQexec(fConnection,cmd.c_str());

      if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        errStr = "Table::Table(): command failed: " + std::string(PQerrorMessage(fConnection));
        if (fVerbosity > 0)
          std::cerr << errStr << std::endl;
        PQclear(res);
        CloseConnection();
        throw std::runtime_error(errStr);
      }

      int nRow = PQntuples(res);
      if (nRow == 0) {
        errStr = std::string("Table::Table(): no primary keys defined for table \'") + tableName + std::string("\', unable to proceed.");
        PQclear(res);
        fExistsInDB = false;
        CloseConnection();
        throw std::runtime_error(errStr);
      }

      for (int i=0; i<nRow; ++i) {
        std::string key = std::string(PQgetvalue(res,i,0));
        pkeyList.push_back(key);
      }

      PQclear(res);

      // now get names and types of all columns
      this->GetColsFromDB(pkeyList);

      if (!hasConn) CloseConnection();

      // now set the dB command cache file name
      std::string dirName;
      tmpStr = getenv("DBICACHEDIR");
      if (tmpStr)
        dirName = tmpStr;
      else {
        tmpStr = getenv("PWD");
        if (tmpStr)
          dirName = tmpStr;
        else
          dirName = "/";
      }

      fDBCacheFile = dirName + "/" + "." + Name() + ".cache";

    }

    //************************************************************

    Table::~Table()
    {
      this->Clear();
      if (fHasConnection) CloseConnection();
    }

    //************************************************************

    bool Table::GetColsFromDB(std::vector<std::string> pkeyList)
    {
      if (fTableType == kUnstructuredConditionsTable) {
	std::cerr << "Table::GetColsFromDB() currently disabled for unstructured conditions tables." << std::endl;
	abort();
      }
      
      bool hasConn = fHasConnection;
      if (! fHasConnection) {
        GetConnection();
        hasConn = false;
      }
      
      // now get names and types of all columns
      std::string cmd = "SELECT column_name, data_type from information_schema.columns where table_name = \'" + std::string(fTableName);
      if (fTableType == kConditionsTable)
	cmd += "_update";
      cmd += "\' and table_schema=\'" + fSchema + "\'";

      PGresult* res = PQexec(fConnection,cmd.c_str());

      if (PQresultStatus(res) != PGRES_TUPLES_OK) {
	std::string errStr = "Table::Table() command failed: " + std::string(PQerrorMessage(fConnection));
        if (fVerbosity > 0)
          std::cerr << errStr << std::endl;
        PQclear(res);
        fExistsInDB = false;  // set this to false just in case
        CloseConnection();
        throw std::runtime_error(errStr);
      }

      int nRow = PQntuples(res);

      for (int i=0; i<nRow; ++i) {	
        std::string cname = std::string(PQgetvalue(res,i,0));
        std::string ctype = std::string(PQgetvalue(res,i,1));

	if (fTableType == kConditionsTable) {	  
	  if (cname == "__snapshot_id") continue;
	  if (cname == "__tr") continue;
	  if (cname == "__channel") continue; //cname = "channel";
	  if (cname == "__tv") continue; //cname = "tv";
	}
	
	if (ctype == "smallint") ctype="short";
        else if (ctype == "double precision") ctype="double";
        else if (ctype == "boolean") ctype="bool";
        else if (ctype == "timestamp without time zone") ctype="timestamp";
        else if (ctype.substr(0,7) == "varchar" || ctype == "text")
          ctype = "text"; //varchar" + ctype.substr(8,ctype.find(')')-1);
	
        // check if this column is "auto_incr", only if !conditions table
        if (fTableType != kConditionsTable && ctype == "integer") {
	  std::string stName = fSchema + std::string(".") + std::string(fTableName);
          cmd = "SELECT pg_get_serial_sequence(\'" + stName +
            "\',\'" + cname + "\')";
          PGresult* res2 = PQexec(fConnection,cmd.c_str());
          int nRow2 = PQntuples(res2);
          for (int j=0; j<nRow2; ++j) {
            std::string tStr = std::string(PQgetvalue(res2,j,0));
            if (tStr != "") ctype = "auto_incr";
          }
          PQclear(res2);
        }
	
        // now create Column based on this info
	ColumnDef cdef(cname,ctype);

        if (find(pkeyList.begin(),pkeyList.end(),cname) != pkeyList.end()) {
	  cdef.SetCanBeNull(false);
        }
        fCol.insert(fCol.begin(),cdef);

        if (cname == "inserttime") addInsertTime = true;
        if (cname == "insertuser") addInsertUser = true;
        if (cname == "updatetime") addUpdateTime = true;
        if (cname == "updateuser") addUpdateUser = true;
      }

      PQclear(res);

      if (!hasConn) CloseConnection();

      return true;
    }
    
    //************************************************************

    int Table::AddCol(std::string cname, std::string ctype)
    {
      for (size_t i=0; i<fCol.size(); ++i) {
	if (fCol[i].Name() == cname) {
	  std::cerr << "Table::AddCol: column \'" << cname << "\' already exists!  Fatal, aborting..." << std::endl;
	  abort();
	}
      }

      ColumnDef cdef(cname,ctype);
      
      fCol.push_back(cdef);
      
      if (cname == "inserttime") addInsertTime = true;
      if (cname == "insertuser") addInsertUser = true;
      if (cname == "updatetime") addUpdateTime = true;
      if (cname == "updateuser") addUpdateUser = true;
      
      return fCol.size()-1;
    }
    
    //************************************************************

    void Table::AddRow(const Row* row)
    {
      if (!row) return;

      Row r2(*row);

      for (unsigned int i=0; i<fCol.size(); ++i) {
        if (fCol[i].Name() == "inserttime" ||
            fCol[i].Name() == "insertuser" ||
            fCol[i].Name() == "updatetime" ||
            fCol[i].Name() == "updateuser" ) continue;
        if (!fCol[i].CanBeNull())
          if (r2.Col(i).IsNull())
            fNullList.push_back(std::pair<int,int>(fRow.size(),i));
      }

      fRow.push_back(r2);

    }

    //************************************************************

    void Table::AddEmptyRows(unsigned int nrow)
    {
      Row* row = this->NewRow();

      fRow.resize(fRow.size()+nrow,*row);
    }

    //************************************************************
    void Table::AddRow(const Row& row)
    {
      AddRow(&row);
    }

    //************************************************************
    bool Table::RemoveRow(int i)
    {
      if (i < 0) return false;

      unsigned int j = i;

      if (j >= fRow.size()) return false;

      unsigned int kEnd = fNullList.size();
      for (unsigned int k=0; k<kEnd; ++k)
        if (fNullList[k].first == i) {
          fNullList.erase(fNullList.begin()+k);
          kEnd = fNullList.size();
        }

      fRow.erase(fRow.begin()+j);

      return true;
    }

    //************************************************************
    Row* const Table::GetRow(int i)
    {
      if (i >= 0 && i < (int)fRow.size())
        return &fRow[i];
      else
        return 0;
    }

    //************************************************************
    bool Table::CheckForNulls()
    {
      bool isOk = fNullList.empty();

      if (!isOk) // print out list of null columns
        for (unsigned int i=0; i<fNullList.size(); ++i)
          if (fVerbosity>0)
            std::cerr << fCol[fNullList[i].second].Name() << " is NULL in row "
                      << fNullList[i].first << std::endl;

      return isOk;

    }

    //************************************************************

    void Table::CacheDBCommand(std::string cmd)
    {
      std::ofstream fout;
      fout.open(fDBCacheFile.c_str(),std::ios_base::app);

      fout << cmd << std::endl;

      fout.close();
    }

    //************************************************************

    void Table::SetRecordTime(float t)
    {
      fRecordTime = t;
      fHasRecordTime = true;
    }

    //************************************************************

    bool Table::SetTableType(int t)
    {
      if (t < 0 || t >= kNTableType) return false;
      fTableType = t;

      return true;
    }

    //************************************************************
    void Table::Reset()
    {
      fConnection = 0;
      fHasConnection = 0;
      fPKeyList.clear();
      fDistinctCol.clear();
      fVerbosity = 0;
      fDescOrder = true;
      fSelectLimit = 0;
      fSelectOffset = 0;
      ClearValidity();
      fMinChannel = 0;
      fMaxChannel = 0;
      fExcludeCol.clear();
    }

    //************************************************************
    void Table::ClearValidity()
    {
      fValidityStart.clear();
      fValidityEnd.clear();
      fValiditySQL = "";
      fValidityChanged=true;
    }

    //************************************************************
    void Table::PrintPQErrorMsg() const
    {
      if (fConnection)
        std::cerr << PQerrorMessage(fConnection) << std::endl;
    }

    //************************************************************
    bool Table::SetDetector(std::string det)
    {
      fDetector = det;

      if (fTableType != kHardwareTable)
        fSchema = det;
      else
        fSchema = "public";

      boost::to_lower(fSchema);

      return true;
    }

    //************************************************************
    bool Table::GetDetector(std::string& det) const
    {
      if (fDetector == "") return false;
      det = fDetector;

      return true;
    }

    //************************************************************
    void Table::SetTableName(std::string tname) {
      boost::to_lower(tname);
      fTableName = tname;
    }

    //************************************************************
    void Table::SetTableName(const char* tname) {
      std::string tnameStr = tname;
      boost::to_lower(tnameStr);
      fTableName = tnameStr;
    }

    //************************************************************
    void Table::SetDataSource(std::string ds) {
      if (ds == std::string("DAQ"))
	this->SetDataSource(nutools::dbi::kDAQ);
      else if (ds == std::string("DCS"))
	this->SetDataSource(nutools::dbi::kDCS);
      else if (ds == std::string("Offline"))
	this->SetDataSource(nutools::dbi::kOffline);
      else
	this->SetDataSource(nutools::dbi::kUnknownSource);
    }

    //************************************************************
    void Table::SetDataSource(int ids) {
      if (ids >= 0 && ids < nutools::dbi::kNDataSources)
	fDataSource = ids;
      else
	fDataSource = nutools::dbi::kUnknownSource;
    }

    //************************************************************
    void Table::SetDBInfo(std::string name, std::string host, std::string port,
                          std::string user)
    {
      SetDBName(name);
      SetDBHost(host);
      SetDBPort(port);
      SetUser(user);
    }

    //************************************************************
    void Table::SetDBInfo(const char* name, const char* host, const char* port,
                          const char* user)
    {
      SetDBName(name);
      SetDBHost(host);
      SetDBPort(port);
      SetUser(user);
    }

    //************************************************************
    const nutools::dbi::ColumnDef* Table::GetCol(std::string& cname)
    {
      unsigned int i=0;
      for ( ; i < fCol.size(); ++i)
        if (fCol[i].Name() == cname) break;

      if (i >= fCol.size()) return 0;

      return &fCol[i];
    }

    //************************************************************
    int Table::GetColIndex(std::string cname) 
    {
      for (unsigned int i=0; i<fCol.size(); ++i)
	if (fCol[i].Name() == cname) return (int)i;
      std::cerr << "No such column \"" << cname << "\". Returning -1" << std::endl;
      return -1;
    }

    //************************************************************
    std::map<std::string,int> Table::GetColNameToIndexMap()
    {
      std::map<std::string,int> tmap;
      for (unsigned int i=0; i<fCol.size(); ++i) {
	tmap[fCol[i].Name()] = int(i);
      }
      return tmap;
    }

    //************************************************************
    std::vector<std::string> Table::GetColNames()
    {
      std::vector<std::string> nameList;

      for (unsigned int i=0; i<fCol.size(); ++i)
        nameList.push_back(fCol[i].Name());

      return nameList;
    }

    //************************************************************
    void Table::SetTolerance(std::string& cname, float t)
    {
      unsigned int i=0;
      for ( ; i < fCol.size(); ++i)
        if (fCol[i].Name() == cname) break;

      if (i >= fCol.size()) return;

      fCol[i].SetTolerance(t);

    }

    //************************************************************
    float Table::Tolerance(std::string& cname)
    {
      unsigned int i=0;
      for ( ; i < fCol.size(); ++i)
        if (fCol[i].Name() == cname) break;

      if (i >= fCol.size()) return 0.;

      return fCol[i].Tolerance();

    }

    //************************************************************
    void Table::PrintColumns()
    {
      std::cout << std::endl;
      int i = 0;
      std::vector<int> len(0);
      int tlen;
      int sumlen = 0;

      for ( ; i<this->NCol(); ++i) {
        tlen = this->GetCol(i)->Name().length();
        if ((int)this->GetCol(i)->Type().length() > tlen)
          tlen = this->GetCol(i)->Type().length();
        len.push_back(tlen);
        sumlen += tlen;
      }

      int nsp = 0;
      i = 0;
      int j = 0;
      while (i < this->NCol()) {
        for ( ; i<this->NCol() && nsp<78; ++i)
          nsp += len[i] + 1;

        for (int k=0; k<nsp; ++k) {
          std::cout << "_" ;
        }
        std::cout << std::endl;

        int j_save = j;
        for (; j<i; ++j)
          std::cout << "|" << std::setw(len[j]) << std::left << this->GetCol(j)->Name();
        std::cout << "|" << std::endl;

        for (int k=0; k<nsp; ++k) {
          std::cout << "-" ;
        }
        std::cout << std::endl;

        j = j_save;
        for ( ; j<i; ++j)
          std::cout << "|" << std::setw(len[j]) << std::left << this->GetCol(j)->Type();
        std::cout << "|" << std::endl;

        for (int k=0; k<nsp; ++k) {
          std::cout << "-" ;
        }
        std::cout << std::endl;

        nsp = 0;
      }

    }

    //************************************************************
    bool Table::GetConnectionInfo(int ntry)
    {
      char* tmpStr;
      char hname[256];

      if (!fIgnoreEnvVar) {
        // now check environment variables, which will override any previous settings
	if (ntry == 0) {
	  tmpStr = getenv("DBIHOST");
	  if (tmpStr)
	    fDBHost = tmpStr;
	}
	else {
	  sprintf(hname,"DBIHOST%d",ntry);
	  tmpStr = getenv(hname);	    
	  if (tmpStr) {
	    std::cerr << "Switching to " << tmpStr << std::endl;
	    fDBHost = tmpStr;
	  }
	  else 
	    return false;
	}
	
	tmpStr = getenv("DBINAME");
	if (tmpStr)
	  fDBName = tmpStr;
	tmpStr = getenv("DBIPORT");
	if (tmpStr)
	  fDBPort = tmpStr;
	tmpStr = getenv("DBIUSER");
	if (tmpStr)
	  fUser = tmpStr;
      }

      if (fUser == "") {
        tmpStr = getenv("USER");
        if (tmpStr) {
          fUser = tmpStr;
          std::cerr << "Table::GetConnectionInfo: DB User undefined.  Setting to \""
                    << fUser << "\"" << std::endl;
        }	
        else {
	  throw std::runtime_error("Table::GetConnectionInfo: DB USER undefined.");
        }
	
      }
      if (fDBHost == "") {
        throw std::runtime_error("Table::GetConnectionInfo: DB HOST undefined.");
      }
      if (fDBName == "") {
        throw std::runtime_error("Table::GetConnectionInfo: DB NAME undefined.");
      }

      /*      if (fDBPort == "")
        if (fTable->hasDbPort())
          fDBPort = fTable->getDbPort();
      */
      return true;
    }
    //************************************************************
    bool Table::GetConnection(int ntry)
    {
      if (fIgnoreDB) return false;

      bool gotConnInfo = false;
      try {
        gotConnInfo = GetConnectionInfo(ntry);
      }
      catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return false;
      }

      if (!gotConnInfo) return false;

      // now get the password file name for read-only access

      if (Util::RunningOnGrid()) {
	char* tmpStr = getenv("DBIGRIDPWDFILE");
	if(tmpStr){
	  if(!this->SetPasswordFile(tmpStr)){
	    return false;
	  }
	}	
      }
      else {	
	char* tmpStr = getenv("DBIPWDFILE");
	if(tmpStr){
	  if(!this->SetPasswordFile(tmpStr)) {
	    return false;
	  }
        }
      }

      if (!fConnection) {
        std::string cmd = "dbname = " + fDBName + " host = " + fDBHost + " user = " + fUser;
        if (fDBPort != "")
          cmd += " port = " + fDBPort;

        if (fPassword != "")
          cmd += " password = " + fPassword;

        fConnection = PQconnectdb(cmd.c_str());

        int nTry=0;
        int sleepTime = 2;
	time_t t0 = time(NULL);
	time_t t1 = t0;

        while (PQstatus(fConnection) != CONNECTION_OK &&
               ((t1-t0) < fConnectionTimeout) ) { 
          std::cerr << "Connection to " << fDBHost << ":"
                    << fDBName << " failed: "
                    << PQerrorMessage(fConnection) << std::endl;
	  
          CloseConnection();	  
          sleepTime = 1 + ((double)random()/(double)RAND_MAX)*(1 << nTry++);
          sleep(sleepTime);
	  t1 = time(NULL);
	  fConnection = PQconnectdb(cmd.c_str());
        }
        if (PQstatus(fConnection) != CONNECTION_OK) {
	  CloseConnection();
	  if (! GetConnection(ntry+1)) {
	    std::cerr << "Too many attempts to connect to the database, " 
		      << ", giving up." << std::endl;
	    CloseConnection();
	    return false;
	  }
	}
        fHasConnection = true;
	if (fVerbosity > 0)
	  std::cout << "Got new connection" << std::endl;
      }
      
      return true;
    }
    //************************************************************
    bool Table::CloseConnection()
    {
      if (fConnection) {
        PQfinish(fConnection);
        if (fVerbosity > 0)
          std::cout << "Closed connection" << std::endl;
      }

      fConnection = 0;
      fHasConnection = false;

      return true;
    }

    //************************************************************
    bool Table::SetRole(std::string role)
    {
      fRole = role;
      return true;
    }

    //************************************************************
    bool Table::SetPasswordFile(const char* fname)
    {
      std::string fNameStr = "";

      if (fname == 0) {
        char* tmpStr = getenv("DBIPWDFILE");
        if (tmpStr)
          fNameStr = tmpStr;
        else {
          std::cerr << "DBIPWDFILE env. variable is not set, disabling "
                    << "password-access to the dB." << std::endl;
          fPassword = "";
          return false;
        }
      }
      else
        fNameStr = fname;

      std::ifstream fin;
      fin.open(fNameStr.c_str());
      if (!fin.is_open() || !fin.good()) {
        std::cerr << "Could not open password file " << fNameStr
                  << ".  Disabling password-access to the dB." << std::endl;
        return false;
      }
      else {
        fin >> fPassword;
        fin.close();
      }

      return true;
    }

    //************************************************************
    bool Table::ExistsInDB()
    {
      if (fIgnoreDB) return false;

      if (fTestedExists) return fExistsInDB;

      std::string tname = this->Name();

      fTestedExists = true;

      bool hasConn = fHasConnection;
      if (! fHasConnection) {
        GetConnection();
        hasConn = false;
      }

      std::string cmd = "SELECT tablename FROM pg_tables WHERE schemaname=\'" +
        fSchema + "\'";
      //  std::cout << tname << ": " << cmd << std::endl;
      PGresult* res = PQexec(fConnection,cmd.c_str());

      if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (fVerbosity > 0)
          std::cerr << "Table::ExistsInDB command failed: "
                    << PQerrorMessage(fConnection) << std::endl;
        PQclear(res);
        fExistsInDB = false;  // set this to false just in case
        CloseConnection();
        return false;
      }

      bool retVal = false;
      int nRow = PQntuples(res);

      int tc=0;
      std::vector<std::string> tList;
      tList.push_back(tname+std::string("_snapshot"));
      tList.push_back(tname+std::string("_snapshot_data"));
      tList.push_back(tname+std::string("_tag"));
      tList.push_back(tname+std::string("_tag_snapshot"));
      tList.push_back(tname+std::string("_update"));

      for (int i=0; i<nRow; ++i) {
        //    std::cout << string(PQgetvalue(res,i,0)) << std::endl;
        std::string tStr = std::string(PQgetvalue(res,i,0));

        if (fTableType != kConditionsTable) {
          if (tStr == tname) {
            retVal = true;
            break;
          }
        }
        else {
          if (std::string(PQgetvalue(res,i,0)) == tList[0] ||
              std::string(PQgetvalue(res,i,0)) == tList[1] ||
              std::string(PQgetvalue(res,i,0)) == tList[2] ||
              std::string(PQgetvalue(res,i,0)) == tList[3] ||
              std::string(PQgetvalue(res,i,0)) == tList[4] )
            ++tc;

          if (tc == 5) {
            retVal = true;
            break;
          }
        }
      }

      PQclear(res);

      if (!hasConn) CloseConnection();

      fExistsInDB = true;
      return retVal;

    }

    //************************************************************
    bool Table::GetCurrSeqVal(std::string col, long& iseq)
    {
      if (fIgnoreDB) return false;

      bool hasConn = fHasConnection;
      if (! fHasConnection) {
        GetConnection();
        hasConn = false;
      }

      // now get current sequence value:

      std::string cmd = "SELECT last_value FROM ";
      cmd += Schema() + "." + Name();
      cmd += "_" + col + "_seq";

      if (fVerbosity > 0)
        std::cerr << "Table::GetCurrSeqVal: Executing PGSQL command: \n\t"
                  << cmd << std::endl;

      PGresult* res = PQexec(fConnection, cmd.c_str());
      if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (fVerbosity > 0)
          std::cerr << "SELECT failed: " << PQerrorMessage(fConnection) << std::endl;
        PQclear(res);
        return false;
      }

      // check that the number of columns is consistent
      if (PQnfields(res) != 1) {
        PQclear(res);
        return false;
      }

      // now cache rows
      int nRow = PQntuples(res);

      if (nRow != 1) {
        PQclear(res);
        return false;
      }

      if (! PQgetisnull(res,0,0)) {
        std::string vstr = PQgetvalue(res,0,0);
        try {
          iseq = boost::lexical_cast<long>(vstr);
        }
        catch (boost::bad_lexical_cast &) {
          PQclear(res);
          return false;
        }
      }

      PQclear(res);

      if (!hasConn) CloseConnection();

      return true;
    }
    //************************************************************
    bool Table::ExecuteSQL(std::string cmd, PGresult*& res)
    {
      if (fIgnoreDB) return false;

      bool hasConn = fHasConnection;
      if (! fHasConnection) {
        GetConnection();
        hasConn = false;
      }

      if (!fConnection) {
        std::cerr << "Table::ExecuteSQL: No connection to the database!" << std::endl;
        return false;
      }

      if (cmd == "") return false;

      if (fVerbosity)
        std::cerr << "Executing SQL query: " << cmd << std::endl;

      boost::posix_time::ptime ctt1;
      boost::posix_time::ptime ctt2;
    
      if (fTimeQueries) {
	ctt1 = boost::posix_time::microsec_clock::local_time();
      }

      res = PQexec(fConnection,cmd.c_str());
      if (fTimeQueries) {
	ctt2 = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration tdiff = ctt2 - ctt1;
	std::cerr << "Table::ExecuteSQL(" << cmd << "): query took " 
		  << tdiff.total_milliseconds() << " ms" << std::endl;
      }
      
      // close connection to the dB if necessary
      if (! hasConn) CloseConnection();

      return (res != 0);
    }

    //************************************************************
    bool Table::LoadFromDB()
    {
      if (fIgnoreDB) return false;

      if (fSchema == "undef") {
        std::cerr << "Table::LoadFromDB: Detector not set!  Table::SetDetector()"
                  << " must be called first!" << std::endl;
        return false;
      }

      if (!fValidityChanged) return true;

      // make a connection to the dB if there isn't one already
      bool hasConn = fHasConnection;
      if (! fHasConnection) {
        GetConnection();
        hasConn = false;
      }

      if (!fConnection) {
        std::cerr << "Table::LoadFromDB: No connection to the database!" << std::endl;
        return false;
      }

      if (!ExistsInDB()) {
        std::cerr << "Table::LoadFromDB: Table \"" << Name()
                  << "\" not found in database!" << std::endl;
        CloseConnection();
        return false;
      }

      std::string cmd;
      cmd.clear();
      PGresult* res;

      std::ostringstream outs;
      outs << "BEGIN";
      res = PQexec(fConnection, outs.str().c_str());
      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "BEGIN command failed: " << PQerrorMessage(fConnection) << std::endl;
        PQclear(res);
        CloseConnection();
        return false;
      }

      PQclear(res);

      outs.str("");
      outs << "DECLARE myportal CURSOR FOR SELECT ";
      if (!fDistinctCol.empty()) {
        outs << "DISTINCT ON (";
        if (! fDistinctCol.empty()) {
          for (unsigned int i=0; i<fDistinctCol.size(); ++i) {
            outs << fDistinctCol[i]->Name();
            if (i<(fDistinctCol.size()-1)) outs << ", ";
          }
        }
        outs << ") ";
      }

      outs << "* from ";
      outs << Schema() << "." << Name();

      if (! fValidityStart.empty() || fValiditySQL != "" ) {
        outs << " WHERE " << fValiditySQL;
        if (fValiditySQL != "" && !fValidityStart.empty()) outs << " and ";

        for (unsigned int i=0; i<fValidityStart.size(); ++i) {
          bool isEqualTo = (fValidityStart[i].Value() == fValidityEnd[i].Value());
	  bool needsQuotes=false;
	  if (fValidityStart[i].Type() == "string" ||
	      fValidityStart[i].Type() == "text" ||
	      fValidityStart[i].Type() == "timestamp" ||
	      fValidityStart[i].Type() == "date") needsQuotes=true;

          outs << fValidityStart[i].Name();
          if (!isEqualTo)
            outs << ">=";
          else
            outs << "=";

	  if (needsQuotes) outs << "'";
          outs << fValidityStart[i].Value();
	  if (needsQuotes) outs << "'";
	  
          if (!isEqualTo) {
            outs << " and ";
            outs << fValidityEnd[i].Name() + "<=";
	    if (needsQuotes) outs << "'";
            outs << fValidityEnd[i].Value();
	    if (needsQuotes) outs << "'";
          }

          if (i < (fValidityStart.size()-1)) outs << " and ";
        }
      }

      if (!fDistinctCol.empty() || !fOrderCol.empty()) {
        outs << " ORDER BY ";

        if (!fDistinctCol.empty()) {
          for (unsigned int i=0; i<fDistinctCol.size(); ++i) {
            outs << fDistinctCol[i]->Name();
            if (i<(fDistinctCol.size()-1)) outs << ", ";
          }
        }

        if (!fOrderCol.empty()) {
          for (unsigned int i=0; i<fOrderCol.size(); ++i) {
            outs << fOrderCol[i]->Name();
            if (i<(fOrderCol.size()-1)) outs << ", ";
          }
        }

        if (fDescOrder)
          outs << " DESC";
        else
          outs << " ASC";
      }

      if (fSelectLimit>0) {
        outs << " LIMIT " << boost::lexical_cast<std::string>(fSelectLimit);
      }

      if (fSelectOffset>0) {
        outs << " OFFSET " << boost::lexical_cast<std::string>(fSelectOffset);
      }

      if (fVerbosity > 0)
        std::cerr << "Table::LoadFromDB: Executing PGSQL command: \n\t" << outs.str() << std::endl;
      res = PQexec(fConnection,outs.str().c_str());

      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "DECLARE CURSOR failed: " << PQerrorMessage(fConnection) << std::endl;
        PQclear(res);
        CloseConnection();
        return false;
      }
      PQclear(res);


      boost::posix_time::ptime ctt1;
      boost::posix_time::ptime ctt2;
    
      if (fTimeQueries) {
	ctt1 = boost::posix_time::microsec_clock::local_time();
      }

      res = PQexec(fConnection, "FETCH ALL in myportal");
      if (fTimeQueries) {
	ctt2 = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration tdiff = ctt2 - ctt1;
	std::cerr << "Table::LoadFromDB(" << Name() << "): query took " 
		  << tdiff.total_milliseconds() << " ms" << std::endl;
      }

      if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "FETCH ALL failed: %" << PQerrorMessage(fConnection) << std::endl;
        PQclear(res);
        CloseConnection();
        return false;
      }

      // now cache rows
      int nRow = PQntuples(res);
      if (fVerbosity>0)
	std::cerr << "Table::LoadFromDB(" << Name() << "): got " << nRow 
		  << " rows of data." << std::endl;

      if (fTimeParsing)
 	ctt1 = boost::posix_time::microsec_clock::local_time();

      if (nRow > 0) {
        std::vector<int> colMap(fCol.size());

        for (unsigned int i=0; i<fCol.size(); ++i) {
          colMap[i] = PQfnumber(res,fCol[i].Name().c_str());
        }

        int k;

        unsigned int ioff = fRow.size();
        AddEmptyRows(nRow);

        for (int i=0; i < nRow; i++) {
          for (unsigned int j=0; j < fCol.size(); j++) {
            k = colMap[j];
            if (k >= 0) {
              if (! PQgetisnull(res,i,k)) {
                std::string vstr = PQgetvalue(res,i,k);
                fRow[ioff+i].Col(j).FastSet(vstr);
              }
	      //              else
	      //                fRow[ioff+i].Col(j).FastSet("");
            }
          }
	  fRow[ioff+i].SetInDB();
        }
      }

      if (fTimeParsing) {
	ctt2 = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration tdiff = ctt2 - ctt1;
	std::cerr << "Table::LoadFromDB(" << Name() << "): parsing took " 
		  << tdiff.total_milliseconds() << " ms" << std::endl;
      }

      PQclear(res);

      /* close the portal ... we don't bother to check for errors ... */
      res = PQexec(fConnection, "CLOSE myportal");
      PQclear(res);

      /* end the transaction */
      res = PQexec(fConnection, "END");
      PQclear(res);

      // close connection to the dB if necessary
      if (! hasConn) CloseConnection();

      fValidityChanged = false;

      return true;
    }

    //************************************************************
    bool Table::LoadFromCSV(std::string fname)
    {
      std::cout << "Reading " << fname << std::endl;

      std::ifstream fin;
      fin.open(fname.c_str());
      if (!fin.is_open()) {
        std::cerr << "Could not open " << fname << std::endl;
        return false;
      }
      if (!fin.good()) {
	std::cerr << "Stream not good " << fname << std::endl;
	return false;
      }
      
      std::string s;
      
      char buff[256];
      std::string value;

      std::vector<int> colMap(fCol.size());
      for (unsigned int i=0; i<fCol.size(); ++i) {
        colMap[i] = int(i);
      }
      
      bool hasColNames = true;
      bool hasTols = true;

      int chanIdx=-1;
      int tvIdx=-1;
      int tvEndIdx=-1;

      // check first line to see if it is column names.  Should begin with a '#'
      std::getline(fin,s);
      if (s[0] == '#' || fTableType == kConditionsTable) {
        unsigned int ic=1;
	if (fTableType == kConditionsTable && s[0] != '#') ic=0;
        int k;

	int joff=0;
        for (int j=0; ic<s.length(); ++j) {
          k=0;

          while (s[ic++] != ',' && ic<s.length())
            buff[k++] = s[ic-1];

	  if (ic==s.length()) buff[k++] = s[s.length()-1];
          buff[k] = '\0';
          value = buff;

	  boost::algorithm::trim(value);
	  if (value == "channel") { chanIdx=j; ++joff;}
	  else if (value == "tv") { tvIdx=j; ++joff;}
	  else if (value == "tvend") { tvEndIdx=j; ++joff;}
	  else {
	    for (unsigned int jc=0; jc<fCol.size(); ++jc)
	      if (fCol[jc].Name() == value) {
		colMap[j-joff] = jc;
		break;
	      }
	  }
        }

      }
      else {
        hasColNames = false;
        fin.clear();
        fin.seekg(0);
      }

      // now check for tolerances
      std::getline(fin,s);
      if (fTableType == kConditionsTable && s.substr(0,10) == "tolerance,") {
        unsigned int ic=11;
        int k;

	int joff=0;
        for (int j=0; ic<s.length(); ++j) {
          k=0;
          while (s[ic] != ',' && ic<s.length())
            buff[k++] = s[ic++];
	  //          if (ic==s.length()) buff[k++] = s[s.length()-1];
	  ++ic;
	  if (ic < s.length())
	    buff[k] = '\0';
	  else {
	    buff[k] = '\0';
	      }
          value = buff;
	  if (value.length() > 0) {
	    if (j==chanIdx || j==tvIdx || j==tvEndIdx) 
	      ++joff;
	    else 
	      fCol[colMap[j-joff]].SetTolerance(atof(buff));
	  }
        }	
      }
      else {
	hasTols = false;
        fin.clear();
        fin.seekg(0);
      }

      Row* r = NewRow();

      int nRow=0;
      unsigned int ioff=fRow.size();
      unsigned int irow=0;
      fin.clear();
      fin.seekg(0);
      while (std::getline(fin,s))
        ++nRow;
      fin.clear();
      fin.seekg(0);
      if (hasColNames) {
	--nRow;
        std::getline(fin,s);
      }
      if (hasTols) {
	--nRow;
	std::getline(fin,s);
      }

      if (nRow<=0) {
	std::cout << "Table::LoadFromCSV() found no rows in "
		  << fname << std::endl;
	return false;
      }

      AddEmptyRows(nRow);
      std::cout << "Added " << nRow << " empty rows" << std::endl;

      for (int jrow=0; jrow<nRow; ++jrow) {
	std::getline(fin,s);
	
        unsigned int ic=0;
        int k;
        bool hasX;
	int joff=0;
        for (int j=0; ic<s.length(); ++j) {
          k=0;
          hasX=false;
          while (s[ic++] != ',' && ic<s.length()) {
            buff[k++] = s[ic-1];
            if (buff[k-1] == 'x') hasX=true;
          }
          if (ic==s.length()) buff[k++] = s[s.length()-1];
          buff[k] = '\0';
          value = buff;

	  if (j==chanIdx) {
	    fRow[ioff+irow].SetChannel(strtoull(buff,NULL,10));
	    ++joff;
	  }
	  else if (j==tvIdx) {
	    fRow[ioff+irow].SetVldTime(strtoull(buff,NULL,10));
	    ++joff;
	  }
	  else if (j==tvEndIdx) {
	    fRow[ioff+irow].SetVldTimeEnd(strtoull(buff,NULL,10));
	    ++joff;
	  }
          else {
	    if (hasX) {
	      if (fCol[j-joff].Type() == "bigint" ||
		  fCol[j-joff].Type() == "long") {
		try {
		  std::istringstream iss(value);
		  uint64_t ulongValue;
		  iss >> std::hex >> ulongValue;
		  int64_t longValue = (int64_t) ulongValue;
		  value = boost::lexical_cast<std::string>(longValue);
		}
		catch (...) {
		  // simply let "value" remain unchanged
		}
	      }
	      else if (fCol[j-joff].Type() == "int") {
		try {
		  std::istringstream iss(value);
		  uint32_t uintValue;
		  iss >> std::hex >> uintValue;
		  int32_t intValue = (int32_t) uintValue;
		  value = boost::lexical_cast<std::string>(intValue);
		}
		catch (...) {
		  // simply let "value" remain unchanged
		}
	      }
	      else if (fCol[j-joff].Type() == "short") {
		try {
		  std::istringstream iss(value);
		  uint16_t ushortValue;
		  iss >> std::hex >> ushortValue;
		  int16_t shortValue = (int16_t) ushortValue;
		  value = boost::lexical_cast<std::string>(shortValue);
		}
		catch (...) {
		  // simply let "value" remain unchanged
		}
	      }	      
	    } // if (hasX)
	    if (fCol[j-joff].Type() == "text") {
	      	  boost::algorithm::trim(value);
		  if ((value[0] == '"' && value[value.length()-1] == '"') ||
		      (value[0] == '\'' && value[value.length()-1] == '\''))
		    value = value.substr(1,value.length()-2);
	    }
	    fRow[ioff+irow].Col(colMap[j-joff]).FastSet(value);
	  } // else not a validity channel or time
	}

	fRow[ioff+irow].SetInDB();
        ++irow;
      }
      delete r;

      fin.close();

      return true;
    }

    //************************************************************

    int Table::ParseSelfStatusLine(char* line)
    {
      int i = strlen(line);
      while (*line < '0' || *line > '9') line++;
      line[i-3] = '\0';
      i = atoi(line);
      return i;
    }
    

    //************************************************************

    void Table::PrintVMUsed(){ //Note: this value is in MB!
      FILE* file = fopen("/proc/self/status", "r");
      int result = -1;
      char line[128];
    

      while (fgets(line, 128, file) != NULL){
	if (strncmp(line, "VmSize:", 7) == 0){
	  result = this->ParseSelfStatusLine(line);
	  break;
	}
      }
      fclose(file);
      std::cerr << Schema() << "." << Name() << ": this process using " 
		<< result/1024 << " MB of VirtualMemory" << std::endl;
    }

    //************************************************************

    void Table::PrintPMUsed(){ //Note: this value is in MB!
      FILE* file = fopen("/proc/self/status", "r");
      int result = -1;
      char line[128];
    

      while (fgets(line, 128, file) != NULL){
	if (strncmp(line, "VmRSS:", 6) == 0){
	  result = this->ParseSelfStatusLine(line);
	  break;
	}
      }
      fclose(file);
      std::cerr << Schema() << "." << Name() << ": this process using " 
		<< result/1024 << " MB of PhysicalMemory" << std::endl;
    }

    //************************************************************
    
    bool Table::GetDataFromWebService(Dataset& ds, std::string myss)
    {
      Tuple tu;
      char ss[1024]; 
      char ss2[1024]; 
      int wda_err, err;
      std::vector<int> colMap(fCol.size());
      std::vector<bool> isString(fCol.size());
      std::vector<bool> isKnownField(fCol.size());
      
      const char* uagent = NULL;

      if(fVerbosity > 0)
	std::cout << "DBWeb query: " << myss << std::endl;
      
      boost::posix_time::ptime ctt1;
      boost::posix_time::ptime ctt2;
      
      if (fTimeQueries) {
	ctt1 = boost::posix_time::microsec_clock::local_time();
      }
      
      ds = getDataWithTimeout(myss.c_str(), uagent, 
			      fConnectionTimeout, &wda_err);
      
      if (fTimeQueries) {
	ctt2 = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration tdiff = ctt2 - ctt1;
	std::cerr << "Table::Load(" << Name() << "): query took " 
		  << tdiff.total_milliseconds() << " ms" << std::endl;
      }

      int httpStatus = getHTTPstatus(ds);

      if (httpStatus == 504) {
        int nTry=0;
        int sleepTime = 2;
	time_t t0 = time(NULL);
	time_t t1 = t0;

        while (httpStatus == 504 && ((t1-t0) < fConnectionTimeout) ) { 
          sleepTime = 1 + ((double)random()/(double)RAND_MAX)*(1 << nTry++);

          std::cerr << "Table::Load() for " << Name() 
		    << " failed with error 504, retrying in " << sleepTime 
		    << " seconds." << std::endl;
	  
          sleep(sleepTime);
	  t1 = time(NULL);
	  if (fTimeQueries) 
	    ctt1 = boost::posix_time::microsec_clock::local_time();
	  
	  ds = getDataWithTimeout(myss.c_str(), uagent,
				  fConnectionTimeout, &wda_err);
	  
	  if (fTimeQueries) {
	    ctt2 = boost::posix_time::microsec_clock::local_time();
	    boost::posix_time::time_duration tdiff = ctt2 - ctt1;
	    std::cerr << "Table::Load(" << Name() << "): query took " 
		      << tdiff.total_milliseconds() << " ms" << std::endl;
	  }
	  httpStatus = getHTTPstatus(ds);
        }
      }

      if (httpStatus != 200) {
	std::cerr << "Table::Load: Web Service returned HTTP status " 
		  << httpStatus << ": " << getHTTPmessage(ds) << std::endl;
	return false;
      }

      if (fTimeParsing)
	ctt1 = boost::posix_time::microsec_clock::local_time();

      int ntup = getNtuples(ds);

      // Getting no rows back can be legitimate
      if(ntup == 0){
	if(fVerbosity > 0)
	  std::cout << "Got zero rows from database. Is that expected?" << std::endl;

	fRow.clear();

	return true;
      }

      if(fVerbosity > 0)
	std::cout << "Got " << ntup-1 << " rows from database" << std::endl;

      int ioff=fRow.size();

      AddEmptyRows(ntup);

      tu = getFirstTuple(ds);
      if (tu == NULL) {
	std::cerr << "Table::Load(" << Name() << ") has NULL first tuple!"
		  << std::endl;
	return false;
      }
      int ncol2 = getNfields(tu);
      std::string chanStr = "channel";
      std::string tvStr = "tv";
      std::string tvEndStr = "tvend";
      int chanIdx=-1;
      int tvIdx=-1;
      int tvEndIdx=-1;
      for (int i=0; i<ncol2; ++i) {
	getStringValue(tu,i,ss,sizeof(ss),&err);
	if (chanStr == ss)  { chanIdx=i;  continue;}
	if (tvStr == ss)    { tvIdx=i;    continue;}
	if (tvEndStr == ss) { tvEndIdx=i; continue;}

	bool foundMatch=false;
	for (unsigned int icol=0; icol<fCol.size(); ++icol) {
	  if (fCol[icol].Name() == ss) {
	    colMap[i] = icol;
	    isString[i] = false;
	    if (fCol[icol].Type() == "string" || fCol[icol].Type() == "text")
	      isString[i] = true;
	    foundMatch=true;
	    break;
	  }
	}
	if (!foundMatch) // this means this field was unexpected, so
	                 // ignore it downstream
	  isKnownField[i] = false;
	else
	  isKnownField[i] = true;	  
      }
      
      releaseTuple(tu);
      tu = getNextTuple(ds);
      int irow=0;
      while (tu != NULL) {
	for (int i=0; i<ncol2; ++i) {	  
	  getStringValue(tu,i,ss,sizeof(ss),&err);
	  if (i == chanIdx) {
	    uint64_t chan = strtoull(ss,NULL,10);
	    fRow[ioff+irow].SetChannel(chan);
	    continue;
	  }
	  else if (i == tvIdx) {
	    float t1 = strtof(ss,NULL);
	    fRow[ioff+irow].SetVldTime(t1);
	  }
	  else if (i == tvEndIdx) {
	    float t1 = strtof(ss,NULL);
	    fRow[ioff+irow].SetVldTimeEnd(t1);	    
	  }
	  else {
	    if (isKnownField[i]) {
	      if (isString[i] && (ss[0]=='\'' || ss[0]=='\"')) { // remove quotes
		int k = strlen(ss);
		strncpy(ss2,&ss[1],k-2);
		ss2[k-2] = '\0';
		fRow[ioff+irow].Col(colMap[i]).FastSet(ss2);
	      }
	      else
		fRow[ioff+irow].Col(colMap[i]).FastSet(ss);
	    }
	  }
	}
	releaseTuple(tu);
	tu = getNextTuple(ds);
	++irow;
      };

      if (fTimeParsing) {
	ctt2 = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration tdiff = ctt2 - ctt1;
	std::cerr << "Table::Load(" << Name() << "): parsing took " 
		  << tdiff.total_milliseconds() << " ms" << std::endl;
      }

      // Make sure that the rows list is no longer than what we actually
      // filled. This happens because ntup above included the header row that
      // gives the column names.
      while(int(fRow.size()) > irow) fRow.pop_back();
	
      releaseDataset(ds);

      return true;
    }

    //************************************************************
    
    bool Table::LoadNonConditionsTable()
    {
      if (fQEURL == "") {
        std::cerr << "Table::LoadNonConditionsTable: Query Engine URL is not set!  using Table::LoadFromDB() instead." << std::endl;
        return LoadFromDB();
      }

      if (fValiditySQL != "") {
	std::cerr << "Table::LoadNonConditionsTable: pure SQL statements are not supported, using Table::LoadFromDB() instead." << std::endl;
	return LoadFromDB();
      }

      std::stringstream myss;

      myss << fQEURL << "query?t=" << Schema() << "." << Name() << "&";

      int ncol = this->NCol();

      myss << "&c=";
      int nc=0;
      for (int i=0; i<ncol; ++i) {
	std::string cname = this->GetCol(i)->Name();
	bool skipCol = false;
	for (size_t j=0; j<fExcludeCol.size(); ++j) {
	  if (fExcludeCol[j] == cname) {
	    skipCol = true;
	    break;
	  }
	}
	if (skipCol) continue;

	if (nc>0)
	  myss << ",";
        myss << cname;
	++nc;
      }
      
      if (! fValidityStart.empty()) {
	for (unsigned int i=0; i<fValidityStart.size(); ++i) {
	  if (fValidityStart[i].Type() == "string" ||
	      fValidityStart[i].Type() == "text" ||
	      fValidityStart[i].Type() == "timestamp" ||
	      fValidityStart[i].Type() == "date") {
	    std::cerr << "Table::LoadNonConditionsTable: validity strings are not supported, using Table::LoadFromDB() instead." << std::endl;
	    return LoadFromDB();	    
	  }
	  
	  myss << "&w=";	  
	  bool isEqualTo = (fValidityStart[i].Value() == fValidityEnd[i].Value());
	  if (isEqualTo) {
	    myss << fValidityStart[i].Name() << ":"
		 << fValidityStart[i].Value();
	  }
	  else {
	    myss << fValidityStart[i].Name() << ":ge:" 
		 << fValidityStart[i].Value() << "&w="
		 << fValidityEnd[i].Name() << ":le:"
		 << fValidityEnd[i].Value();
	  }

	}
      }

      if (!fOrderCol.empty()) {
	myss << "&o="; 
	if (fDescOrder)
	  myss << "-";
	for (unsigned int i=0; i<fOrderCol.size(); ++i) {
	  myss << fOrderCol[i]->Name();
	  if (i<(fOrderCol.size()-1)) myss << ", ";
	}
	
      }
      
      if (fSelectLimit>0)
	myss << "&l=" << fSelectLimit;

      if (fDisableCache) {
	if (fFlushCache)
	  myss << "&x=clear";
	else
	  myss << "&x=no";
      }

      Dataset ds;

      return GetDataFromWebService(ds,myss.str());

    }

    //************************************************************

    bool Table::LoadUnstructuredConditionsTable()
    {
      if (fMinTSVld == 0 || fMaxTSVld == 0) {
        std::cerr << "Table::LoadUnstructuredConditionsTable: No validity time is set!" << std::endl;
        return false;
      }

      if (fUConDBURL == "") {
        std::cerr << "Table::LoadConditionsTable: Web Service URL is not set!" << std::endl;
        return false;
      }

      if (!Util::RunningOnGrid()) {
	std::string interactiveURL = getenv("DBIUCONDBURLINT");
	if (!interactiveURL.empty())
	  fUConDBURL = interactiveURL;
      }

      //      int ncol = this->NCol();

      std::stringstream myss;

      myss << fUConDBURL << "get?folder=" << Folder() << "." << Name() << "&";

      
      return false;
    }
    
    //************************************************************

    bool Table::LoadConditionsTable()
    {
      if (fDataTypeMask == 0) {
        std::cerr << "Table::LoadConditionsTable: Data type mask is not set!" << std::endl;
        return false;
      }

      if (fMinTSVld == 0 || fMaxTSVld == 0) {
        std::cerr << "Table::LoadConditionsTable: No validity time is set!" << std::endl;
        return false;
      }

      if (fWSURL == "") {
        std::cerr << "Table::LoadConditionsTable: Web Service URL is not set!" << std::endl;
        return false;
      }

      if (!Util::RunningOnGrid()) {
	std::string interactiveURL = getenv("DBIWSURLINT");
	if (!interactiveURL.empty())
	  fWSURL = interactiveURL;
      }

      int ncol = this->NCol();

      std::stringstream myss;

      myss << fWSURL << "get?table=" << Schema() << "." << Name() << "&";

      if (fDataTypeMask > kNone) {
	myss << "type=";
	
	if ((fDataTypeMask & kMCOnly)) myss << "mc";
	if ((fDataTypeMask & kDataOnly)) myss << "data";
      
	myss << "&";
      }
      
      if (fMaxChannel > fMinChannel) {
	myss << "cr=" << fMinChannel << "-" << fMaxChannel << "&";
      }

      if (fValiditySQL != "") {
	myss << "where=" << fValiditySQL << "&";
      }

      if (fTag != "") myss << "tag=" << fTag << "&";

      //      char ts[256];
      
      if (fMinTSVld == fMaxTSVld) {
	//	sprintf(ts,"t=%" PRIu64,fMinTSVld);
	//        myss << ts; //"t=" << fMinTSVld;
        myss << "t=" << std::setprecision(12) << fMinTSVld;
      }
      else {
	//	sprintf(ts,"t0=%" PRIu64 "&t1=" PRIu64,fMinTSVld,fMaxTSVld);
	//        myss << ts; //"t0=" << fMinTSVld << "&t1=" << fMaxTSVld;
        myss << "t0=" << std::setprecision(12) << fMinTSVld << "&t1=" << std::setprecision(12) << fMaxTSVld;
      }

      if (fHasRecordTime) myss << "&rtime=" << fRecordTime;

      if (fFlushCache) myss << "&cache=flush";
      if (fDisableCache) myss << "&cache=no";

      myss << "&columns=";
      bool firstCol = true;
      for (int i=0; i<ncol; ++i) {
        std::string cName = this->GetCol(i)->Name();
	bool skipCol = false;
	for (size_t j=0; j<fExcludeCol.size(); ++j) 
	  if (fExcludeCol[j] == cName) {
	    skipCol = true;
	    break;
	  }
	if (skipCol) continue;
	if(!firstCol) myss << ",";
        myss << this->GetCol(i)->Name();
	firstCol = false;
      }

      //      std::cout << myss.str() << std::endl;
      Dataset ds;
      
      return GetDataFromWebService(ds,myss.str());

    }

    //************************************************************

    bool Table::Load()
    {      
      if (Util::RunningOnGrid()) {
	fConnectionTimeout = 1800;
      }
      if (fTableType==kConditionsTable) 
	return LoadConditionsTable();
      else if (fTableType==kUnstructuredConditionsTable) 
	return LoadUnstructuredConditionsTable();
      else
	return LoadNonConditionsTable();
    }

    //************************************************************
    // Create a look-up table of time-ordered validity rows based on
    // channel number
    //************************************************************

    void Table::FillChanRowMap()
    {
      nutools::dbi::Row* row;
      uint64_t chan;
      float tv;
      fChanRowMap.clear();

      for (int i=0; i<this->NRow(); ++i) {
        row = this->GetRow(i);
	chan = row->Channel();
	tv = row->VldTime();
	if (fChanRowMap[chan].empty())
	  fChanRowMap[chan].push_back(row);
	else {
	  bool wasInserted=false;
	  for (unsigned int j=0; j<fChanRowMap[chan].size(); ++j) {
	    if (tv < fChanRowMap[chan][j]->VldTime()) {
	      fChanRowMap[chan].insert(fChanRowMap[chan].begin()+j,row);
	      wasInserted=true;
	      break;
	    }
	  }
	  if (!wasInserted)
	    fChanRowMap[chan].push_back(row);
	}
      }

      fChannelVec.clear();
      std::unordered_map<uint64_t,std::vector<nutools::dbi::Row*> >::iterator itr = fChanRowMap.begin();
      std::unordered_map<uint64_t,std::vector<nutools::dbi::Row*> >::iterator itrEnd = fChanRowMap.end();

      for (; itr != itrEnd; ++itr) 
        fChannelVec.push_back(itr->first);


    }

    //************************************************************

    std::vector<nutools::dbi::Row*> Table::GetVldRows(uint64_t channel)
    {
      return fChanRowMap[channel];
    }

    //************************************************************

    nutools::dbi::Row* Table::GetVldRow(uint64_t channel, float t)
    {      
      std::vector<nutools::dbi::Row*>& rlist = fChanRowMap[channel];
      if (rlist.empty()) return 0;
      int irow=-1;
      float tv;
      // fChanRowMap is time-ordered, so this simplifies things
      unsigned int i=0;
      for ( ; i<rlist.size(); ++i) {
	tv = rlist[i]->VldTime();
	if (t >= tv) irow=i;
	else break;
      }
      if (irow>=0) return rlist[irow];
      return 0;
    }

    //************************************************************
    bool Table::Tag(std::string tn, bool override)
    {
      if (tn != "") fTag = tn;

      if (fTag == "") return false;

      std::stringstream myss;

      myss << fWSURL << "tag?table=" << Schema() << "." << Name() << "&";
      myss << "tag=" << fTag;
      if (override)
	myss << "&override=yes";

      int status;
      std::string pwd = GetPassword();

      postHTTPsigned(myss.str().c_str(), pwd.c_str(), NULL, 0, NULL, 0, &status);

      return (status==0);
    }

    //************************************************************
    bool Table::WriteToDB(bool commit)
    {
      if (! CheckForNulls()) return false;

      bool doWrite = ! fIgnoreDB;
      bool hasConn = fHasConnection;

      try {
        GetConnectionInfo();
      }
      catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return false;
      }

      // make a connection to the dB if one doesn't already exist
      if (doWrite) {
        if (! fHasConnection) {
          GetConnection();
          hasConn = false;
        }

        if (!fConnection) {
          std::cerr << "Table::WriteToDB: No connection to the database!" << std::endl;
          doWrite = false;
        }
        else {
          // now check that the table actually exists...
          if (!ExistsInDB()) {
            std::cerr << "Table::WriteToDB: Table does not exist in database!"
                      << std::endl;
            doWrite = false;
          }
        }
      }

      bool retVal = true;

      // get current timestamp:
      std::string ts = Util::GetCurrentTimeAsString();

      PGresult* res = PQexec(fConnection, "BEGIN");
      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "BEGIN command failed: " << PQerrorMessage(fConnection) << std::endl;
        PQclear(res);
        CloseConnection();
        return false;
      }

      PQclear(res);

      std::string cmd = "SET search_path TO " + fSchema;
      res = PQexec(fConnection, cmd.c_str());
      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "\'" << cmd << "\' command failed" << std::endl;
        PQclear(res);
        CloseConnection();
        return false;
      }
      PQclear(res);
      cmd.clear();

      std::map<std::string,int> colMap = GetColNameToIndexMap();
      int insertTimeIdx = colMap["inserttime"];
      int insertUserIdx = colMap["insertuser"];
      int updateTimeIdx = colMap["updatetime"];
      int updateUserIdx = colMap["updateuser"];
      // now create the INSERT command
      for (unsigned int i=0; i<fRow.size(); ++i) {
        // do an INSERT only if this entry does not already exists in the dB
        if (! fRow[i].InDB()) {
          Row r(fRow[i]);
          if (addInsertTime) r.Set(insertTimeIdx,ts);
          if (addInsertUser) r.Set(insertUserIdx,fUser);

          int nrowInsert = fCol.size();
          for (unsigned int j=0; j<fCol.size(); ++j) {
            if (fCol[j].Name() == "updatetime")
              nrowInsert--;
            else if (fCol[j].Name() == "updateuser")
              nrowInsert--;
            else if (fCol[j].Type() == "autoincr")
              nrowInsert--;
          }

          std::ostringstream outs;

          int ic=0;
          outs << "INSERT INTO " << Schema() << "." << Name() << " (";
          for (unsigned int j=0; j<fCol.size(); ++j) {
            if (fCol[j].Name() == "updatetime") continue;
            if (fCol[j].Name() == "updateuser") continue;
            if (fCol[j].Type() == "autoincr") continue;

            outs << fCol[j].Name();
            if (ic < nrowInsert-1) outs << ",";
            ++ic;
          }
          outs << ") VALUES (";

          ic = 0;
          for (unsigned int j=0; j<fCol.size(); ++j) {
            if (fCol[j].Name() == "updatetime") continue;
            if (fCol[j].Name() == "updateuser") continue;
            if (fCol[j].Type() == "autoincr") continue;

            outs << r.Col(j);

            if (ic < nrowInsert-1)  outs << ",";
            ++ic;
          }

          outs << ")";

          if (fVerbosity > 0)
            std::cerr << "Table::WriteToDB: Executing PGSQL command: \n\t"
                      << outs.str() << std::endl;

          if (!commit)
            std::cout << outs.str() << std::endl;
          else {
            if (doWrite) {
	      boost::posix_time::ptime ctt1;
	      boost::posix_time::ptime ctt2;
	      
	      if (fTimeQueries) 
		ctt1 = boost::posix_time::microsec_clock::local_time();
	      
              res = PQexec(fConnection, outs.str().c_str());
	      
	      if (fTimeQueries) {
		ctt2 = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration tdiff = ctt2 - ctt1;
		std::cerr << "Table::WriteToDB(" << Name() << "): query took " 
			  << tdiff.total_milliseconds() << " ms" << std::endl;
	      }
	      
              if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                CacheDBCommand(outs.str());
                std::cerr << "INSERT failed: " << PQerrorMessage(fConnection)
                          << std::endl;
                retVal = false;
              }
              else {
                fRow[i].SetInDB();
                // set insert columns
                if (addInsertTime) fRow[i].Col(insertTimeIdx).Set(ts);
                if (addInsertUser) fRow[i].Col(insertUserIdx).Set(fUser);
                // set autoincr columns
                long iseq;
                std::string seqstr;
                for (unsigned int j=0; j<fCol.size(); ++j) {
                  if (fCol[j].Type() == "autoincr") {
                    if (this->GetCurrSeqVal(fCol[j].Name(),iseq)) {
                      seqstr = boost::lexical_cast<std::string>(iseq);
                      fRow[i].Col(j).Set(seqstr,true);
                    }
                  }
                }
              }
              PQclear(res);
            }
            else
              CacheDBCommand(outs.str());
          }
        }
        else {
          if ( fRow[i].NModified() > 0 ) {
            Row r(fRow[i]);
            if (addUpdateTime) r.Update(updateTimeIdx,ts);
            if (addUpdateUser) r.Update(updateUserIdx,fUser);
            std::ostringstream outs;
            outs << "UPDATE " << Schema() << "." << Name() << " SET ";
            int im = 0;
            for (unsigned int j=0; j<fCol.size() && im < r.NModified(); ++j) {
              if (r.Col(j).Modified()) {
                outs << fCol[j].Name() + "=";
                outs << r.Col(j);
                ++im;
                if (im < r.NModified()) outs << ",";
              }
            }
            outs << " WHERE ";
            // now print out all pkey values
            int nkey = fPKeyList.size();
            for (int j=0; j<nkey; ++j) {
              std::string pkey = fPKeyList[j]->Name();
	      int pkeyIdx = colMap[pkey];
              outs << pkey << "=" << r.Col(pkeyIdx);
              if (j < (nkey-1)) outs << " and ";
            }

            if (fVerbosity > 0)
              std::cerr << "Table::WriteToDB: Executing PGSQL command: \n\t"
                        << outs.str() << std::endl;

            if (!commit)
              std::cout << outs.str() << std::endl;
            else {
              if (doWrite) {
                res = PQexec(fConnection, outs.str().c_str());
                if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                  CacheDBCommand(outs.str());
                  std::cerr << "UPDATE failed: " << PQerrorMessage(fConnection) << std::endl;
                  retVal = false;
                }
                else {
                  // set update columns
                  if (addUpdateTime) fRow[i].Col(updateTimeIdx).Set(ts);
                  if (addUpdateUser) fRow[i].Col(updateUserIdx).Set(fUser);
                }
                PQclear(res);
              }
              else
                CacheDBCommand(outs.str());
            }
          }
        }
      }

      res = PQexec(fConnection, "END");
      PQclear(res);

      // close connection to the dB if necessary
      if (! hasConn) CloseConnection();

      return retVal;
    }

    //************************************************************
    bool Table::MakeConditionsCSVString(std::stringstream& ss) 
    {
      int ncol = this->NCol();
      int nrow = this->NRow();

      ss << "channel,tv,";
      bool first = true;
      for (int i=0; i<ncol; ++i) {
        std::string cname = this->GetCol(i)->Name();
	if(!first) ss << ",";
	first = false;
        ss << cname;
      }
      ss << std::endl;
      
      ss << "tolerance,,";
      first = true;
      for (int j=0; j<ncol; ++j) {
        std::string cname = this->GetCol(j)->Name();
        std::string ctype = this->GetCol(j)->Type();
	if(!first) ss << ",";
	first = false;
        float tol = this->Tolerance(cname);
        if (tol == 0.) {
          if (ctype == "double")
            ss << "1.e-10";
          else if (ctype == "float")
            ss << "1.e-5";
        }
        else
          ss << this->Tolerance(cname);
      }
      ss << std::endl;
      for (int i=0; i<nrow; ++i) {
        ss << GetRow(i)->Channel() << ","
           << GetRow(i)->VldTime() << ",";
	if (GetRow(i)->VldTimeEnd() > GetRow(i)->VldTime())
	  ss << GetRow(i)->VldTimeEnd() << ",";
	first = true;
        for (int j=0; j<ncol; ++j) {
	  if(!first) ss << ",";
	  first = false;
	  ss << GetRow(i)->Col(j);
        }
        ss << std::endl;
      }

      return true;
    }

    //************************************************************
    bool Table::Write(bool commit)
    {
      if (fDataTypeMask == 0){
        std::cerr << "Table::Write: Data type mask is not set!" << std::endl;
        return false;
      }

      if (fWSURL == "") {
        std::cerr << "Table::Write: Web Service URL is not set!" << std::endl;
        return false;
      }

      if (!Util::RunningOnGrid()) {
	std::string putURL = getenv("DBIWSURLPUT");
	if (!putURL.empty())
	  fWSURL = putURL;
      }
      
      std::stringstream ss;

      MakeConditionsCSVString(ss);

      int status;
      std::string url = fWSURL + "put?table=" + Schema() + "." + Name();

      std::stringstream typeStr;
      typeStr << "&type=";
      if ((fDataTypeMask & kMCOnly)) typeStr << "mc";
      if ((fDataTypeMask & kDataOnly)) typeStr << "data";
      
      url += typeStr.str();

      // get web service password
      std::string pwd = GetPassword();

      boost::posix_time::ptime ctt1;
      boost::posix_time::ptime ctt2;
      
      if (fTimeQueries) 
	ctt1 = boost::posix_time::microsec_clock::local_time();
	      
      if (fVerbosity>0)
	std::cout << "Posting data to: " << url << std::endl;

      postHTTPsigned(url.c_str(), pwd.c_str(), NULL, 0,
                     ss.str().c_str(), ss.str().length(), &status);
      if (fTimeQueries) {
	ctt2 = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration tdiff = ctt2 - ctt1;
	std::cerr << "Table::Write(" << Name() << "): query took " 
		  << tdiff.total_milliseconds() << " ms" << std::endl;
      }
      return (status == 0);
    }

    //************************************************************
    bool Table::WriteToCSV(std::string fname, bool appendToFile,
                           bool writeColNames)
    {
      if (! CheckForNulls()) return false;
      
      std::ofstream fout;
      if (!appendToFile)
        fout.open(fname.c_str());
      else
        fout.open(fname.c_str(),std::ios_base::app);

      if (fTableType==kConditionsTable) {	
	std::stringstream ss;
	MakeConditionsCSVString(ss);
	fout << ss.str();
      }
      else {
	if (writeColNames) {
	  for (unsigned int j=0; j<fCol.size(); ++j) {
	    fout << fCol[j].Name();
	    if (j<fCol.size()-1) fout << ",";	  
	  }
	}
	
	for (unsigned int i=0; i<fRow.size(); ++i) {
	  for (unsigned int j=0; j<fCol.size(); ++j) {
	    fout << fRow[i].Col(j);
	    if (j<fCol.size()-1) fout << ",";
	  }
	  fout << std::endl;
	}
      }

      fout.close();

      return true;
    }

    //************************************************************
    void Table::RemoveValidityRange(std::string& cname)
    {
      unsigned int i=0;
      for (; i<fValidityStart.size(); ++i)
        if (fValidityStart[i].Name() == cname) {
          fValidityStart.erase(fValidityStart.begin()+i);
          fValidityEnd.erase(fValidityEnd.begin()+i);
        }
    }

    //************************************************************
    // Add the ith column in the table to the list of columns
    // that are to be "distinct"
    //************************************************************
    bool Table::AddDistinctColumn(unsigned int i)
    {
      if (i >= fCol.size()) return false;

      const ColumnDef* c = &fCol[i];
      for (unsigned int j=0; j<fDistinctCol.size(); ++j)
        if (fDistinctCol[j] == c) return true;

      fDistinctCol.push_back(c);

      return true;
    }

    //************************************************************
    // Remove the ith column in the table to the list of columns
    // that are to be "distinct"
    //************************************************************
    bool Table::RemoveDistinctColumn(unsigned int i)
    {
      if (i >= fCol.size()) return false;

      const ColumnDef* c = &fCol[i];
      for (unsigned int j=0; j<fDistinctCol.size(); ++j)
        if (fDistinctCol[j] == c) {
          fDistinctCol.erase(fDistinctCol.begin() + j);
          return true;
        }

      return false;
    }

    //************************************************************
    // Add the column with name "cname" in the table to the list
    // of columns that are to be "distinct"
    //************************************************************
    bool Table::AddDistinctColumn(std::string cname)
    {
      const ColumnDef* c = GetCol(cname);

      if (!c) return false;

      for (unsigned int j=0; j<fDistinctCol.size(); ++j)
        if (fDistinctCol[j] == c) return true;

      fDistinctCol.push_back(c);

      return true;
    }

    //************************************************************
    // Remove the column with name "cname" in the table to the
    // list of columns that are to be "distinct"
    //************************************************************
    bool Table::RemoveDistinctColumn(std::string cname)
    {
      const ColumnDef* c = GetCol(cname);

      if (!c) return false;

      for (unsigned int j=0; j<fDistinctCol.size(); ++j)
        if (fDistinctCol[j] == c) {
          fDistinctCol.erase(fDistinctCol.begin() + j);
          return true;
        }

      return false;
    }

    //************************************************************
    // Add the ith column in the table to the list of columns
    // that define the sorting order
    //************************************************************
    bool Table::AddOrderColumn(unsigned int i)
    {
      if (i >= fCol.size()) return false;
      
      const ColumnDef* c = &fCol[i];
      for (unsigned int j=0; j<fOrderCol.size(); ++j)
        if (fOrderCol[j] == c) return true;

      fOrderCol.push_back(c);

      return true;
    }

    //************************************************************
    // Remove the ith column in the table to the list of columns
    // that define the sorting order
    //************************************************************
    bool Table::RemoveOrderColumn(unsigned int i)
    {
      if (i >= fCol.size()) return false;

      const ColumnDef* c = &fCol[i];
      for (unsigned int j=0; j<fOrderCol.size(); ++j)
        if (fOrderCol[j] == c) {
          fOrderCol.erase(fOrderCol.begin() + j);
          return true;
        }

      return false;
    }

    //************************************************************
    // Add the column with name "cname" in the table to the list
    // of columns that define the sorting order
    //************************************************************
    bool Table::AddOrderColumn(std::string cname)
    {
      const ColumnDef* c = GetCol(cname);

      if (!c) return false;

      for (unsigned int j=0; j<fOrderCol.size(); ++j)
        if (fOrderCol[j] == c) return true;

      fOrderCol.push_back(c);

      return true;
    }

    //************************************************************
    // Remove the column with name "cname" in the table to the
    // list of columns that define the sorting order
    //************************************************************
    bool Table::RemoveOrderColumn(std::string cname)
    {
      const ColumnDef* c = GetCol(cname);

      if (!c) return false;

      for (unsigned int j=0; j<fOrderCol.size(); ++j)
        if (fOrderCol[j] == c) {
          fOrderCol.erase(fOrderCol.begin() + j);
          return true;
        }

      return false;
    }

    //************************************************************
    std::string Table::GetPassword()
    {
      std::string pwd = "";

      char* pwdFile = getenv("DBIWSPWDFILE");
      if (pwdFile) {
        std::ifstream fin;
        fin.open(pwdFile);
        if (!fin.is_open() || !fin.good()) {
          std::cerr << "Could not open password file " << pwdFile
                    << ".  Canceling Table::Write()" << std::endl;
          return pwd;
        }
        else {
          fin >> pwd;
          fin.close();
        }
      }

      return pwd;
    }

  }

}
