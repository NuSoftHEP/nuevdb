#ifndef __DBICOLUMNDEF_HPP_
#define __DBICOLUMNDEF_HPP_

#include <string>

namespace nutools {
  namespace dbi {

    /**
     * Database Column Defintion Interface
     *
     * @author Jonathan Paley
     * @version $Id: ColumnDef.h,v 1.0 2014/07/02 19:53:02 jpaley Exp $
     */
    class ColumnDef
    {
    public:
      ColumnDef(std::string cname, std::string ctype);
      ~ColumnDef();
      
      std::string Name()      const { return fName;}
      std::string Type()      const { return fType;}
      std::string Value()     const { return fValue;}
      bool        CanBeNull() const { return fCanBeNull; }
      float       Tolerance()   const { return fTolerance; }
      
      void        SetName(std::string n) { fName = n; }
      void        SetType(std::string t) { fType = t; }
      void        SetValue(std::string v) { fValue = v; }
      void        SetCanBeNull(bool f) { fCanBeNull = f;}
      void        SetTolerance(float t) { fTolerance = t; }
      
    private:
      bool        fCanBeNull;
      float       fTolerance;
      std::string fType;
      std::string fName;
      std::string fValue;

    }; // class end


  } // namespace dbi close
} // namespace nutools close

#endif
