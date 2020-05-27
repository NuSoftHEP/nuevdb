#ifndef __DBIROW_HPP_
#define __DBIROW_HPP_

#include <string>
#include <vector>

#include "nuevdb/IFDatabase/Column.h"
#include "nuevdb/IFDatabase/ColumnDef.h"

namespace nutools {
  namespace dbi {

    /**
     * Generalized Database Row Interface
     *
     * @author Jonathan Paley
     * @version $Id: Row.h,v 1.16 2011/11/01 16:15:18 anorman Exp $
     */
    class Row
    {
    public:
      Row(int ncol) : fIsVldRow(false), fNModified(0), fCol(ncol) { };
      
      Row(const std::vector<Column>&);
      Row(std::vector<ColumnDef>&);
      ~Row();
      
      void    Clear();
      
      template <class T>      
	bool    Set(int idx, T value) {
	if (idx < (int)fCol.size() && idx>=0) 
	  return (fCol[idx].Set(value));
	return false;
      }

      template <class T>
	bool    Update(int idx, T value) {
	if (idx < (int)fCol.size() && idx>=0) {
	  if (!fCol[idx].Modified()) fNModified++;
	  return (fCol[idx].Update(value));
	}
	return false;
      }

      bool    InDB() { return fInDB; }
      void    SetInDB() { fInDB = true; }

      int     NModified() { return fNModified; }

      int     NCol() { return fCol.size(); }

      Column& Col(int i) {return fCol[i]; }

      uint64_t Channel() { return fChannel; }
      double    VldTime() { return fVldTime; } 
      double    VldTimeEnd() { return fVldTimeEnd; }
      bool     IsVldRow() { return fIsVldRow; }

      bool    SetChannel(uint64_t ch) { fIsVldRow=true; return (fChannel=ch); }
      bool    SetVldTime(double t) { fIsVldRow=true; return (fVldTime=t); }
      bool    SetVldTimeEnd(double t) { fIsVldRow=true; return (fVldTime=t); }
      
      //      bool operator==(const Row& other) const;

      friend std::ostream& operator<< (std::ostream& stream, const Row& row);
      //      friend std::istream& operator>> (std::istream& stream, Row& row);

    private:
      bool                fInDB;
      bool                fIsVldRow;
      int                 fNModified;
      uint64_t            fChannel;
      double               fVldTime;
      double               fVldTimeEnd;
      std::vector<Column> fCol;

    }; // class end

    //************************************************************
    
    inline std::ostream& operator<< (std::ostream& stream, const Row& row) { 
      for (unsigned int i=0; i<row.fCol.size(); ++i) {
	stream << row.fCol[i];
	if (i < row.fCol.size()-1)
	  stream << ", ";
      }      
      return stream;
    }
    
  } // namespace dbi close
} // namespace nova close

#endif
