
art_make( LIB_LIBRARIES nusimdata_SimulationBase
                        art_Utilities
                        canvas
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        ${CETLIB}
                        cetlib_except
                        ${XML2}
                        ${LOG4CPP}
                        ${PYTHIA6}
                        ${LHAPDF}
                        ${GENIE_LIB_LIST}
                        ${DK2NU_TREE}
                        ${DK2NU_GENIE}
                        ${IFDH}
                        ${ROOT_GEOM}
                        ${ROOT_GEOMPAINTER}
                        ${ROOT_BASIC_LIB_LIST} )

install_headers()
install_fhicl()
install_source()
