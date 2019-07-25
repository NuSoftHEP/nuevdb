#!/bin/bash

#  BEFORE YOU RUN THIS SCRIPT:
# git clone ssh://p-nutools@cdcvs.fnal.gov/cvs/projects/nutools nurandom
# cd nurandom
# git remote remove origin
# git mv nutools/EventGeneratorBase/GeneratedEventTimestamp_plugin.cc nutools/RandomUtils/GeneratedEventTimestamp_plugin.cc
# git mv test/EventGeneratorBase test/GeneratedEventTimestamp
# git commit

#  AFTER EVERYTHING IS WORKING
# remote add origin ssh://p-nurandom@cdcvs.fnal.gov/cvs/projects/nurandom
# git remote -v
# git push -u origin develop

# use git-delete-history.sh

cd /home/garren/scratch/larsoft/nu/srcs/nurandom
/home/garren/larsoft/laradmin/svnToGit/helpers/git-delete-history.sh \
  tools \
  nutools/EventDisplayBase \
  nutools/EventGeneratorBase \
  nutools/G4Base \
  nutools/G4NuPhysicsLists \
  nutools/IFDatabase \
  nutools/MagneticField \
  nutools/NuBeamWeights \
  nutools/NuReweight \
  nutools/ParticleNavigation

git repack -A
git gc --aggressive

# now rename
git mv nutools nurandom

exit 0

