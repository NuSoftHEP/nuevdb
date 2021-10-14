# ======================================================================
#  nuevdb main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/nuevdb/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#
#  make
#  make test
#  make install
#  make package (builds distribution tarfile)
# ======================================================================
cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

find_package(cetmodules 2.29.06 REQUIRED)
project(nuevdb VERSION 1.05.05 LANGUAGES CXX)

include(CetCMakeEnv)
cet_cmake_env()

# -D_GLIBCXX_USE_NANOSLEEP is needed to make std::this_thread::sleep_for available. (IFDatabase)
cet_set_compiler_flags(DIAGS CAUTIOUS WERROR
                       NO_UNDEFINED
                       EXTRA_FLAGS -pedantic
                       EXTRA_CXX_FLAGS -D_GLIBCXX_USE_NANOSLEEP
                                       -Wno-unused-local-typedefs -Wno-variadic-macros)
cet_report_compiler_flags(REPORT_THRESHOLD VERBOSE)

# these are minimum required versions, not the actual product versions
find_ups_product( art ) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(art_root_io) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(canvas_root_io) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product( libwda ) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product( postgresql ) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product( nusimdata ) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product( cetlib ) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product( cetlib_except ) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_root()
find_ups_boost()

message(STATUS "ROOTSYS is ${ROOTSYS}")

# macros
include(ArtMake)
include(BuildPlugins)
include(CetRootCint)

# include search path
include_directories ( $ENV{CRYHOME}/src ) ### MIGRATE-ACTION-RECOMMENDED: use target_link_directories() with target semantics

# included for Mac OSX using XQuartz for X11
if(APPLE)
  include_directories ( /opt/X11/include ) ### MIGRATE-ACTION-RECOMMENDED: use target_link_directories() with target semantics
endif()

# source
add_subdirectory (nuevdb)

# ups - table and config files

# packaging utility

cet_cmake_config()
