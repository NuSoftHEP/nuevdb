# FIXME: these are needed for c++17
# This reflects a problem in genie
cet_add_compiler_flags(-Wno-overloaded-virtual)

art_make( MODULE_LIBRARIES nusimdata_SimulationBase
                           nutools_EventGeneratorBase_CRY
                           nutools_EventGeneratorBase_GENIE
                        art_Framework_Services_Optional_RandomNumberGenerator_service
                        art_Framework_Services_Registry
# for MixGenieEvents_module
#                        art_Framework_Principal
#                        art_Framework_Services_Optional_TFileService_service
#                        art_Framework_IO_ProductMix
#                        art_Framework_IO_Root
#                        art_Framework_Services_Optional
# end-of-list for MixGenieEvents_module
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        ${CETLIB}
                        ${LOG4CPP}
                        ${XML2}
                        ${PYTHIA6}
                        ${LHAPDF}
                        ${GENIE_LIB_LIST}
                        ${CLHEP}
                        ${DK2NU_TREE}
                        ${CRY}
                        ${ROOT_BASIC_LIB_LIST}
                        ${ROOT_EG}
                        ${ROOT_TREEPLAYER}
                        ${ROOT_FFTW}
                        ${ROOT_REFLEX}
                        ${ROOTSYS}/lib/libEGPythia6.so
                        ${ROOT_GUI}
                        )

install_headers()
install_fhicl()
install_source( EXTRAS README )
