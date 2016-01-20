
# support building for -q xx:noifdh on any system

cet_find_library( IFDH NAMES ifdh PATHS ENV IFDHC_LIB NO_DEFAULT_PATH )
if( IFDH )
  include_directories( $ENV{IFDHC_FQ_DIR}/inc )
else()
  add_definitions(-DNO_IFDH_LIB)
  SET(IFDH "")
endif()

art_make( LIBRARY_NAME EventGeneratorBaseGENIE
          LIB_LIBRARIES SimulationBase
                        ${ART_UTILITIES}
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        ${CETLIB}
                        ${XML2}
                        ${LOG4CPP}
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
                        ${IFDH}
                        ${ROOT_GEOM}
                        ${ROOT_GEOMPAINTER}
                        ${ROOT_BASIC_LIB_LIST} )

install_headers()
install_fhicl()
install_source()
