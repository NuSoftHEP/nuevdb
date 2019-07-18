use strict;

use vars qw(%dir_list);
BEGIN { %dir_list = (
  "nutools_G4Base" => "nug4_G4Base",
  "nutools_MagneticField_MagneticField_service" => "nug4_MagneticField_MagneticField_service",
  "nutools_ParticleNavigation" => "nug4_ParticleNavigation"
                       ); }

foreach my $lib (sort keys %dir_list) {
   next if m&add_subdirectory&i;
   next if m&simple_plugin&i;
   next if m&SUBDIRNAME&i;
   next if m&SUBDIRS&i;
  #s&\b\Q${lib}\E([^\.\s]*\b)([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
  s&\b\Q${lib}\E\b([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
}
