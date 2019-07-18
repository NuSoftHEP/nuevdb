use strict;

use vars qw(%subdir_list);
use vars qw(%header_list);

# explicit headers to avoid conflicts with experiment code
BEGIN { %header_list = (
"nutools/ParticleNavigation/EmEveIdCalculator.h" => "nug4/ParticleNavigation/EmEveIdCalculator.h",
"nutools/ParticleNavigation/ParticleList.h" => "nug4/ParticleNavigation/ParticleList.h",
"nutools/ParticleNavigation/ParticleHistory.h" => "nug4/ParticleNavigation/ParticleHistory.h",
"nutools/ParticleNavigation/EveIdCalculator.h" => "nug4/ParticleNavigation/EveIdCalculator.h",
"nutools/MagneticField/.h" => "nug4/MagneticField/MagneticField.h",
"nutools/G4Base/UserAction.h" => "nug4/G4Base/UserAction.h",
"nutools/G4Base/ConvertMCTruthToG4.h" => "nug4/G4Base/ConvertMCTruthToG4.h",
"nutools/G4Base/UserActionManager.h" => "nug4/G4Base/UserActionManager.h",
"nutools/G4Base/DetectorConstruction.h" => "nug4/G4Base/DetectorConstruction.h",
"nutools/G4Base/PrimaryParticleInformation.h" => "nug4/G4Base/PrimaryParticleInformation.h",
"nutools/G4Base/ExampleAction.h" => "nug4/G4Base/ExampleAction.h",
"nutools/G4Base/UserActionFactory.h" => "nug4/G4Base/UserActionFactory.h",
"nutools/G4Base/G4Helper.h" => "nug4/G4Base/G4Helper.h"
		       ); }

foreach my $inc (sort keys %header_list) {
  s&^(\s*#include\s+["<])\Q$inc\E(.*)&${1}$header_list{$inc}${2}& and last;
}
