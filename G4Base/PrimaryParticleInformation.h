////////////////////////////////////////////////////////////////////////
/// \file PrimaryParticleInformation.h
//
/// \version $Id: PrimaryParticleInformation.h,v 1.7 2012-09-20 21:47:05 greenc Exp $
/// \author  seligman@nevis.columbia.edu, brebel@fnal.gov
////////////////////////////////////////////////////////////////////////
/// PrimaryParticleInformation
/// 10-Sep-2007 Bill Seligman
///
/// 11-Feb-2009 <seligman@nevis.columbia.edu> Revised for LArSoft
///
/// Purpose: This class is "attached" to the G4PrimaryParticle.  It's used to
/// save the MCTruth object associated with the event.
///
/// Background: Read this carefully, because this class probably
/// doesn't do what you think it does.
///
/// Geant4 has various "truth" classes: G4Event, G4Track,
/// G4PrimaryVertex, G4PrimaryParticle, etc.  For all of these
/// classes, Geant4 provides a facility for the user to include
/// additional information that's "attached" to the class in question.
///
/// In this case, this class defines additional information to
/// included with the G4PrimaryParticle class.  In particular, it
/// stores the pointer to the simb::MCTruth object that was the
/// source of the G4PrimaryParticle information.
/// 
/// The reason why this class is necessary for the G4Base application
/// is that it allows the ParticleListAction class access to the
/// MCTruth pointer during Geant4's tracking.

#ifndef G4BASE_PrimaryParticleInformation_h
#define G4BASE_PrimaryParticleInformation_h

// G4 Includes
#include "Geant4/G4VUserPrimaryParticleInformation.hh"
#include "Geant4/G4Allocator.hh"

// ART Includes
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"


// Forward declaration for this namespace.
namespace simb {
  class MCTruth;
}

namespace g4b {
 
  class PrimaryParticleInformation : public G4VUserPrimaryParticleInformation {

  public:
    PrimaryParticleInformation();    
    virtual ~PrimaryParticleInformation();

    inline void* operator new(size_t);
    inline void operator delete(void*);
    
    // Accessors:
    const simb::MCTruth* GetMCTruth() const { return fMCTruth; }
    size_t const& MCTruthIndex()      const { return fMCTIndex; }
    void SetMCTruth(const simb::MCTruth* m,
		    const size_t         idx=0) { fMCTruth = m; fMCTIndex = idx; }

    // Required by Geant4:
    void Print() const;

  private:

    // The MCTruth object associated with the G4PrimaryParticle.  If
    // this is zero, then there is no MCTruth object for this
    // particle (although in that case it's more likely that a
    // G4Base::PrimaryParticleInformation object would not have been
    // created in the first place.)
    // The MCTIndex is the index of the MCTruth object in the vector
    // of the ConvertMCTruthToG4 creating this object
    const simb::MCTruth* fMCTruth;
          size_t         fMCTIndex;
  };

  // It's not likely, but there could be memory issues with these
  // PrimaryParticleInformation objects.  To make things work more smoothly
  // and quickly, use Geant4's memory allocation mechanism.
  
  extern G4Allocator<PrimaryParticleInformation> PrimaryParticleInformationAllocator;
  
  inline void* PrimaryParticleInformation::operator new(size_t)
  {
    void *aPrimaryParticleInformation;
    aPrimaryParticleInformation = (void *) PrimaryParticleInformationAllocator.MallocSingle();
    return aPrimaryParticleInformation;
  }
  
  inline void PrimaryParticleInformation::operator delete(void *aPrimaryParticleInformation)
  {
    PrimaryParticleInformationAllocator.FreeSingle((PrimaryParticleInformation*) aPrimaryParticleInformation);
  }

}

#endif // G4BASE_PrimaryParticleInformation_h
