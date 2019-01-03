////////////////////////////////////////////////////////////////////////
/// \file  NuReweight.cxx
/// \brief Wrapper for reweightings neutrino interactions within the ART framework
///
/// \author  nathan.mayer@tufts.edu
////////////////////////////////////////////////////////////////////////

// C/C++ includes
#include <math.h>
#include <map>
#include <fstream>
#include <memory>

//ROOT includes
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TSystem.h"

//GENIE includes
#ifdef GENIE_PRE_R3
  #include "Conventions/Units.h"
  #include "EVGCore/EventRecord.h"
  #include "GHEP/GHepUtils.h"

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

//#include "Geo/ROOTGeomAnalyzer.h"
//#include "Geo/GeomVolSelectorFiducial.h"
//#include "Geo/GeomVolSelectorRockBox.h"
//#include "Utils/StringUtils.h"
//#include "Utils/XmlParserUtils.h"
  #include "Interaction/InitialState.h"
  #include "Interaction/Interaction.h"
  #include "Interaction/Kinematics.h"
  #include "Interaction/KPhaseSpace.h"
  #include "Interaction/ProcessInfo.h"
  #include "Interaction/XclsTag.h"
  #include "GHEP/GHepParticle.h"
  #include "PDG/PDGCodeList.h"
  #include "Conventions/Constants.h" //for calculating event kinematics

#else
  // GENIE includes R-3 and beyond
  #include "GENIE/Framework/Messenger/Messenger.h"
  #include "GENIE/Framework/Conventions/Units.h"
  #include "GENIE/Framework/Conventions/Constants.h"
  #include "GENIE/Framework/GHEP/GHepUtils.h"
  #include "GENIE/Framework/EventGen/EventRecord.h"

//  #include "GENIE/Framework/Interaction/InitialState.h"
//  #include "GENIE/Framework/Interaction/Interaction.h"
//  #include "GENIE/Framework/Interaction/Kinematics.h"
//  #include "GENIE/Framework/Interaction/KPhaseSpace.h"
//  #include "GENIE/Framework/Interaction/ProcessInfo.h"
//  #include "GENIE/Framework/Interaction/XclsTag.h"

//  #include "GENIE/Framework/ParticleData/PDGCodes.h"
//  #include "GENIE/Framework/ParticleData/PDGCodeList.h"
//  #include "GENIE/Framework/ParticleData/PDGLibrary.h"
//  #include "GENIE/Framework/GHEP/GHepUtils.h"

  #include "GENIE/Framework/GHEP/GHepParticle.h"

  #include "RwFramework/GReWeightI.h"
  #include "RwFramework/GSystSet.h"
  #include "RwFramework/GSyst.h"
  #include "RwFramework/GReWeight.h"
  #include "RwFramework/GSystUncertainty.h"
  #include "RwCalculators/GReWeightNuXSecNCEL.h"
  #include "RwCalculators/GReWeightNuXSecCCQE.h"
  #include "RwCalculators/GReWeightNuXSecCCRES.h"
  #include "RwCalculators/GReWeightNuXSecCOH.h"
  #include "RwCalculators/GReWeightNonResonanceBkg.h"
  #include "RwCalculators/GReWeightFGM.h"
  #include "RwCalculators/GReWeightDISNuclMod.h"
  #include "RwCalculators/GReWeightResonanceDecay.h"
  #include "RwCalculators/GReWeightFZone.h"
  #include "RwCalculators/GReWeightINuke.h"
  #include "RwCalculators/GReWeightAGKY.h"
  #include "RwCalculators/GReWeightNuXSecCCQEvec.h"
  #include "RwCalculators/GReWeightNuXSecNCRES.h"
  #include "RwCalculators/GReWeightNuXSecDIS.h"
  #include "RwCalculators/GReWeightNuXSecNC.h"
  #include "RwCalculators/GReWeightUtils.h"

#endif

//NuTools includes
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCNeutrino.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nutools/NuReweight/art/NuReweight.h"

namespace rwgt {

  ///<constructor
  NuReweight::NuReweight() {
    //mf::LogVerbatim("GENIEReweight") << "Create GENIEReweight object";
  }

  ///<destructor
  NuReweight::~NuReweight() {
    // Don't delete fWcalc here. The GENIEReweight parent class handles it.
  }

  double NuReweight::CalcWeight(const simb::MCTruth & truth, const simb::GTruth & gtruth) const {
    genie::EventRecord evr = this->RetrieveGHEP(truth, gtruth);
    double wgt = this->CalculateWeight(evr);
    //mf::LogVerbatim("GENIEReweight") << "New Event Weight is: " << wgt;
    return wgt;
  }

  genie::EventRecord NuReweight::RetrieveGHEP(const simb::MCTruth & truth, const simb::GTruth & gtruth) const {
    genie::EventRecord newEvent;
    newEvent.SetWeight(gtruth.fweight);
    newEvent.SetProbability(gtruth.fprobability);
    newEvent.SetXSec(gtruth.fXsec);

    genie::KinePhaseSpace_t space
      = (genie::KinePhaseSpace_t)gtruth.fGPhaseSpace;
    newEvent.SetDiffXSec(gtruth.fDiffXsec,space);

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

    p_ginstate->SetPdgs(gtruth.ftgtPDG, gtruth.fProbePDG);

    int targetNucleon = nu.HitNuc();
    int struckQuark = nu.HitQuark();

    genie::Target* target123 = p_ginstate->TgtPtr();
    target123->SetHitNucPdg(targetNucleon);
    target123->SetHitQrkPdg(struckQuark);
    target123->SetHitSeaQrk(gtruth.fIsSeaQuark);

    if(newEvent.HitNucleonPosition()> 0) {
      genie::GHepParticle * hitnucleon = newEvent.HitNucleon();
      std::unique_ptr<TLorentzVector> p4hitnucleon(hitnucleon->GetP4());
      target123->SetHitNucP4(*p4hitnucleon);
    }
    else {
      TLorentzVector dummy(0.,0.,0.,0.);
      target123->SetHitNucP4(dummy);
    }

    genie::GHepParticle * probe = newEvent.Probe();
    std::unique_ptr<TLorentzVector> p4probe(probe->GetP4());
    p_ginstate->SetProbeP4(*p4probe);
    if(newEvent.TargetNucleusPosition()> 0) {
      genie::GHepParticle * target = newEvent.TargetNucleus();
      std::unique_ptr<TLorentzVector> p4target(target->GetP4());
      p_ginstate->SetTgtP4(*p4target);
    }  else {
      TLorentzVector dummy(0.,0.,0.,0.);
      p_ginstate->SetTgtP4(dummy);
    }
    p_gint->SetProcInfo(proc_info);
    p_gint->SetKine(gkin);
    p_gint->SetExclTag(gxt);
    newEvent.AttachSummary(p_gint);

    /*
    //For temporary debugging purposes
    genie::Interaction *inter = newEvent.Summary();
    const genie::InitialState &initState  = inter->InitState();
    const genie::Target &tgt = initState.Tgt();

    std::cout << "TargetPDG as Recorded: " << gtruth.ftgtPDG << std::endl;
    std::cout << "TargetZ as Recorded:   " << gtruth.ftgtZ << std::endl;
    std::cout << "TargetA as Recorded:   " << gtruth.ftgtA << std::endl;
    std::cout << "TargetPDG as Recreated: " << tgt.Pdg() << std::endl;
    std::cout << "TargetZ as Recreated: " << tgt.Z() << std::endl;
    std::cout << "TargetA as Recreated: " << tgt.A() << std::endl;
    */

    return newEvent;

  }

}
