
simple_plugin( MagneticField service
               ${MF_MESSAGELOGGER}
               ${MF_UTILITIES}
	       cetlib_except
               ${CLHEP}
               ${ROOT_BASIC_LIB_LIST}
               ${ROOT_GEOM} )

install_headers()
install_fhicl()
install_source()
