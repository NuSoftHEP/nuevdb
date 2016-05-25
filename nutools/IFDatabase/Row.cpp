#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cstdlib>
#include <cctype>
#include <string>

#include <nutools/IFDatabase/Row.h>

namespace nutools {  
  namespace dbi {
    
    Row::Row(const std::vector<Column>& col) : 
      fInDB(false), fIsVldRow(false), fNModified(0),
      fChannel(0xffffffff),fVldTime(0),fVldTimeEnd(0)      
    {
      for (unsigned int i=0; i<col.size(); ++i) {
	fCol.push_back(Column(col[i]));
      }
    }
    
    //************************************************************
    
    Row::Row(std::vector<ColumnDef>& col) : 
      fInDB(false), fIsVldRow(false), fNModified(0),
      fChannel(0xffffffff),fVldTime(0),fVldTimeEnd(0) 
    {
      for (unsigned int i=0; i<col.size(); ++i)
	fCol.push_back(Column(col[i]));
    }

    //************************************************************
    
    Row::~Row()
    {
      
    }
    
    //************************************************************
    void Row::Clear()
    {
      for (unsigned int i=0; i<fCol.size(); ++i) 
	fCol[i].Clear();
      
    }
  }
}
    
    
