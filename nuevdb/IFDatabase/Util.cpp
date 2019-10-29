#include <nutools/IFDatabase/Util.h>

#include <libpq-fe.h>

#include <iostream>

namespace nutools {

  namespace dbi {

    //************************************************************
    
    bool Util::CheckConnection(std::string dbname, std::string host, 
			       std::string user, std::string port)
    {
      bool retVal = true;
      
      std::string cmd = "dbname = " + dbname + " host = " + host;
      if (user != "")
	cmd += " user = " + user;
      if (port != "")
	cmd += " port = " + port;
      
      PGconn* conn = PQconnectdb(cmd.c_str());
      if (conn) {
	if (PQstatus(conn) != CONNECTION_OK) {
	  std::cerr << "Connection to " << host << ":" << dbname << " failed: " 
		    << PQerrorMessage(conn) << std::endl;
	  retVal = false;
	}
	else 
	  PQfinish(conn);      
      }
      else
	retVal = false;
      
      return retVal;
      
    }

    //************************************************************
    std::string Util::GetCurrentTimeAsString()
    {
      time_t rawtime;
      struct tm* timeinfo;
      char buffer[256];
      time(&rawtime);
      timeinfo = gmtime(&rawtime);
      strftime(buffer,256,"%Y-%m-%d %H:%M:%S",timeinfo);
      
      return std::string(buffer);
      
    }

    //************************************************************
    std::string Util::GetCurrentDateAsString()
    {
      time_t rawtime;
      struct tm* timeinfo;
      char buffer[256];
      time(&rawtime);
      timeinfo = gmtime(&rawtime);
      strftime(buffer,256,"%Y-%m-%d",timeinfo);
      
      return std::string(buffer);
      
    }

    //************************************************************
    std::string Util::GetFarPastTimeAsString()
    {
      return "1900-01-01 00:00:00";
    }
    //************************************************************
    std::string Util::GetFarFutureTimeAsString()
    {
      return "2099-12-31 23:59:59";
    }

    //************************************************************
    bool Util::TimeAsStringToTime_t(std::string ts, time_t& t)
    {
      bool isOk = false;
      struct tm ta;
      if ( (sscanf(ts.c_str(),"%d/%d/%d %d:%d:%d",
		   &ta.tm_year,&ta.tm_mon,&ta.tm_mday,
		   &ta.tm_hour,&ta.tm_min,&ta.tm_sec) == 6))
	isOk = true;
      if (!isOk)
	if ( (sscanf(ts.c_str(),"%d-%d-%d %d:%d:%d",
		     &ta.tm_year,&ta.tm_mon,&ta.tm_mday,
		     &ta.tm_hour,&ta.tm_min,&ta.tm_sec) == 6))
	  isOk = true;
      
      if (isOk) {
	ta.tm_year -= 1900;
	ta.tm_mon -= 1;
	t = timegm(&ta);
      }
      
      return isOk;
      
    }

    //************************************************************
    bool Util::DateAsStringToTime_t(std::string ts, time_t& t)
    {
      bool isOk = false;
      struct tm ta;
      if ( (sscanf(ts.c_str(),"%d/%d/%d",
		   &ta.tm_year,&ta.tm_mon,&ta.tm_mday) == 3))
	isOk = true;
      if (!isOk)
	if ( (sscanf(ts.c_str(),"%d-%d-%d",
		     &ta.tm_year,&ta.tm_mon,&ta.tm_mday) == 3))
	  isOk = true;
      
      if (isOk) {
	ta.tm_year -= 1900;
	ta.tm_mon -= 1;
	ta.tm_hour = 0;
	ta.tm_min = 0;
	ta.tm_sec = 0;
	t = timegm(&ta);
      }
      
      return isOk;
      
    }
    
    //************************************************************
    bool Util::RunningOnGrid()
    {
      char* tStr = getenv("_CONDOR_SCRATCH_DIR");
      return (tStr != 0);
    }
    
  }
}
