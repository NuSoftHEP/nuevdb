use strict;

use vars qw(%subdir_list);
use vars qw(%header_list);

# explicit headers to avoid conflicts with experiment code
BEGIN { %header_list = (
"nutools/RandomUtils/ArtState.h" => "nurandom/RandomUtils/ArtState.h",
"nutools/RandomUtils/NuRandomService.h" => "nurandom/RandomUtils/NuRandomService.h",
"nutools/RandomUtils/Providers/EventSeedInputData.h" => "nurandom/RandomUtils/Providers/EventSeedInputData.h",
"nutools/RandomUtils/Providers/MapKeyIterator.h" => "nurandom/RandomUtils/Providers/MapKeyIterator.h",
"nutools/RandomUtils/Providers/RandomPolicy.h" => "nurandom/RandomUtils/Providers/RandomPolicy.h",
"nutools/RandomUtils/Providers/EngineId.h" => "nurandom/RandomUtils/Providers/EngineId.h",
"nutools/RandomUtils/Providers/BasePolicies.h" => "nurandom/RandomUtils/Providers/BasePolicies.h",
"nutools/RandomUtils/Providers/StandardPolicies.h" => "nurandom/RandomUtils/Providers/StandardPolicies.h",
"nutools/RandomUtils/Providers/Policies.h" => "nurandom/RandomUtils/Providers/Policies.h",
"nutools/RandomUtils/Providers/PerEventPolicy.h" => "nurandom/RandomUtils/Providers/PerEventPolicy.h",
"nutools/RandomUtils/Providers/SeedMaster.h" => "nurandom/RandomUtils/Providers/SeedMaster.h",
"nutools/RandomUtils/Providers/RandomSeedPolicyBase.h" => "nurandom/RandomUtils/Providers/RandomSeedPolicyBase.h"
		       ); }

foreach my $inc (sort keys %header_list) {
  s&^(\s*#include\s+["<])\Q$inc\E(.*)&${1}$header_list{$inc}${2}& and last;
}

# also deal with definitions
s/NUTOOLS_RANDOMUTILS_NuRandomService_USECLHEP/NURANDOM_RANDOMUTILS_NuRandomService_USECLHEP/g;
s/NUTOOLS_RANDOMUTILS_NURANDOMSERVICE_USECLHEP/NURANDOM_RANDOMUTILS_NuRandomService_USECLHEP/g;
s/NUTOOLS_RANDOMUTILS_NuRandomService_USEROOT/NURANDOM_RANDOMUTILS_NuRandomService_USEROOT/g;
