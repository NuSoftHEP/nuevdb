#ifndef __DBIUTIL_HPP_
#define __DBIUTIL_HPP_

#include <string>
#include <ctime>

namespace nutools {
  namespace dbi {

    /**
     * Generalized Database Table Interface Utilities
     *
     * @author Jonathan Paley
     * @version $Id: Util.h,v 1.9 2011/08/08 16:03:04 jpaley Exp $
     */
    class Util
    {
    public:
      
      static bool CheckConnection(std::string dbname,
				  std::string server,
				  std::string user = "",
				  std::string port = "");
      static std::string GetFarPastTimeAsString();
      static std::string GetCurrentTimeAsString();
      static std::string GetCurrentDateAsString();
      static std::string GetFarFutureTimeAsString();
      static bool TimeAsStringToTime_t(std::string ts, time_t& t);
      static bool DateAsStringToTime_t(std::string date, time_t& t);
      static bool RunningOnGrid();
    };    
    
  } // namespace dbi close
} // namespace nova close

#endif
