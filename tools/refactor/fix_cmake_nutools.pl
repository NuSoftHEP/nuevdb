use strict;

use vars qw(%dir_list);
BEGIN { %dir_list = (
	"Colors_service" => "nutools_Colors_service",
	"DBI_service" => "nutools_DBI_service",
	"EventDisplayBase" => "nutools_EventDisplayBase",
	"EventDisplay_service" => "nutools_EventDisplay_service",
	"EventGeneratorBaseCRY" => "nutools_EventGeneratorBase_CRY",
	"EventGeneratorBaseGENIE" => "nutools_EventGeneratorBase_GENIE",
	"EventGeneratorBaseGiBUU" => "nutools_EventGeneratorBase_GiBUU",
	"G4Base" => "nutools_G4Base",
	"IFDatabase" => "nutools_IFDatabase",
	"MagneticField_service" => "nutools_MagneticField_service",
	"NuBeamWeights" => "nutools_NuBeamWeights",
	"NuReweightArt" => "nutools_NuReweightArt",
	"NuReweight" => "nutools_NuReweight",
	"ReweightAna_module" => "nutools_ReweightAna_module",
	"ScanOptions_service" => "nutools_ScanOptions_service",
	"SimulationBase_dict" => "nutools_SimulationBase_dict",
	"SimulationBase" => "nusimdata_SimulationBase"
		       ); }

foreach my $lib (sort keys %dir_list) {
   next if m&add_subdirectory&i;
   next if m&simple_plugin&i;
   next if m&SUBDIRNAME&i;
   next if m&SUBDIRS&i;
   next if m&LIBRARY_NAME&i;
   next if m&PACKAGE&i;
  #s&\b\Q${lib}\E([^\.\s]*\b)([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
  s&\b\Q${lib}\E\b([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
}

#s&\$\{SIMULATIONBASE\}&nusimdata_SimulationBase&g;
#	"EventGeneratorBase_test_EventGeneratorTest_module" => "nutools_",
