use strict;

use vars qw(%dir_list);
BEGIN { %dir_list = (
        "nutools_RandomUtils_nurandom_service" => "nurandom_RandomUtils_nurandom_service",
        "nutools_RandomUtils_NuRandomService_service" => "nurandom_RandomUtils_NuRandomService_service",
        "nutools_test_RandomUtils" => "nurandom_test_RandomUtils",
        "NUTOOLS_RANDOMUTILS_NURANDOM_SERVICE" => "NURANDOM_RANDOMUTILS_NURANDOM_SERVICE",
        "NUTOOLS_RANDOMUTILS_NURANDOMSERVICE_SERVICE" => "NURANDOM_RANDOMUTILS_NURANDOMSERVICE_SERVICE",
        "NUTOOLS_TEST_RANDOMUTILS" => "NURANDOM_TEST_RANDOMUTILS"
                       ); }

foreach my $lib (sort keys %dir_list) {
   next if m&add_subdirectory&i;
   next if m&simple_plugin&i;
   next if m&SUBDIRNAME&i;
   next if m&SUBDIRS&i;
  #s&\b\Q${lib}\E([^\.\s]*\b)([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
  s&\b\Q${lib}\E\b([^\.]|$)&$dir_list{$lib}${1}${2}&g and last;
}
