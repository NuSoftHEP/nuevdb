////////////////////////////////////////////////////////////////////////
/// \file  GENIEReweight.cxx
/// \brief Wrapper for reweight neutrino interactions with GENIE base class
///
/// \author  nathan.mayer@tufts.edu
////////////////////////////////////////////////////////////////////////

// C/C++ includes
#include <math.h>
#include <map>
#include <fstream>

//ROOT includes
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TSystem.h"

//GENIE includes
#include "Conventions/Units.h"
#include "EVGCore/EventRecord.h"
#include "GHEP/GHepUtils.h"
#include "Messenger/Messenger.h"
// Necessary because the GENIE LOG_* macros don't fully qualify Messenger
using genie::Messenger;

#include "ReWeight/GReWeightI.h"
#include "ReWeight/GSystSet.h"
#include "ReWeight/GSyst.h"
#include "ReWeight/GReWeight.h"
#include "ReWeight/GReWeightNuXSecNCEL.h"
#include "ReWeight/GReWeightNuXSecCCQE.h"
#include "ReWeight/GReWeightNuXSecCCRES.h"
#include "ReWeight/GReWeightNuXSecCOH.h"
#include "ReWeight/GReWeightNonResonanceBkg.h"
#include "ReWeight/GReWeightFGM.h"
#include "ReWeight/GReWeightDISNuclMod.h"
#include "ReWeight/GReWeightResonanceDecay.h"
#include "ReWeight/GReWeightFZone.h"
#include "ReWeight/GReWeightINuke.h"
#include "ReWeight/GReWeightAGKY.h"
#include "ReWeight/GReWeightNuXSecCCQEvec.h"
#include "ReWeight/GReWeightNuXSecNCRES.h" 
#include "ReWeight/GReWeightNuXSecDIS.h"   
#include "ReWeight/GReWeightNuXSecNC.h"  
#include "ReWeight/GSystUncertainty.h"
#include "ReWeight/GReWeightUtils.h" 

#include "Geo/ROOTGeomAnalyzer.h"
#include "Geo/GeomVolSelectorFiducial.h"
#include "Geo/GeomVolSelectorRockBox.h"
#include "Utils/StringUtils.h"
#include "Utils/XmlParserUtils.h"
#include "Interaction/InitialState.h"
#include "Interaction/Interaction.h"
#include "Interaction/Kinematics.h"
#include "Interaction/KPhaseSpace.h"
#include "Interaction/ProcessInfo.h"
#include "Interaction/XclsTag.h"
#include "GHEP/GHepParticle.h"
#include "PDG/PDGCodeList.h"
#include "Conventions/Constants.h" //for calculating event kinematics

//NuTools includes
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCNeutrino.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "NuReweight/GENIEReweight.h"

// Framework includes
//#include "messagefacility/MessageLogger/MessageLogger.h"



namespace rwgt {
  ///<constructor
  GENIEReweight::GENIEReweight() :
    fReweightNCEL(false),
    fReweightQEMA(false),
    fReweightQEVec(false),
    fReweightCCRes(false),
    fReweightNCRes(false),
    fReweightResBkg(false),
    fReweightResDecay(false),
    fReweightNC(false),
    fReweightDIS(false),
    fReweightCoh(false),
    fReweightAGKY(false),
    fReweightDISNucMod(false),
    fReweightFGM(false),
    fReweightFZone(false),
    fReweightINuke(false), 
    fReweightMEC(false),
    fMaQEshape(false),
    fMaCCResShape(false),
    fMaNCResShape(false),
    fDISshape(false),
    fUseSigmaDef(true) {
    
    LOG_INFO("GENIEReweight") << "Create GENIEReweight object";
    
    fWcalc = new genie::rew::GReWeight();
    this->SetNominalValues();
  }
  
  ///<destructor
  GENIEReweight::~GENIEReweight() {
    delete fWcalc;
  }
  
