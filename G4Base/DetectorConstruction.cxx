////////////////////////////////////////////////////////////////////////
/// \file  DetectorConstruction.cxx
/// \brief Build Geant4 geometry from GDML
///
/// \version $Id: DetectorConstruction.cxx,v 1.10 2012-12-03 23:29:49 rhatcher Exp $
/// \author  brebel@fnal.gov
////////////////////////////////////////////////////////////////////////

#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

#include "G4Base/DetectorConstruction.h"
#include "MagneticField/MagneticField.h"

#include "Geant4/G4VPhysicalVolume.hh"
#include "Geant4/G4GDMLParser.hh"
#include "Geant4/G4LogicalVolumeStore.hh"
#include "Geant4/G4Material.hh"
#include "Geant4/G4UniformMagField.hh"
#include "Geant4/G4FieldManager.hh"

namespace g4b{

  // Allocate static variables.
  G4VPhysicalVolume* DetectorConstruction::fWorld    = 0;
  G4FieldManager*    DetectorConstruction::fFieldMgr = 0;

  //---------------------------------------------------
  // Constructor
  DetectorConstruction::DetectorConstruction(std::string const& gdmlFile,
                                             bool overlapCheck,
                                             bool validateSchema)
  {
    if ( gdmlFile.empty() ) {
      throw cet::exception("DetectorConstruction") << "Supplied GDML filename is empty\n"
						   << __FILE__ << ":" << __LINE__ << "\n";
    }
    // Get the path to the GDML file from the Geometry interface.
    const G4String GDMLfile = static_cast<const G4String>( gdmlFile );

    // Use Geant4's GDML parser to convert the geometry to Geant4 format.
    G4GDMLParser parser;
    parser.SetOverlapCheck(overlapCheck);
    parser.Read(GDMLfile,validateSchema);

    // Fetch the world physical volume from the parser.  This contains
    // the entire detector, not just the outline of the experimental
    // hall.
    fWorld = parser.GetWorldVolume();
  }
  
  //---------------------------------------------------
  // Destructor.
  DetectorConstruction::~DetectorConstruction() 
  {
  }
  
  //---------------------------------------------------
  G4VPhysicalVolume* DetectorConstruction::Construct()
  {
    // Setup the magnetic field situation 
    art::ServiceHandle<mag::MagneticField> bField;
    switch (bField->UseField()) {
    case mag::kNoBFieldMode: 
      /* NOP */
      break;
    case mag::kConstantBFieldMode: {
      // Define the basic field
      G4UniformMagField* magField = new G4UniformMagField( bField->FieldAtPoint() * CLHEP::tesla );
      fFieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
      fFieldMgr->SetDetectorField(magField);
      fFieldMgr->CreateChordFinder(magField);
      
      // Reset the chord finding accuracy
      // fFieldMgr->GetChordFinder()->SetDeltaChord(1.0 * cm);
      
      // Attach this to the magnetized volume only
      /// \todo This isn't the Right (tm) thing to do, but it will do
      G4LogicalVolumeStore *lvs  = G4LogicalVolumeStore::GetInstance();
      G4LogicalVolume      *bvol = lvs->GetVolume(bField->MagnetizedVolume());
      
      // the boolean tells the field manager to use local volume
      bvol->SetFieldManager(fFieldMgr,true);
      break; 
    } // case mag::kConstantBFieldMode
    default: // Complain if the user asks for something not handled
      mf::LogError("DetectorConstruction") << "Unknown or illegal Magneticfield "
					   << "mode specified: " 
					   << bField->UseField()
					   << ". Note that AutomaticBFieldMode is reserved.";
      break;
    }
    return fWorld;
  }

}// namespace
