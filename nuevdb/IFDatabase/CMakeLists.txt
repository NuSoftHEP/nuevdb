set( PACKAGE IFDatabase )
FILE( GLOB src_files *.cpp )

cet_find_library( LIBWDA NAMES wda PATHS ENV LIBWDA_LIB NO_DEFAULT_PATH )
cet_find_library( PQ NAMES pq PATHS ENV POSTGRESQL_LIBRARIES NO_DEFAULT_PATH )
find_library( CURL NAMES curl )

include_directories( $ENV{LIBWDA_FQ_DIR}/include )

art_make_library( SOURCE ${src_files} #${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
                  LIBRARIES     ${LIBWDA}
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
               ${ART_FRAMEWORK_CORE}
               ${ART_FRAMEWORK_PRINCIPAL}
               ${ART_FRAMEWORK_SERVICES_REGISTRY}
               ${ART_FRAMEWORK_SERVICES_BASIC}
               art_Persistency_Common
               art_Utilities
               canvas
               messagefacility::MFMessageLogger
               messagefacility::headers
               fhiclcpp::fhiclcpp fhiclcpp::types
               cetlib
               cetlib_except
             )

install_headers()
install_fhicl()
install_source()
