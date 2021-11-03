
find_package(libwda REQUIRED PUBLIC)
find_package(PostgreSQL REQUIRED)
### MIGRATE-ACTION-RECOMMENDED we are using UPS variables here because find_package returns a path
find_library( PQ NAMES pq PATHS ENV POSTGRESQL_LIBRARIES NO_DEFAULT_PATH )
find_library( CURL NAMES curl )

art_make_library( SOURCE Column.cpp  ColumnDef.cpp  Row.cpp  Table.cpp  Util.cpp
                     #${CMAKE_CURRENT_BINARY_DIR}/IFDatabaseCint.cc
                  LIBRARIES PRIVATE
                                Boost::date_time
                                Boost::filesystem
                                Boost::thread
                                Boost::regex
                                PQ
                                ${CURL}
                 PUBLIC wda::wda
                 )

cet_make_exec( NAME tagConditionsTableInDB
               SOURCE tagConditionsTableInDB.cc
               LIBRARIES PRIVATE nuevdb::IFDatabase
                         ${CURL}
               )

cet_make_exec( NAME writeConditionsCSVToDB.cc
               SOURCE writeConditionsCSVToDB.cc
               LIBRARIES PRIVATE nuevdb::IFDatabase
                         ${CURL}
               )

cet_make_exec( NAME dumpConditionsToCSV
               SOURCE dumpConditionsToCSV.cc
               LIBRARIES PRIVATE nuevdb::IFDatabase
                         ${CURL}
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
