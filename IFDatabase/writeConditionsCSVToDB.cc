#include <iostream>
#include <iomanip>
#include <vector>
#include "IFDatabase/Table.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 5) {
    cout << "Usage: writeConditionsCSVToDB [detector name] [data|mc|datamc] [table name] [CSV data file]"
	 << endl;
    exit(1);
  }

  nutools::dbi::Table* t = new nutools::dbi::Table();
  t->SetDetector(argv[1]);
  t->SetTableName(argv[3]);
  t->SetTableType(nutools::dbi::kConditionsTable);
  t->GetColsFromDB();
  
  std::string dt = argv[2];
  if (dt == "data") 
    t->SetDataTypeMask(nutools::dbi::kDataOnly);
  else if (dt == "mc") 
    t->SetDataTypeMask(nutools::dbi::kMCOnly);
  else if (dt == "datamc") 
    t->SetDataTypeMask(nutools::dbi::kDataOnly|nutools::dbi::kMCOnly);

  t->SetVerbosity(100);
  
  if (t->LoadFromCSV(argv[4]))
    t->Write();

  return 0;

}
