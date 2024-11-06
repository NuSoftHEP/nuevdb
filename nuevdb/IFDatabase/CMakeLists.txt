find_package(libwda REQUIRED)
find_package(PostgreSQL REQUIRED)

art_make_library(SOURCE Column.cpp  ColumnDef.cpp  Row.cpp  Table.cpp  Util.cpp
                 LIBRARIES PRIVATE
                        Boost::date_time
                        PostgreSQL::PostgreSQL
                 PUBLIC wda::wda
                        Boost::headers
                 )

cet_make_exec( NAME tagConditionsTableInDB
               SOURCE tagConditionsTableInDB.cc
               LIBRARIES PRIVATE nuevdb::IFDatabase
               )

cet_make_exec( NAME writeConditionsCSVToDB.cc
               SOURCE writeConditionsCSVToDB.cc
               LIBRARIES PRIVATE nuevdb::IFDatabase
               )

cet_make_exec( NAME dumpConditionsToCSV
               SOURCE dumpConditionsToCSV.cc
               LIBRARIES PRIVATE nuevdb::IFDatabase
               )

cet_build_plugin( DBI art::service
               LIBRARIES PRIVATE
               nuevdb::EventDisplayBase
               art::Framework_Core
               art::Framework_Principal
               art::Framework_Services_Registry
               art::Persistency_Common
               art::Utilities
               canvas::canvas
               messagefacility::MF_MessageLogger
               fhiclcpp::fhiclcpp
               cetlib::cetlib
               cetlib_except::cetlib_except
               PUBLIC
               nuevdb::IFDatabase
             )

install_headers()
install_fhicl()
install_source()
