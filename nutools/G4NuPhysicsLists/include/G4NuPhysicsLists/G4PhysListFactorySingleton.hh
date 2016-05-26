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
// $Id: G4PhysListFactorySingleton.hh,v 1.4 2012-09-20 21:47:05 greenc Exp $
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
// Class Description:  A singleton holding a map between names and
//   pointers-to-functions (that call a class default constructor).
//   The functions pointers must return G4VModularPhysicList*.
//
//   Methods other than those related to the singleton-ness and
//   registering function pointers are based on the original
//   G4PhysListFactory methods and supply the same functionality.
//
//   Class header also defines cpp macros for automatically defining
//   and registering functions mapped to PhysicsList classes.
//
//   Special handling is performed for names that include one of:
//     "_EMV" "_EMX" "_EMY" "_LIV" "_PEN"
//   which override the base physics list default EM physics
//
//----------------------------------------------------------------------------
//
#ifndef G4PhysListFactorySingleton_h
#define G4PhysListFactorySingleton_h 1

#include "Geant4/G4VModularPhysicsList.hh"
#include "Geant4/globals.hh"

// define a type for the pointer to a function that returns a 
//    G4VModularPhysicsList* 
// i.e. calls the (typically default) ctor for the class.
typedef G4VModularPhysicsList* (*PhysListCtorFuncPtr_t)();

namespace g4nu {

class G4PhysListFactorySingleton
{
public:
  static G4PhysListFactorySingleton& Instance();
  // no public ctor for singleton, all user access is through Instance()

  G4VModularPhysicsList* GetReferencePhysList(const G4String&);
  // instantiate a PhysList by name

  G4VModularPhysicsList* ReferencePhysList();
  // instantiate a PhysList indicated by the environment variable "PHYSLIST"

  G4bool IsReferencePhysList(const G4String&);
  // check if the name is in the list of PhysLists names

  const std::vector<G4String>& AvailablePhysLists() const;
  // return a list of available PhysLists

  void PrintAvailablePhysLists() const;
  // print a list of available PhysLists

  G4bool RegisterCreator(G4String name, PhysListCtorFuncPtr_t ctorptr, G4bool* ptr);
  // register a new PhysList type by passing pointer to creator function

  G4bool RegisterPhysicsReplacement(G4String key, G4String physics);
  // register a mapping between a key and a G4PhysicsConstructor
  // assumes G4PhysicsConstructor is registered w/ 
  //   G4PhysicsProcessFactorySingleton

  void SetDefaultName(const G4String& defname) { defName = defname; }
  const G4String& GetDefaultName() const { return defName; }

private:
  static G4PhysListFactorySingleton* fgTheInstance;
  // the one and only instance

  std::map<G4String, PhysListCtorFuncPtr_t> fFunctionMap;
  // mapping between known class names and a registered ctor function

  std::map<G4String, G4bool*> fBoolPtrMap;

  std::map<G4String, G4String> fPhysicsReplaceList;
  // mapping between EM variant key to G4VPhysicsConstructor class
  //    uses G4PhysicsProcessFactorySingleton to get class 
  //

  G4String defName;
  // name of default in case of unset PHYSLIST

  mutable std::vector<G4String> listnames;
  // copy of list of names, used solely due to AvailablePhysList() method
  // returning a const reference rather than a vector object.
  // mutable because AvailablePhysLists() is const, but list might need 
  // recreation if new entries have been registered.

private:
  G4PhysListFactorySingleton();
  // private ctor, users access via Instance()

  G4String GetBaseName(G4String name, std::vector<G4String>& physicsReplacements, G4bool& allKnown);
  // strip out physics replacement keys and return base physics list name
  // update vector with the name of the physics processes to be replaced

  virtual ~G4PhysListFactorySingleton();

  G4PhysListFactorySingleton(const G4PhysListFactorySingleton&);
  // method private and not implement, declared to prevent copying

  void operator=(const G4PhysListFactorySingleton&);
  // method private and not implement, declared to prevent assignment

  // sub-class Cleaner struct is used to clean up singleton at the end of job.
  struct Cleaner {
     void UseMe() { }                  // Dummy method to quiet compiler
    ~Cleaner() {
       if (G4PhysListFactorySingleton::fgTheInstance != 0) {
         delete G4PhysListFactorySingleton::fgTheInstance;
         G4PhysListFactorySingleton::fgTheInstance = 0;
  } } };
  friend struct Cleaner; 

};

} // end-of-namespace g4nu

// Define macro to create a function to call the class' ctor
// and then registers this function with the factory instance for later use
// Users should have in their  myPhyList.cc two lines that look like:
//     #include "G4PhysListFactorySingleton.hh"
//     PHYSLISTREG(myPhysList)  // no semicolon
// where "myPhysList" is the name of the class (assuming no special namespace)
// If the class is defined in a namespace use:
//     #include "G4PhysListFactorySingleton.hh"
//     PHYSLISTREG3(myspace,myAltPhyList,myspace::myAltPhyList) // no semicolon
// and either can then be retrieved from the factory using:
//     G4PhysListFactory factory;
//     G4VModularPhysicsList* p = 0;
//     p = factory.GetReferencePhysList("myPhyList");
//     p = factory.GetReferencePhysList("myspace::myAltPhysList");
//
// The expanded code looks like:
//   G4VModularPhysicsList* myPhysList_ctor_function () { return new myPhysList; }
//   static G4bool myPhysList_creator_registered = 
//     G4PhysListFactorySingleton::Instance().RegisterCreator("myPhysList",
//                                               & myPhysList_ctor_function );
//   namespace myspace {
//     G4VModularPhysicsList* myAltPhysList_ctor_function () { return new myspace::myAltPhysList; }
//     static G4bool myPhysList_creator_registered = 
//       G4PhysListFactorySingleton::Instance().RegisterCreator("myspace::myAltPhysList",
//                                                 & myspace::myAltPhysList_ctor_function ); }

#define PHYSLISTREG( _name ) \
  G4VModularPhysicsList* _name ## _ctor_function () { return new _name; } \
  static G4bool _name ## _creator_registered =                            \
    g4nu::G4PhysListFactorySingleton::Instance().RegisterCreator(# _name, \
                                        & _name ## _ctor_function,        \
                                        & _name ## _creator_registered ); 

#define PHYSLISTREG3( _ns, _name, _fqname ) \
namespace _ns { \
  G4VModularPhysicsList* _name ## _ctor_function () { return new _fqname; }   \
  static G4bool _name ## _creator_registered =                                \
    g4nu::G4PhysListFactorySingleton::Instance().RegisterCreator(# _fqname, \
                                        & _fqname ## _ctor_function,          \
                                        & _fqname ## _creator_registered );}
#endif
