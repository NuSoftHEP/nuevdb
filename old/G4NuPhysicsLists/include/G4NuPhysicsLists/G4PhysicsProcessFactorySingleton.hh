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
// $Id: G4PhysicsProcessFactorySingleton.hh,v 1.2 2012-09-20 21:47:05 greenc Exp $
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
// Class Description:  A singleton holding a map between names and
//   pointers-to-functions (that call a class default constructor).
//   The functions pointers must return G4VPhysicsConstructor*.
//
//   Class header also defines cpp macros for automatically defining
//   and registering functions mapped to G4VPhysicsConstructor classes.
//
//----------------------------------------------------------------------------
//
#ifndef G4PhysicsProcessFactorySingleton_h
#define G4PhysicsProcessFactorySingleton_h 1

#include "Geant4/G4VPhysicsConstructor.hh"
#include "Geant4/globals.hh"

// define a type for the pointer to a function that returns a 
//    G4VPhysicsConstructor* 
// i.e. calls the (typically default) ctor for the class.
typedef G4VPhysicsConstructor* (*PhysProcCtorFuncPtr_t)();

namespace g4nu {

class G4PhysicsProcessFactorySingleton
{
public:
  static G4PhysicsProcessFactorySingleton& Instance();
  // no public ctor for singleton, all user access is through Instance()

  G4VPhysicsConstructor* GetPhysicsProcess(const G4String&);
  // instantiate a PhysProc by name

  G4bool IsKnownPhysicsProcess(const G4String&);
  // check if the name is in the list of PhysicsProcess names

  const std::vector<G4String>& AvailablePhysicsProcesses() const;
  // return a list of available PhysicsProcess names

  void PrintAvailablePhysicsProcesses() const;
  // print a list of available PhysicsProcess names

  G4bool RegisterCreator(G4String name, PhysProcCtorFuncPtr_t ctorptr, G4bool* ptr);
  // register a new PhysProc type by passing pointer to creator function

private:
  static G4PhysicsProcessFactorySingleton* fgTheInstance;
  // the one and only instance

  std::map<G4String, PhysProcCtorFuncPtr_t> fFunctionMap;
  // mapping between known class names and a registered ctor function

  std::map<G4String, G4bool*> fBoolPtrMap;

  mutable std::vector<G4String> listnames;
  // copy of list of names, used solely due to AvailablePhysicsProcesses() 
  // method returning a const reference rather than a vector object.
  // mutable because AvailablePhysicsProcesses() is const, but list might 
  // need recreation if new entries have been registered.

private:
  G4PhysicsProcessFactorySingleton();
  // private ctor, users access class via Instance()

  virtual ~G4PhysicsProcessFactorySingleton();

  G4PhysicsProcessFactorySingleton(const G4PhysicsProcessFactorySingleton&);
  // method private and not implement, declared to prevent copying

  void operator=(const G4PhysicsProcessFactorySingleton&);
  // method private and not implement, declared to prevent assignment

  // sub-class Cleaner struct is used to clean up singleton at the end of job.
  struct Cleaner {
     void UseMe() { }                  // Dummy method to quiet compiler
    ~Cleaner() {
       if (G4PhysicsProcessFactorySingleton::fgTheInstance != 0) {
         delete G4PhysicsProcessFactorySingleton::fgTheInstance;
         G4PhysicsProcessFactorySingleton::fgTheInstance = 0;
  } } };
  friend struct Cleaner; 

};

} // end-of-namespace g4nu

// Define macro to create a function to call the class' ctor
// and then registers this function with the factory instance for later use
// Users should have in their  myPhyList.cc two lines that look like:
//     #include "G4PhysicsProcessFactorySingleton.hh"
//     PHYSPROCREG(myPhysProc)  // no semicolon
// where "myPhysProc" is the name of the class (assuming no special namespace)
// If the class is defined in a namespace use:
//     #include "G4PhysicsProcessFactorySingleton.hh"
//     PHYSPROCREG3(myspace,myAltPhysProc,myspace::myAltPhysProc) // no semicolon
// and either can then be retrieved from the factory using:
//     G4PhysicsProcessFactorySingleton& factory =
//         G4PhysicsProcessFactorySingleton::Instance();
//     G4VPhysicsConstructor* p = 0;
//     p = factory.GetPhysicsProcess("myPhyList");
//     p = factory.GetPhysicsProcess("myspace::myAltPhysProc");
//
// The expanded code looks like:
//   G4VPhysicsConstructor* myPhysProc_ctor_function () { return new myPhysProc; }
//   static G4bool myPhysProc_creator_registered = 
//     G4PhysicsProcessFactorySingleton::Instance().RegisterCreator("myPhysProc",
//                                               & myPhysProc_ctor_function );
//   namespace myspace {
//     G4VPhysicsConstructor* myAltPhysProc_ctor_function () { return new myspace::myAltPhysProc; }
//     static G4bool myPhysProc_creator_registered = 
//       G4PhysicsProcessFactorySingleton::Instance().RegisterCreator("myspace::myAltPhysProc",
//                                                 & myspace::myAltPhysProc_ctor_function ); }

#define PHYSPROCREG( _name ) \
  G4VPhysicsConstructor* _name ## _ctor_function () { return new _name; } \
  static G4bool _name ## _creator_registered =                            \
    g4nu::G4PhysicsProcessFactorySingleton::Instance().RegisterCreator(# _name, \
                                        & _name ## _ctor_function,        \
                                        & _name ## _creator_registered ); 

#define PHYSPROCREG3( _ns, _name, _fqname ) \
namespace _ns { \
  G4VPhysicsConstructor* _name ## _ctor_function () { return new _fqname; }   \
  static G4bool _name ## _creator_registered =                                \
    g4nu::G4PhysicsProcessFactorySingleton::Instance().RegisterCreator(# _fqname, \
                                        & _fqname ## _ctor_function,          \
                                        & _fqname ## _creator_registered );}
#endif
