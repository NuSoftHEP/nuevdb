#ifndef EVGENBASE_H
#define EVGENBASE_H

#include "TRandom3.h"

/// Physics generators for neutrinos, cosmic rays, and others
namespace evgb {
  /// Enumerate mother codes for primary particles. 
  ///
  /// Normally the mother code for a primary particle would be set to
  /// some arbitrary invalid value like -1, however, we can use this
  /// to mark the source of the particle as being either, eg.,
  /// neutrino induced or from cosmic-rays.
  enum {
    kNeutrinoGenerator  = -100,
    kCosmicRayGenerator = -200
  };

  unsigned int GetRandomNumberSeed();
}

inline unsigned int evgb::GetRandomNumberSeed()
{

  // the maximum allowed seed for the art::RandomNumberGenerator
  // is 900000000. Use the system random number generator to get a pseudo-random
  // number for the seed value, and take the modulus of the maximum allowed 
  // seed to ensure we don't ever go over that maximum
  
  // Set gRandom to be a TRandom3 based on this state in case we need to pull
  // random values from histograms, etc
  TRandom3 *rand = new TRandom3(0);
    gRandom = rand;
    return rand->Integer(900000000);
}

#endif
