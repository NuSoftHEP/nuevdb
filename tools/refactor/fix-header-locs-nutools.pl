use strict;

use vars qw(%inc_translations);
BEGIN { %inc_translations = (
		"SimulationBase/GTruth.h" => "nusimdata/SimulationBase/GTruth.h",
		"SimulationBase/MCFlux.h" => "nusimdata/SimulationBase/MCFlux.h",
		"SimulationBase/MCNeutrino.h" => "nusimdata/SimulationBase/MCNeutrino.h",
		"SimulationBase/MCParticle.h" => "nusimdata/SimulationBase/MCParticle.h",
		"SimulationBase/MCTrajectory.h" => "nusimdata/SimulationBase/MCTrajectory.h",
		"SimulationBase/MCTruth.h" => "nusimdata/SimulationBase/MCTruth.h",
		"SimulationBase/classes.h" => "nusimdata/SimulationBase/classes.h",
                            );

      }
foreach my $inc (sort keys %inc_translations) {
  s&^(\s*#include\s+["<])\Q$inc\E([">].*)$&${1}$inc_translations{$inc}${2}& and last;
}

### Local Variables:
### mode: cperl
### End:
