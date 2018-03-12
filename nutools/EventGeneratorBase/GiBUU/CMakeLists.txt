
art_make( LIB_LIBRARIES nutools_EventGeneratorBase_GENIE
                        nusimdata_SimulationBase
                        art_Utilities
                        canvas
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        ${CETLIB}
                        ${XML2}
                        ${LOG4CPP}
                        ${PYTHIA6}
                        ${LHAPDF}
                        ${GENIE_LIB_LIST}
                        ${IFDH}
                        ${ROOT_GEOM}
                        ${ROOT_GEOMPAINTER}
                        ${ROOT_BASIC_LIB_LIST} )

install_headers()
install_fhicl()
install_source()
