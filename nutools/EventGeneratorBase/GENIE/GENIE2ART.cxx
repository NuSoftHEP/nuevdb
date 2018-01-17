////////////////////////////////////////////////////////////////////////
/// \file  GENIE2ART.cxx
/// \brief Functions for transforming GENIE objects into ART objects (and back)
///
/// \version $Id: GENIE2ART.cxx,v 1.0 2016-04-20 18:42:01 rhatcher Exp $
/// \author  rhatcher@fnal.gov
///   Parts taken from GENIEHelper & NuReweight classes
///
////////////////////////////////////////////////////////////////////////

#include "GENIE2ART.h"
//#include <math.h>
//#include <map>
//#include <fstream>
#include <memory>   // for unique_ptr

// ROOT includes
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TSystem.h"

//GENIE includes
#include "Conventions/Units.h"
#include "EVGCore/EventRecord.h"
#include "GHEP/GHepUtils.h"
#include "PDG/PDGCodes.h"
#include "PDG/PDGLibrary.h"

#include "Interaction/InitialState.h"
#include "Interaction/Interaction.h"
#include "Interaction/Kinematics.h"
#include "Interaction/KPhaseSpace.h"
#include "Interaction/ProcessInfo.h"
#include "Interaction/XclsTag.h"
#include "GHEP/GHepParticle.h"
#include "PDG/PDGCodeList.h"
#include "Conventions/Constants.h" //for calculating event kinematics

// GENIE
#include "EVGDrivers/GFluxI.h"
#include "FluxDrivers/GFluxBlender.h"
#include "FluxDrivers/GNuMIFlux.h"
#include "FluxDrivers/GSimpleNtpFlux.h"

#ifndef ART_V1
  #include "nusimdata/SimulationBase/MCTruth.h"
  #include "nusimdata/SimulationBase/MCParticle.h"
  #include "nusimdata/SimulationBase/MCNeutrino.h"
  #include "nusimdata/SimulationBase/GTruth.h"
  #include "nusimdata/SimulationBase/MCFlux.h"
#else
  #include "SimulationBase/MCTruth.h"
  #include "SimulationBase/MCParticle.h"
  #include "SimulationBase/MCNeutrino.h"
  #include "SimulationBase/GTruth.h"
  #include "SimulationBase/MCFlux.h"
#endif

// dk2nu
#include "dk2nu/tree/dk2nu.h"
#include "dk2nu/tree/NuChoice.h"
#include "dk2nu/tree/dkmeta.h"
#include "dk2nu/genie/GDk2NuFlux.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

//---------------------------------------------------------------------------
//choose a spill time (ns) to shift the vertex times by:
// double spillTime = fGlobalTimeOffset + 
//    fHelperRandom->Uniform()*fRandomTimeOffset;

