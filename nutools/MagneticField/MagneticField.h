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

  // Specifies the magnetic field over all space
  //
  // The default implementation, however, uses a nearly trivial,
  // non-physical hack.
  class MagneticField {
  public:
    MagneticField(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
    ~MagneticField(){};

    void reconfigure(fhicl::ParameterSet const& pset);

    MagFieldMode_t const& UseField() const { return fUseField; }

    // return the field at a particular point
    G4ThreeVector const FieldAtPoint(G4ThreeVector const& p=G4ThreeVector(0)) const;

    // This method will only return a uniform field based on the input
    // volume name.  If the input volume does not have a uniform field
    // caveat emptor
    G4ThreeVector const UniformFieldInVolume(std::string const& volName) const;
    
    // return the outermost affected volume
    std::string MagnetizedVolume() const { return fVolume; }

  private:
    // The simplest implmentation has a constant field inside a named
    // detector volume
    MagFieldMode_t      fUseField; ///< What field description to use
    G4ThreeVector       fField;    ///< the three vector of the field
    G4String            fVolume;   ///< the volume of the field
    static TGeoVolume*  fGeoVol;   ///< ROOT geometry volume for checking points
    
    ///\todo Need to add ability to read in a field from a database
  };

}

DECLARE_ART_SERVICE(mag::MagneticField, LEGACY)
#endif // MAG_MAGNETICFIELD_H
