////////////////////////////////////////////////////////////////////////
/// \file Conventions.h
///
/// \version $Id: Conventions.h,v 1.1.1.1 2011-01-27 19:06:32 p-nusoftart Exp $
/// \author  ???
////////////////////////////////////////////////////////////////////////
#ifndef NBW_CONVENTIONS_H
#define NBW_CONVENTIONS_H

namespace nbw{

  class Conventions {
    
    public:
    
    //PDG hadronic particle types 
    typedef enum EParticleType 
    {
      kPiPlus  = 8,
      kPiMinus = 9,
      kKPlus   = 11,
      kKMinus  = 12,
      kK0L     = 10,
      kUnknown = 0
    } ParticleType_t;

    //beam systematic effects used exclusively for skzpReweight
    typedef enum EBeamSys 
    {
      kUnknownEff  = 0,
      kHornIMiscal = 1,
      kHornIDist   = 2,
      kBeamSysEnd  = 3
    } BeamSys_t;
    
    //Possible beam configurations
    typedef enum EBeamType
    {
      kUnknownBeam  =  0,
      kLE           =  1,
      kLE010z185i   =  2,
      kLE100z200i   =  3,
      kLE250z200i   =  4,
      kLE010z185iL  =  5,
      kLE010z170i   =  6,
      kLE010z200i   =  7,
      kLE010z000i   =  8,
      kLE150z200i   =  9,
      kBeamEnd      = 10
    } BeamType_t;

    //Detector type. For example, here the Near could be the Near detector for either MINOS or NOvA, depending on context.
    typedef enum EDetType
    {
      kUnknownDet = 0,
      kNOvAnd     = 1,
      kNOvAfd     = 2,
      kIPND       = 3,
      kMINOSnd    = 4,
      kMINOSfd    = 5,
      kNOvArat    = 6, //Far over Near NOvA Ratio
      kMINOSrat   = 7, //Far over Near MINOS Ratio
      kDetEnd     = 8
    } DetType_t;
  };

}
#endif //NBW_CONVENTIONS_H
