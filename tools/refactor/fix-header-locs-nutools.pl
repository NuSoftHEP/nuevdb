use strict;

use vars qw(%inc_translations $g4physics_list);
BEGIN { %inc_translations = (
		"EventGeneratorBase/CRY/CRYHelper.h"  => "nutools/EventGeneratorBase/CRY/CRYHelper.h",
		"EventGeneratorBase/GENIE/GENIEHelper.h"  => "nutools/EventGeneratorBase/GENIE/GENIEHelper.h",
		"EventGeneratorBase/GiBUU/GiBUUHelper.h"  => "nutools/EventGeneratorBase/GiBUU/GiBUUHelper.h",
		"EventGeneratorBase/evgenbase.h"  => "nutools/EventGeneratorBase/evgenbase.h",
		"IFDatabase/DBIService.h"  => "nutools/IFDatabase/DBIService.h",
		"IFDatabase/ColumnDef.h"  => "nutools/IFDatabase/ColumnDef.h",
		"IFDatabase/Column.h"  => "nutools/IFDatabase/Column.h",
		"IFDatabase/Util.h"  => "nutools/IFDatabase/Util.h",
		"IFDatabase/Row.h"  => "nutools/IFDatabase/Row.h",
		"IFDatabase/DataType.h"  => "nutools/IFDatabase/DataType.h",
		"IFDatabase/Table.h"  => "nutools/IFDatabase/Table.h",
		"NuBeamWeights/Conventions.h"  => "nutools/NuBeamWeights/Conventions.h",
		"NuBeamWeights/skzpReweight.h"  => "nutools/NuBeamWeights/skzpReweight.h",
		"EventDisplayBase/ScanWindow.h"  => "nutools/EventDisplayBase/ScanWindow.h",
		"EventDisplayBase/EventDisplay.h"  => "nutools/EventDisplayBase/EventDisplay.h",
		"EventDisplayBase/LinkDef.h"  => "nutools/EventDisplayBase/LinkDef.h",
		"EventDisplayBase/JobMenu.h"  => "nutools/EventDisplayBase/JobMenu.h",
		"EventDisplayBase/ScanOptions.h"  => "nutools/EventDisplayBase/ScanOptions.h",
		"EventDisplayBase/EventHolder.h"  => "nutools/EventDisplayBase/EventHolder.h",
		"EventDisplayBase/ParameterSetEdit.h"  => "nutools/EventDisplayBase/ParameterSetEdit.h",
		"EventDisplayBase/Colors.h"  => "nutools/EventDisplayBase/Colors.h",
		"EventDisplayBase/WindowMenu.h"  => "nutools/EventDisplayBase/WindowMenu.h",
		"EventDisplayBase/FileMenu.h"  => "nutools/EventDisplayBase/FileMenu.h",
		"EventDisplayBase/ListWindow.h"  => "nutools/EventDisplayBase/ListWindow.h",
		"EventDisplayBase/Functors.h"  => "nutools/EventDisplayBase/Functors.h",
		"EventDisplayBase/DisplayWindow.h"  => "nutools/EventDisplayBase/DisplayWindow.h",
		"EventDisplayBase/ObjListCanvas.h"  => "nutools/EventDisplayBase/ObjListCanvas.h",
		"EventDisplayBase/View2D.h"  => "nutools/EventDisplayBase/View2D.h",
		"EventDisplayBase/EditMenu.h"  => "nutools/EventDisplayBase/EditMenu.h",
		"EventDisplayBase/NavState.h"  => "nutools/EventDisplayBase/NavState.h",
		"EventDisplayBase/Canvas.h"  => "nutools/EventDisplayBase/Canvas.h",
		"EventDisplayBase/MenuBar.h"  => "nutools/EventDisplayBase/MenuBar.h",
		"EventDisplayBase/View3D.h"  => "nutools/EventDisplayBase/View3D.h",
		"EventDisplayBase/StatusBar.h"  => "nutools/EventDisplayBase/StatusBar.h",
		"EventDisplayBase/HelpMenu.h"  => "nutools/EventDisplayBase/HelpMenu.h",
		"EventDisplayBase/ButtonBar.h"  => "nutools/EventDisplayBase/ButtonBar.h",
		"EventDisplayBase/ColorScale.h"  => "nutools/EventDisplayBase/ColorScale.h",
		"EventDisplayBase/PrintDialog.h"  => "nutools/EventDisplayBase/PrintDialog.h",
		"EventDisplayBase/ParameterSetEditDialog.h"  => "nutools/EventDisplayBase/ParameterSetEditDialog.h",
		"EventDisplayBase/ServiceTable.h"  => "nutools/EventDisplayBase/ServiceTable.h",
		"EventDisplayBase/RootEnv.h"  => "nutools/EventDisplayBase/RootEnv.h",
		"EventDisplayBase/Printable.h"  => "nutools/EventDisplayBase/Printable.h",
		"EventDisplayBase/evdb.h"  => "nutools/EventDisplayBase/evdb.h",
		"G4Base/PrimaryParticleInformation.h"  => "nutools/G4Base/PrimaryParticleInformation.h",
		"G4Base/UserActionFactory.h"  => "nutools/G4Base/UserActionFactory.h",
		"G4Base/G4Helper.h"  => "nutools/G4Base/G4Helper.h",
		"G4Base/DetectorConstruction.h"  => "nutools/G4Base/DetectorConstruction.h",
		"G4Base/ExampleAction.h"  => "nutools/G4Base/ExampleAction.h",
		"G4Base/UserAction.h"  => "nutools/G4Base/UserAction.h",
		"G4Base/UserActionManager.h"  => "nutools/G4Base/UserActionManager.h",
		"G4Base/ConvertMCTruthToG4.h"  => "nutools/G4Base/ConvertMCTruthToG4.h",
		"MagneticField/MagneticField.h"  => "nutools/MagneticField/MagneticField.h",
		"NuReweight/art/NuReweight.h"  => "nutools/NuReweight/art/NuReweight.h",
		"NuReweight/GENIEReweight.h"  => "nutools/NuReweight/GENIEReweight.h",
		"NuReweight/ReweightLabels.h"  => "nutools/NuReweight/ReweightLabels.h",
		"G4Base/G4PhysicsProcessFactorySingleton.hh"  => "nutools/G4Base/G4PhysicsProcessFactorySingleton.hh",
		"G4Base/G4PhysListFactory.hh"  => "nutools/G4Base/G4PhysListFactory.hh",
		"G4Base/G4PhysListFactorySingleton.hh"  => "nutools/G4Base/G4PhysListFactorySingleton.hh",
		"G4Base/G4PhysicsProcessRegisterOld.icc"  => "nutools/G4Base/G4PhysicsProcessRegisterOld.icc",
		"G4Base/G4PhysListRegisterOld.icc"  => "nutools/G4Base/G4PhysListRegisterOld.icc",
		"SimulationBase/GTruth.h" => "nusimdata/SimulationBase/GTruth.h",
		"SimulationBase/MCFlux.h" => "nusimdata/SimulationBase/MCFlux.h",
		"SimulationBase/MCNeutrino.h" => "nusimdata/SimulationBase/MCNeutrino.h",
		"SimulationBase/MCParticle.h" => "nusimdata/SimulationBase/MCParticle.h",
		"SimulationBase/MCTrajectory.h" => "nusimdata/SimulationBase/MCTrajectory.h",
		"SimulationBase/MCTruth.h" => "nusimdata/SimulationBase/MCTruth.h",
		"SimulationBase/classes.h" => "nusimdata/SimulationBase/classes.h",
                            );

      }

