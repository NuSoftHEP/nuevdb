use strict;

use vars qw(%dir_list);
BEGIN { %dir_list = (
        "nutools_EventGeneratorBase_GENIE" => "nugen_EventGeneratorBase_GENIE",
        "nutools_EventGeneratorBase_GiBUU" => "nugen_EventGeneratorBase_GiBUU",
        "nutools_NuReweight" => "nugen_NuReweight",
        "nutools_NuReweight_art" => "nugen_NuReweight_art",
        "nutools_NuReweight_art_ReweightAna_module" => "nugen_NuReweight_art_ReweightAna_module",
                       ); }

foreach my $lib (sort keys %dir_list) {
   next if m&add_subdirectory&i;
   next if m&simple_plugin&i;
   next if m&SUBDIRNAME&i;
   next if m&SUBDIRS&i;
  #s&\b\Q${lib}\E([^\.\s]*\b)([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
  s&\b\Q${lib}\E\b([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
}
