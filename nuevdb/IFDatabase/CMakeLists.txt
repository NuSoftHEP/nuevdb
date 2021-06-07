set( PACKAGE IFDatabase )
FILE( GLOB src_files *.cpp )

cet_find_library( LIBWDA NAMES wda PATHS ${CMAKE_PREFIX_PATH})
find_library( CURL NAMES curl )

include_directories( ${LIBWDA_DIR}/include )

art_make_library( SOURCE ${src_files}
                  LIBRARIES     ${LIBWDA}
                                ${BOOST_DATE_TIME}
                                ${BOOST_SYSTEM}
                                ${BOOST_FILESYSTEM}
                                ${BOOST_THREAD}
                                ${BOOST_REGEX}
                                ${XCLIB}
                                ${PostgreSQL_LIBRARIES}
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
               ${MF_MESSAGELOGGER}
               ${MF_UTILITIES}
               ${FHICLCPP}
               cetlib
               cetlib_except
             )

install_headers()
install_fhicl()
