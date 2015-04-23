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
// $Id: G4PhysListFactorySingleton.cc,v 1.4 2012-09-20 21:43:53 greenc Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
//---------------------------------------------------------------------------
//
// ClassName:  G4PhysListFactorySingleton
//
// Author: 2011-10-06  R. Hatcher
//
// Modified:
//
//----------------------------------------------------------------------------
//

#include "G4NuPhysicsLists/G4PhysListFactorySingleton.hh"

#include <iomanip>
#include "G4NuPhysicsLists/G4PhysicsProcessFactorySingleton.hh"

namespace g4nu {

// Define static variable which holds the one-and-only instance
G4PhysListFactorySingleton* G4PhysListFactorySingleton::fgTheInstance;

G4PhysListFactorySingleton::G4PhysListFactorySingleton() 
{
  defName = "QGSP_BERT";  // default list to use if no PHYSLIST defined
  fgTheInstance = this;   // record created self in static pointer
}

G4PhysListFactorySingleton::~G4PhysListFactorySingleton()
{
  fgTheInstance = 0;
}

G4PhysListFactorySingleton& G4PhysListFactorySingleton::Instance()
{
  // Cleaner dtor calls G4PhysListFactorySingleton dtor at job end
  static Cleaner cleaner;

  if ( ! fgTheInstance ) {
    // need to create one
    cleaner.UseMe();   // dummy call to quiet compiler warnings
    fgTheInstance = new G4PhysListFactorySingleton();

    // special pre-register some physics overrides (common EM overrides)
    fgTheInstance->RegisterPhysicsReplacement("_EMV","G4EmStandardPhysics_option1");
    fgTheInstance->RegisterPhysicsReplacement("_EMX","G4EmStandardPhysics_option2");
    fgTheInstance->RegisterPhysicsReplacement("_EMY","G4EmStandardPhysics_option3");
    fgTheInstance->RegisterPhysicsReplacement("_LIV","G4EmLivermorePhysics");
    fgTheInstance->RegisterPhysicsReplacement("_PEN","G4EmPenelopePhysics");


  }
  
  return *fgTheInstance;
}

G4VModularPhysicsList* G4PhysListFactorySingleton::ReferencePhysList()
{
  // instantiate PhysList by environment variable "PHYSLIST"
  G4String name = "";
  char* path = getenv("PHYSLIST");
  if (path) {
    name = G4String(path);
  } else {
    name = defName;
    G4cout << "### G4PhysListFactorySingleton WARNING: "
	   << " environment variable PHYSLIST is not defined"
	   << G4endl
	   << "    Default Physics Lists " << name 
	   << " is instantiated" 
	   << G4endl;
  }
  return GetReferencePhysList(name);
}

G4VModularPhysicsList* 
G4PhysListFactorySingleton::GetReferencePhysList(const G4String& name)
{
  G4VModularPhysicsList* p = 0;
  
  // Replacing the very fixed list of if/else with a std::map lookup
  /*
  if(name == "CHIPS")              {p = new CHIPS();}
  else if(name == "FTFP_BERT")     {p = new FTFP_BERT();}
  else if(name == "FTFP_BERT_EMV") {p = new FTFP_BERT_EMV();}
  else if(name == "FTFP_BERT_EMX") {p = new FTFP_BERT_EMX();}
  else if(name == "FTFP_BERT_TRV") {p = new FTFP_BERT_TRV();}
  else if(name == "FTF_BIC")       {p = new FTF_BIC();}
  else if(name == "LBE")           {p = new LBE();}
  else if(name == "LHEP")          {p = new LHEP();}
  else if(name == "LHEP_EMV")      {p = new LHEP_EMV();}
  else if(name == "QBBC")          {p = new QBBC();}
  else if(name == "QGSC_BERT")     {p = new QGSC_BERT();}
  else if(name == "QGSP")          {p = new QGSP();}
  else if(name == "QGSP_BERT")     {p = new QGSP_BERT();}
  else if(name == "QGSP_BERT_EMV") {p = new QGSP_BERT_EMV();}
  else if(name == "QGSP_BERT_EMX") {p = new QGSP_BERT_EMX();}
  else if(name == "QGSP_BERT_HP")  {p = new QGSP_BERT_HP();}
  else if(name == "QGSP_BERT_NOLEP") {p = new QGSP_BERT_NOLEP();}
  else if(name == "QGSP_BERT_TRV") {p = new QGSP_BERT_TRV();}
  else if(name == "QGSP_BERT_CHIPS") {p = new QGSP_BERT_CHIPS();}
  else if(name == "QGSP_BIC")      {p = new QGSP_BIC();}
  else if(name == "QGSP_BIC_EMY")  {p = new QGSP_BIC_EMY();}
  else if(name == "QGSP_BIC_HP")   {p = new QGSP_BIC_HP();}
  else if(name == "QGSP_FTFP_BERT"){p = new QGSP_FTFP_BERT();}
  else if(name == "QGS_BIC")       {p = new QGS_BIC();}
  else if(name == "QGSP_INCL_ABLA"){p = new QGSP_INCL_ABLA();}
  else if(name == "Shielding")     {p = new Shielding();}
  */

  // look for key bits that indicate physics to be replaced in a 
  // base physics list; remove those keys from the physics list name
  std::vector<G4String> physicsReplacements;
  G4bool allKnown;
  G4String nameNoReplace = GetBaseName(name,physicsReplacements,allKnown);

  // we don't want map creating an entry if it doesn't exist
  // so use map::find() not map::operator[]
  std::map<G4String, PhysListCtorFuncPtr_t>::iterator itr
    = fFunctionMap.find(nameNoReplace);
  if ( fFunctionMap.end() != itr ) { 
    // found an appropriate entry in the list
    PhysListCtorFuncPtr_t foo = itr->second;  // this is the function
    p = (*foo)();  // use function to create the physics list
  }
  if ( ! p ) {
    G4cout << "### G4PhysListFactorySingleton WARNING: "
	   << "PhysicsList " << nameNoReplace 
           << "(originally=\"" << name << "\")"
           << " is not known"
	   << G4endl;
  } else if ( physicsReplacements.size() != 0 ) {
    //
    // if there is physics to be replaced, do so now
    //
    G4PhysicsProcessFactorySingleton& procFactory =
      G4PhysicsProcessFactorySingleton::Instance();
    for (size_t k=0; k<physicsReplacements.size(); ++k) {
      G4String procName = physicsReplacements[k];
      if ( ! procFactory.IsKnownPhysicsProcess(procName) ) {
        G4cout << "### G4PhysListFactorySingleton WARNING: "
               << "G4PhysicsProcesFactorySingleton had no process \""
               << procName << "\" registered" << G4endl;        
      } else {
        G4cout << "### G4PhysListFactorySingleton: ReplacePhysics("
               << procName << ")" << G4endl;
        G4VPhysicsConstructor* pctor = procFactory.GetPhysicsProcess(procName);
        p->ReplacePhysics(pctor);
      }
    }
  }

  return p;
}
  
G4bool G4PhysListFactorySingleton::IsReferencePhysList(const G4String& name)
{
  //  check if we know the name (after stripping off replacement keys)
  std::vector<G4String> physicsReplacements;
  G4bool allKnown;
  G4String nameNoReplace = GetBaseName(name,physicsReplacements,allKnown);

  return allKnown;
}

const std::vector<G4String>& 
G4PhysListFactorySingleton::AvailablePhysLists() const
{
  // list of names might be out of date due to new registrations
  // rescan the std::map on each call (which won't be frequent)
  listnames.clear();

  // scan map for registered names
  std::map<G4String, PhysListCtorFuncPtr_t>::const_iterator itr;
  for ( itr = fFunctionMap.begin(); itr != fFunctionMap.end(); ++itr )
    listnames.push_back(itr->first);

  return listnames;
}

void G4PhysListFactorySingleton::PrintAvailablePhysLists() const
{
  const std::vector<G4String>& list = AvailablePhysLists();
  G4cout << "G4VModularPhysicsLists in "
         << "G4PhysicsProcessFactorySingleton are: " 
         << G4endl;
  if ( list.empty() ) G4cout << " ... no registered lists" << G4endl;
  else {
    for (size_t indx=0; indx < list.size(); ++indx ) {
      G4cout << " [" << std::setw(2) << indx << "] " 
             << "\"" << list[indx] << "\"" << G4endl;
    }
  }
  G4cout << "G4PhysicsProcessFactorySingleton supports variants of the above"
         << G4endl << "with physics process replacements:" << G4endl;
  if ( fPhysicsReplaceList.empty() ) {
    G4cout << " ... no registered replacements" << G4endl;
  } else {
    G4bool printPhysicsProcesses = false;
    G4PhysicsProcessFactorySingleton& procFactory =
      G4PhysicsProcessFactorySingleton::Instance();
    std::map<G4String, G4String>::const_iterator repitr = 
      fPhysicsReplaceList.begin();
    for ( ; repitr != fPhysicsReplaceList.end(); ++repitr ) {
      G4String key = repitr->first;
      G4String procName = repitr->second;
      G4bool known = procFactory.IsKnownPhysicsProcess(procName);
      if ( ! known ) printPhysicsProcesses = true;
      G4cout << "  " << std::setw(10) << key << "  ==> "
             << std::setw(30) << procName << "  "
             << ( (known)?"known":"*** unknown ***" ) << G4endl;
    }
    if ( printPhysicsProcesses ) procFactory.PrintAvailablePhysicsProcesses();
  }
}

G4bool G4PhysListFactorySingleton::RegisterCreator(G4String name, 
                                                   PhysListCtorFuncPtr_t foo,
                                                   G4bool* boolptr)
{
  // record new functions for creating lists
  fFunctionMap[name] = foo;
  fBoolPtrMap[name]  = boolptr;
  return true;
}

G4bool G4PhysListFactorySingleton::RegisterPhysicsReplacement(G4String key,
                                                              G4String physics)
{
  fPhysicsReplaceList[key] = physics;
  return true;
}

G4String 
G4PhysListFactorySingleton::GetBaseName(G4String name, 
                                        std::vector<G4String>& physicsReplace,
                                        G4bool& allKnown)
{
  allKnown = true;
  G4String nameNoReplace = name;
  physicsReplace.clear();
  G4PhysicsProcessFactorySingleton& procFactory =
    G4PhysicsProcessFactorySingleton::Instance();

  std::map<G4String, G4String>::iterator repitr = fPhysicsReplaceList.begin();
  for ( ; repitr != fPhysicsReplaceList.end(); ++repitr ) {
    G4String key = repitr->first;
    size_t i = nameNoReplace.find(key);
    if ( i != std::string::npos ) {
      // remove key from base physics list name
      nameNoReplace.erase(i,key.size());

      // add to list of things needing replacement
      G4String procName = repitr->second;
      physicsReplace.push_back(procName);
      if ( ! procFactory.IsKnownPhysicsProcess(procName) ) {
        G4cout << "G4PhysListFactorySingleton::GetBaseName "
               << "\"" << key << "\" ==> \"" << procName << "\" not found"
               << G4endl;
        //procFactory.PrintAvailablePhysicsProcesses();
        allKnown = false;
      }
    }
  }

  std::map<G4String, PhysListCtorFuncPtr_t>::iterator itr
    = fFunctionMap.find(nameNoReplace);
  if ( fFunctionMap.end() == itr ) allKnown = false;

  return nameNoReplace;
}

} // end-of-namespace g4nu

/// !!!!!! register existing classes without disturbing their .cc files (yet)
#include "G4NuPhysicsLists/G4PhysListRegisterOld.icc"
/// !!!!!! register some existing G4PhysicsConstructor objects w/ G4PhysicsProcessFactorySingleton
/// !!!!!! so that one can do the "normal" EM overrides
#include "G4NuPhysicsLists/G4PhysicsProcessRegisterOld.icc"