void evgb::FillMCTruth(const genie::EventRecord *record, double spillTime,
                       simb::MCTruth &truth)
{   
  TLorentzVector *vertex = record->Vertex();
  
  // get the Interaction object from the record - this is the object
  // that talks to the event information objects and is in m
  const genie::Interaction *inter = record->Summary();
  
  // get the different components making up the interaction
  const genie::InitialState &initState  = inter->InitState();
  const genie::ProcessInfo  &procInfo   = inter->ProcInfo();

  //const genie::Kinematics   &kine       = inter->Kine();
  //const genie::XclsTag      &exclTag    = inter->ExclTag();
  //const genie::KPhaseSpace  &phaseSpace = inter->PhaseSpace();
  
  // add the particles from the interaction
  TIter partitr(record);
  genie::GHepParticle *part = 0;
  // GHepParticles return units of GeV/c for p.  the V_i are all in fermis
  // and are relative to the center of the struck nucleus.
  // add the vertex X/Y/Z to the V_i for status codes 0 and 1
  int trackid = 0;
  std::string primary("primary");
  
  while( (part = dynamic_cast<genie::GHepParticle *>(partitr.Next())) ){
    
    simb::MCParticle tpart(trackid, 
                           part->Pdg(),
                           primary,
                           part->FirstMother(),
                           part->Mass(), 
                           part->Status());
    double vtx[4] = {part->Vx(), part->Vy(), part->Vz(), part->Vt()};
    /*
    if ( vtx[0] == 0 && 
         vtx[1] == 0 &&
         vtx[2] == 0 &&
         vtx[3] == 0 ) {
      //mf::LogInfo("GENIEHelper") << "tweak vtx[3] for MCParticle";
      vtx[3] = 1.0e-30;
    }
    */
    tpart.SetGvtx(vtx);
    tpart.SetRescatter(part->RescatterCode());
    
    // set the vertex location for the neutrino, nucleus and everything
    // that is to be tracked.  vertex returns values in meters.
    if (part->Status() == 0 || part->Status() == 1){
      vtx[0] = 100.*(part->Vx()*1.e-15 + vertex->X());
      vtx[1] = 100.*(part->Vy()*1.e-15 + vertex->Y());
      vtx[2] = 100.*(part->Vz()*1.e-15 + vertex->Z());
      vtx[3] = part->Vt() + spillTime;
    }
    TLorentzVector pos(vtx[0], vtx[1], vtx[2], vtx[3]);
    TLorentzVector mom(part->Px(), part->Py(), part->Pz(), part->E());
    tpart.AddTrajectoryPoint(pos,mom);
    if (part->PolzIsSet()) {
      TVector3 polz;
      part->GetPolarization(polz);
      tpart.SetPolarization(polz);
    }
    truth.Add(tpart);
    
    ++trackid;        
  }// end loop to convert GHepParticles to MCParticles
  
  // is the interaction NC or CC
  int CCNC = simb::kCC;
  if (procInfo.IsWeakNC()) CCNC = simb::kNC;
  
  // what is the interaction type
  int mode = simb::kUnknownInteraction;
  
  if     (procInfo.IsQuasiElastic()       ) mode = simb::kQE;
  else if (procInfo.IsDeepInelastic()      ) mode = simb::kDIS;
  else if (procInfo.IsResonant()           ) mode = simb::kRes;
  else if (procInfo.IsCoherent()           ) mode = simb::kCoh;
  else if (procInfo.IsCoherentElas()       ) mode = simb::kCohElastic;
  else if (procInfo.IsElectronScattering() ) mode = simb::kElectronScattering;
  else if (procInfo.IsNuElectronElastic()  ) mode = simb::kNuElectronElastic;
  else if (procInfo.IsInverseMuDecay()     ) mode = simb::kInverseMuDecay;
  else if (procInfo.IsIMDAnnihilation()    ) mode = simb::kIMDAnnihilation;
  else if (procInfo.IsInverseBetaDecay()   ) mode = simb::kInverseBetaDecay;
  else if (procInfo.IsGlashowResonance()   ) mode = simb::kGlashowResonance;
  else if (procInfo.IsAMNuGamma()          ) mode = simb::kAMNuGamma;
  else if (procInfo.IsMEC()                ) mode = simb::kMEC;
  else if (procInfo.IsDiffractive()        ) mode = simb::kDiffractive;
  else if (procInfo.IsEM()                 ) mode = simb::kEM;
  else if (procInfo.IsWeakMix()            ) mode = simb::kWeakMix;
  
  int itype = simb::kNuanceOffset + genie::utils::ghep::NuanceReactionCode(record);
  
  // set the neutrino information in MCTruth
  truth.SetOrigin(simb::kBeamNeutrino);
  
#ifdef OLD_KINE_CALC
  // The genie event kinematics are subtle different from the event 
  // kinematics that a experimentalist would calculate
  // Instead of retriving the genie values for these kinematic variables 
  // calcuate them from the the final state particles 
  // while ingnoring the fermi momentum and the off-shellness of the bound nucleon.
  genie::GHepParticle * hitnucl = record->HitNucleon();
  TLorentzVector pdummy(0, 0, 0, 0);
  // these don't exist if it came from nucleon decay ..   RWH
  const TLorentzVector v4_null;
  genie::GHepParticle* probe = record->Probe();
  genie::GHepParticle* finallepton = record->FinalStatePrimaryLepton();
  const TLorentzVector & k1 = ( probe ? *(probe->P4()) : v4_null );
  const TLorentzVector & k2 = ( finallepton ? *(finallepton->P4()) : v4_null );
  //const TLorentzVector & p1 = (hitnucl) ? *(hitnucl->P4()) : pdummy;
  
  double M  = genie::constants::kNucleonMass; 
  TLorentzVector q  = k1-k2;                     // q=k1-k2, 4-p transfer
  double Q2 = -1 * q.M2();                       // momemtum transfer
  double v  = (hitnucl) ? q.Energy()       : -1; // v (E transfer to the nucleus)
  double x  = (hitnucl) ? 0.5*Q2/(M*v)     : -1; // Bjorken x
  double y  = (hitnucl) ? v/k1.Energy()    : -1; // Inelasticity, y = q*P1/k1*P1
  double W2 = (hitnucl) ? M*M + 2*M*v - Q2 : -1; // Hadronic Invariant mass ^ 2
  double W  = (hitnucl) ? std::sqrt(W2)    : -1;
#else
  // The internal GENIE event kinematics are subtly different from the event
  // kinematics that a experimentalist would calculate.
  // Instead of retriving the genie values for these kinematic variables ,
  // calculate them from the the final state particles
  // while ignoring the Fermi momentum and the off-shellness of the bound nucleon.
  // (same strategy as in gNtpConv.cxx::ConvertToGST().)
  genie::GHepParticle * hitnucl = record->HitNucleon();
  const TLorentzVector & k1 = *((record->Probe())->P4());
  const TLorentzVector & k2 = *((record->FinalStatePrimaryLepton())->P4());
  
  // also note that since most of these variables are calculated purely from the leptonic system,
  // they have meaning reactions that didn't strike a nucleon (or even a hadron) as well.
  TLorentzVector q  = k1-k2;      // q=k1-k2, 4-p transfer
  
  double Q2 = -1 * q.M2();        // momemtum transfer
  double v  = q.Energy();         // v (E transfer to the had system)
  double y  = v/k1.Energy();      // Inelasticity, y = q*P1/k1*P1
  double x, W2, W;
  x = W2 = W = -1;
  
  if ( hitnucl || procInfo.IsCoherent() ) {
    const double M  = genie::constants::kNucleonMass;
    // Bjorken x.  
    // Rein & Sehgal use this same formulation of x even for Coherent
    x  = 0.5*Q2/(M*v);
    // Hadronic Invariant mass ^ 2.  
    // ("wrong" for Coherent, but it's "experimental", so ok?)
    W2 = M*M + 2*M*v - Q2;   
    W  = std::sqrt(W2);
  }
#endif

  truth.SetNeutrino(CCNC,
                    mode,
                    itype,
                    initState.Tgt().Pdg(),
                    initState.Tgt().HitNucPdg(),
                    initState.Tgt().HitQrkPdg(),
                    W,
                    x,
                    y,
                    Q2);
  return;
}