# since we do not build G4NuPhysicsLists, we do not translate anything
# this is for future reference
BEGIN { %g4physics_list = (
 		"G4NuPhysicsLists/HadronPhysicsNuBeam.hh"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/HadronPhysicsNuBeam.hh",
		"G4NuPhysicsLists/G4PhysicsProcessFactorySingleton.hh"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/G4PhysicsProcessFactorySingleton.hh",
		"G4NuPhysicsLists/G4PhysListFactory.hh"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/G4PhysListFactory.hh",
		"G4NuPhysicsLists/QGSPStrFragmLundProtonBuilder.hh"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/QGSPStrFragmLundProtonBuilder.hh",
		"G4NuPhysicsLists/G4PhysListFactorySingleton.hh"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/G4PhysListFactorySingleton.hh",
		"G4NuPhysicsLists/NuBeam.hh"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/NuBeam.hh",
		"G4NuPhysicsLists/G4PhysicsProcessRegisterOld.icc"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/G4PhysicsProcessRegisterOld.icc",
		"G4NuPhysicsLists/G4PhysListRegisterOld.icc"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/G4PhysListRegisterOld.icc",
		"G4NuPhysicsLists/NuBeam.icc"  => "nutools/G4NuPhysicsLists/include/G4NuPhysicsLists/NuBeam.icc",
                           );

      }

foreach my $inc (sort keys %inc_translations) {
  s&^(\s*#include\s+["<])\Q$inc\E(.*)&${1}$inc_translations{$inc}${2}& and last;
}



### Local Variables:
### mode: cperl
### End:
