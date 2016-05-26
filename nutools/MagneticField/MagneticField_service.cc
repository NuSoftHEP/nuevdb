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
#include "messagefacility/MessageLogger/MessageLogger.h"

// nutools includes
#include "nutools/MagneticField/MagneticField.h"

#include "TGeoManager.h"

#include <vector>
#include <string>

namespace mag {

  TGeoVolume* MagneticField::fGeoVol = nullptr;
  
  MagneticField::MagneticField(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg)
  {
    this->reconfigure(pset);
  }

  //------------------------------------------------------------
  void MagneticField::reconfigure(fhicl::ParameterSet const& pset)
  {
    fVolume      = pset.get<std::string >("MagnetizedVolume");
    int mode     = pset.get< int        >("UseField"        );
    fUseField    = (mag::MagFieldMode_t)mode;

    // These need to be read as types that FHICL know about, but they
    // are used by Geant, so I store them in Geant4 types.
    std::vector<double> field = pset.get<std::vector<double> >("ConstantField");

    // Force the dimension of the field definition
    field.resize(3);
    for(size_t i = 0; i < 3; ++i) fField[i] = field[i];
    
    return;
  }

  //------------------------------------------------------------
  G4ThreeVector const MagneticField::FieldAtPoint(G4ThreeVector const& p) const
  {
    // check that the input point is in the magnetized volume
    // Use the gGeoManager to determine what node the point
    // is in
    if( fGeoVol == nullptr){
      fGeoVol = gGeoManager->FindVolumeFast(fVolume.c_str());
      if( fGeoVol == nullptr )
        throw cet::exception("MagneticField")
        << "cannot locat volume "
        << fVolume
        << " in gGeoManager, bail";
    }
    double point[3] = { p.x(), p.y(), p.z() };
    // we found a node, see if its name is the same as
    // the volume with the field
    if (fGeoVol->Contains(point)) return fField;

    // if we get here, we can't find a field
    return G4ThreeVector(0);
  }

  //------------------------------------------------------------
  G4ThreeVector const MagneticField::UniformFieldInVolume(std::string const& volName) const
  {
    // if the input volume name is the same as the magnetized volume
    // return the uniform field
    if (volName.compare(fVolume) == 0) return fField;
    
    // if we get here, we can't find a field
    return G4ThreeVector(0);
  }

}// namespace

namespace mag {

  DEFINE_ART_SERVICE(MagneticField)

} // namespace mag
