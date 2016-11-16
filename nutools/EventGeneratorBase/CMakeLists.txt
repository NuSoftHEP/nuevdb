
simple_plugin( GeneratedEventTimestamp "plugin"
                                       ${ART_FRAMEWORK_CORE}
                                       art_Persistency_Provenance canvas_Persistency_Provenance
                                       ${MF_MESSAGELOGGER}
                                       ${MF_UTILITIES}
                                       ${CETLIB}
				       ${Boost_FILESYSTEM_LIBRARY}
				       ${Boost_SYSTEM_LIBRARY}
               
             )
 
install_headers()
install_fhicl()
install_source()

add_subdirectory(CRY)
add_subdirectory(GENIE)
add_subdirectory(GiBUU)
add_subdirectory(test)
