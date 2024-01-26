#ifndef __DBICOLUMN_HPP_
#define __DBICOLUMN_HPP_

#include <string>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace nutools {
  namespace dbi {

    class ColumnDef;

    enum ColType {
      kAutoIncr=0x1,
      kBool=0x2,
      kIntLike=0x4,
      kFloatLike=0x8,
      kString=0x10,
      kTimeStamp=0x20,
      kDateStamp=0x40
    };

    /**
     * Generalized Database Column Interface
     *
     * @author Jonathan Paley
     * @version $Id: Column.h,v 1.24 2013/03/12 19:53:02 jpaley Exp $
     */

    class Column 
    {
    public:
      Column() {fValue=0; fType=kIntLike;};
      Column(const ColumnDef& c);
      Column(const Column& c);
      ~Column();
      
      uint8_t Type()          const { return fType;}
      std::string Value()     const { 
	if (!fValue) return std::string("");
	else return std::string(fValue); }
      bool        IsNull()    const { return (!fValue); }
      bool        Modified()  const { return fModified; }
      
      void        Clear();

      void        SetType(uint8_t t) { fType = t; }

      // WARNING: the casual user should NOT use this method.  Only use it
      // if you _really_ know what you're doing!
      void        FastSet(std::string v) {
	if (fValue) {
	  delete fValue;
	  fValue=0;
	}
	fValue = new char[v.length()+1];
	strcpy(fValue,v.c_str());
      }

      void        FastSet(const char* v) {
	if (fValue) {
	  delete fValue;
	  fValue=0;
	}
	fValue = new char[strlen(v)+1];
	strcpy(fValue,v);
      }

      template <class T>
	bool Get(T& val) const { 
	if (fValue) {
	  try {
	    val = boost::lexical_cast<T>(std::string(fValue)); 
	  }
	  catch (boost::bad_lexical_cast &) {
	    std::cerr << "Column::Get(): Bad_lexical_cast! Value = " 
		      << fValue << std::endl;
	    return false;
	  }
	  return true;
	}

	return false;
      }
    
      template <class T>
	bool Set(const T& val,bool ignoreAutoIncr=false) { 
	if (!ignoreAutoIncr && fType==kAutoIncr) {
	  std::cerr << "Cannot set a column of type \"autoincr\"!" 
		    << std::endl;
	  return false;
	}
	try {	  
	  if (fValue) {
	    delete fValue;
	    fValue=0;
	  }
	  std::string tstr = boost::lexical_cast<std::string>(val);
	  if (tstr == "" || tstr=="NULL") {
	    return true;
	  }
	  if (fType == kBool) {
	    fValue = new char[1];
	    if (tstr == "TRUE" || tstr == "t" || tstr == "true" || 
		tstr == "y" || tstr == "yes" || tstr == "1" || tstr == "on") 
	      fValue[0] = '1';
	    else 
              fValue[0] = '0';
	    return true;
	  }
	  else {
	    fValue = new char[tstr.length()];
	    strcpy(fValue,tstr.c_str());
	    return true;
	  }
	}
	catch (boost::bad_lexical_cast &) {
	  std::cerr << "Column::Set(): Bad lexical cast! Value = " 
	       << val << std::endl;
	  return false;
	}
	return false;
      }

      template <class T>
	bool Update(const T& val) { 
	if (Set(val)) {
	  fModified = true; return true; }
	else
	  return false;
      }

      friend std::ostream& operator<< (std::ostream& stream, const Column& col);
	
      bool        operator >= (const Column& c) const;
      bool        operator <= (const Column& c) const;
      bool        operator >  (const Column& c) const;
      bool        operator <  (const Column& c) const;
      bool        operator == (const Column& c) const;

    private:
      bool        fModified;
      uint16_t    fType;
      char* fValue;

    }; // class end

    //************************************************************
    
    inline std::ostream& operator<< (std::ostream& stream, const Column& col) { 
      if (!col.fValue) {
	stream << "NULL";
      }
      else {
	if (col.fType == kBool) {
	  if (col.fValue[0] == '1')
	    stream << "true";
	  else
	    stream << "false";
	}
	else {
	  bool needsQuotes = (col.fType == kString || 
			      col.fType == kTimeStamp || 
			      col.fType == kDateStamp );
	  if (needsQuotes)	stream << "\'";
	  stream << col.fValue;
	  if (needsQuotes)	stream << "\'";
	}
      }
      return stream;
    }

  } // namespace dbi close
} // namespace nutools close

#endif
