#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <getopt.h>
#include <string>

#include "Database/Table.h"

std::string dbHost = "";
std::string dbName = "";
std::string dbPort = "";
std::string tableName = "";
std::string detectorName = "";
std::string tagName = "";
bool override=false;

void PrintUsage()
{
  std::cout << "Usage: tagConditionsTableInDB [options]" << std::endl;
  std::cout << "options:\n";
  std::cout << "\t -h (--host) [dB host, REQUIRED]" << std::endl;
  std::cout << "\t -n (--name) [dB name, REQUIRED]" << std::endl;
  std::cout << "\t -p (--port) [dB port, REQUIRED]" << std::endl;
  std::cout << "\t -d (--detector) [detector name, REQUIRED]" << std::endl;
  std::cout << "\t -T (--tablename) [table name, REQUIRED]" << std::endl;
  std::cout << "\t -t (--tag) [tag name, REQUIRED]" << std::endl;
  std::cout << "\t -o (--override)" << std::endl;
}

//------------------------------------------------------------
bool ParseCLArgs(int argc, char* argv[])
{
  if (argc == 1) {
    PrintUsage();
    exit(0);
  }

  struct option long_options[] = {
    {"host",      1, 0, 'h'},
    {"port",      1, 0, 'p'},
    {"name",      1, 0, 'n'},
    {"detector",  1, 0, 'd'},
    {"tablename", 1, 0, 'T'},
    {"tag",       1, 0, 't'},
    {"override",       1, 0, 'o'},
    {0,0,0,0}
  };
  
  while (1) {
    int optindx;
    int c = getopt_long(argc,argv,"t:h:p:n:d:f:o",long_options,&optindx);
        
    if (c==-1) break;
    
    switch(c) {
    case 'h':
      dbHost = optarg;
      break;
    case 'p':
      dbPort = optarg;
      break;
    case 'n':
      dbName = optarg;
      break;
    case 'd':
      detectorName = optarg;
      break;
    case 'T':
      tableName = optarg;
      break;
    case 't':
      tagName = optarg;
      break;
    case 'o':
      override=true;
      break;
    default:
      break;
    }
    
  }

  if (optind<argc) {
    std::cerr << "Unrecognized argument." << std::endl;
    PrintUsage();
    exit(0);
  }
  
  if (tagName == "") {
    std::cerr << "No tag name provided.  Aborting." << std::endl;
    exit(1);
  }

  if (dbHost == "") {
    std::cerr << "No value set for dB host.  Aborting." << std::endl;
    exit(0);
  }
  if (dbPort == "") {
    std::cerr << "No value set for dB port.  Aborting." << std::endl;
    exit(0);
  }
  if (dbName == "") {
    std::cerr << "No value set for dB name.  Aborting." << std::endl;
    exit(0);
  }
  if (tableName == "") {
    std::cerr << "No value set for table name.  Aborting." << std::endl;
    exit(0);
  }
  
  return true;
}

//------------------------------------------------------------

int main(int argc, char *argv[])
{
  ParseCLArgs(argc, argv);

  nutools::dbi::Table* dbt;
  try {
    dbt = new nutools::dbi::Table(detectorName,tableName,
			       nutools::dbi::kConditionsTable,
			       dbHost,dbName,dbPort);
  }
  catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    exit(1);
  }

  std::cerr << "Tagging validity dB table " << dbName << " in database with tag " 
	    << tagName << std::endl;

  if (!dbt->Tag(tagName,override)) 
    std::cerr << "Tag failed!" << std::endl;
   
  return 0;
}

