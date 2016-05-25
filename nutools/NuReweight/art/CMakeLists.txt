
set( NURW_LIBS ${ART_FRAMEWORK_SERVICES_REGISTRY}
               ${ART_FRAMEWORK_SERVICES_OPTIONAL}
               ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
               ${ART_FRAMEWORK_CORE}
               art_Persistency_Common canvas_Persistency_Common
               ${ART_FRAMEWORK_IO_SOURCES}
               ${ART_FRAMEWORK_IO_ROOT}
               ${ART_FRAMEWORK_IO_ROOT_ROOTINPUT_SOURCE}
               ${ART_FRAMEWORK_PRINCIPAL}
               art_Persistency_Provenance canvas_Persistency_Provenance
               art_Utilities canvas_Utilities
               ${MF_MESSAGELOGGER}
               ${MF_UTILITIES}
               ${FHICLCPP}
               ${CETLIB}
               ${LOG4CPP}
               ${XML2}
               ${PYTHIA6}
               ${LHAPDF}
               ${GALGORITHM}
               ${GBARYONRESONANCE}
               ${GBASE}
               ${GBODEKYANG}
               ${GCHARM}
               ${GCOH}
               ${GDFRC}
               ${GDIS}
               ${GCROSSSECTIONS}
               ${GDECAY}
               ${GELAS}
               ${GELFF}
               ${GHEP}
               ${GEVGCORE}
               ${GEVGMODULES}
               ${GEVGDRIVERS}
               ${GGIBUU}
               ${GHADRONTRANSP}
               ${GFRAGMENTATION}
               ${GINTERACTION}
               ${GLLEWELLYNSMITH}
               ${GMEC}
               ${GMESSENGER}
               ${GNUGAMMA}
               ${GNUE}
               ${GNTUPLE}
               ${GNUCLEAR}
               ${GNUMERICAL}
               ${GQPM}
               ${GPDG}
               ${GPDF}
               ${GQEL}
               ${GRES}
               ${GREGISTRY}
               ${GREINSEHGAL}
               ${GUTILS}
               ${GGEO}
               ${GFLUXDRIVERS}
               ${GMUELOSS}
               ${GREWEIGHT}
               ${GNUCLEONDECAY}
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

art_make_library( SOURCE NuReweight.cxx
                  LIBRARIES nusimdata_SimulationBase
                            NuReweight
                            ${NURW_LIBS} )

simple_plugin( ReweightAna module NuReweightArt
                                  NuReweight
                                  nusimdata_SimulationBase
                                 ${NURW_LIBS} )

install_headers()
install_fhicl()
install_source()
