#
# CMakeLists.txt for G4NuPhysicsLists
#
# setup UPS products for ROOT (and get Geant4 + gcc)
#
#  WHICH_G4=`ls -d ${GEANT4_FQ_DIR}/lib64/Geant4-*`
#  WHICH_CLHEP=`ls -d ${CLHEP_LIB_DIR}/CLHEP-*`
# 
#  cmake -DGeant4_DIR=${WHICH_G4} -DCLHEP_DIR=${WHICH_CLHEP} \
#        -DCMAKE_INSTALL_PREFIX=/path/to/install/dir  /path/to/src
#
# expects following environment variables are set:
#     [none]
#
#
#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(G4NuPhysicsLists)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

#set(CMAKE_INSTALL_PREFIX ../G4NuPhysicsLists-install)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

find_package(CLHEP REQUIRED)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/G4NuPhysicsLists)

#MESSAGE("Geant4_DIR=${Geant4_DIR}")
#MESSAGE("Geant4_INCLUDE_DIRS=${Geant4_INCLUDE_DIRS}")
#MESSAGE("Geant4_INCLUDE_DIR=${Geant4_INCLUDE_DIR}")

# Geant4_INCLUDE_DIR --->  /path/to/version/include/Geant4 
#        ... but also want /path/to/version/include
# INCLUDE_DIRS also includes paths for CLHEP and XERCES_C
include_directories(${Geant4_INCLUDE_DIR}/..)  # allow for #include "Geant4/file.h"
include_directories(${Geant4_INCLUDE_DIRS})    # all the standard paths

#link_directories(${Geant4_LIBRARY_DIR})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_library(g4nupl SHARED ${sources})
target_link_libraries(g4nupl ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy include and lib areas to final resting place

INSTALL (
  DIRECTORY   include/G4NuPhysicsLists
  DESTINATION include
  PATTERN ".svn" EXCLUDE
  )
INSTALL(TARGETS g4nupl DESTINATION lib)

