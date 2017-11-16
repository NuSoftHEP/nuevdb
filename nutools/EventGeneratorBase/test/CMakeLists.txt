
art_make( MODULE_LIBRARIES nusimdata_SimulationBase
                           nutools_EventGeneratorBase_CRY
                           nutools_EventGeneratorBase_GENIE
                        art_Framework_Services_Optional_RandomNumberGenerator_service
                        art_Framework_Services_Registry
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        ${CETLIB}
                        ${LOG4CPP}
                        ${XML2}
                        ${PYTHIA6}
                        ${LHAPDF}
                        ${GENIE_LIB_LIST}
                        ${CRY}
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
                        ${ROOT_THREAD}
                        ${ROOT_EG}
                        ${ROOT_TREEPLAYER}
                        ${ROOT_FFTW}
                        ${ROOT_REFLEX}
                        ${ROOTSYS}/lib/libEGPythia6.so
                        ${ROOT_GUI}
                        )

install_headers()
install_fhicl()
install_source( EXTRAS README )
