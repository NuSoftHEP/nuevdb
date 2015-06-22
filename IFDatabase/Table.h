#ifndef __DBITABLE_HPP_
#define __DBITABLE_HPP_

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdlib>
#include <wda.h>

#include "IFDatabase/DataType.h"
#include "IFDatabase/Column.h"
#include "IFDatabase/ColumnDef.h"
#include "IFDatabase/Row.h"

// Forward declarations for postgres types
struct pg_conn;
typedef pg_conn PGconn;
struct pg_result;
typedef pg_result PGresult;

namespace nutools {
  namespace dbi {

    enum DBTableType {
      kGenericTable,
      kConditionsTable,
      kHardwareTable,
      kNTableType
    };

    enum DataSource {
      kDAQ,
      kDCS,
      kOffline,
      kUnknownSource,
      kNDataSources
    };

    /**
     * Database Table Interface
     *
     * @author Jonathan Paley
     * @version $Id: Table.h,v 1.61 2013/02/11 20:48:41 jpaley Exp $
     */
    class Table
    {
    public:

      Table();
      //      Table(std::string fileName);
      Table(std::string schemaName, std::string tableName,
            int tableType=kGenericTable,
            std::string dbhost="", std::string dbname="",
            std::string dbport="", std::string dbuser="");
      ~Table();

      std::string Name() { return fTableName; }
      std::string DBName() { return fDBName;}
      std::string DBHost() { return fDBHost;}
      std::string User() { return fUser; }
      std::string Role() { return fRole; }
      std::string DBPort() { return fDBPort; }
      int  TableType()     { return fTableType; }
      int  DataSource()    { return fDataSource; }
      int  DataTypeMask()  { return fDataTypeMask; }

      void SetTableName(std::string tname);
      void SetTableName(const char* tname);

      void SetDataSource(std::string ds);
      void SetDataSource(int ids);

      bool SetTableType(int t);
      void SetDataTypeMask(int mask) { fDataTypeMask = mask;}

      void SetIgnoreEnvVar(bool f) { fIgnoreEnvVar = f;}
      void SetUser(std::string uname) { fUser = uname; }
      void SetUser(const char* uname) { fUser = uname; }
      bool SetRole(std::string role);
      bool SetRole(const char* role);
      void SetDBName(std::string dbname) { fDBName = dbname; }
      void SetDBName(const char* dbname) { fDBName = dbname; }
      void SetDBHost(std::string dbhost) { fDBHost = dbhost; }
      void SetDBHost(const char* dbhost) { fDBHost = dbhost; }
      void SetDBPort(std::string p) { fDBPort = p; }
      void SetDBPort(const char* p) { fDBPort = p; }
      void SetDBInfo(std::string name, std::string host, std::string port,
                     std::string user);
      void SetDBInfo(const char* name, const char* host, const char* port,
                     const char* user);

      bool SetPasswordFile(const char* fname=0); ///< fname should be the name of the file
      void ClearPassword() { fPassword=""; }
      void DisablePasswordAccess() { fPassword = ""; }

      void ResetRole() { fRole = fUser; }

      void SetVerbosity(int i) { fVerbosity = i;}

      int NCol() {return fCol.size();}
      int NRow() {return fRow.size();}

      void Clear() {
        fRow.clear(); fValidityStart.clear(); fValidityEnd.clear();
        fOrderCol.clear(); fDistinctCol.clear(); fNullList.clear();
        fValiditySQL = "";
        fValidityChanged = true;
      }

      void ClearRows() { fRow.clear(); fNullList.clear(); fValidityChanged=true;}

      nutools::dbi::Row* const GetRow(int i);

      void AddRow(const Row* row);
      void AddRow(const Row& row);
      void AddEmptyRows(unsigned int nrow);

      bool RemoveRow(int i); ///< note, this will only delete a row from
                             ///< memory, it will not delete an existing
                             ///< row in a dB!

      nutools::dbi::Row* const NewRow() { Row* r = new Row(fCol); return r;}

      std::vector<std::string> GetColNames();
      std::map<std::string,int> GetColNameToIndexMap();
      std::string GetColName(int i) {return fCol[i].Name(); }
      int GetColIndex(std::string cname);

      const nutools::dbi::ColumnDef* GetCol(int i) {return &fCol[i]; }
      const nutools::dbi::ColumnDef* GetCol(std::string& cname);
      const nutools::dbi::ColumnDef* GetCol(const char* cname)
        { std::string cstr(cname); return GetCol(cstr); }

      void SetTolerance(std::string& cname, float t);
      float Tolerance(std::string& cname);

      bool ExistsInDB();
      bool ExecuteSQL(std::string cmd, PGresult*& res);

      bool LoadFromCSV(std::string fname);
      bool LoadFromCSV(const char* fname)
      { return LoadFromCSV(std::string(fname)); }

