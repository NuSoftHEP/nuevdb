
set( IFDH_AS_SERVICE_LIB_LIST
           art_Framework_Services_Registry
           ${Boost_SYSTEM_LIBRARY}
           ${IFDH_SERVICE} )

art_make( LIB_LIBRARIES nusimdata_SimulationBase
                        art_Framework_Principal
                        art_Utilities
                        ${IFDH_AS_SERVICE_LIB_LIST}
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
                        ${CLHEP}
                        ${IFDH}
                        ${ROOT_GEOM}
                        ${ROOT_GEOMPAINTER}
                        ${ROOT_BASIC_LIB_LIST} )


install_headers()
install_fhicl()
install_source()
