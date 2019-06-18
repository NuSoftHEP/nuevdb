use strict;

use vars qw(%subdir_list);
use vars qw(%header_list);

# explicit headers to avoid conflicts with experiment code
BEGIN { %header_list = (
"nutools/EventGeneratorBase/evgenbase.h" => "nugen/EventGeneratorBase/evgenbase.h",
"nutools/EventGeneratorBase/GENIE/EvtTimeFlat.h" => "nugen/EventGeneratorBase/GENIE/EvtTimeFlat.h",
"nutools/EventGeneratorBase/GENIE/EVGBAssociationUtil.h" => "nugen/EventGeneratorBase/GENIE/EVGBAssociationUtil.h",
"nutools/EventGeneratorBase/GENIE/EvtTimeFNALBeam.h" => "nugen/EventGeneratorBase/GENIE/EvtTimeFNALBeam.h",
"nutools/EventGeneratorBase/GENIE/GENIEHelper.h" => "nugen/EventGeneratorBase/GENIE/GENIEHelper.h",
"nutools/EventGeneratorBase/GENIE/MCTruthAndFriendsItr.h" => "nugen/EventGeneratorBase/GENIE/MCTruthAndFriendsItr.h",
"nutools/EventGeneratorBase/GENIE/EvtTimeNone.h" => "nugen/EventGeneratorBase/GENIE/EvtTimeNone.h",
"nutools/EventGeneratorBase/GENIE/EvtTimeShiftFactory.h" => "nugen/EventGeneratorBase/GENIE/EvtTimeShiftFactory.h",
"nutools/EventGeneratorBase/GENIE/GENIE2ART.h" => "nugen/EventGeneratorBase/GENIE/GENIE2ART.h",
"nutools/EventGeneratorBase/GENIE/EvtTimeShiftI.h" => "nugen/EventGeneratorBase/GENIE/EvtTimeShiftI.h",
"nutools/EventGeneratorBase/GiBUU/GiBUUHelper.h" => "nugen/EventGeneratorBase/GiBUU/GiBUUHelper.h",
"nutools/NuReweight/ReweightLabels.h" => "nugen/NuReweight/ReweightLabels.h",
"nutools/NuReweight/GENIEReweight.h" => "nugen/NuReweight/GENIEReweight.h",
"nutools/NuReweight/art/NuReweight.h" => "nugen/NuReweight/art/NuReweight.h"
		       ); }

foreach my $inc (sort keys %header_list) {
  s&^(\s*#include\s+["<])\Q$inc\E(.*)&${1}$header_list{$inc}${2}& and last;
}
