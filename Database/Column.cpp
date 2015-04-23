#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <algorithm>

#include <Database/Column.h>
#include <Database/ColumnDef.h>
#include <Database/Util.h>

//************************************************************
namespace nutools {
  namespace dbi {

    Column::Column(const ColumnDef &c) :
	fModified(false)
    {
      fValue = 0;
      fType = kIntLike;

      std::string t = c.Type();
      if (t == "timestamp") fType = kTimeStamp;
      else if (t == "date") fType = kDateStamp;
      else if (t == "bool") fType = kBool;
      else if (t == "float" || t == "double") fType = kFloatLike;
      else if (t == "string" || t == "text") fType = kString;
      else if (t == "autoincr") fType = kAutoIncr;

    }
    
    //************************************************************
    
    Column::Column(const Column& c)
    {
      if (!c.fValue)
	fValue=0;
      else {
	fValue = new char[strlen(c.fValue)+1];
	strcpy(fValue,c.fValue);
      }
      fType = c.fType;
      fModified = c.fModified;

    }

    //************************************************************
    
    Column::~Column()
    {
      if (fValue) delete[] fValue;
    }
    
    //************************************************************
    void Column::Clear() 
    {
      if (fValue) {
	delete fValue;
	fValue = 0;
      }
      //      fIsNull = true;
      fModified = false; 
    }
    
    //************************************************************
    bool Column::operator >= (const Column& c) const
    {
      if (c.Type() != fType) return false;
      
      std::string val1 = fValue;
      std::string val2 = c.fValue;
      
      if (fType == kBool) {
	bool a = (val1 == "1");
	bool b = (val2 == "1");
	return ((a == b) || (a&!b));
      }
      
      if (fType == kIntLike) {
	long a = boost::lexical_cast<long>(val1);
	long b = boost::lexical_cast<long>(val2);
	return (a >= b);
      }
      
      if (fType == kFloatLike) {
	double a = boost::lexical_cast<double>(val1);
	double b = boost::lexical_cast<double>(val2);
	return (a >= b);
      }
      
      if (fType == kString) 
	return (val1 >= val2);
      
      if (fType == kTimeStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::TimeAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::TimeAsStringToTime_t(b,ttb) ) 
	    return (tta >= ttb);
	}
	else 
	  return false;
      }
      
      if (fType == kDateStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::DateAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::DateAsStringToTime_t(b,ttb) ) 
	    return (tta >= ttb);
	}
	else 
	  return false;
      }
      return false;
      
    }
    
    //************************************************************
    bool Column::operator > (const Column& c) const
    {
      if (c.Type() != fType) return false;
      
      std::string val1 = fValue;
      std::string val2 = c.fValue;
      
      if (fType == kBool) {
	bool a = (val1 == "1");
	bool b = (val2 == "1");
	return (a&!b);
      }
      
      if (fType == kIntLike) {
	long a = boost::lexical_cast<long>(val1);
	long b = boost::lexical_cast<long>(val2);
	return (a > b);
      }
      
      if (fType == kFloatLike) {
	double a = boost::lexical_cast<double>(val1);
	double b = boost::lexical_cast<double>(val2);
	return (a > b);
      }
      
      if (fType == kString) 
	return (val1 > val2);
      
      if (fType == kTimeStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::TimeAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::TimeAsStringToTime_t(b,ttb) ) 
	    return (tta > ttb);
	}
	else 
	  return false;
      }
      
      if (fType == kDateStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::DateAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::DateAsStringToTime_t(b,ttb) ) 
	    return (tta > ttb);
	}
	else 
	  return false;
      }
      return false;
      
    }
    
    //************************************************************
    bool Column::operator <= (const Column& c) const
    {
      if (c.fType != fType) return false;
      
      std::string val1 = fValue;
      std::string val2 = c.fValue;
      
      if (fType == kBool) {
	bool a = (val1 == "1");
	bool b = (val2 == "1");
	return ((a == b) || (!a&b));
      }
      
      if (fType == kIntLike) {
	long a = boost::lexical_cast<long>(val1);
	long b = boost::lexical_cast<long>(val2);
	return (a <= b);
      }
      
      if (fType == kFloatLike) {
	double a = boost::lexical_cast<double>(val1);
	double b = boost::lexical_cast<double>(val2);
	return (a <= b);
      }
      
      if (fType == kString) 
	return (val1 <= val2);
      
      if (fType == kTimeStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::TimeAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::TimeAsStringToTime_t(b,ttb) ) 
	    return (tta <= ttb);
	}
	else 
	  return false;
      }
      
      if (fType == kDateStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::DateAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::DateAsStringToTime_t(b,ttb) ) 
	    return (tta <= ttb);
	}
	else 
	  return false;
      }
      
      return false;
      
    }

    //************************************************************
    bool Column::operator < (const Column& c) const
    {
      if (c.fType != fType) return false;
      
      std::string val1 = fValue;
      std::string val2 = c.fValue;
      
      if (fType == kBool) {
	bool a = (val1 == "1");
	bool b = (val2 == "1");
	return (!a&b);
      }
      
      if (fType == kIntLike) {
	long a = boost::lexical_cast<long>(val1);
	long b = boost::lexical_cast<long>(val2);
	return (a < b);
      }
      
      if (fType == kFloatLike) {
	double a = boost::lexical_cast<double>(val1);
	double b = boost::lexical_cast<double>(val2);
	return (a < b);
      }
      
      if (fType == kString) 
	return (val1 < val2);
      
      if (fType == kTimeStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::TimeAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::TimeAsStringToTime_t(b,ttb) ) 
	    return (tta < ttb);
	}
	else 
	  return false;
      }
      
      if (fType == kDateStamp) {
	std::string a, b;
	if (this->Get(a) && c.Get(b)) {
	  time_t tta, ttb;
	  if (nutools::dbi::Util::DateAsStringToTime_t(a,tta) && 
	      nutools::dbi::Util::DateAsStringToTime_t(b,ttb) ) 
	    return (tta < ttb);
	}
	else 
	  return false;
      }
      
      return false;
      
    }
    //************************************************************
    bool Column::operator == (const Column& c) const
    {
      if (c.fType != fType) return false;

      if (fValue == 0 && (fValue==c.fValue)) return true;

      return (strcmp(fValue,c.fValue)==0);
      
    }
  }
}