//---------------------------------------------------------------------------
void evgb::FillGTruth(const genie::EventRecord* record, 
                      simb::GTruth& truth) {
    
  //interactions info
  genie::Interaction *inter = record->Summary();
  const genie::ProcessInfo  &procInfo = inter->ProcInfo();
  truth.fGint = (int)procInfo.InteractionTypeId(); 
  truth.fGscatter = (int)procInfo.ScatteringTypeId(); 
  
  //Event info
  truth.fweight = record->Weight(); 
  truth.fprobability = record->Probability(); 
  truth.fXsec = record->XSec(); 
  truth.fDiffXsec = record->DiffXSec(); 
  // needs change to GTruth
  // truth.fPhaseSpace (type KinePhaseSpace_t) = record->DiffXSecVars();

  TLorentzVector vtx;
  TLorentzVector *erVtx = record->Vertex();
  vtx.SetXYZT(erVtx->X(), erVtx->Y(), erVtx->Z(), erVtx->T() );
  truth.fVertex = vtx;
  
  //true reaction information and byproducts
  //(PRE FSI)
  const genie::XclsTag &exclTag = inter->ExclTag();
  truth.fIsCharm = exclTag.IsCharmEvent();   
  truth.fResNum = (int)exclTag.Resonance();

  //note that in principle this information could come from the XclsTag,
  //but that object isn't completely filled for most reactions,
  //at least for GENIE <= 2.12
//    truth.fNumPiPlus = exclTag.NPiPlus();
//    truth.fNumPiMinus = exclTag.NPiMinus();
//    truth.fNumPi0 = exclTag.NPi0();
//    truth.fNumProton = exclTag.NProtons();
//    truth.fNumNeutron = exclTag.NNucleons();
  truth.fNumPiPlus = truth.fNumPiMinus = truth.fNumPi0 = truth.fNumProton = truth.fNumNeutron = 0;
  for (int idx = 0; idx < record->GetEntries(); idx++)
  {
    // want hadrons that are about to be sent to the FSI model
    const genie::GHepParticle * particle = record->Particle(idx);
    if (particle->Status() != genie::kIStHadronInTheNucleus)
      continue;

    int pdg = particle->Pdg();
    if (pdg == genie::kPdgPi0)
      truth.fNumPi0++;
    else if (pdg == genie::kPdgPiP)
      truth.fNumPiPlus++;
    else if (pdg == genie::kPdgPiM)
      truth.fNumPiMinus++;
    else if (pdg == genie::kPdgNeutron)
      truth.fNumNeutron++;
    else if (pdg == genie::kPdgProton)
      truth.fNumProton++;
  } // for (idx)
  
  // Get the GENIE kinematics info 
  const genie::Kinematics &kine = inter->Kine();  
  // RWH: really should be looping of GENIE Conventions/KineVar_t enum
  // and only recording/resetting those that were originally there ...
  truth.fgQ2 = kine.Q2(true);
  truth.fgq2 = kine.q2(true);
  truth.fgW = kine.W(true);
  if ( kine.KVSet(genie::kKVSelt) ) {
    // only get this if it is set in the Kinematics class
    // to avoid a warning message
    truth.fgT = kine.t(true);
  }
  truth.fgX = kine.x(true);
  truth.fgY = kine.y(true);
    
  /*
    truth.fgQ2 = kine.Q2(false); 
    truth.fgW = kine.W(false);
    truth.fgT = kine.t(false);
    truth.fgX = kine.x(false);
    truth.fgY = kine.y(false);
  */
  truth.fFShadSystP4 = kine.HadSystP4();
  
  //Initial State info
  const genie::InitialState &initState  = inter->InitState();
  truth.fProbePDG = initState.ProbePdg();
  truth.fProbeP4 = *initState.GetProbeP4();
  
  //Target info
  const genie::Target &tgt = initState.Tgt();
  truth.fIsSeaQuark = tgt.HitSeaQrk();
  truth.fHitNucP4 = tgt.HitNucP4();
  truth.ftgtZ = tgt.Z();
  truth.ftgtA = tgt.A();
  truth.ftgtPDG = tgt.Pdg();
  
  return;
  
}