      bool LoadFromDB();
      bool WriteToDB(bool commit=true); ///< use commit=false if just testing
      bool WriteToCSV(std::string fname, bool appendToFile=false, bool writeColNames=false);
      bool WriteToCSV(const char* fname, bool appendToFile=false, bool writeColNames=false)
      { return WriteToCSV(std::string(fname),appendToFile,writeColNames); }

      void ClearValidity();

      bool AddDistinctColumn(unsigned int i);
      bool AddDistinctColumn(std::string col);
      bool AddDistinctColumn(const char* col)
      { return AddDistinctColumn(std::string(col)); }
      bool RemoveDistinctColumn(unsigned int i);
      bool RemoveDistinctColumn(std::string col);
      bool RemoveDistinctColumn(const char* col)
      { return RemoveDistinctColumn(std::string(col)); }

      bool AddOrderColumn(unsigned int i);
      bool AddOrderColumn(std::string col);
      bool AddOrderColumn(const char* col)
      { return AddOrderColumn(std::string(col)); }
      bool RemoveOrderColumn(unsigned int i);
      bool RemoveOrderColumn(std::string col);
      bool RemoveOrderColumn(const char* col)
      { return RemoveOrderColumn(std::string(col)); }

      void SetSelectLimit(int limit) { fSelectLimit=limit; }
      void SetSelectOffset(int offset) { fSelectOffset=offset; }

      void SetOrderDesc() { fDescOrder = true; }
      void SetOrderAsc() { fDescOrder = false; }

      void AddExcludeCol(std::string col) {fExcludeCol.push_back(col); }
      void ClearExcludedCols() { fExcludeCol.clear(); }

      bool GetCurrSeqVal(std::string col, long& iseq);
      bool GetCurrSeqVal(const char* col, long& iseq)
      { return GetCurrSeqVal(std::string(col), iseq); }

      //      bool HasPKey() { return fTable->hasDBPKey(); }

      int  GetNPKeyCol() { return fPKeyList.size(); }
      const nutools::dbi::ColumnDef* GetPKeyCol(int i) { return fPKeyList[i]; }
      /*
      int  GetNUnique() { return fTable->getDBUniqueCount(); }
      const dBRow_t* GetUniqueRow(int i) {
        if (fTable->hasDBUnique() &&
            i < this->GetNUnique())
          return &fTable->getDBUnique(i);
        else return 0;
      }
      */
      /*
      int  GetNCheck() { return fTable->getDBCheckCount(); }
      std::string GetCheckConstraint(int i) {
        return std::string(fTable->getDBCheck(i).getConstraint());
      }
      */
      void PrintPQErrorMsg() const;

      std::string GetValiditySQL() { return fValiditySQL; }
      void SetValiditySQL(std::string cmd) { fValiditySQL = cmd; fValidityChanged=true;}

      bool SetDetector(std::string det);
      bool GetDetector(std::string& det ) const;

      void SetSchema(std::string s) { fSchema = s; }
      std::string Schema() { return fSchema; }

      template <class T>
        bool SetValidityRange(std::string cname, T start, T end)
        {
          const ColumnDef* c = this->GetCol(cname);
          if (c) {
            // check to see if this makes sense for a boolean column.
            if (c->Type() == "bool")
              if (start != end) return false;

            // check to see that this column wasn't already added to valid.
            // list if it is, overwrite it

            unsigned int i=0;
            for (; i<fValidityStart.size(); ++i)
              if (fValidityStart[i].Name() == c->Name()) break;

            if (i == fValidityStart.size()) {
              fValidityStart.push_back(ColumnDef(*c));
              fValidityEnd.push_back(ColumnDef(*c));
            }
	    std::stringstream startSS;
	    startSS << start;
	    std::stringstream endSS;
	    endSS << end;
            fValidityStart[i].SetValue(startSS.str());
            fValidityEnd[i].SetValue(endSS.str());
            fValidityChanged=true;
            return true;
          }

          return false;
        }

      template <class T>
        bool SetValidityRange(std::string cname, T start) {
        return (this->SetValidityRange(cname,start,start));
      }

      void RemoveValidityRange(std::string& cname);
      void RemoveValidityRange(const char* cname)
      { std::string cstr(cname); return RemoveValidityRange(cstr); fValidityChanged=true;}

      void PrintColumns();

      bool GetConnection(int ntry=0);
      bool CloseConnection();
      void SetConnectionTimeout(int n) { fConnectionTimeout=n;} // units in sec
      int  GetConnectionTimeout() { return fConnectionTimeout; }
      bool ResetConnectionInfo();

      void CacheDBCommand(std::string cmd);

      friend std::ostream& operator<< (std::ostream& stream, const Table& t);