  ///<Set the nominal values for the reweight parameters.
  void GENIEReweight::SetNominalValues() {
    //CCQE Nominal Values
    fNominalParameters[(int)rwgt::fReweightMaNCEL] = 0.99;

    fNominalParameters[(int)rwgt::fReweightEtaNCEL] = 0.12;
    
    //CCQE Nominal Values
    fNominalParameters[(int)rwgt::fReweightNormCCQE] = 1.0;
        
    fNominalParameters[(int)rwgt::fReweightNormCCQEenu] = 1.0;
        
    fNominalParameters[(int)rwgt::fReweightMaCCQEshape] = 0.99;
        
    fNominalParameters[(int)rwgt::fReweightMaCCQE] = 0.99;
    fNominalParameters[(int)rwgt::fReweightVecCCQEshape] = 0.84;
        
    //Resonance Nominal Values
    fNominalParameters[(int)rwgt::fReweightNormCCRES] = 1.0;
    fNominalParameters[(int)rwgt::fReweightMaCCRESshape] = 1.12;
    fNominalParameters[(int)rwgt::fReweightMvCCRESshape]  = 0.84;
    fNominalParameters[(int)rwgt::fReweightMaCCRES] = 1.12;
    fNominalParameters[(int)rwgt::fReweightMvCCRES] = 0.84;
    
    fNominalParameters[(int)rwgt::fReweightNormNCRES] = 1.0;
    fNominalParameters[(int)rwgt::fReweightMaNCRESshape] = 1.12;
    fNominalParameters[(int)rwgt::fReweightMvNCRESshape] = 0.84;
    fNominalParameters[(int)rwgt::fReweightMaNCRES] = 1.12;
    fNominalParameters[(int)rwgt::fReweightMvNCRES] = 0.84;
 
    //Coherent pion nominal values
    fNominalParameters[(int)rwgt::fReweightMaCOHpi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightR0COHpi] = 1.0;
    
    
    // Non-resonance background tweaking parameters:
    fNominalParameters[(int)rwgt::fReweightRvpCC1pi] = 1.0; 
    fNominalParameters[(int)rwgt::fReweightRvpCC2pi] = 1.0; 
    fNominalParameters[(int)rwgt::fReweightRvpNC1pi] = 1.0; 
    fNominalParameters[(int)rwgt::fReweightRvpNC2pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvnCC1pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvnCC2pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvnNC1pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvnNC2pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvbarpCC1pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvbarpCC2pi] = 1.0; 
    fNominalParameters[(int)rwgt::fReweightRvbarpNC1pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvbarpNC2pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvbarnCC1pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvbarnCC2pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvbarnNC1pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightRvbarnNC2pi] = 1.0;

    
    // DIS tweaking parameters - applied for DIS events with [Q2>Q2o, W>Wo], typically Q2okReweight =1GeV^2, WokReweight =1.7-2.0GeV
    fNominalParameters[(int)rwgt::fReweightAhtBY] = 0.538;
    fNominalParameters[(int)rwgt::fReweightBhtBY] = 0.305;
    fNominalParameters[(int)rwgt::fReweightCV1uBY] = 0.291;
    fNominalParameters[(int)rwgt::fReweightCV2uBY] = 0.189;
    
    fNominalParameters[(int)rwgt::fReweightAhtBYshape] = 0.538;
    fNominalParameters[(int)rwgt::fReweightBhtBYshape] = 0.305;
    fNominalParameters[(int)rwgt::fReweightCV1uBYshape] = 0.291;
    fNominalParameters[(int)rwgt::fReweightCV2uBYshape] = 0.189;
    
    fNominalParameters[(int)rwgt::fReweightNormDISCC] = 1.0;
    

    fNominalParameters[(int)rwgt::fReweightRnubarnuCC] = 0.0;//v to vbar ratio reweighting is not working in GENIE at the moment
    fNominalParameters[(int)rwgt::fReweightDISNuclMod] = 0.0;//The DIS nuclear model switch is not working in GENIE at the moment
    //

    fNominalParameters[(int)rwgt::fReweightNC] = 1.0;
        
    //
    // Hadronization [free nucleon target]
    // 
    fNominalParameters[(int)rwgt::fReweightAGKY_xF1pi] = 0.385; 
    fNominalParameters[(int)rwgt::fReweightAGKY_pT1pi] = 1./6.625;
        
    //
    // Medium-effects to hadronization
    // 
    fNominalParameters[(int)rwgt::fReweightFormZone] = 1.0;
        
    //
    // Intranuclear rescattering systematics.
    fNominalParameters[(int)rwgt::fReweightMFP_pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightMFP_N] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrCEx_pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrElas_pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrInel_pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrAbs_pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrPiProd_pi] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrCEx_N] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrElas_N] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrInel_N] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrAbs_N] = 1.0;
    fNominalParameters[(int)rwgt::fReweightFrPiProd_N] = 1.0;
    

    //
    //RFG Nuclear model
    // 
    fNominalParameters[(int)rwgt::fReweightCCQEPauliSupViaKF] = 1.0;
    fNominalParameters[(int)rwgt::fReweightCCQEMomDistroFGtoSF] = 0.0; 
    //Continous "switch" at 0.0 full FG model.  At 1.0 full spectral function model.  
    //From genie code it looks like weird stuff may happen for <0 and >1.
    //This parameter does not have an "uncertainty" value associated with it.  
    //The tweaked dial value gets passed all the way through unchanged to the weight calculator
    
    //
    // Resonance decays
    // 
    fNominalParameters[(int)rwgt::fReweightBR1gamma] = 1.0;
    fNominalParameters[(int)rwgt::fReweightBR1eta] = 1.0;
    fNominalParameters[(int)rwgt::fReweightTheta_Delta2Npi] = 0.0; 
    //Continous "switch" at 0.0 full isotropic pion angular distribution.  At 1.0 full R/S pion angular distribtuion.
    //This parameter does not have an "uncertainty" value associated with it.  
    //The tweaked dial value gets passed all the way through unchanged to the weight calculator
  }

  ///<Return the nominal value for the given parameter.
  double GENIEReweight::NominalParameterValue(ReweightLabel_t rLabel) {
    double nominal_value;
    nominal_value = fNominalParameters[rLabel];
    return nominal_value;
  }

  ///<Return the configured value of the given parameter.
  double GENIEReweight::ReweightParameterValue(ReweightLabel_t rLabel) {
    int label = (int)rLabel;
    bool in_loop = false;
    bool found_par = false;
    double parameter = -10000;
    for(unsigned int i = 0; i < fReWgtParameterName.size(); i++) {
      in_loop = true;
      if(label == fReWgtParameterName[i]) {
	parameter = fReWgtParameterValue[i];
	found_par = true;
      }
    }
    if(in_loop) {
      if(found_par) return parameter;
      else {
	LOG_WARN("GENIEReweight") << "Parameter " << label << " not set yet or does not exist";
	return parameter;
      }
    }
    else {
      LOG_WARN("GENIEReweight") << "Vector of parameters has not been initialized yet (Size is 0).";
      return parameter;
    }
  }
  
  ///<Add reweight parameters to the list
  void GENIEReweight::AddReweightValue(ReweightLabel_t rLabel, double value) {
    int label = (int)rLabel;   
    LOG_INFO("GENIEReweight") << "Adding parameter: " <<  genie::rew::GSyst::AsString(genie::rew::EGSyst(label)) << ".  With value: " << value;
    fReWgtParameterName.push_back(label);
    fReWgtParameterValue.push_back(value);
    
  }
  
  ///<Change a reweight parameter.  If it hasn't been added yet add it.
  void GENIEReweight::ChangeParameterValue(ReweightLabel_t rLabel, double value) {
    int label = (int)rLabel;
    bool found = false;
    for(unsigned int i = 0; i < fReWgtParameterName.size(); i++) {
      if(fReWgtParameterName[i] == label) {
	fReWgtParameterValue[i] = value;
	found = true;
      }
    }
    if(!found) {
      this->AddReweightValue(rLabel, value);
    }
  }
  
  ///<Configure the weight calculators.
  void GENIEReweight::Configure() {
    LOG_INFO("GENIEReweight") << "Configure weight calculator";
    
    for(unsigned int i = 0; i < fReWgtParameterName.size(); i++) {
      
      //NC Elastic parameters
      if( (fReWgtParameterName[i] == rwgt::fReweightMaNCEL) || 
	  (fReWgtParameterName[i] == rwgt::fReweightEtaNCEL) ) {
	fReweightNCEL = true;
      }
      //CC QE Axial parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightNormCCQE) ||
	       (fReWgtParameterName[i]==rwgt::fReweightNormCCQEenu) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMaCCQEshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMaCCQE) ) {
	fReweightQEMA = true;
      }
      //CC QE Vector parameters
      else if(fReWgtParameterName[i]==rwgt::fReweightVecCCQEshape) {
	fReweightQEVec = true;
      }
      //CC Resonance parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightNormCCRES) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMaCCRESshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMvCCRESshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMaCCRES) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMvCCRES) ) {
	fReweightCCRes = true;
      }
      //NC Resonance parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightNormNCRES) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMaNCRESshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMvNCRESshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMaNCRES) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMvNCRES) ) {
	fReweightNCRes = true;
      }
      //Coherant parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightMaCOHpi) || 
	       (fReWgtParameterName[i]==rwgt::fReweightR0COHpi) ) {
	fReweightCoh = true;
      }
      //Non-resonance background KNO parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightRvpCC1pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvpCC2pi) || 
	       (fReWgtParameterName[i]==rwgt::fReweightRvpNC1pi) || 
	       (fReWgtParameterName[i]==rwgt::fReweightRvpNC2pi) || 
	       (fReWgtParameterName[i]==rwgt::fReweightRvnCC1pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvnCC2pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvnNC1pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvnNC2pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarpCC1pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarpCC2pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarpNC1pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarpNC2pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarnCC1pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarnCC2pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarnNC1pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRvbarnNC2pi) ) {
	fReweightResBkg = true;
      }
      //DIS parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightAhtBY) ||
	       (fReWgtParameterName[i]==rwgt::fReweightBhtBY) ||
	       (fReWgtParameterName[i]==rwgt::fReweightCV1uBY) || 
	       (fReWgtParameterName[i]==rwgt::fReweightCV2uBY) ||
	       (fReWgtParameterName[i]==rwgt::fReweightAhtBYshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightBhtBYshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightCV1uBYshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightCV2uBYshape) ||
	       (fReWgtParameterName[i]==rwgt::fReweightNormDISCC) ||
	       (fReWgtParameterName[i]==rwgt::fReweightRnubarnuCC) ) {
	fReweightDIS = true;
      }
      //DIS nuclear model parameters
      else if ( fReWgtParameterName[i]==rwgt::fReweightDISNuclMod ) {
	fReweightDISNucMod = true;
      }
      //NC cross section
      else if( fReWgtParameterName[i]==rwgt::fReweightNC) {
	fReweightNC = true;
      }
      //Hadronization parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightAGKY_xF1pi) || 
	       (fReWgtParameterName[i]==rwgt::fReweightAGKY_pT1pi) ) {
	fReweightAGKY = true;
      }
      //Elastic (and QE) nuclear model parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightCCQEPauliSupViaKF) || 
	       (fReWgtParameterName[i]==rwgt::fReweightCCQEMomDistroFGtoSF) ) {
	fReweightFGM = true;
      }
      //Formation Zone
      else if ( fReWgtParameterName[i]==rwgt::fReweightFormZone) {
	fReweightFZone = true;
      }
      //Intranuke Parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightMFP_pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightMFP_N) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrCEx_pi) || 
	       (fReWgtParameterName[i]==rwgt::fReweightFrElas_pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrInel_pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrAbs_pi) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrPiProd_pi) || 
	       (fReWgtParameterName[i]==rwgt::fReweightFrCEx_N) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrElas_N) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrInel_N ) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrAbs_N ) ||
	       (fReWgtParameterName[i]==rwgt::fReweightFrPiProd_N) ) {
	fReweightINuke = true;
      }
      //Resonance Decay parameters
      else if( (fReWgtParameterName[i]==rwgt::fReweightBR1gamma) ||
	       (fReWgtParameterName[i]==rwgt::fReweightBR1eta) ||
	       (fReWgtParameterName[i]==rwgt::fReweightTheta_Delta2Npi)){
	
	fReweightResDecay = true;
      }
    } //end for loop
    
    //configure the individual weight calculators
    if(fReweightNCEL) this->ConfigureNCEL();
    if(fReweightQEMA) this->ConfigureQEMA();
    if(fReweightQEVec) this->ConfigureQEVec();
    if(fReweightCCRes) this->ConfigureCCRes();
    if(fReweightNCRes) this->ConfigureNCRes();
    if(fReweightResBkg) this->ConfigureResBkg();
    if(fReweightResDecay) this->ConfgureResDecay();
    if(fReweightNC) this->ConfigureNC();
    if(fReweightDIS) this->ConfigureDIS();
    if(fReweightCoh) this->ConfigureCoh();
    if(fReweightAGKY) this->ConfigureAGKY();
    if(fReweightDISNucMod) this->ConfigureDISNucMod();
    if(fReweightFGM) this->ConfigureFGM();
    if(fReweightFZone) this->ConfigureFZone();
    if(fReweightINuke) this->ConfigureINuke();
    this->ConfigureParameters();
    
  }
  
  ///<Reconfigure the weight calculators
  void GENIEReweight::Reconfigure() {
    delete fWcalc;
    fWcalc = new genie::rew::GReWeight();
    this->Configure();
  }
  
  ///<Simple Configuration functions for configuring a single weight calculator
  
  ///<Simple Configuraiton of the NC elastic weight calculator
  void GENIEReweight::ReweightNCEL(double ma, double eta) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for NC Elastic Reweighting";
    if(ma!=0.0) {
      this->AddReweightValue(rwgt::fReweightMaNCEL, ma);
    }
    if(eta!=0.0) {
      this->AddReweightValue(rwgt::fReweightEtaNCEL, eta);
    }
    this->Configure();
  }
  
  ///<Simple Configurtion of the CCQE axial weight calculator
  void GENIEReweight::ReweightQEMA(double ma) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for QE Axial Mass Reweighting";
    fMaQEshape=false;
    this->AddReweightValue(rwgt::fReweightMaCCQE, ma);
    this->Configure();
  }

  ///<Simple Configuration of the CCQE vector weight calculator
  void GENIEReweight::ReweightQEVec(double mv) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for QE Vector Mass Reweighting";
    this->AddReweightValue(rwgt::fReweightVecCCQEshape, mv);
    this->Configure();
  }
  
  ///<Simple Configuration of the CC Resonance weight calculator
  void GENIEReweight::ReweightCCRes(double ma, double mv) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for CC Resonance Reweighting";
    fMaCCResShape=false;
    this->AddReweightValue(rwgt::fReweightMaCCRES, ma);
    if(mv!=0.0) {
      this->AddReweightValue(rwgt::fReweightMvCCRES, mv);
    }
    this->Configure();
  }
  
  ///<Simple Configurtion of the NC Resonance weight calculator
  void GENIEReweight::ReweightNCRes(double ma, double mv) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for NC Resonance Reweighting";
    fMaNCResShape=false;
    this->AddReweightValue(rwgt::fReweightMaNCRES, ma);
    if(mv!=0.0) {
      this->AddReweightValue(rwgt::fReweightMvNCRES, mv);
    }
    this->Configure();
  }

  ///<Simple Configuration of the NC and CC Resonance weight calculator with the axial mass parameter for NC/CC ganged together
  void GENIEReweight::ReweightResGanged(double ma, double mv) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for CC and NC Resonance Reweighting";
    fMaCCResShape=false;
    fMaNCResShape=false;
    this->AddReweightValue(rwgt::fReweightMaCCRES, ma);
    this->AddReweightValue(rwgt::fReweightMaNCRES, ma);
    if(mv!=0.0) {
      this->AddReweightValue(rwgt::fReweightMvCCRES, mv);
      this->AddReweightValue(rwgt::fReweightMvNCRES, mv);
    }
    this->Configure();
  }
  
  ///<Simple Configuration of the Coherant weight calculator
  void GENIEReweight::ReweightCoh(double ma, double r0) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for Coherant Reweighting";
    this->AddReweightValue(rwgt::fReweightMaCOHpi, ma);
    this->AddReweightValue(rwgt::fReweightR0COHpi, r0);
    this->Configure();
  }
  
  ///<Simple Configuration of the Non-Resonance Background weight calculator.  
  //Here it is being configured for v+p and vbar + n (1 pi) type interactions
  void GENIEReweight::ReweightNonResRvp1pi(double sigma) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for  Non-Resonance Background Reweighting (Neutrino Single Pion)";
    this->AddReweightValue(rwgt::fReweightRvpCC1pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarnCC1pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvpNC1pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarnNC1pi, sigma);
    this->Configure();
  }
  
  ///<Simple Configuration of the Non-Resonance Background weight calculator.  
  //Here it is being configured for v+n and vbar + p (1 pi) type interactions
  void GENIEReweight::ReweightNonResRvbarp1pi(double sigma) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for  Non-Resonance Background Reweighting (Anti-Neutrino Single Pion)";
    this->AddReweightValue(rwgt::fReweightRvnCC1pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarpCC1pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvnNC1pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarpNC1pi, sigma);
    this->Configure();
  }
  
  ///<Simple Configuration of the Non-Resonance Background weight calculator.  Here it is being configured for v+p and vbar + n (2 pi) type interactions
  void GENIEReweight::ReweightNonResRvp2pi(double sigma) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for  Non-Resonance Background Reweighting (Neutrino Two Pion)";
    this->AddReweightValue(rwgt::fReweightRvpCC2pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarnCC2pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvpNC2pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarnNC2pi, sigma);
    this->Configure();
  }
  
  ///<Simple Configuration of the Non-Resonance Background weight calculator. 
  // Here it is being configured for v+n and vbar + p (2 pi) type interactions
  void GENIEReweight::ReweightNonResRvbarp2pi(double sigma) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for  Non-Resonance Background Reweighting (Anti-Neutrino Two Pion)";
    this->AddReweightValue(rwgt::fReweightRvnCC2pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarpCC2pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvnNC2pi, sigma);
    this->AddReweightValue(rwgt::fReweightRvbarpNC2pi, sigma);
    this->Configure();
  }
  
  ///<Simple Configuration of the Resonance decay model weight calculator
  void GENIEReweight::ReweightResDecay(double gamma, double eta, double theta) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for Resoncance Decay Parameters";
    if(gamma!=0.0) {
      this->AddReweightValue(rwgt::fReweightBR1gamma, gamma);
    }
    if(eta!=0.0) {
      this->AddReweightValue(rwgt::fReweightBR1eta, eta);
    }
    if(theta!=0.0) {
      this->AddReweightValue(rwgt::fReweightTheta_Delta2Npi, theta);
    }
    this->Configure();
  }
  
  ///<Simple Configuration of the Total NC cross section
  void GENIEReweight::ReweightNC(double norm) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for NC Cross Section Scale";
    this->AddReweightValue(rwgt::fReweightNC, norm);
    this->Configure();
  }

  ///<Simple Configuration of the DIS FF model weight calculator
  void GENIEReweight::ReweightDIS(double aht, double bht, double cv1u, double cv2u) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for DIS Form Factor Model Reweighting";
    fDISshape = false;
    if(aht != 0.0) {
      this->AddReweightValue(rwgt::fReweightAhtBY, aht);
    }
    if(bht != 0.0) {
      this->AddReweightValue(rwgt::fReweightBhtBY, bht);
    }
    if(cv1u != 0.0) {
      this->AddReweightValue(rwgt:: fReweightCV1uBY, cv1u);
    }
    if(cv2u != 0.0) {
      this->AddReweightValue(rwgt::fReweightCV2uBY, cv2u);
    }
    this->Configure();
  }
  
  ///<Simple Configuration of the DIS nuclear model
  void GENIEReweight::ReweightDISnucl(bool mode) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for DIS Nuclear Model";
    this->AddReweightValue(rwgt::fReweightDISNuclMod, mode);
    this->Configure();
  }
  
  ///<Simple Configuration of the DIS AGKY hadronization model
  void GENIEReweight::ReweightAGKY(double xF, double pT) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for DIS AGKY Hadronization Model Reweighting";
    if(xF==0.0) {
      this->AddReweightValue(rwgt::fReweightAGKY_xF1pi, xF);
    }
    if(pT==0.0) {
      this->AddReweightValue(rwgt::fReweightAGKY_pT1pi, pT);
    }
    this->Configure();
  }
  
  ///<Simple Configuration of the Intranuke Nuclear model
  void GENIEReweight::ReweightIntraNuke(ReweightLabel_t name, double sigma) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for Intranuke Model Reweighting";
    if ( (name==rwgt::fReweightMFP_pi) ||
	 (name==rwgt::fReweightMFP_N) ||
	 (name==rwgt::fReweightFrCEx_pi) || 
	 (name==rwgt::fReweightFrElas_pi) ||
	 (name==rwgt::fReweightFrInel_pi) ||
	 (name==rwgt::fReweightFrAbs_pi) ||
	 (name==rwgt::fReweightFrPiProd_pi) || 
	 (name==rwgt::fReweightFrCEx_N) ||
	 (name==rwgt::fReweightFrElas_N) ||
	 (name==rwgt::fReweightFrInel_N ) ||
	 (name==rwgt::fReweightFrAbs_N ) ||
	 (name==rwgt::fReweightFrPiProd_N) ) {
      this->AddReweightValue(name, sigma);
    }
    else {
      LOG_WARN("GENIEReweight") << "That is not a valid Intranuke parameter Intranuke not configured";
    }
    this->Configure();
  }

  ///<Simple Configuration of the Formation Zone reweight calculator
  void GENIEReweight::ReweightFormZone(double sigma) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for Formation Zone Reweighting";
    this->AddReweightValue(rwgt::fReweightFormZone, sigma);
    this->Configure();
  }
  
  ///<Simple Configuration of the Fermigas model reweight calculator
  void GENIEReweight::ReweightFGM(double kF, double sf) {
    LOG_INFO("GENIEReweight") << "Configuring GENIEReweight for Fermi Gas Model Reweighting";
    this->AddReweightValue(rwgt::fReweightCCQEPauliSupViaKF, kF);
    this->AddReweightValue(rwgt::fReweightCCQEMomDistroFGtoSF, sf);
    this->Configure();
  }

  ///<End of Simple Reweight Configurations.
  
  ///<Private Member functions to configure individual weight calculators.
  ///<Configure the NCEL weight calculator
  void GENIEReweight::ConfigureNCEL() {
    LOG_INFO("GENIEReweight") << "Adding NC elastic weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_ncel",       new GReWeightNuXSecNCEL      );
  }
  
  ///<Configure the MaQE weight calculator
  void GENIEReweight::ConfigureQEMA() {
    LOG_INFO("GENIEReweight") << "Adding CCQE axial FF weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_ccqe",       new GReWeightNuXSecCCQE      );
    if(!fMaQEshape) {
      LOG_INFO("GENIEReweight") << "in axial mass (QE) rate+shape mode";
      GReWeightNuXSecCCQE *rwccqe = dynamic_cast <GReWeightNuXSecCCQE*> (fWcalc->WghtCalc("xsec_ccqe"));
      rwccqe->SetMode(GReWeightNuXSecCCQE::kModeMa);
    }
    else {
      LOG_INFO("GENIEReweight") << "in axial mass (QE) shape only mode";
    }
  }

  ///<Configure the QE vector FF weightcalculator
  void GENIEReweight::ConfigureQEVec() {
    LOG_INFO("GENIEReweight") << "Adding CCQE vector FF weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_ccqe_vec",   new GReWeightNuXSecCCQEvec   );
  }

  ///<Configure the CCRES calculator
  void GENIEReweight::ConfigureCCRes() {
    LOG_INFO("GENIEReweight") << "Adding CC resonance weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_ccres",      new GReWeightNuXSecCCRES     );
    if(!fMaCCResShape) {
      LOG_INFO("GENIEReweight") << "in axial mass (Res) rate+shape mode";
      GReWeightNuXSecCCRES * rwccres = dynamic_cast<GReWeightNuXSecCCRES *> (fWcalc->WghtCalc("xsec_ccres")); 
      rwccres->SetMode(GReWeightNuXSecCCRES::kModeMaMv);
    }
    else {
      LOG_INFO("GENIEReweight") << "in axial mass (Res) shape only mode";
    }
  }

  ///<Configure the NCRES calculator
  void GENIEReweight::ConfigureNCRes() {
    LOG_INFO("GENIEReweight") << "Adding NC resonance weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_ncres",      new GReWeightNuXSecNCRES     );
    if(!fMaNCResShape) {
      LOG_INFO("GENIEReweight") << "in axial mass (Res) rate+shape mode";
      GReWeightNuXSecNCRES * rwncres = dynamic_cast<GReWeightNuXSecNCRES *> (fWcalc->WghtCalc("xsec_ncres")); 
      rwncres->SetMode(GReWeightNuXSecNCRES::kModeMaMv);
    }
    else {
      LOG_INFO("GENIEReweight") << "in axial mass (Res) shape only mode";
    }
  }

  ///<Configure the ResBkg (kno) weight calculator
  void GENIEReweight::ConfigureResBkg() {
    LOG_INFO("GENIEReweight") << "Adding low Q^2 DIS (KNO) weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_nonresbkg",  new GReWeightNonResonanceBkg );
  }

  ///<Configure the ResDecay weight calculator
  void GENIEReweight::ConfgureResDecay() {
    LOG_INFO("GENIEReweight") << "Adding resonance decay weight calculator";
    fWcalc->AdoptWghtCalc( "hadro_res_decay", new GReWeightResonanceDecay  );
  }

  ///<Configure the NC weight calculator
  void GENIEReweight::ConfigureNC() {
    LOG_INFO("GENIEReweight") << "Adding NC total cross section weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_nc", new GReWeightNuXSecNC );
  }

  ///<Configure the DIS (Bodek-Yang) weight calculator
  void GENIEReweight::ConfigureDIS() {
    LOG_INFO("GENIEReweight") << "Adding DIS (Bodek-Yang) weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_dis",        new GReWeightNuXSecDIS       );
    if(!fDISshape) {
      LOG_INFO("GENIEReweight") << "in shape+rate mode";
      GReWeightNuXSecDIS * rwdis = dynamic_cast<GReWeightNuXSecDIS *> (fWcalc->WghtCalc("xsec_dis"));
      rwdis->SetMode(GReWeightNuXSecDIS::kModeABCV12u);
    }
    else {
      LOG_INFO("GENIEReweight") << "in shape only mode";
    }
  }

  ///<Configure the Coherant model weight calculator
  void GENIEReweight::ConfigureCoh() {
    LOG_INFO("GENIEReweight") << "Adding coherant interaction model weight calculator";
    fWcalc->AdoptWghtCalc( "xsec_coh",        new GReWeightNuXSecCOH       );
  }

  ///<Configure the hadronization (AGKY) weight calculator
  void GENIEReweight::ConfigureAGKY() {
    LOG_INFO("GENIEReweight") << "Adding hadronization (AGKY) model weight calculator";
    fWcalc->AdoptWghtCalc( "hadro_agky",      new GReWeightAGKY            );
  }

  ///<Configure the DIS nuclear model weight calculator
  void GENIEReweight::ConfigureDISNucMod() {
    LOG_INFO("GENIEReweight") << "Adding DIS nuclear model weight calculator";
    fWcalc->AdoptWghtCalc( "nuclear_dis",     new GReWeightDISNuclMod      );
  }

  ///<Configure the FG model weight calculator
  void GENIEReweight::ConfigureFGM() {
    LOG_INFO("GENIEReweight") << "Adding Fermi Gas Model (FGM) weight calculator";
    fWcalc->AdoptWghtCalc( "nuclear_qe",      new GReWeightFGM             );
  }

  ///<Configure the Formation Zone weight calculator
  void GENIEReweight::ConfigureFZone() {
    LOG_INFO("GENIEReweight") << "Adding Formation Zone weight calculator";
    fWcalc->AdoptWghtCalc( "hadro_fzone",     new GReWeightFZone           );
  }

  ///<Configure the intranuke weight calculator
  void GENIEReweight::ConfigureINuke() {
    LOG_INFO("GENIEReweight") << "Adding the Intra-Nuke weight calculator";
    fWcalc->AdoptWghtCalc( "hadro_intranuke", new GReWeightINuke           );
  }

  ///<configure the weight parameters being used
  void GENIEReweight::ConfigureParameters() {
    GSystSet & syst = fWcalc->Systematics();
    for(unsigned int i = 0; i < fReWgtParameterName.size(); i++) {
      LOG_INFO("GENIEReweight") << "Configuring GENIEReweight parameter: " << genie::rew::GSyst::AsString(genie::rew::EGSyst(fReWgtParameterName[i])) << " with value: " << fReWgtParameterValue[i];
      if(fUseSigmaDef) {
	syst.Set( (GSyst_t)fReWgtParameterName[i], fReWgtParameterValue[i]);
      }
      else {
	double parameter = this->CalculateSigma((ReweightLabel_t)fReWgtParameterName[i], fReWgtParameterValue[i]);
	syst.Set( (GSyst_t)fReWgtParameterName[i], parameter);
      }
    }
    fWcalc->Reconfigure();
  }

  ///Used in parameter value mode (instead of parameter sigma mode) Given a user passed parameter value calculate the corresponding sigma value 
  ///that needs to be passed to genie to give the same weight.
  double GENIEReweight::CalculateSigma(ReweightLabel_t label, double value) {
  //double GENIEReweight::CalculateSigma(int label, double value) {
    int iLabel = (int) label;
    double nominal_def;
    double frac_err;
    double sigma;
    int sign;
    GSystUncertainty * gsysterr = GSystUncertainty::Instance();
    if(label==rwgt::fReweightCCQEMomDistroFGtoSF || 
       label==rwgt::fReweightTheta_Delta2Npi || 
       label==rwgt::fReweightDISNuclMod) {
      //These parameters don't use the sigma definition just pass them through the function unchanged
      sigma = value;
    }
    else {
      nominal_def = this->NominalParameterValue(label);
      sign = genie::utils::rew::Sign(value-nominal_def);
      frac_err = gsysterr->OneSigmaErr( (GSyst_t)iLabel, sign);
      sigma = (value - nominal_def)/(frac_err*nominal_def);
    }
    return sigma;
  }

  ///<Calculate the weights
  //double GENIEReweight::CalcWeight(simb::MCTruth truth, simb::GTruth gtruth) {
  double GENIEReweight::CalculateWeight(const genie::EventRecord& evr) {
    //genie::EventRecord evr = this->RetrieveGHEP(truth, gtruth);
    double wgt = fWcalc->CalcWeight(evr);
    //mf::LogVerbatim("GENIEReweight") << "New Event Weight is: " << wgt;
    return wgt;
  }

  /*
  ///< Recreate the a genie::EventRecord from the MCTruth and GTruth objects.
  genie::EventRecord GENIEReweight::RetrieveGHEP(simb::MCTruth truth, simb::GTruth gtruth) {
    
    genie::EventRecord newEvent;
    newEvent.SetWeight(gtruth.fweight);
    newEvent.SetProbability(gtruth.fprobability);
    newEvent.SetXSec(gtruth.fXsec);
    newEvent.SetDiffXSec(gtruth.fDiffXsec);
    TLorentzVector vtx = gtruth.fVertex;
    newEvent.SetVertex(vtx);

    for(int i = 0; i < truth.NParticles(); i++) {
      simb::MCParticle mcpart = truth.GetParticle(i);
      
      int gmid = mcpart.PdgCode();
      genie::GHepStatus_t gmst = (genie::GHepStatus_t)mcpart.StatusCode();
      int gmmo = mcpart.Mother();
      int ndaughters = mcpart.NumberDaughters();
      //find the track ID of the first and last daughter particles
      int fdtrkid = 0;
      int ldtrkid = 0;
      if(ndaughters !=0) {
	fdtrkid = mcpart.Daughter(0);
	if(ndaughters == 1) {
	  ldtrkid = 1;
	}
	else if(ndaughters >1) {
	  fdtrkid = mcpart.Daughter(ndaughters-1);
	}
      }      
      int gmfd = -1;
      int gmld = -1;
      //Genie uses the index in the particle array to reference the daughter particles.
      //MCTruth keeps the particles in the same order so use the track ID to find the proper index.
      for(int j = 0; j < truth.NParticles(); j++) {
	simb::MCParticle temp = truth.GetParticle(i);
	if(temp.TrackId() == fdtrkid) {
	  gmfd = j;
	}
	if(temp.TrackId() == ldtrkid) {
	  gmld = j;
	}
      }
    
      double gmpx = mcpart.Px(0);
      double gmpy = mcpart.Py(0);
      double gmpz = mcpart.Pz(0);
      double gme = mcpart.E(0);
      double gmvx = mcpart.Gvx();
      double gmvy = mcpart.Gvy();
      double gmvz = mcpart.Gvz();
      double gmvt = mcpart.Gvt();
      int gmri = mcpart.Rescatter();
      
      genie::GHepParticle gpart(gmid, gmst, gmmo, -1, gmfd, gmld, gmpx, gmpy, gmpz, gme, gmvx, gmvy, gmvz, gmvt);
      gpart.SetRescatterCode(gmri);
      TVector3 polz = mcpart.Polarization();
      if(polz.x() !=0 || polz.y() !=0 || polz.z() !=0) {
	gpart.SetPolarization(polz);
      }
      newEvent.AddParticle(gpart);

    }

    genie::ProcessInfo proc_info;
    genie::ScatteringType_t gscty = (genie::ScatteringType_t)gtruth.fGscatter;
    genie::InteractionType_t ginty = (genie::InteractionType_t)gtruth.fGint;

    proc_info.Set(gscty,ginty);

    genie::XclsTag gxt;
    
    //Set Exclusive Final State particle numbers
    genie::Resonance_t gres = (genie::Resonance_t)gtruth.fResNum;
    gxt.SetResonance(gres);
    gxt.SetNPions(gtruth.fNumPiPlus, gtruth.fNumPi0, gtruth.fNumPiMinus);
    gxt.SetNNucleons(gtruth.fNumProton, gtruth.fNumNeutron);
    
    if(gtruth.fIsCharm) {
      gxt.SetCharm(0);
    }
     else {
       gxt.UnsetCharm();
     }
    
    //Set the GENIE kinematic info
    genie::Kinematics gkin;
    gkin.Setx(gtruth.fgX, true);
    gkin.Sety(gtruth.fgY, true);
    gkin.Sett(gtruth.fgT, true);
    gkin.SetW(gtruth.fgW, true);
    gkin.SetQ2(gtruth.fgQ2, true);
    gkin.Setq2(gtruth.fgq2, true);
    simb::MCNeutrino nu = truth.GetNeutrino();
    simb::MCParticle lep = nu.Lepton();
    gkin.SetFSLeptonP4(lep.Px(), lep.Py(), lep.Pz(), lep.E());
    gkin.SetHadSystP4(gtruth.fFShadSystP4.Px(), gtruth.fFShadSystP4.Py(), gtruth.fFShadSystP4.Pz(), gtruth.fFShadSystP4.E());
    
    //Set the GENIE final state interaction info
    genie::Interaction * p_gint = new genie::Interaction;
    genie::InitialState * p_ginstate = p_gint->InitStatePtr();
    //int Z = gtruth.ftgtZ;
    //int A = gtruth.ftgtA;
    int targetNucleon = nu.HitNuc();
    int struckQuark = nu.HitQuark(); 
    int incoming = gtruth.fProbePDG;
    p_ginstate->SetProbePdg(incoming);
    
    genie::Target* target123 = p_ginstate->TgtPtr();
    
    target123->SetId(gtruth.ftgtPDG);
    //target123->SetId(Z,A);
    
    //int pdg_code = pdg::IonPdgCode(A, Z);
    //TParticlePDG * p = PDGLibrary::Instance()->Find(pdg_code);
    
    //mf::LogWarning("GENIEReweight") << "Setting Target Z and A";
    //mf::LogWarning("GENIEReweight") << "Saved PDG: " << gtruth.ftgtPDG;
    //mf::LogWarning("GENIEReweight") << "Target PDG: " << target123->Pdg();
    target123->SetHitNucPdg(targetNucleon);
    target123->SetHitQrkPdg(struckQuark);
    target123->SetHitSeaQrk(gtruth.fIsSeaQuark);
    
    if(newEvent.HitNucleonPosition()> 0) {
      genie::GHepParticle * hitnucleon = newEvent.HitNucleon();
      std::auto_ptr<TLorentzVector> p4hitnucleon(hitnucleon->GetP4());
      target123->SetHitNucP4(*p4hitnucleon);
    }  
    else {
      TLorentzVector dummy(0.,0.,0.,0.);
      target123->SetHitNucP4(dummy);
    }
   
    genie::GHepParticle * probe = newEvent.Probe();
    std::auto_ptr<TLorentzVector> p4probe(probe->GetP4());
    p_ginstate->SetProbeP4(*p4probe);
    if(newEvent.TargetNucleusPosition()> 0) {
      genie::GHepParticle * target = newEvent.TargetNucleus();
      std::auto_ptr<TLorentzVector> p4target(target->GetP4());
      p_ginstate->SetTgtP4(*p4target);
    }  else {
      TLorentzVector dummy(0.,0.,0.,0.);
      p_ginstate->SetTgtP4(dummy);
    }
    p_gint->SetProcInfo(proc_info);
    p_gint->SetKine(gkin);
    p_gint->SetExclTag(gxt);
    newEvent.AttachSummary(p_gint);
    
    
    //For temporary debugging purposes
    //genie::Interaction *inter = newEvent.Summary();
    //const genie::InitialState &initState  = inter->InitState();
    //const genie::Target &tgt = initState.Tgt();
    //std::cout << "TargetPDG as Recorded: " << gtruth.ftgtPDG << std::endl;
    //std::cout << "TargetZ as Recorded:   " << gtruth.ftgtZ << std::endl;
    //std::cout << "TargetA as Recorded:   " << gtruth.ftgtA << std::endl;
    //std::cout << "TargetPDG as Recreated: " << tgt.Pdg() << std::endl;   
    //std::cout << "TargetZ as Recreated: " << tgt.Z() << std::endl;   
    //std::cout << "TargetA as Recreated: " << tgt.A() << std::endl;   
    
    return newEvent;
 
  }
*/

  
}
