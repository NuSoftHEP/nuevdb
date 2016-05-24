////////////////////////////////////////////////////////////////////////
/// \file  G4Helper.h
/// \brief Use Geant4 to run the LArSoft detector simulation
///
/// \version $Id: G4Helper.cxx,v 1.20 2012-12-03 23:29:49 rhatcher Exp $
/// \author  seligman@nevis.columbia.edu, brebel@fnal.gov
////////////////////////////////////////////////////////////////////////

#include "G4Base/G4Helper.h"
#include "G4Base/DetectorConstruction.h"
#include "G4Base/UserActionManager.h"

#include "nusimdata/SimulationBase/MCTruth.h"

#include "Geant4/G4UImanager.hh"
#include "Geant4/G4VUserDetectorConstruction.hh"
#include "Geant4/G4VUserPrimaryGeneratorAction.hh"
#include "Geant4/G4VUserPhysicsList.hh"
#include "Geant4/G4UserRunAction.hh"
#include "Geant4/G4UserEventAction.hh"
#include "Geant4/G4UserTrackingAction.hh"
#include "Geant4/G4UserSteppingAction.hh"
#include "Geant4/G4VisExecutive.hh"

#include <boost/algorithm/string.hpp>

#include "Geant4/QGSP_BERT.hh"
#define TRY_NEW_PL_FACTORY
#ifdef  TRY_NEW_PL_FACTORY
#include "G4Base/G4PhysListFactory.hh"
#else
#include "Geant4/G4PhysListFactory.hh"
#endif
// 
#include "G4Base/G4PhysicsProcessFactorySingleton.hh"
#include "Geant4/G4VModularPhysicsList.hh"

#include <Rtypes.h>

#include <iostream>
#include <cstring>
#include <sys/stat.h>

#include "messagefacility/MessageLogger/MessageLogger.h"

namespace g4b{

  //------------------------------------------------
  // Constructor
  G4Helper::G4Helper()
  {
    fParallelWorlds.clear();
  }

  //------------------------------------------------
  // Constructor
  G4Helper::G4Helper(std::string const& g4macropath, 
		     std::string const& g4physicslist,
		     std::string const& gdmlFile) 
    : fG4MacroPath(g4macropath)
    , fG4PhysListName(g4physicslist)
    , fGDMLFile(gdmlFile)
    , fCheckOverlaps(false)
    , fValidateGDMLSchema(true)
    , fUIManager(0)
    , fConvertMCTruth(0)
    , fDetector(0)
  {
    // Geant4 run manager.  Nothing happens in Geant4 until this object
    // is created.
    fRunManager = new G4RunManager;

    // Get the pointer to the User Interface manager   
    fUIManager = G4UImanager::GetUIpointer();  

    fParallelWorlds.clear();
  }

  //------------------------------------------------
  // Destructor
  G4Helper::~G4Helper() 
  {
    if ( fRunManager != 0 ){
      // In SetUserAction(), we set all the G4 user-action classes to be the
      // same action: G4Base::UserActionManager This is convenient, but
      // it creates a problem here: First the G4RunManager deletes the
      // G4UserRunAction, then it tries to delete the
      // G4UserEventAction... but that's the same object, which has
      // already been deleted.  Crash.
    
      // To keep this from happening, handle the UserActionManager
      // clean-up manually, then tell the G4RunManager that all those
      // classes no longer exist.
    
      g4b::UserActionManager* uaManager = UserActionManager::Instance();
      bool wasStacking = uaManager->DoesAnyActionProvideStacking();
      uaManager->Close();
    
      // Each one of these G4RunManager::SetUserAction methods calls a
      // different method, based on the type of the argument.  We want
      // to use "0" (a null pointer), but we have to cast that "0" to a
      // particular type in order for the right SetUserAction method to
      // be called.
    
      fRunManager->SetUserAction( static_cast<G4UserRunAction*>(0) );
      fRunManager->SetUserAction( static_cast<G4UserEventAction*>(0) );
      fRunManager->SetUserAction( static_cast<G4UserTrackingAction*>(0) );
      fRunManager->SetUserAction( static_cast<G4UserSteppingAction*>(0) );
      if ( wasStacking ) {
        fRunManager->SetUserAction( static_cast<G4UserStackingAction*>(0) );
      }

      delete fRunManager;
    }
    else{
      std::cerr << "ERROR: " << __FILE__ << ": line " << __LINE__ 
		<< ": G4Helper never initialized; probably because there were no input primary events"
		<< std::endl;
    }

    for(size_t i = 0; i < fParallelWorlds.size(); ++i){
      if(fParallelWorlds[i]) delete fParallelWorlds[i];
    }
    fParallelWorlds.clear();
  
  }

