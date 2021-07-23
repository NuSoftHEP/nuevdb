set( PACKAGE IFDatabase )
FILE( GLOB src_files *.cpp )

cet_find_library( LIBWDA NAMES wda PATHS ENV LIBWDA_LIB NO_DEFAULT_PATH )
cet_find_library( PQ NAMES pq PATHS ENV POSTGRESQL_LIBRARIES NO_DEFAULT_PATH )
find_library( CURL NAMES curl )

art_make_library( SOURCE ${src_files} #${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
                  LIBRARIES     LIBWDA
                                Boost::date_time
                                ${BOOST_SYSTEM}
                                Boost::filesystem
                                Boost::thread
                                Boost::regex
                                ${XCLIB}
                                PQ
                                ${CURL}
                 )

art_make_exec( NAME tagConditionsTableInDB
               SOURCE tagConditionsTableInDB.cc
               LIBRARIES nuevdb_IFDatabase
                         ${CURL}
               )

art_make_exec( NAME writeConditionsCSVToDB.cc
               SOURCE writeConditionsCSVToDB.cc
               LIBRARIES nuevdb_IFDatabase
                         ${CURL}
               )

art_make_exec( NAME dumpConditionsToCSV
               SOURCE dumpConditionsToCSV.cc
               LIBRARIES nuevdb_IFDatabase
                         ${CURL}
               )

simple_plugin( DBI "service"
               nuevdb_EventDisplayBase
               nuevdb_IFDatabase
               art::Framework_Core
               art::Framework_Principal
               art::Framework_Services_Registry
               ${ART_FRAMEWORK_SERVICES_BASIC}
               art::Persistency_Common
               art::Utilities
               canvas::canvas
               messagefacility::MF_MessageLogger
               ${MF_UTILITIES}
               fhiclcpp::fhiclcpp
               cetlib::cetlib
               cetlib_except::cetlib_except
             )

install_headers()
install_fhicl()
install_source()
