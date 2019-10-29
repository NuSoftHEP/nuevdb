
art_make( LIB_LIBRARIES nusimdata_SimulationBase
                        ${CRY}
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        cetlib
			cetlib_except
                        ${ROOT_GEOM}
                        ${ROOT_GEOMPAINTER}
			${ROOT_EG}
                        ${ROOT_BASIC_LIB_LIST} )


install_headers()
install_fhicl()
install_source()
