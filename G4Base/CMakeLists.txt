
art_make( LIBRARY_NAME G4Base
          LIB_LIBRARIES SimulationBase
                        MagneticField_service
                        ${MF_MESSAGELOGGER}
                        ${MF_UTILITIES}
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
