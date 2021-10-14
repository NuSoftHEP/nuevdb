set( PACKAGE IFDatabase )
FILE( GLOB src_files *.cpp )

### MIGRATE-ACTION-RECOMMENDED we are using UPS variables here 
find_library( LIBWDA NAMES wda PATHS ENV LIBWDA_LIB NO_DEFAULT_PATH )
include_directories($ENV{LIBWDA_INC})

find_package(PostgreSQL REQUIRED)
find_library( CURL NAMES curl )

art_make_library( SOURCE ${src_files} #${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
                  LIBRARIES     LIBWDA
                                Boost::date_time
                                Boost::filesystem
                                Boost::thread
                                Boost::regex
                                ${XCLIB}
                                ${PostgreSQL_LIBRARIES}
                                ${CURL}
                 )

cet_make_exec( NAME tagConditionsTableInDB
               SOURCE tagConditionsTableInDB.cc
               LIBRARIES nuevdb_IFDatabase
                         ${CURL}
               )

cet_make_exec( NAME writeConditionsCSVToDB.cc
               SOURCE writeConditionsCSVToDB.cc
               LIBRARIES nuevdb_IFDatabase
                         ${CURL}
               )

cet_make_exec( NAME dumpConditionsToCSV
               SOURCE dumpConditionsToCSV.cc
               LIBRARIES nuevdb_IFDatabase
                         ${CURL}
               )

cet_build_plugin( DBI art::service
               LIBRARIES nuevdb::EventDisplayBase
               nuevdb_IFDatabase
               art::Framework_Core
               art::Framework_Principal
               art::Framework_Services_Registry
               ${ART_FRAMEWORK_SERVICES_BASIC}
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
