
##cet_add_compiler_flags(CXX -DSETDIFFXSEC_1ARG )

art_make( LIB_LIBRARIES
                        ${LOG4CPP}
                        ${XML2}
                        ${PYTHIA6}
                        ${LHAPDF}
                        ${GENIE_LIB_LIST}
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
                        ${ROOT_THREAD}
                        )
 
install_headers()
install_fhicl()
install_source()

add_subdirectory(art)
