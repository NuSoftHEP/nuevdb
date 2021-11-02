set( PACKAGE IFDatabase )
FILE( GLOB src_files *.cpp )

find_package(libwda REQUIRED)
find_package(PostgreSQL REQUIRED)
### MIGRATE-ACTION-RECOMMENDED we are using UPS variables here because find_package returns a path
find_library( PQ NAMES pq PATHS ENV POSTGRESQL_LIBRARIES NO_DEFAULT_PATH )
find_library( CURL NAMES curl )

art_make_library( SOURCE ${src_files} #${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
                  LIBRARIES     wda::wda
                                Boost::date_time
                                Boost::filesystem
                                Boost::thread
                                Boost::regex
                                PQ
                                ${CURL}
                 )

cet_make_exec( NAME tagConditionsTableInDB
               SOURCE tagConditionsTableInDB.cc
               LIBRARIES nuevdb::IFDatabase
                         ${CURL}
               )

cet_make_exec( NAME writeConditionsCSVToDB.cc
               SOURCE writeConditionsCSVToDB.cc
               LIBRARIES nuevdb::IFDatabase
                         ${CURL}
               )

cet_make_exec( NAME dumpConditionsToCSV
               SOURCE dumpConditionsToCSV.cc
               LIBRARIES nuevdb::IFDatabase
                         ${CURL}
               )

cet_build_plugin( DBI art::service
               LIBRARIES nuevdb::EventDisplayBase
               nuevdb::IFDatabase
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
             )

install_headers()
install_fhicl()
install_source()
