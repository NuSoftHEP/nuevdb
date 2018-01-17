
set( NURW_LIBS ${ART_FRAMEWORK_SERVICES_REGISTRY}
               ${ART_FRAMEWORK_SERVICES_OPTIONAL}
               ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
               ${ART_FRAMEWORK_CORE}
               art_Persistency_Common
               ${ART_FRAMEWORK_IO_SOURCES}
               ${ART_FRAMEWORK_IO_ROOT}
               ${ART_FRAMEWORK_IO_ROOT_ROOTINPUT_SOURCE}
               ${ART_FRAMEWORK_PRINCIPAL}
               art_Persistency_Provenance
               art_Utilities
               canvas
               ${MF_MESSAGELOGGER}
               ${MF_UTILITIES}
               ${FHICLCPP}
               cetlib
               cetlib_except
               ${LOG4CPP}
               ${XML2}
               ${PYTHIA6}
               ${LHAPDF}
               ${GENIE_LIB_LIST}
               ${ROOT_BASIC_LIB_LIST}
               ${ROOT_EG}
               ${ROOT_TREEPLAYER}
               ${ROOT_FFTW}
               ${ROOT_REFLEX}
               ${ROOTSYS}/lib/libEGPythia6.so
               ${ROOT_GUI}
           )

art_make_library( SOURCE NuReweight.cxx
                  LIBRARIES nusimdata_SimulationBase
                            nutools_NuReweight
                            ${NURW_LIBS} )

simple_plugin( ReweightAna module nutools_NuReweight_art
                                  nutools_NuReweight
                                  nusimdata_SimulationBase
                                 ${NURW_LIBS} )

install_headers()
install_fhicl()
install_source()
