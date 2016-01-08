
art_make( LIBRARY_NAME EventGeneratorBaseCRY
          LIB_LIBRARIES SimulationBase
                        ${CRY}
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${CETLIB}
                        ${ROOT_GEOM}
                        ${ROOT_GEOMPAINTER}
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


install_headers()
install_fhicl()
install_source()