      void SetMinTSVld(float t) { fMinTSVld = t;}
      void SetMaxTSVld(float t) { fMaxTSVld = t;}

      float GetMaxTSVld() const {return fMaxTSVld; }
      float GetMinTSVld() const {return fMinTSVld; }

      void SetTag(std::string s) { fTag = s; }
      std::string GetTag() { return fTag; }
      bool Tag(std::string tn="", bool override=false);

      bool Load();
      bool Write(bool commit=true);

      void ClearChanRowMap() { fChanRowMap.clear(); }
      void FillChanRowMap();
      int  NVldRows(uint64_t channel) { return fChanRowMap[channel].size(); }
      int  NVldChannels() { return fChanRowMap.size(); }
      std::vector<uint64_t> VldChannels() { return fChannelVec; }

      nutools::dbi::Row* GetVldRow(uint64_t channel, float t);
      std::vector<nutools::dbi::Row*> GetVldRows(uint64_t channel);

      void SetRecordTime(float t);
      void ClearRecordTime() { fHasRecordTime = false;}

      void EnableFlushCache() { fFlushCache = true; }
      void DisableFlushCache() { fFlushCache = false; }

      void DisableCache() { fDisableCache = true; }
      void EnableCache() { fDisableCache = false; }

      void SetWSURL(std::string url) { fWSURL = url;}
      void SetQEURL(std::string url) { fQEURL = url;}

      void SetTimeQueries(bool f) {fTimeQueries = f; }
      void SetTimeParsing(bool f) {fTimeParsing = f; }
      bool TimeQueries() {return fTimeQueries; }
      bool TimeParsing() {return fTimeParsing; }

      void SetMinChannel(uint64_t chan) {fMinChannel = chan;}
      void SetMaxChannel(uint64_t chan) {fMaxChannel = chan;}
      void SetChannelRange(uint64_t chan1, uint64_t chan2) 
      { fMinChannel=chan1; fMaxChannel=chan2;}

      void PrintVMUsed();
      void PrintPMUsed();

    private:

      bool LoadConditionsTable();
      bool LoadNonConditionsTable();
      bool GetDataFromWebService(Dataset&, std::string);

      void Reset();
      bool GetConnectionInfo(int ntry=0);

      bool CheckForNulls();

      bool MakeConditionsCSVString(std::stringstream& ss);

      std::string GetPassword();

      int ParseSelfStatusLine(char* line);

      bool    addInsertTime;
      bool    addInsertUser;
      bool    addUpdateTime;
      bool    addUpdateUser;
      bool    fIgnoreEnvVar;
      bool    fValidityChanged;
      bool    fDescOrder;
      bool    fIgnoreDB;
      bool    fTestedExists;
      bool    fExistsInDB;
      bool    fHasConnection;
      bool    fHasRecordTime;
      bool    fFlushCache;
      bool    fDisableCache;
      bool    fTimeQueries;
      bool    fTimeParsing;
      short   fVerbosity;

      int     fSelectLimit;
      int     fSelectOffset;
      int     fConnectionTimeout;
      int     fTableType;
      int     fDataTypeMask;
      int     fDataSource;
      uint64_t fMinChannel;
      uint64_t fMaxChannel;

      std::string fTableName;
      std::string fUser;
      std::string fRole;
      std::string fDBPort;
      std::string fDBHost;
      std::string fDBName;
      std::string fSchema;

      std::string fDBCacheFile;
      std::string fPassword;
      std::string fValiditySQL;
      std::string fDetector;

      std::string fTag;
      std::string fWSURL;
      std::string fQEURL;

      std::vector<nutools::dbi::ColumnDef> fCol;
      std::vector<nutools::dbi::Row>    fRow;

      std::vector<nutools::dbi::ColumnDef> fValidityStart;
      std::vector<nutools::dbi::ColumnDef> fValidityEnd;
      std::vector<const nutools::dbi::ColumnDef*> fPKeyList;
      std::vector<const nutools::dbi::ColumnDef*> fDistinctCol;
      std::vector<const nutools::dbi::ColumnDef*> fOrderCol;
      std::vector<std::pair<int,int> > fNullList;
      std::vector<std::string> fExcludeCol;

      std::vector<uint64_t> fChannelVec;
      std::unordered_map<uint64_t,std::vector<nutools::dbi::Row*> > fChanRowMap;

      PGconn* fConnection;

      //      static boost::mutex _xsdLock;

      float  fMaxTSVld;
      float  fMinTSVld;
      float  fRecordTime;


    }; // class end

    //************************************************************

    inline std::ostream& operator<< (std::ostream& stream, const Table& t) {
      for (unsigned int j=0; j<t.fRow.size(); ++j) {
        stream << t.fRow[j] << std::endl;
      }
      return stream;
    }

  } // namespace dbi close
} // namespace nutools close

#endif
