/// \file MagneticField.h
/// \brief Describe the magnetic field structure of a detector
/// 
/// \version $Id: MagneticField.h,v 1.3 2012-09-21 02:46:38 greenc Exp $
/// \author dmckee@phys.ksu.edu
//////////////////////////////////////////////////////////////////////////
/// \namespace mag
/// A namespace for simulated magnetic fields
//////////////////////////////////////////////////////////////////////////
#ifndef MAG_MAGNETICFIELD_H
#define MAG_MAGNETICFIELD_H

#include <string>

#include "TGeoVolume.h"

// Geant4 includes
#include "Geant4/G4ThreeVector.hh"

// Framework includes
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace mag {

  typedef enum MagneticFieldMode {
    kAutomaticBFieldMode=-1, // Used by DriftElectronsAlg
    kNoBFieldMode=0,         // no field
    kConstantBFieldMode=1    // constant field
    /*, kFieldMapMode, ... */
  } MagFieldMode_t;

  struct MagneticFieldDescription
  {
    MagFieldMode_t fMode;   ///< type of field used
    G4ThreeVector  fField;  ///< description of the field (uniform only)
    G4String       fVolume; ///< G4 volume containing the field
    TGeoVolume*    fGeoVol; ///< pointer to TGeoVolume with the field
  };

  // Specifies the magnetic field over all space
  //
  // The default implementation, however, uses a nearly trivial,
  // non-physical hack.
  class MagneticField {
  public:
    MagneticField(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
    ~MagneticField(){};

    void reconfigure(fhicl::ParameterSet const& pset);

    std::vector<MagneticFieldDescription> const& Fields()                   const { return fFieldDescriptions;            }
    size_t                                       NumFields()                const { return fFieldDescriptions.size();     }
    MagFieldMode_t                        const& UseField(size_t f)         const { return fFieldDescriptions[f].fMode;   }
    std::string                           const& MagnetizedVolume(size_t f) const { return fFieldDescriptions[f].fVolume; }

    // return the field at a particular point
    G4ThreeVector  const FieldAtPoint(G4ThreeVector const& p=G4ThreeVector(0)) const;

    // This method will only return a uniform field based on the input
    // volume name.  If the input volume does not have a uniform field
    // caveat emptor
    G4ThreeVector  const UniformFieldInVolume(std::string const& volName) const;
    
  private:
    // The simplest implmentation has a constant field inside a named
    // detector volume
    std::vector<MagneticFieldDescription> fFieldDescriptions; ///< Descriptions of the fields
    
    ///\todo Need to add ability to read in a field from a database
  };

}

DECLARE_ART_SERVICE(mag::MagneticField, LEGACY)
#endif // MAG_MAGNETICFIELD_H
