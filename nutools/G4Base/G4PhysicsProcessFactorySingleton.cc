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
// $Id: G4PhysicsProcessFactorySingleton.cc,v 1.2 2012-09-20 21:43:53 greenc Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
//---------------------------------------------------------------------------
//
// ClassName:  G4PhysicsProcessFactorySingleton
//
// Author: 2012-02-02  R. Hatcher
//
// Modified:
//
//----------------------------------------------------------------------------
//

#include "G4Base/G4PhysicsProcessFactorySingleton.hh"

#include <iomanip>

// Define static variable which holds the one-and-only instance
G4PhysicsProcessFactorySingleton* G4PhysicsProcessFactorySingleton::fgTheInstance;

G4PhysicsProcessFactorySingleton::G4PhysicsProcessFactorySingleton() 
{
  fgTheInstance = this;   // record created self in static pointer
}

G4PhysicsProcessFactorySingleton::~G4PhysicsProcessFactorySingleton()
{
  fgTheInstance = 0;
}

G4PhysicsProcessFactorySingleton& G4PhysicsProcessFactorySingleton::Instance()
{
  // Cleaner dtor calls G4PhysicsProcessFactorySingleton dtor at job end
  static Cleaner cleaner;

  if ( ! fgTheInstance ) {
    // need to create one
    cleaner.UseMe();   // dummy call to quiet compiler warnings
    fgTheInstance = new G4PhysicsProcessFactorySingleton();
  }
  
  return *fgTheInstance;
}

G4VPhysicsConstructor* 
G4PhysicsProcessFactorySingleton::GetPhysicsProcess(const G4String& name)
{
  G4VPhysicsConstructor* p = 0;
  
  // we don't want map creating an entry if it doesn't exist
  // so use map::find() not map::operator[]
  std::map<G4String, PhysProcCtorFuncPtr_t>::iterator itr
    = fFunctionMap.find(name);
  if ( fFunctionMap.end() != itr ) { 
    // found an appropriate entry in the list
    PhysProcCtorFuncPtr_t foo = itr->second;  // this is the function
    p = (*foo)();  // use function to create the physics process
  }
  if ( ! p ) {
    G4cout << "### G4PhysicsProcessFactorySingleton WARNING: "
	   << "PhysicsProcess " << name << " is not known"
	   << G4endl;
  }
  return p;
}
  
G4bool G4PhysicsProcessFactorySingleton::IsKnownPhysicsProcess(const G4String& name)
{
  //  check if we know the name
  G4bool res = false;
  std::map<G4String, PhysProcCtorFuncPtr_t>::iterator itr
    = fFunctionMap.find(name);
  if ( fFunctionMap.end() != itr ) res = true;
  return res;
}

const std::vector<G4String>& 
G4PhysicsProcessFactorySingleton::AvailablePhysicsProcesses() const
{
  // list of names might be out of date due to new registrations
  // rescan the std::map on each call (which won't be frequent)
  listnames.clear();

  // scan map for registered names
  std::map<G4String, PhysProcCtorFuncPtr_t>::const_iterator itr;
  for ( itr = fFunctionMap.begin(); itr != fFunctionMap.end(); ++itr )
    listnames.push_back(itr->first);

  return listnames;
}

void G4PhysicsProcessFactorySingleton::PrintAvailablePhysicsProcesses() const
{
  std::vector<G4String> list = AvailablePhysicsProcesses();
  G4cout << "G4VPhysicsConstructors in "
         << "G4PhysicsProcessFactorySingleton are: " 
         << G4endl;
  if ( list.empty() ) G4cout << " ... no registered processes" << G4endl;
  else {
    for (size_t indx=0; indx < list.size(); ++indx ) {
      G4cout << " [" << std::setw(2) << indx << "] " 
             << "\"" << list[indx] << "\"" << G4endl;
    }
  }

}

G4bool G4PhysicsProcessFactorySingleton::RegisterCreator(G4String name, 
                                                   PhysProcCtorFuncPtr_t foo,
                                                   G4bool* boolptr)
{
  // record new functions for creating processes
  fFunctionMap[name] = foo;
  fBoolPtrMap[name]  = boolptr;
  return true;
}

/// !!!!!! register existing classes without disturbing their .cc files (yet)
//#include "G4PhysProcRegisterOld.icc"
