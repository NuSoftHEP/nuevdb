#include <IFDatabase/ColumnDef.h>

//************************************************************
namespace nutools {
  namespace dbi {
    ColumnDef::ColumnDef(std::string cname, std::string ctype) :
      fCanBeNull(true),fTolerance(0.),fType(ctype),fName(cname),fValue("")
    {

    }
    
    //************************************************************

    ColumnDef::~ColumnDef()
    {
      
    }
    
  }
}
