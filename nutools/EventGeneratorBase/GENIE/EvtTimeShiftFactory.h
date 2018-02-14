////////////////////////////////////////////////////////////////////////
/// \file  EvtTimeShiftFactory.h
/// \brief A class for generating concrete EvtTimeShiftI derived classes
///        based on the factory pattern.  This code supplies a CPP
///        macro which allows the classes to self-register and thus
///        no modification of this class is needed in order to expand
///        the list of classes it knows about.
///
///        Implemented as a singleton holding a map between names and
///        pointers-to-functions (that call a class default constructor).
///        The functions pointers must return EvtTimeShiftI*.
///
/// \version /// \author  Robert Hatcher <rhatcher \at fnal.gov>
///          Fermi National Accelerator Laboratory
///
////////////////////////////////////////////////////////////////////////
#ifndef SIMB_EVTTIMESHIFTFACTORY_H
#define SIMB_EVTTIMESHIFTFACTORY_H

#include <string>
#include <vector>
#include <map>

#include "EvtTimeShiftI.h"

namespace evgb {

// define a type for the pointer to a function that returns a 
//    evgb::EvtTimeShiftI* 
// i.e. calls the (typically default) ctor for the class.
typedef evgb::EvtTimeShiftI* (*EvtTimeShiftICtorFuncPtr_t)(const std::string& );

class EvtTimeShiftFactory
{
public:
  static EvtTimeShiftFactory& Instance();
  // no public ctor for singleton, all user access is through Instance()

  evgb::EvtTimeShiftI* GetEvtTimeShift(const std::string& name, 
                                       const std::string& config="") const;
  // instantiate a EvtTimeShift by name (1st arg), pass 2nd arg as config in ctor

  bool IsKnownEvtTimeShift(const std::string&);
  // check if the name is in the list of names

  const std::vector<std::string>& AvailableEvtTimeShift() const;
  // return a list of available names

  void Print() const;
  // print what we know

  bool RegisterCreator(std::string name, 
                       EvtTimeShiftICtorFuncPtr_t ctorptr, bool* ptr);
  // register a new EvtTimeShiftI type by passing pointer to creator function

private:
  static EvtTimeShiftFactory* fgTheInstance;
  // the one and only instance

  std::map<std::string, EvtTimeShiftICtorFuncPtr_t> fFunctionMap;
  // mapping between known class names and a registered ctor function

  std::map<std::string, bool*> fBoolPtrMap;

  mutable std::vector<std::string> listnames;
  // copy of list of names, used solely due to AvailableFlavorMixers() 
  // method returning a const reference rather than a vector object.
  // mutable because AvailableFlavorMixers() is const, but list might 
  // need recreation if new entries have been registered.

private:
  EvtTimeShiftFactory();
  // private ctor, users access class via Instance()

  virtual ~EvtTimeShiftFactory();

  EvtTimeShiftFactory(const EvtTimeShiftFactory&);
  // method private and not implement, declared to prevent copying

  void operator=(const EvtTimeShiftFactory&);
  // method private and not implement, declared to prevent assignment

  // sub-class Cleaner struct is used to clean up singleton at the end of job.
  struct Cleaner {
     void UseMe() { }  // Dummy method to quiet compiler
    ~Cleaner() {
       if (EvtTimeShiftFactory::fgTheInstance != 0) {
         delete EvtTimeShiftFactory::fgTheInstance;
         EvtTimeShiftFactory::fgTheInstance = 0;
  } } };
  friend struct Cleaner; 

};

} // namespace evgb

// Define macro to create a function to call the class' ctor
// and then registers this function with the factory instance for later use
// Users should have in their  myPhyList.cc two lines that look like:
//     #include "EvtTimeShiftFactory.h"
//     TIMESHIFTREG(MyTimeShiftClass)  // no semicolon
// where "MyTimeShiftClass" is the name of the class (assuming no special namespace)
// If the class is defined in a namespace (or two) use:
//     #include "EvtTimeShiftFactory.h"
//     TIMESHIFTREG3(myspace,myAltTimeShift,myspace::myAltTimeShift) // no semicolon
//     TIMESHIFTREG4(myspace,evgb,YATimeShift,myspace::evgb::YATimeShift) // no semicolon
// and either can then be retrieved from the factory using:
//     EvtTimeShiftFactory& factory =
//         EvtTimeShiftFactory::Instance();
//     evgb::EvtTimeShiftI* p = 0;
//     std::string myConfig = "..."
//     p = factory.GetEvtTimeShift("MyTimeShiftClass",myConfig);
//     p = factory.GetEvtTimeShift("myspace::myAltTimeShift",myConfig);
//     p = factory.GetEvtTimeShift("evgb::YATimeShift",myConfig);
//
// The expanded code looks like:
//   evgb::EvtTimeShiftI* MyTimeShiftClass_ctor_function () { return new MyTimeShiftClass; }
//   static bool MyTimeShiftClass_creator_registered = 
//     EvtTimeShiftFactory::Instance().RegisterCreator("MyTimeShiftClass",
//                                               & MyTimeShiftClass_ctor_function );
//   namespace myspace {
//     evgb::EvtTimeShiftI* myAltAltTimeShift_ctor_function () { return new myspace::myAltAltTimeShift; }
//     static bool myAltTimeShift_creator_registered = 
//       EvtTimeShiftFactory::Instance().RegisterCreator("myspace::myAltAltTimeShift",
//                                                 & myspace::myAltAltTimeShift_ctor_function ); }

#define TIMESHIFTREG( _name ) \
  evgb::EvtTimeShiftI* _name ## _ctor_function (const std::string& config) { return new _name(config); } \
  static bool _name ## _creator_registered =                            \
    evgb::EvtTimeShiftFactory::Instance().RegisterCreator(# _name, \
                                        & _name ## _ctor_function,        \
                                        & _name ## _creator_registered ); 

#define TIMESHIFTREG3( _ns, _name, _fqname ) \
namespace _ns { \
  evgb::EvtTimeShiftI* _name ## _ctor_function (const std::string& config) { return new _fqname(config); } \
  static bool _name ## _creator_registered =                                \
    evgb::EvtTimeShiftFactory::Instance().RegisterCreator(# _fqname, \
                                        & _fqname ## _ctor_function,          \
                                        & _fqname ## _creator_registered );}

#define TIMESHIFTREG4( _nsa, _nsb, _name, _fqname )  \
namespace _nsa { \
 namespace _nsb { \
   evgb::EvtTimeShiftI* _name ## _ctor_function (const std::string& config) { return new _fqname(config); } \
  static bool _name ## _creator_registered =                                \
    evgb::EvtTimeShiftFactory::Instance().RegisterCreator(# _fqname, \
                                        & _fqname ## _ctor_function,          \
                                        & _fqname ## _creator_registered );}}
#endif
