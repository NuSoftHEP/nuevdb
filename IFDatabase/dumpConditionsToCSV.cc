#include <iostream>
#include <iomanip>
#include <vector>
#include "IFDatabase/Table.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 6) {
    cout << "Usage: dumpValidityTabletoCSV [detector name] [data|mc|datamc] [Validity Time Stamp (seconds)] [table name] [CSV data file]"
	 << endl;
    exit(1);
  }

  nutools::dbi::Table* t;

  try {
    t = new nutools::dbi::Table(argv[1],argv[4],nutools::dbi::kConditionsTable);
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

  std::cout << argv[3] << std::endl;
  time_t tStart = atof(argv[3]);
  time_t tEnd = time_t(1<<31);

  t->SetMinTSVld(tStart);
  t->SetMaxTSVld(tEnd);

  t->Load();
  
  t->WriteToCSV(argv[5]);

  return 0;

}
