
simple_plugin( MagneticField service
                        ${CLHEP}
			${Boost_FILESYSTEM_LIBRARY}
			${Boost_SYSTEM_LIBRARY}
                        BASENAME_ONLY )

install_headers()
install_fhicl()
install_source()