//---------------------------------------------------------------------------
genie::EventRecord* evgb::RetrieveGHEP(const simb::MCTruth& mctruth, 
                                       const simb::GTruth&  gtruth,
                                       bool useFirstTrajPosition)
{
  genie::EventRecord* newEvent = new genie::EventRecord;
  
  newEvent->SetWeight(gtruth.fweight);
  newEvent->SetProbability(gtruth.fprobability);
  newEvent->SetXSec(gtruth.fXsec);
#ifndef SETDIFFXSEC_1ARG
  genie::KinePhaseSpace_t space = genie::kPSNull; // kPSQ2fE; // ????
  // dsig/dQ2, dsig/dQ2dW, dsig/dxdy ...
  
  // needs change to GTruth
  // space = truth.fPhaseSpace (type KinePhaseSpace_t)

  newEvent->SetDiffXSec(gtruth.fDiffXsec,space);
  
  // TODO:  we don't know currently know what to use ... 
  // for now just to get things to compile ... Nate needs to look at this
  static int nmsg = 2;
  if ( nmsg > 0 ) {
    --nmsg;
    std::string andOut = "";
    if ( nmsg == 0 ) andOut = "... last of such messages";
    mf::LogWarning("GENIE2ART")
      << "RetrieveGHEP(simb::MCTruth,simb::GTruth) is not correctly setting KinePhaseSpace_t in SetDiffXSec()\n"
      << "At the time of the conversion to R-2_8_0 (2013-05-01) this is not critical\n"
      << "But it should be fixed" << std::endl
      << andOut << std::endl;
  }
#else
  newEvent->SetDiffXSec(gtruth.fDiffXsec);
#endif
  TLorentzVector vtx = gtruth.fVertex;
  newEvent->SetVertex(vtx);
  
  //mf::LogWarning("GENIE2ART")
  //  << "####### mctruth.NParticles() " << mctruth.NParticles();

  for (int i = 0; i < mctruth.NParticles(); i++) {
    simb::MCParticle mcpart = mctruth.GetParticle(i);
    
    int gmid = mcpart.PdgCode();
    genie::GHepStatus_t gmst = (genie::GHepStatus_t)mcpart.StatusCode();
    int gmmo = mcpart.Mother();
    int gmfd = -1;
    int gmld = -1;

    /* 
       // GENIE will update daughter references as particles are added
       // without a need to jump through these hoops ... which gets
       // it wrong anyway (always sets 0th particles daughter to 
       // mctruth.NParticles()-1, and leave the others at -1 (which then
       // GENIE handles correctly ...

    int ndaughters = mcpart.NumberDaughters();
    //find the track ID of the first and last daughter particles
    int fdtrkid = 0;
    int ldtrkid = 0;
    if (ndaughters !=0) {
      fdtrkid = mcpart.Daughter(0);
      if (ndaughters == 1) {
        ldtrkid = 1;
      }
      else if (ndaughters >1) {
        fdtrkid = mcpart.Daughter(ndaughters-1);
      }
    }

    // Genie uses the index in the particle array to reference 
    // the daughter particles.
    // MCTruth keeps the particles in the same order so use the
    // track ID to find the proper index.
    for (int j = 0; j < mctruth.NParticles(); j++) {
      simb::MCParticle temp = mctruth.GetParticle(i);
      if (temp.TrackId() == fdtrkid) {
        gmfd = j;
      }
      if (temp.TrackId() == ldtrkid) {
        gmld = j;
      }
    }
    */

    double gmpx = mcpart.Px(0);
    double gmpy = mcpart.Py(0);
    double gmpz = mcpart.Pz(0);
    double gme  = mcpart.E(0);

    double gmvx = mcpart.Gvx();
    double gmvy = mcpart.Gvy();
    double gmvz = mcpart.Gvz();
    double gmvt = mcpart.Gvt();
    bool GvtxFunky = false;
    if ( gmvx == 0 && gmvy == 0 && 
         gmvz == 0 && gmvt == 0 ) GvtxFunky = true;
    if ( gmvx == simb::GTruth::kUndefinedValue && 
         gmvy == simb::GTruth::kUndefinedValue && 
         gmvz == simb::GTruth::kUndefinedValue &&
         gmvt == simb::GTruth::kUndefinedValue    ) GvtxFunky = true;
    if (GvtxFunky) {
      static int nmsg = 0;  // don't warn about this for now
      if (nmsg > 0) {
        --nmsg;
        std::string andOut = "";
        if (nmsg == 0 ) andOut = "... last of such messages";
        mf::LogWarning("GENIE2ART")
          << "RetrieveGHEP(simb::MCTruth,simb::Gtruth) ... Gv[xyzt] all "
          << gmvx << " for index " << i
          << "; probably not filled ..."
          << andOut << std::endl;
      }
      // MCParticle Vx(), Vy(), Vz() implicitly are  index=0
      // put it's likely we want the _last_ position ...
      const TLorentzVector mcpartTrjPos = 
        ( useFirstTrajPosition ) ? mcpart.Position() : // default indx=0
                                   mcpart.EndPosition();
      unsigned int nTrj = mcpart.NumberTrajectoryPoints();
      if ( nTrj < 1 ) // || nTrj > 1 )
        mf::LogWarning("GENIE2ART") << "############### nTrj = " << nTrj;

      // set the vertex location for the neutrino, nucleus and everything
      // that is to be tracked.  vertex returns values in meters.
      if ( mcpart.StatusCode() == 0 || mcpart.StatusCode() == 1 ){
        // inverse of this....
        // mcpartvtx[0] = 100.*(gpart->Vx()*1.e-15 + vertex->X());
        // mcpartvtx[1] = 100.*(gpart->Vy()*1.e-15 + vertex->Y());
        // mcpartvtx[2] = 100.*(gpart->Vz()*1.e-15 + vertex->Z());
        // mcpartvtx[3] = gpart->Vt() + spillTime;
        // local "vtx" is equiv to "vertex"  ; solve for (genie)part->V
        gmvx = 1.e15*((mcpartTrjPos.X()*1.e-2) - vtx.X());
        gmvy = 1.e15*((mcpartTrjPos.Y()*1.e-2) - vtx.Y());
        gmvz = 1.e15*((mcpartTrjPos.Z()*1.e-2) - vtx.Z());
        gmvt = mcpartTrjPos.T() - vtx.T();
      } else {
        gmvx = mcpartTrjPos.X();
        gmvy = mcpartTrjPos.Y();
        gmvz = mcpartTrjPos.Z();
        gmvt = mcpartTrjPos.T();
      }

    } // if Gv[xyzt] seem unset

    int gmri = mcpart.Rescatter();
    
    genie::GHepParticle gpart(gmid, gmst, gmmo, -1, gmfd, gmld, 
                              gmpx, gmpy, gmpz, gme, gmvx, gmvy, gmvz, gmvt);
    gpart.SetRescatterCode(gmri);
    TVector3 polz = mcpart.Polarization();
    if (polz.x() !=0 || polz.y() !=0 || polz.z() !=0) {
      gpart.SetPolarization(polz);
    }
    newEvent->AddParticle(gpart);
  }

  genie::ProcessInfo proc_info;
  genie::ScatteringType_t  gscty = (genie::ScatteringType_t)gtruth.fGscatter;
  genie::InteractionType_t ginty = (genie::InteractionType_t)gtruth.fGint;
  
  proc_info.Set(gscty,ginty);
  
  genie::XclsTag gxt;
  
  //Set Exclusive Final State particle numbers
  genie::Resonance_t gres = (genie::Resonance_t)gtruth.fResNum;
  gxt.SetResonance(gres);
  gxt.SetNPions(gtruth.fNumPiPlus, gtruth.fNumPi0, gtruth.fNumPiMinus);
  gxt.SetNNucleons(gtruth.fNumProton, gtruth.fNumNeutron);
  
  if (gtruth.fIsCharm) {
    gxt.SetCharm(0);
  }
  else {
    gxt.UnsetCharm();
  }
  
  // Set the GENIE kinematics info
  genie::Kinematics gkin;
  // RWH: really should be looping of GENIE Conventions/KineVar_t enum
  // and only recording/resetting those that were originally there ...
  const double flagVal = -99999;
  if ( gtruth.fgX  != flagVal) gkin.Setx(gtruth.fgX, true);
  if ( gtruth.fgY  != flagVal) gkin.Sety(gtruth.fgY, true);
  if ( gtruth.fgT  != flagVal) gkin.Sett(gtruth.fgT, true);
  if ( gtruth.fgW  != flagVal) gkin.SetW(gtruth.fgW, true);
  if ( gtruth.fgQ2 != flagVal) gkin.SetQ2(gtruth.fgQ2, true);
  if ( gtruth.fgq2 != flagVal) gkin.Setq2(gtruth.fgq2, true);
  simb::MCNeutrino nu = mctruth.GetNeutrino();
  simb::MCParticle lep = nu.Lepton();
  // is this even real?
  if ( lep.NumberTrajectoryPoints() > 0 ) {
    gkin.SetFSLeptonP4(lep.Px(), lep.Py(), lep.Pz(), lep.E());
  }
  gkin.SetHadSystP4(gtruth.fFShadSystP4.Px(), 
                    gtruth.fFShadSystP4.Py(),
                    gtruth.fFShadSystP4.Pz(),
                    gtruth.fFShadSystP4.E());

  // reordering this to avoid warning (A=0,Z=0) 
  int probe_pdgc = gtruth.fProbePDG;
  int tgtZ       = gtruth.ftgtZ;
  int tgtA       = gtruth.ftgtA;

  //std::cerr << " tgtZ " << tgtZ << " tgtA " << tgtA << " probe " << probe_pdgc << std::endl;

  // genie::InitialState::Init() will fail if target_pdgc or probe_pdgc
  // come back with nothign from PDGLibrary::Instance()->Find()
  // fake it ... (what does nucleon decay do here??)
  if ( tgtZ == 0 || tgtA == 0 ) { tgtZ = tgtA = 1; }  // H1
  if ( probe_pdgc == 0 || probe_pdgc == -1 ) { probe_pdgc = 22; } // gamma

  //std::cerr << " tgtZ " << tgtZ << " tgtA " << tgtA << " probe " << probe_pdgc << std::endl;

  int target_pdgc = genie::pdg::IonPdgCode(tgtA,tgtZ);  

  /*
  TParticlePDG * t = genie::PDGLibrary::Instance()->Find(target_pdgc);
  TParticlePDG * p = genie::PDGLibrary::Instance()->Find(probe_pdgc );

  std::cerr << " target " << target_pdgc << " t " << t << " p " << p << std::endl;
  */

  int targetNucleon = nu.HitNuc();
  int struckQuark   = nu.HitQuark();
  
  //genie::Target tmptgt(gtruth.ftgtZ, gtruth.ftgtA, targetNucleon);
  // this ctor doesn't copy the state of the Target beyond the PDG value!
  // so don't bother creating a tmptgt ...
  //genie::InitialState ginitstate(tmptgt,probe_pdgc);

  genie::InitialState ginitstate(target_pdgc,probe_pdgc);

  // do this here _after_ creating InitialState
  genie::Target* tgtptr = ginitstate.TgtPtr();
  tgtptr->SetHitNucPdg(targetNucleon);
  tgtptr->SetHitQrkPdg(struckQuark);
  tgtptr->SetHitSeaQrk(gtruth.fIsSeaQuark);
  
  if (newEvent->HitNucleonPosition() >= 0) {
    genie::GHepParticle * hitnucleon = newEvent->HitNucleon();
    std::unique_ptr<TLorentzVector> p4hitnucleon(hitnucleon->GetP4());
    tgtptr->SetHitNucP4(*p4hitnucleon);
  } else {
    if ( targetNucleon != 0 ) {
      mf::LogWarning("GENIE2ART")
        << "evgb::RetrieveGHEP() no hit nucleon position " 
        << " but targetNucleon is " << targetNucleon
        << " at " << __FILE__ << ":" << __LINE__
        << std::endl << std::flush;
    }
    TLorentzVector dummy(0.,0.,0.,0.);
    tgtptr->SetHitNucP4(dummy);
  }

  if (newEvent->TargetNucleusPosition() >= 0) {
    genie::GHepParticle * target = newEvent->TargetNucleus();
    std::unique_ptr<TLorentzVector> p4target(target->GetP4());
    ginitstate.SetTgtP4(*p4target);
  } else {
    double Erest = 0.;
    if ( gtruth.ftgtPDG != 0 ) {
      TParticlePDG* ptmp = genie::PDGLibrary::Instance()->Find(gtruth.ftgtPDG);
      if ( ptmp ) Erest = ptmp->Mass();
    } else {
      mf::LogWarning("GENIE2ART")
        << "evgb::RetrieveGHEP() no target nucleus position " 
        << " but gtruth.ftgtPDG is " << gtruth.ftgtPDG
        << " at " << __FILE__ << ":" << __LINE__
        << std::endl << std::flush;
    }
    TLorentzVector dummy(0.,0.,0.,Erest);
    ginitstate.SetTgtP4(dummy);
  }
  
  genie::GHepParticle * probe = newEvent->Probe();
  if ( probe ) {
    std::unique_ptr<TLorentzVector> p4probe(probe->GetP4());
    ginitstate.SetProbeP4(*p4probe);
  } else {
    // this can happen ...
    mf::LogDebug("GENIE2ART")
      << "evgb::RetrieveGHEP() no probe "
      << " at " << __FILE__ << ":" << __LINE__
      << std::endl << std::flush;
    TLorentzVector dummy(0.,0.,0.,0.);
    ginitstate.SetProbeP4(dummy);
  }
  
  genie::Interaction * p_gint = new genie::Interaction(ginitstate,proc_info);
  
  p_gint->SetProcInfo(proc_info);
  p_gint->SetKine(gkin);
  p_gint->SetExclTag(gxt);
  newEvent->AttachSummary(p_gint);
  
  /*
  //For temporary debugging purposes
  genie::Interaction *inter = newEvent->Summary();
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

//---------------------------------------------------------------------------
void evgb::FillMCFlux(genie::GFluxI* fdriver, simb::MCFlux& mcflux)
{
  // is the real driver hidden behind a blender?
  genie::flux::GFluxBlender* gblender =
    dynamic_cast<genie::flux::GFluxBlender *>(fdriver);
  if ( gblender ) {
    // it is, it is ... proceed with that ...
    fdriver = gblender->GetFluxGenerator();
  }
  
  genie::flux::GNuMIFlux* gnumi = 
    dynamic_cast<genie::flux::GNuMIFlux *>(fdriver);
  if ( gnumi ) {
    FillMCFlux(gnumi,mcflux);
    return;
  } else {
    genie::flux::GSimpleNtpFlux* gsimple = 
      dynamic_cast<genie::flux::GSimpleNtpFlux *>(fdriver);
    if ( gsimple ) {
      FillMCFlux(gsimple,mcflux);
      return;
    } else {
      genie::flux::GDk2NuFlux* gdk2nu = 
        dynamic_cast<genie::flux::GDk2NuFlux *>(fdriver);
      if ( gdk2nu ) {
        FillMCFlux(gdk2nu,mcflux);
        return;
      } else {
        mf::LogProblem("GENIE2ART")
          << "no FillMCFlux for this flux driver " 
          ///<< fdriver->GetClass()->GetName()
          << std::endl;
        abort();
      }
    }
  }
}

//---------------------------------------------------------------------------

void evgb::FillMCFlux(genie::flux::GNuMIFlux* gnumi,
                      simb::MCFlux& flux)
{
  const genie::flux::GNuMIFluxPassThroughInfo& numiflux = 
    gnumi->PassThroughInfo();
  const genie::flux::GNuMIFluxPassThroughInfo* nflux = &numiflux;
  double dk2gen = gnumi->GetDecayDist();
  evgb::FillMCFlux(nflux,dk2gen,flux);
}
void evgb::FillMCFlux(const genie::flux::GNuMIFluxPassThroughInfo* nflux,
                      double dk2gen,
                      simb::MCFlux& flux)
{
  flux.Reset();
  flux.fFluxType = simb::kNtuple;

  // check the particle codes and the units passed through
  //  nflux->pcodes: 0=original GEANT particle codes, 1=converted to PDG
  //  nflux->units:  0=original GEANT cm, 1=meters
  if (nflux->pcodes != 1 && nflux->units != 0) {
    mf::LogError("FillMCFlux") 
      << "either wrong particle codes or units "
      << "from flux object - beware!!";
  }  

  // maintained variable names from gnumi ntuples
  // see http://www.hep.utexas.edu/~zarko/wwwgnumi/v19/[/v19/output_gnumi.html]
  
  flux.frun      = nflux->run;
  flux.fevtno    = nflux->evtno;
  flux.fndxdz    = nflux->ndxdz;
  flux.fndydz    = nflux->ndydz;
  flux.fnpz      = nflux->npz;
  flux.fnenergy  = nflux->nenergy;
  flux.fndxdznea = nflux->ndxdznea;
  flux.fndydznea = nflux->ndydznea;
  flux.fnenergyn = nflux->nenergyn;
  flux.fnwtnear  = nflux->nwtnear;
  flux.fndxdzfar = nflux->ndxdzfar;
  flux.fndydzfar = nflux->ndydzfar;
  flux.fnenergyf = nflux->nenergyf;
  flux.fnwtfar   = nflux->nwtfar;
  flux.fnorig    = nflux->norig;
  flux.fndecay   = nflux->ndecay;
  flux.fntype    = nflux->ntype;
  flux.fvx       = nflux->vx;
  flux.fvy       = nflux->vy;
  flux.fvz       = nflux->vz;
  flux.fpdpx     = nflux->pdpx;
  flux.fpdpy     = nflux->pdpy;
  flux.fpdpz     = nflux->pdpz;
  flux.fppdxdz   = nflux->ppdxdz;
  flux.fppdydz   = nflux->ppdydz;
  flux.fpppz     = nflux->pppz;
  flux.fppenergy = nflux->ppenergy;
  flux.fppmedium = nflux->ppmedium;
  flux.fptype    = nflux->ptype;     // converted to PDG
  flux.fppvx     = nflux->ppvx;
  flux.fppvy     = nflux->ppvy;
  flux.fppvz     = nflux->ppvz;
  flux.fmuparpx  = nflux->muparpx;
  flux.fmuparpy  = nflux->muparpy;
  flux.fmuparpz  = nflux->muparpz;
  flux.fmupare   = nflux->mupare;
  flux.fnecm     = nflux->necm;
  flux.fnimpwt   = nflux->nimpwt;
  flux.fxpoint   = nflux->xpoint;
  flux.fypoint   = nflux->ypoint;
  flux.fzpoint   = nflux->zpoint;
  flux.ftvx      = nflux->tvx;
  flux.ftvy      = nflux->tvy;
  flux.ftvz      = nflux->tvz;
  flux.ftpx      = nflux->tpx;
  flux.ftpy      = nflux->tpy;
  flux.ftpz      = nflux->tpz;
  flux.ftptype   = nflux->tptype;   // converted to PDG
  flux.ftgen     = nflux->tgen;
  flux.ftgptype  = nflux->tgptype;  // converted to PDG
  flux.ftgppx    = nflux->tgppx;
  flux.ftgppy    = nflux->tgppy;
  flux.ftgppz    = nflux->tgppz;
  flux.ftprivx   = nflux->tprivx;
  flux.ftprivy   = nflux->tprivy;
  flux.ftprivz   = nflux->tprivz;
  flux.fbeamx    = nflux->beamx;
  flux.fbeamy    = nflux->beamy;
  flux.fbeamz    = nflux->beamz;
  flux.fbeampx   = nflux->beampx;
  flux.fbeampy   = nflux->beampy;
  flux.fbeampz   = nflux->beampz;    
  
  flux.fdk2gen   = dk2gen;
  
  return;
}

//---------------------------------------------------------------------------
void evgb::FillMCFlux(genie::flux::GSimpleNtpFlux* gsimple,
                      simb::MCFlux& flux)
{
  const genie::flux::GSimpleNtpEntry* nflux_entry =
    gsimple->GetCurrentEntry();
  const genie::flux::GSimpleNtpNuMI*  nflux_numi  =
    gsimple->GetCurrentNuMI();
  const genie::flux::GSimpleNtpAux*   nflux_aux   = 
    gsimple->GetCurrentAux();
  const genie::flux::GSimpleNtpMeta*  nflux_meta  =
    gsimple->GetCurrentMeta();
  evgb::FillMCFlux(nflux_entry, nflux_numi, nflux_aux, nflux_meta, flux);
}
void evgb::FillMCFlux(const genie::flux::GSimpleNtpEntry* nflux_entry,
                      const genie::flux::GSimpleNtpNuMI*  nflux_numi,
                      const genie::flux::GSimpleNtpAux*   nflux_aux,
                      const genie::flux::GSimpleNtpMeta*  nflux_meta,
                      simb::MCFlux& flux)
{
  flux.Reset();
  flux.fFluxType = simb::kSimple_Flux;
  
  // maintained variable names from gnumi ntuples
  // see http://www.hep.utexas.edu/~zarko/wwwgnumi/v19/[/v19/output_gnumi.html]
  
  
  flux.fntype  = nflux_entry->pdg;
  flux.fnimpwt = nflux_entry->wgt;
  flux.fdk2gen = nflux_entry->dist;
  flux.fnenergyn = flux.fnenergyf = nflux_entry->E;
  
  if ( nflux_numi ) {
    flux.frun      = nflux_numi->run;
    flux.fevtno    = nflux_numi->evtno;
    flux.ftpx      = nflux_numi->tpx;
    flux.ftpy      = nflux_numi->tpy;
    flux.ftpz      = nflux_numi->tpz;
    flux.ftptype   = nflux_numi->tptype;   // converted to PDG
    flux.fvx       = nflux_numi->vx;
    flux.fvy       = nflux_numi->vy;
    flux.fvz       = nflux_numi->vz;
    
    flux.fndecay   = nflux_numi->ndecay;
    flux.fppmedium = nflux_numi->ppmedium;
    
    flux.fpdpx     = nflux_numi->pdpx;
    flux.fpdpy     = nflux_numi->pdpy;
    flux.fpdpz     = nflux_numi->pdpz;
    
    double apppz = nflux_numi->pppz;
    if ( TMath::Abs(nflux_numi->pppz) < 1.0e-30 ) apppz = 1.0e-30;
    flux.fppdxdz   = nflux_numi->pppx / apppz;
    flux.fppdydz   = nflux_numi->pppy / apppz;
    flux.fpppz     = nflux_numi->pppz;
    
    flux.fptype    = nflux_numi->ptype;
    
  }
  
  // anything useful stuffed into vdbl or vint?
  // need to check the metadata  auxintname, auxdblname
  
  if ( nflux_aux && nflux_meta ) {
    
    // references just for reducing complexity
    const std::vector<std::string>& auxdblname = nflux_meta->auxdblname;
    const std::vector<std::string>& auxintname = nflux_meta->auxintname;
    const std::vector<int>&    auxint = nflux_aux->auxint;
    const std::vector<double>& auxdbl = nflux_aux->auxdbl;
    
    for (size_t id=0; id<auxdblname.size(); ++id) {
      if ("muparpx"   == auxdblname[id]) flux.fmuparpx  = auxdbl[id];
      if ("muparpy"   == auxdblname[id]) flux.fmuparpy  = auxdbl[id];
      if ("muparpz"   == auxdblname[id]) flux.fmuparpz  = auxdbl[id];
      if ("mupare"    == auxdblname[id]) flux.fmupare   = auxdbl[id];
      if ("necm"      == auxdblname[id]) flux.fnecm     = auxdbl[id];
      if ("nimpwt"    == auxdblname[id]) flux.fnimpwt   = auxdbl[id];
      if ("fgXYWgt"   == auxdblname[id]) {
        flux.fnwtnear = flux.fnwtfar = auxdbl[id]; 
      }
    }
    for (size_t ii=0; ii<auxintname.size(); ++ii) {
      if ("tgen"      == auxintname[ii]) flux.ftgen     = auxint[ii];
      if ("tgptype"   == auxintname[ii]) flux.ftgptype  = auxint[ii];
    }
    
  }
  
//#define RWH_TEST
#ifdef RWH_TEST
  static bool first = true;
  if (first) {
    first = false;
      mf::LogDebug("GENIE2ART")
        << __FILE__ << ":" << __LINE__
        << " one time dump of GSimple objects\n";
    if ( nflux_meta ) {
      mf::LogDebug("GENIE2ART")
        << "evgb::FillMCFlux() GSimpleNtpMeta:\n"
        << *nflux_meta << "\n";
    } else {
      mf::LogDebug("GENIE2ART")
        << "evgb::FillMCFlux() no GSimpleNtpMeta:\n";
    }
  }
  //mf::LogDebug("GENIEHelper")
  mf::LogDebug("GENIE2ART")
    << "simb::MCFlux:\n"
    << flux << "\n"
    << "GSimpleNtpFlux:\n";
  if ( nflux_entry) mf::LogDebug("GENIE2ART") << *nflux_entry << "\n";
  else              mf::LogDebug("GENIE2ART") << "no GSimpleNtpEntry\n";
  if ( nflux_numi ) mf::LogDebug("GENIE2ART") << *nflux_numi << "\n";
  else              mf::LogDebug("GENIE2ART") << "no GSimpleNtpNuMI\n";
  if ( nflux_aux  ) mf::LogDebug("GENIE2ART") << *nflux_aux << "\n";
  else              mf::LogDebug("GENIE2ART") << "no GSimpleNtpAux\n";
#endif
  
  //   flux.fndxdz    = nflux.ndxdz;
  //   flux.fndydz    = nflux.ndydz;
  //   flux.fnpz      = nflux.npz;
  //   flux.fnenergy  = nflux.nenergy;
  //   flux.fndxdznea = nflux.ndxdznea;
  //   flux.fndydznea = nflux.ndydznea;
  //   flux.fnenergyn = nflux.nenergyn;
  //   flux.fnwtnear  = nflux.nwtnear;
  //   flux.fndxdzfar = nflux.ndxdzfar;
  //   flux.fndydzfar = nflux.ndydzfar;
  //   flux.fnenergyf = nflux.nenergyf;
  //   flux.fnwtfar   = nflux.nwtfar;
  //   flux.fnorig    = nflux.norig;
  // in numi //   flux.fndecay   = nflux.ndecay;
  //   flux.fntype    = nflux.ntype;
  // in numi //   flux.fvx       = nflux.vx;
  // in numi //  flux.fvy       = nflux.vy;
  // in numi //  flux.fvz       = nflux.vz;
  //   flux.fppenergy = nflux.ppenergy;
  // in numi //   flux.fppmedium = nflux.ppmedium;
  //   flux.fppvx     = nflux.ppvx;
  //   flux.fppvy     = nflux.ppvy;
  //   flux.fppvz     = nflux.ppvz;
  // see above //   flux.fmuparpx  = nflux.muparpx;
  // see above //   flux.fmuparpy  = nflux.muparpy;
  // see above //   flux.fmuparpz  = nflux.muparpz;
  // see above //   flux.fmupare   = nflux.mupare;
  // see above //   flux.fnecm     = nflux.necm;
  // see above //   flux.fnimpwt   = nflux.nimpwt;
  //   flux.fxpoint   = nflux.xpoint;
  //   flux.fypoint   = nflux.ypoint;
  //   flux.fzpoint   = nflux.zpoint;
  //   flux.ftvx      = nflux.tvx;
  //   flux.ftvy      = nflux.tvy;
  //   flux.ftvz      = nflux.tvz;
  // see above //   flux.ftgen     = nflux.tgen;
  // see above //   flux.ftgptype  = nflux.tgptype;  // converted to PDG
  //   flux.ftgppx    = nflux.tgppx;
  //   flux.ftgppy    = nflux.tgppy;
  //   flux.ftgppz    = nflux.tgppz;
  //   flux.ftprivx   = nflux.tprivx;
  //   flux.ftprivy   = nflux.tprivy;
  //   flux.ftprivz   = nflux.tprivz;
  //   flux.fbeamx    = nflux.beamx;
  //   flux.fbeamy    = nflux.beamy;
  //   flux.fbeamz    = nflux.beamz;
  //   flux.fbeampx   = nflux.beampx;
  //   flux.fbeampy   = nflux.beampy;
  //   flux.fbeampz   = nflux.beampz; 
  
  return;
}

//---------------------------------------------------------------------------
void evgb::FillMCFlux(genie::flux::GDk2NuFlux* gdk2nu,
                      simb::MCFlux& flux)
{
  const bsim::Dk2Nu&    dk2nu    = gdk2nu->GetDk2Nu();
  const bsim::NuChoice& nuchoice = gdk2nu->GetNuChoice();
  evgb::FillMCFlux(&dk2nu,&nuchoice,flux);
}
void evgb::FillMCFlux(const bsim::Dk2Nu* dk2nu,
                      const bsim::NuChoice* nuchoice,
                      simb::MCFlux& flux)
{
  flux.Reset();
  flux.fFluxType = simb::kDk2Nu;  

  flux.fntype  = nuchoice->pdgNu;
  flux.fnimpwt = nuchoice->impWgt;
  ///flux.fdk2gen = nflux_entry->dist;
  flux.fnenergyn = flux.fnenergyf = nuchoice->p4NuUser.E();
  
  if ( dk2nu ) {
      flux.frun      = dk2nu->job;
      flux.fevtno    = dk2nu->potnum;
      flux.ftpx      = dk2nu->tgtexit.tpx;
      flux.ftpy      = dk2nu->tgtexit.tpy;
      flux.ftpz      = dk2nu->tgtexit.tpz;
      flux.ftptype   = dk2nu->tgtexit.tptype;   // converted to PDG
      flux.fvx       = dk2nu->decay.vx;
      flux.fvy       = dk2nu->decay.vy;
      flux.fvz       = dk2nu->decay.vz;
      
      flux.fndecay   = dk2nu->decay.ndecay;
      flux.fppmedium = dk2nu->decay.ppmedium;
      
      flux.fpdpx     = dk2nu->decay.pdpx;
      flux.fpdpy     = dk2nu->decay.pdpy;
      flux.fpdpz     = dk2nu->decay.pdpz;

      flux.fppdxdz   = dk2nu->decay.ppdxdz;
      flux.fppdydz   = dk2nu->decay.ppdydz;
      flux.fpppz     = dk2nu->decay.pppz;

      flux.fptype    = dk2nu->decay.ptype;

    }

  /*    
    // anything useful stuffed into vdbl or vint?
    // need to check the metadata  auxintname, auxdblname

    if ( nflux_aux && nflux_meta ) {

      // references just for reducing complexity
      const std::vector<std::string>& auxdblname = nflux_meta->auxdblname;
      const std::vector<std::string>& auxintname = nflux_meta->auxintname;
      const std::vector<int>&    auxint = nflux_aux->auxint;
      const std::vector<double>& auxdbl = nflux_aux->auxdbl;

      for (size_t id=0; id<auxdblname.size(); ++id) {
        if ("muparpx"   == auxdblname[id]) flux.fmuparpx  = auxdbl[id];
        if ("muparpy"   == auxdblname[id]) flux.fmuparpy  = auxdbl[id];
        if ("muparpz"   == auxdblname[id]) flux.fmuparpz  = auxdbl[id];
        if ("mupare"    == auxdblname[id]) flux.fmupare   = auxdbl[id];
        if ("necm"      == auxdblname[id]) flux.fnecm     = auxdbl[id];
        if ("nimpwt"    == auxdblname[id]) flux.fnimpwt   = auxdbl[id];
        if ("fgXYWgt"   == auxdblname[id]) {
          flux.fnwtnear = flux.fnwtfar = auxdbl[id]; 
        }
      }
      for (size_t ii=0; ii<auxintname.size(); ++ii) {
        if ("tgen"      == auxintname[ii]) flux.ftgen     = auxint[ii];
        if ("tgptype"   == auxintname[ii]) flux.ftgptype  = auxint[ii];
      }

    }

    // probably can get this from vx,vy,vz + NuChoice
    flux.fdk2gen   = gdk2nu->GetDecayDist();

  */

    //   flux.fndxdz    = nflux.ndxdz;
    //   flux.fndydz    = nflux.ndydz;
    //   flux.fnpz      = nflux.npz;
    //   flux.fnenergy  = nflux.nenergy;
    //   flux.fndxdznea = nflux.ndxdznea;
    //   flux.fndydznea = nflux.ndydznea;
    //   flux.fnenergyn = nflux.nenergyn;
    //   flux.fnwtnear  = nflux.nwtnear;
    //   flux.fndxdzfar = nflux.ndxdzfar;
    //   flux.fndydzfar = nflux.ndydzfar;
    //   flux.fnenergyf = nflux.nenergyf;
    //   flux.fnwtfar   = nflux.nwtfar;
    //   flux.fnorig    = nflux.norig;
    // in numi //   flux.fndecay   = nflux.ndecay;
    //   flux.fntype    = nflux.ntype;
    // in numi //   flux.fvx       = nflux.vx;
    // in numi //  flux.fvy       = nflux.vy;
    // in numi //  flux.fvz       = nflux.vz;
    //   flux.fppenergy = nflux.ppenergy;
    // in numi //   flux.fppmedium = nflux.ppmedium;
    //   flux.fppvx     = nflux.ppvx;
    //   flux.fppvy     = nflux.ppvy;
    //   flux.fppvz     = nflux.ppvz;
    // see above //   flux.fmuparpx  = nflux.muparpx;
    // see above //   flux.fmuparpy  = nflux.muparpy;
    // see above //   flux.fmuparpz  = nflux.muparpz;
    // see above //   flux.fmupare   = nflux.mupare;
    // see above //   flux.fnecm     = nflux.necm;
    // see above //   flux.fnimpwt   = nflux.nimpwt;
    //   flux.fxpoint   = nflux.xpoint;
    //   flux.fypoint   = nflux.ypoint;
    //   flux.fzpoint   = nflux.zpoint;
    //   flux.ftvx      = nflux.tvx;
    //   flux.ftvy      = nflux.tvy;
    //   flux.ftvz      = nflux.tvz;
    // see above //   flux.ftgen     = nflux.tgen;
    // see above //   flux.ftgptype  = nflux.tgptype;  // converted to PDG
    //   flux.ftgppx    = nflux.tgppx;
    //   flux.ftgppy    = nflux.tgppy;
    //   flux.ftgppz    = nflux.tgppz;
    //   flux.ftprivx   = nflux.tprivx;
    //   flux.ftprivy   = nflux.tprivy;
    //   flux.ftprivz   = nflux.tprivz;
    //   flux.fbeamx    = nflux.beamx;
    //   flux.fbeamy    = nflux.beamy;
    //   flux.fbeamz    = nflux.beamz;
    //   flux.fbeampx   = nflux.beampx;
    //   flux.fbeampy   = nflux.beampy;
    //   flux.fbeampz   = nflux.beampz; 
    
  return;
}

//---------------------------------------------------------------------------