  //------------------------------------------------
  void G4Helper::SetPhysicsList(std::string physicsString)
  {
    /// Set up the physics list for Geant4, and pass it to Geant4's
    /// run manager.
    /// Without a physics list, Geant4 won't do anything.  G4 comes with a
    /// number of pre-constructed lists, and for now I plan to use
    /// "QGSP_BERT".  It has the following properties:
    ///
    /// - Standard EM physics processes.
    /// - Quark-gluon string model for high energies (> 20GeV)
    /// - Low Energy Parameterized (LEP) for medium energies (10<E<20GeV)
    /// - Gertini-style cascade for low energies (< 10GeV)
    /// - LEP, HEP for all anti-baryons (LEP,HEP = low/high energy parameterized, from GHEISHA)
    /// - Gamma-nuclear model added for E<3.5 GeV
    /// (comments from "Guided Tour of Geant4 Physics List II",
    /// talk given at JPL by Dennis Wright)
    ///
    /// if we decide that QGSP_BERT is not what we want, then we will
    /// have to write a new physics list class that derives from 
    /// G4VUserPhysicsList that does what we want.

    G4VUserPhysicsList* physics = 0;
    std::string bywhom = "User";
    std::string factoryname = "G4PhysListFactory";
    bool list_known_procs = true;

    // physics list name is the first part, anything afterwards is
    // extra physics processes to be added to the base list
    // ie. "QGSP_BERT ; myspace::MonopolePhysics ; MyOtherSpecialPhysics "
    std::vector< std::string > pstrings;
    // don't use ":" as a separator because it's used in namespaces
    boost::algorithm::split( pstrings, physicsString, boost::is_any_of(";"),
                             boost::token_compress_on );
    // trim lead/trail space
    for (unsigned int j=0; j < pstrings.size(); ++j )
      boost::algorithm::trim(pstrings[j]);

    if ( pstrings.size() < 1 ) pstrings.push_back("");  // non-empty
    std::string phListName = pstrings[0];

    //for (unsigned int j=0; j < pstrings.size(); ++j )
    //  std::cout << "G4Helper pstrings[" << j << "] = \"" 
    //            << pstrings[j] << "\"" << std::endl;

    if ( ! physics ) {
#ifdef TRY_NEW_PL_FACTORY
      // user extensible physics list factory
      alt::G4PhysListFactory factory;
      factoryname = "alt::G4PhysListFactory";
#else
      // The official Geant4 G4PhysListFactory _isn't_ a modern factory;
      // it can only generate items that have pre-programmed blueprints
      // already known to it (via if/else-if calls to various ctors) and
      // is not user extensible (i.e. you can't send it blueprints and 
      // have it make them for you).  If we have our own physics list 
      // then we need to select on and construct it ourselves before 
      // looking to the factory.

      // Put if/then/else statement here for user defined physics lists
      // when using old stodgy offical G4 factory.
      // Example:
      /*
      //         string name                                actual class ctor
      if      ( "MY_COOL_PL"  == phListName ) {physics = new My_Cool_PL();}
      else if ( "MY_OTHER_PL" == phListName ) {physics = new My_Other_PL();}
      */

      G4PhysListFactory factory;   // official G4 factory
#endif

      if ( ! physics ) {
	if ( factory.IsReferencePhysList(phListName) ) {
	  bywhom  = factoryname;
	  physics = factory.GetReferencePhysList(phListName);
	} else {
	  // in the case of non-default name
	  if ( phListName != "" ) {
	    std::cerr << std::endl << factoryname 
                      << " failed to find ReferencePhysList \"" 
                      << phListName << "\"" << std::endl;                      
#ifdef TRY_NEW_PL_FACTORY
            factory.PrintAvailablePhysLists();
#else
	    std::vector<G4String> list = factory.AvailablePhysLists();
	    std::cout << "For reference: PhysicsLists in G4PhysListFactory are: " 
		      << std::endl;
	    for (size_t indx=0; indx < list.size(); ++indx ) {
	      std::cout << " [" << std::setw(2) << indx << "] " 
			<< "\"" << list[indx] << "\"" << std::endl;
	    }
#endif
	  }
	} // query factory
      }  // no predetermined user list

      if ( ! physics ) {
	std::cerr << "G4PhysListFactory could not construct \""
		  << phListName << "\"," << std::endl 
                  << "fall back to using QGSP_BERT"
		  << std::endl;
	physics = new QGSP_BERT;
        phListName = "QGSP_BERT";
      
      } else {
	std::cout << bywhom << " constructed G4VUserPhysicsList \""
		  << phListName << "\""
		  << std::endl;
      }

    }

    // Extend the physics list with additional physics processes
    // Already used pstrings[0] entry for physics list name.
    // The rest should be semi-colon separated list of:
    //    physicsProcessName ( optional UI command , more UI commands )
    for (unsigned int k=1; k < pstrings.size(); ++k ) {
      std::string physProcAddition = pstrings[k];

      // break off UI commands from process name
      std::vector< std::string > physProcParts;
      boost::algorithm::split( physProcParts, physProcAddition, 
                               boost::is_any_of("(,)"), 
                               boost::token_compress_on );
      // trim lead/trail spaces
      for (unsigned int j=0; j < physProcParts.size(); ++j )
        boost::algorithm::trim(physProcParts[j]);

      // element 0 is the physics process name
      std::string physProcName = physProcParts[0];
      if ( physProcName == "" ) continue;  // not real, user has trailing ";"
      G4PhysicsProcessFactorySingleton& procFactory = 
        G4PhysicsProcessFactorySingleton::Instance();

      if ( ! procFactory.IsKnownPhysicsProcess(physProcName) ) {
        std::cout << "G4PhysicsProcessFactorySingleton could not "
                  << "construct a \"" << physProcName << "\"" << std::endl;
        if ( ! list_known_procs ) continue;
        list_known_procs = false;
        std::vector<G4String> list = procFactory.AvailablePhysicsProcesses();
        std::cout << "For reference: PhysicsProcesses in "
                  << "G4PhysicsProcessFactorySingleton are: " 
                  << std::endl;
        if ( list.empty() ) std::cout << " ... no registered processes" << std::endl;
        else {
          for (size_t indx=0; indx < list.size(); ++indx ) {
            std::cout << " [" << std::setw(2) << indx << "] " 
                      << "\"" << list[indx] << "\"" << std::endl;
          }
        }
        continue;
      }

      std::cout << "Adding \"" << physProcName 
                << "\" physics process to \"" << phListName << "\"" 
                << std::endl;

      // construct physics process, add it to the base physics list
      G4VPhysicsConstructor* pctor = procFactory.GetPhysicsProcess(physProcName);


      G4VModularPhysicsList* mpl = dynamic_cast<G4VModularPhysicsList*>(physics);
      if      ( ! pctor ) std::cout << " ... failed with null pointer" << std::endl;
      else if ( ! mpl )   std::cout << " ... failed, physics list wasn't a G4VModularPhysicsList" << std::endl;
      else mpl->RegisterPhysics(pctor);

      // Handle associated UI commands
      // One must do it here for cases where values need to be set *before*
      // one calls SetUserInitialization(physics)

      for ( unsigned int i=1; i < physProcParts.size(); ++i ) {
        if ( physProcParts[i] == "" ) continue;
        std::cout // << " apply UI command: " 
                  << physProcParts[i] << std::endl;
        fUIManager->ApplyCommand(physProcParts[i]);
      }

    }

    // pass off (possibly augmented) physics list to run manager
    // which calls G4RunManagerKernel->SetPhysics() on it 
    //   which itself call ConstructParticle() for the list
    fRunManager->SetUserInitialization(physics);
  }

