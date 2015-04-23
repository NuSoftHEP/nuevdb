//////////////////////////////////////////////////////////////////////////
/// \file MagneticField_service.cc
///
/// \version $Id: MagneticField.cxx,v 1.2 2012-03-07 19:01:44 brebel Exp $
/// \author dmckee@phys.ksu.edu
//////////////////////////////////////////////////////////////////////////
/// \class MagneticField MagneticField.h 
/// The initial implementation will be trivial: simply supporting a
/// constant field in a named detector volume. In principle we should
/// read a full field map from an external file of some kind.
///
/// We support three FHICL values for now:
///
///    - "UseField" a integer. When 0 we don't even instantiate a
///      Magnetic field object. Describes the description to use.
///    - "Constant Field" a vector< double > which should have three
///      elements and is interpreted in Tesla
///    - "MagnetizedVolume" names the G4logical volume to which the
///      field should be attached
//////////////////////////////////////////////////////////////////////////

// Framework includes

// nutools includes
#include "MagneticField/MagneticField.h"

#include <vector>
#include <string>

namespace mag {

  MagneticField::MagneticField(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg)
  {
    this->reconfigure(pset);
  }

  //------------------------------------------------------------
  void MagneticField::reconfigure(fhicl::ParameterSet const& pset)
  {

    fVolume    = pset.get<std::string >("MagnetizedVolume");
    int mode   = pset.get< int        >("UseField"        );
    fUseField  = (mag::MagFieldMode_t)mode;

    // These need to be read as types that FHICL know about, but they
    // are used by Geant, so I store them in Geant4 types.
    std::vector<double> field = pset.get<std::vector<double> >("ConstantField");

    // Force the dimension of the field definition
    field.resize(3);
    for(size_t i = 0; i < 3; ++i) fField[i] = field[i];
    
    return;
  }

  //------------------------------------------------------------
  G4ThreeVector MagneticField::FieldAtPoint(G4ThreeVector p) const
  {
    /// \todo This does not do what it says. Must test to see if the
    /// \todo point is in the master volume
    //
    // But it is enough to let me code the DetectorConstruction bit

    if ( /* is in the magnetized volume */ true ) return fField;
    return G4ThreeVector(0);
  }

}// namespace

namespace mag {

  DEFINE_ART_SERVICE(MagneticField)

} // namespace mag
