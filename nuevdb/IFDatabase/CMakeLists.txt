set( PACKAGE IFDatabase )
FILE( GLOB src_files *.cpp )

find_library( WDA     NAMES wda     PATHS ENV LIBWDA_LIB)
get_filename_component(WDA_LIB     "${WDA}"     DIRECTORY)
include_directories("${WDA_LIB}/../include")

cet_find_library( PQ NAMES pq PATHS ENV POSTGRESQL_LIBRARIES )

find_library( CURL NAMES curl )
get_filename_component(CURL_LIB     ${CURL}     DIRECTORY)
include_directories("${CURL_LIB}/../include")

art_make_library( SOURCE ${src_files} #${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
                  LIBRARIES     ${WDA}
                                ${BOOST_DATE_TIME}
                                ${BOOST_SYSTEM}
                                ${BOOST_FILESYSTEM}
                                ${BOOST_THREAD}
                                ${BOOST_REGEX}
                                ${XCLIB}
                                ${PQ}
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
               art::Framework_Services_Registry art::Framework_Services_FileServiceInterfaces
               art_Persistency_Common
               art_Utilities
               canvas
               messagefacility::MF_MessageLogger
               messagefacility::headers
               fhiclcpp::fhiclcpp fhiclcpp::types
               cetlib
               cetlib_except
             )

install_headers()
install_fhicl()
install_source()
