find_package(ROOT COMPONENTS Tree Gui REQUIRED EXPORT)
find_package(ROOT COMPONENTS Core Graf Graf3d Rint REQUIRED)

FILE( GLOB src_files *.cxx )

cet_rootcint( EventDisplayBase )

art_make_library( SOURCE ${src_files} ${CMAKE_CURRENT_BINARY_DIR}/EventDisplayBaseCint.cc
                  LIBRARIES PRIVATE
              nusimdata::SimulationBase
              art::Framework_Services_Registry
              art::Framework_Core
              art::Persistency_Common
              art::Framework_IO_Sources
              art_root_io::art_root_io
              art_root_io::RootInput_source
              art::Framework_Principal
              art::Persistency_Provenance
              art::Utilities
              messagefacility::MF_MessageLogger
              fhiclcpp::fhiclcpp
              cetlib::cetlib
              cetlib_except::cetlib_except
              ROOT::Graf
              ROOT::Graf3d
              ROOT::Gui
              ROOT::Rint
              ROOT::Core
              )

cet_build_plugin(EventDisplay art::service
  LIBRARIES PRIVATE nuevdb::EventDisplayBase
              PUBLIC
              art_root_io::RootInput_source
              ROOT::Tree
  )
cet_build_plugin(ScanOptions art::service
  LIBRARIES PRIVATE nuevdb::EventDisplayBase
              PUBLIC
              ROOT::Gui
  )
cet_build_plugin(Colors art::service
  LIBRARIES PRIVATE nuevdb::EventDisplayBase
              PUBLIC
              ROOT::Tree
  )

install_headers()
install_fhicl()
install_source()