  //------------------------------------------------
  void G4Helper::SetParallelWorlds(std::vector<G4VUserParallelWorld*> pworlds)
  {
    for(auto const& pw : pworlds){
      LOG_DEBUG("G4Helper") << pw->GetName();
      fParallelWorlds.push_back(pw);
    }

    return;
  }

  //------------------------------------------------
  void G4Helper::ConstructDetector(std::string const& gdmlFile)
  {
    // Build the Geant4 detector description.
    bool checkOverlaps      = fCheckOverlaps;
    bool validateGDMLSchema = fValidateGDMLSchema;
    fDetector = new DetectorConstruction(gdmlFile,
                                         checkOverlaps,
                                         validateGDMLSchema);

    return;
  }

  //------------------------------------------------
  /// Initialization for the Geant4 Monte Carlo.
  void G4Helper::InitPhysics() 
  {
    if(!fDetector) this->ConstructDetector(fGDMLFile);

    for(size_t i = 0; i < fParallelWorlds.size(); ++i)
      fDetector->RegisterParallelWorld( fParallelWorlds[i] );

    // define the physics list to use
    this->SetPhysicsList(fG4PhysListName);

    // Pass the detector geometry on to Geant4.
    fRunManager->SetUserInitialization(fDetector);
  
    // Tell the Geant4 run manager how to generate events.  The
    // ConvertMCTruthToG4 class will "generate" events by
    // converting MCTruth objects from the input into G4Events.
    fConvertMCTruth = new ConvertMCTruthToG4;
    fRunManager->SetUserAction(fConvertMCTruth);
  }


