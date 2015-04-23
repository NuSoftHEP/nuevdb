#include <iostream>
#include <iomanip>
#include <vector>
#include "Database/Table.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 5) {
    cout << "Usage: writeConditionsCSVToDB [detector name] [data|mc|datamc] [table name] [CSV data file]"
	 << endl;
    exit(1);
  }

  nutools::dbi::Table* t;

  try {
    t = new nutools::dbi::Table(argv[1],argv[3],nutools::dbi::kConditionsTable);
  }
  catch (std::runtime_error& e) {
    std::cerr << e.what() << "  Exiting..." << std::endl;
    exit(2);
  }

  std::string dt = argv[2];
  if (dt == "data") 
    t->SetDataTypeMask(nutools::dbi::kDataOnly);
  else if (dt == "mc") 
    t->SetDataTypeMask(nutools::dbi::kMCOnly);
  else if (dt == "datamc") 
    t->SetDataTypeMask(nutools::dbi::kDataOnly|nutools::dbi::kMCOnly);
  
  if (t->LoadFromCSV(argv[4]))
    t->Write();

  return 0;

}
