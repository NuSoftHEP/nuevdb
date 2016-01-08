set( SIMB_LIBS ${ART_FRAMEWORK_SERVICES_REGISTRY}
               ${ART_FRAMEWORK_CORE}
               ${ART_FRAMEWORK_IO_ROOT_ROOTINPUT_SOURCE}
               ${ART_PERSISTENCY_COMMON}
               ${ART_FRAMEWORK_IO_SOURCES}
               ${ART_FRAMEWORK_PRINCIPAL}
               ${ART_PERSISTENCY_PROVENANCE}
               ${ART_UTILITIES}
               ${FHICLCPP}
               ${CETLIB}
               ${ROOT_EG}
               ${ROOT_CORE}
               ${ROOT_CINT} 
               ${ROOT_RIO}
               ${ROOT_NET}
               ${ROOT_HIST} 
               ${ROOT_GRAF}
               ${ROOT_GRAF3D}
               ${ROOT_GPAD}
               ${ROOT_TREE}
               ${ROOT_RINT}
               ${ROOT_POSTSCRIPT}
               ${ROOT_MATRIX}
               ${ROOT_PHYSICS}
               ${ROOT_MATHCORE}
               ${ROOT_THREAD} )

art_make( LIBRARY_NAME SimulationBase
          LIB_LIBRARIES ${SIMB_LIBS} )
 
install_headers()
install_fhicl()
install_source()
