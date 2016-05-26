//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4PhysListFactory.hh,v 1.2 2012-09-20 21:47:05 greenc Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
//---------------------------------------------------------------------------
//
// ClassName:  G4PhysListFactory
//
// Author: 2011-10-06  R. Hatcher
//
// Modified:
//
// Class Description:  handle for creating physics list objects
//   Defer real work to G4PhyListFactorySingleton.
//
//   Interface based on old G4PhysListFactory to be compatible with prior
//   use (and enable eventual replacement). This class serves as a simple
//   handle to to real singleton factory instance where the actual map from
//   string names to creator functions are registered.
//
//----------------------------------------------------------------------------
//
#ifndef G4PhysListFactory_h
#define G4PhysListFactory_h 1

#include "Geant4/G4VModularPhysicsList.hh"
#include "Geant4/globals.hh"

// if/when officially adopted then DEF_ALT_FACTORY "alt" namespace can go away
#define DEF_ALT_FACTORY 1
#ifdef DEF_ALT_FACTORY
namespace alt {
#endif

class G4PhysListFactory
{
public:

  G4PhysListFactory(const G4String& defname = "<none>");

  ~G4PhysListFactory();
  G4VModularPhysicsList* GetReferencePhysList(const G4String&);
  // instantiate PhysList by name

  G4VModularPhysicsList* ReferencePhysList();
  // instantiate PhysList by environment variable "PHYSLIST"

  G4bool IsReferencePhysList(const G4String&);
  // check if the name is in the list of PhysLists names

  const std::vector<G4String>& AvailablePhysLists() const;
  // list of available Phys Lists

  void PrintAvailablePhysLists() const;
  // print a list of available PhysLists

  void SetDefaultName(const G4String& defname);
  const G4String& GetDefaultName() const;

};

#ifdef DEF_ALT_FACTORY
} // end of namespace alt
#endif

#endif
