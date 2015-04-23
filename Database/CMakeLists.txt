include_directories ( ${CMAKE_CURRENT_SOURCE_DIR} )

set( PACKAGE Database )
FILE( GLOB src_files *.cpp )

cet_find_library( LIBWDA NAMES wda PATHS ENV LIBWDA_LIB NO_DEFAULT_PATH )
cet_find_library( PQ NAMES pq PATHS ENV POSTGRESQL_LIBRARIES NO_DEFAULT_PATH )
find_library( CURL NAMES curl )

include_directories( $ENV{LIBWDA_FQ_DIR}/include )

art_make_library( LIBRARY_NAME  ${PACKAGE}
                  SOURCE ${src_files} #${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
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
               LIBRARIES Database
			 ${CURL}
               )

art_make_exec( NAME writeConditionsCSVToDB.cc
	       SOURCE writeConditionsCSVToDB.cc   
               LIBRARIES Database
			 ${CURL}
               )

art_make_exec( NAME dumpConditionsToCSV
	       SOURCE dumpConditionsToCSV.cc
               LIBRARIES Database
			 ${CURL}
               )

simple_plugin( DBI "service"
               BASENAME_ONLY
	       Database
               ${ART_FRAMEWORK_CORE}
               ${ART_FRAMEWORK_PRINCIPAL}
               ${ART_FRAMEWORK_SERVICES_REGISTRY}
               ${ART_FRAMEWORK_SERVICES_BASIC}
               ${ART_PERSISTENCY_COMMON}
               ${ART_UTILITIES}
               ${MF_MESSAGELOGGER}
               ${MF_UTILITIES}
               ${FHICLCPP}
               ${CETLIB}
             )

install_source(LIST ${src_files})