  //------------------------------------------------
  /// Initialization for the Geant4 Monte Carlo.
  void G4Helper::SetUserAction() 
  {
    // Geant4 comes with "user hooks" that allows users to perform
    // special tasks at the beginning and end of runs, events, tracks,
    // steps.  By using the UserActionManager, we've separated each
    // set of user tasks into their own class; e.g., there can be one
    // class for processing particles, one class for histograms, etc.

    // Use the UserActionManager to handle all the Geant4 user hooks.
    UserActionManager* uaManager = UserActionManager::Instance();

    // Tell the run manager about our user-action classes. We convert
    // the UserActionManager into different types so Geant4's run and
    // event managers will initialize them properly.
    G4UserRunAction*      runAction      = (G4UserRunAction*     ) uaManager;
    G4UserEventAction*    eventAction    = (G4UserEventAction*   ) uaManager;
    G4UserTrackingAction* trackingAction = (G4UserTrackingAction*) uaManager;
    G4UserSteppingAction* steppingAction = (G4UserSteppingAction*) uaManager;
    fRunManager->SetUserAction( runAction      );
    fRunManager->SetUserAction( eventAction    );
    fRunManager->SetUserAction( trackingAction );
    fRunManager->SetUserAction( steppingAction );

    if ( uaManager->DoesAnyActionProvideStacking() ) {
      G4UserStackingAction* stackingAction = (G4UserStackingAction*) uaManager;
      fRunManager->SetUserAction( stackingAction );
    }

    // Tell the manager to execute the contents of the Geant4 macro
    // file.
    if ( ! fG4MacroPath.empty() ) {
      G4String command = "/control/execute " + G4String(fG4MacroPath);
      fUIManager->ApplyCommand(command);
    }
 
    /// Tell Geant4 to initialize the run manager.  We're ready to
    /// simulate events in the detector.
    fRunManager->Initialize();

    return;
  }

  //------------------------------------------------
  bool G4Helper::G4Run(art::Ptr<simb::MCTruth>& primary) 
  {
    return this->G4Run( primary.get() );
  }

  //------------------------------------------------
  bool G4Helper::G4Run(const simb::MCTruth* primary) 
  {
    // Get the event converter ready.
    fConvertMCTruth->Reset();

    // Pass the MCTruth to our event generator.
    fConvertMCTruth->Append( primary );
    
    // Start the simulation for this event.  Note: The following
    // statement increments the G4RunManager's run number.  Because of
    // this, it's important for events to use the run/event number
    // from the EventDataModel Header, not G4's internal numbers.
    fUIManager->ApplyCommand("/run/beamOn 1");

    return true;
  }

  //------------------------------------------------
  bool G4Helper::G4Run(std::vector<const simb::MCTruth*> &primaries) 
  {
    // Get the event converter ready.
    fConvertMCTruth->Reset();

    // Pass all the MCTruths to our event generator.
    for(auto primary : primaries)
      fConvertMCTruth->Append( primary );
    
    // Start the simulation for this event.  Note: The following
    // statement increments the G4RunManager's run number.  Because of
    // this, it's important for events to use the run/event number
    // from the EventDataModel Header, not G4's internal numbers.
    fUIManager->ApplyCommand("/run/beamOn 1");

    return true;
  }

} // namespace
