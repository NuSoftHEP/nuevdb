
art_make( LIB_LIBRARIES nusimdata_SimulationBase
                        nutools_MagneticField_MagneticField_service
			canvas_Persistency_Provenance
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
                        ${FHICLCPP}
                        ${CETLIB}
                        ${G4FR}
                        ${G4GMOCREN}
                        ${G4RAYTRACER}
                        ${G4TREE}
                        ${G4VRML}
                        ${G4EVENT}
                        ${G4GEOMETRY}
                        ${G4GLOBAL}
                        ${G4INTERCOMS}
                        ${G4MATERIALS}
                        ${G4MODELING}
                        ${G4PARTICLES}
                        ${G4PERSISTENCY}
                        ${G4PHYSICSLISTS}
                        ${G4PROCESSES}
                        ${G4READOUT}
                        ${G4RUN}
                        ${G4TRACKING}
                        ${G4VISHEPREP}
                        ${G4VIS_MANAGEMENT}
                        ${XERCESC}
                        ${CLHEP}
                        )

install_headers()
install_fhicl()
install_source()
