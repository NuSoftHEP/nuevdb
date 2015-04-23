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
// $Id: G4PhysListFactory.cc,v 1.2 2012-09-20 21:43:53 greenc Exp $
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
//----------------------------------------------------------------------------
//

#include "G4Base/G4PhysListFactory.hh"
#include "G4Base/G4PhysListFactorySingleton.hh"

#ifdef DEF_ALT_FACTORY
namespace alt {
#endif

G4PhysListFactory::G4PhysListFactory(const G4String& defname) 
{
  // Allow user to override the default in case they use
  // ReferencePhysList() and "PHYSLIST" is not set.
  // By specifying "" as the default arg, this does nothing
  // different in the case of previous use of default constructor.
  if ( defname != "<none>" ) SetDefaultName(defname);
}

G4PhysListFactory::~G4PhysListFactory()
{}

G4VModularPhysicsList* G4PhysListFactory::ReferencePhysList()
{
  // instantiate PhysList by environment variable "PHYSLIST"
  return G4PhysListFactorySingleton::Instance().ReferencePhysList();
}

G4VModularPhysicsList* G4PhysListFactory::GetReferencePhysList(
        const G4String& name)
{
  // intantiate PhysList by name
  return G4PhysListFactorySingleton::Instance().GetReferencePhysList(name);
}
  
G4bool G4PhysListFactory::IsReferencePhysList(const G4String& name)
{
  // check if name is known
  return G4PhysListFactorySingleton::Instance().IsReferencePhysList(name);
}

const std::vector<G4String>& 
G4PhysListFactory::AvailablePhysLists() const
{
  // return list of known names
  return G4PhysListFactorySingleton::Instance().AvailablePhysLists();
}

void G4PhysListFactory::PrintAvailablePhysLists() const
{
  // print a list of available PhysLists
  G4PhysListFactorySingleton::Instance().PrintAvailablePhysLists();
}


void G4PhysListFactory::SetDefaultName(const G4String& defname)
{ 
  G4PhysListFactorySingleton::Instance().SetDefaultName(defname);
}

const G4String& G4PhysListFactory::GetDefaultName() const
{ 
  return   G4PhysListFactorySingleton::Instance().GetDefaultName();
}

#ifdef DEF_ALT_FACTORY
} // end of namespace alt
#endif
