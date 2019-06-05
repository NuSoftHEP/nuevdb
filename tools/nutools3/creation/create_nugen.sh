#!/bin/bash

#  BEFORE YOU RUN THIS SCRIPT:
#  git remote remove origin

#  AFTER EVERYTHING IS WORKING
# remote add origin ssh://p-nurandom@cdcvs.fnal.gov/cvs/projects/nurandom
# git remote -v
# git push -u origin develop

# use git-delete-history.sh

cd /home/garren/scratch/larsoft/nu/srcs/nugen
/home/garren/larsoft/laradmin/svnToGit/helpers/git-delete-history.sh \
  test \
  tools \
  nutools/EventDisplayBase \
  nutools/EventGeneratorBase/GeneratedEventTimestamp_plugin.cc \
  nutools/EventGeneratorBase/CRY \
  nutools/G4Base \
  nutools/G4NuPhysicsLists \
  nutools/IFDatabase \
  nutools/MagneticField \
  nutools/NuBeamWeights \
  nutools/ParticleNavigation \
  nutools/RandomUtils

git repack -A
git gc --aggressive

# now rename
git mv nutools nugen

exit 0

