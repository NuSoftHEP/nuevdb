set( PACKAGE EventDisplayBase )
FILE( GLOB src_files *.cxx )

set( EVD_LIBS ${ART_FRAMEWORK_SERVICES_REGISTRY}
              ${ART_FRAMEWORK_CORE}
              art_Persistency_Common
              ${ART_FRAMEWORK_IO_SOURCES}
              ${ART_ROOT_IO}
              ${ART_ROOT_IO_ROOTINPUT_SOURCE}
              ${ART_FRAMEWORK_PRINCIPAL}
              art_Persistency_Provenance
              art_Utilities
              canvas
              ${MF_MESSAGELOGGER}
              ${MF_UTILITIES}
              ${FHICLCPP}
              cetlib
              cetlib_except
              ${Boost_SYSTEM_LIBRARY}
              ROOT::EG
              ROOT::GX11
              ROOT::TreePlayer
              ROOT::Geom
              ROOT::Ged
              ROOT::RGL
              ROOT::Gui
              ROOT::Core
              ROOT::RIO
              ROOT::Rint
              ROOT::Net
              ROOT::Hist
              ROOT::Graf
              ROOT::Graf3d
              ROOT::Gpad
              ROOT::Tree
              ROOT::Postscript
              ROOT::Matrix
              ROOT::Physics
              ROOT::MathCore
              ROOT::Thread )

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
