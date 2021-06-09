set( PACKAGE EventDisplayBase )
FILE( GLOB src_files *.cxx )

set( EVD_LIBS art::Framework_Services_Registry
              art::Framwork_Core
              art_Persistency_Common
              art::Framework_IO_Sources
              art_root_io::art_root_io
              art_root_io::RootInput_source
              art::Framework_Principal
              art_Persistency_Provenance
              art_Utilities
              canvas
              messagefacility::MF_MessageLogger
              messagefacility::headers
              fhiclcpp::fhiclcpp fhiclcpp::types
              cetlib
              cetlib_except
              ${Boost_SYSTEM_LIBRARY}
              ${ROOT_EVE}
              ${ROOT_EG}
              ${ROOT_GX11}
              ${ROOT_TREEPLAYER}
              ${ROOT_GEOM}
              ${ROOT_GED}
              ${ROOT_RGL}
              ${ROOT_GUI}
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
              ${ROOT_THREAD} )

cet_rootcint( ${PACKAGE} )

art_make_library( SOURCE ${src_files} ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
                  LIBRARIES nusimdata_SimulationBase ${EVD_LIBS} )

set( EVD_SERVICE_LIBS nuevdb_EventDisplayBase ${EVD_LIBS} )
simple_plugin( EventDisplay service ${EVD_SERVICE_LIBS} )
simple_plugin( ScanOptions  service ${EVD_SERVICE_LIBS} )
simple_plugin( Colors       service ${EVD_SERVICE_LIBS} )

install_headers()
install_fhicl()
install_source()
