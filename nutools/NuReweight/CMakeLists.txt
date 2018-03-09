# FIXME: these are needed for c++17
# This reflects a problem in genie
cet_add_compiler_flags(-Wno-overloaded-virtual)

##cet_add_compiler_flags(CXX -DSETDIFFXSEC_1ARG )

art_make( LIB_LIBRARIES
                        ${LOG4CPP}
                        ${XML2}
                        ${PYTHIA6}
                        ${LHAPDF}
                        ${GENIE_LIB_LIST}
                        ${ROOT_BASIC_LIB_LIST}
                        )
# message("######### ROOT_BASIC_LIB_LIST ${ROOT_BASIC_LIB_LIST}")

install_headers()
install_fhicl()
install_source()

add_subdirectory(art)
