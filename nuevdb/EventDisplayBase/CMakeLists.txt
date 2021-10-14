set( PACKAGE EventDisplayBase )

find_package(ROOT COMPONENTS Eve EG GX11 TreePlayer Geom Ged RGL Gui Core RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread REQUIRED PUBLIC)

FILE( GLOB src_files *.cxx )

cet_rootcint( ${PACKAGE} )

art_make_library( SOURCE ${src_files} ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Cint.cc
                  LIBRARIES nusimdata::SimulationBase
              art::Framework_Services_Registry
              art::Framework_Core
              art::Persistency_Common
              art::Framework_IO_Sources
              art_root_io::art_root_io
              art_root_io::RootInput_source
              art::Framework_Principal
              art::Persistency_Provenance
              art::Utilities
              canvas::canvas
              messagefacility::MF_MessageLogger
              fhiclcpp::fhiclcpp
              cetlib::cetlib
              cetlib_except::cetlib_except
              ROOT::Eve
              ROOT::EG
              ROOT::GX11
              ROOT::TreePlayer
              ROOT::Geom
              ROOT::Ged
              ROOT::RGL
              ROOT::Gui
              ROOT::Core
              ROOT::RIO
              ROOT::Net
              ROOT::Hist
              ROOT::Graf
              ROOT::Graf3d
              ROOT::Gpad
              ROOT::Tree
              ROOT::Rint
              ROOT::Postscript
              ROOT::Matrix
              ROOT::Physics
              ROOT::MathCore
              ROOT::Thread )

#set( EVD_SERVICE_LIBS nuevdb::EventDisplayBase ${EVD_LIBS} )
#simple_plugin( EventDisplay service ${EVD_SERVICE_LIBS} )
#simple_plugin( ScanOptions  service ${EVD_SERVICE_LIBS} )
#simple_plugin( Colors       service ${EVD_SERVICE_LIBS} )
cet_build_plugin(EventDisplay art::service
  LIBRARIES nuevdb::EventDisplayBase
  art::Framework_Services_Registry art::Framework_Core
  art::Persistency_Common art::Framework_IO_Sources art_root_io::art_root_io
  art_root_io::RootInput_source art::Framework_Principal
  art::Persistency_Provenance art::Utilities canvas::canvas
  messagefacility::MF_MessageLogger fhiclcpp::fhiclcpp cetlib::cetlib
  cetlib_except::cetlib_except ROOT::Eve ROOT::EG ROOT::GX11 ROOT::TreePlayer
  ROOT::Geom ROOT::Ged ROOT::RGL ROOT::Gui ROOT::Core ROOT::RIO ROOT::Net
  ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::Tree ROOT::Rint
  ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread)
cet_build_plugin(ScanOptions art::service
  LIBRARIES nuevdb::EventDisplayBase
  art::Framework_Services_Registry art::Framework_Core
  art::Persistency_Common art::Framework_IO_Sources art_root_io::art_root_io
  art_root_io::RootInput_source art::Framework_Principal
  art::Persistency_Provenance art::Utilities canvas::canvas
  messagefacility::MF_MessageLogger fhiclcpp::fhiclcpp cetlib::cetlib
  cetlib_except::cetlib_except ROOT::Eve ROOT::EG ROOT::GX11 ROOT::TreePlayer
  ROOT::Geom ROOT::Ged ROOT::RGL ROOT::Gui ROOT::Core ROOT::RIO ROOT::Net
  ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::Tree ROOT::Rint
  ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread)
cet_build_plugin(Colors art::service
  LIBRARIES nuevdb::EventDisplayBase
  art::Framework_Services_Registry art::Framework_Core
  art::Persistency_Common art::Framework_IO_Sources art_root_io::art_root_io
  art_root_io::RootInput_source art::Framework_Principal
  art::Persistency_Provenance art::Utilities canvas::canvas
  messagefacility::MF_MessageLogger fhiclcpp::fhiclcpp cetlib::cetlib
  cetlib_except::cetlib_except ROOT::Eve ROOT::EG ROOT::GX11 ROOT::TreePlayer
  ROOT::Geom ROOT::Ged ROOT::RGL ROOT::Gui ROOT::Core ROOT::RIO ROOT::Net
  ROOT::Hist ROOT::Graf ROOT::Graf3d ROOT::Gpad ROOT::Tree ROOT::Rint
  ROOT::Postscript ROOT::Matrix ROOT::Physics ROOT::MathCore ROOT::Thread)

install_headers()
install_fhicl()
install_source()
