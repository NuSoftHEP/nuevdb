////////////////////////////////////////////////////////////////////////
/// \file  MCTruth.cxx
/// \brief Simple MC truth class, holds a vector of MCParticles
///
/// \version $Id: MCTruth.cxx,v 1.8 2012-10-15 20:36:27 brebel Exp $
/// \author  jpaley@indiana.edu
////////////////////////////////////////////////////////////////////////
#include "SimulationBase/MCTruth.h"
#include "SimulationBase/MCParticle.h"
#include "SimulationBase/MCNeutrino.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TDatabasePDG.h"

#include <iostream>

namespace simb{

  //......................................................................
  MCTruth::MCTruth() 
    : fPartList()
    , fMCNeutrino()
    , fOrigin(simb::kUnknown)
    , fNeutrinoSet(false)
  { 
  }

  //......................................................................
  void MCTruth::SetNeutrino(int CCNC, 
                            int mode,
                            int interactionType,
                            int target,
                            int nucleon,
                            int quark,
                            double w, 
                            double x, 
                            double y, 
                            double qsqr)
  {
    if( !fNeutrinoSet ){
      fNeutrinoSet = true;
      // loop over the MCParticle list and get the outgoing lepton
      // assume this is a neutral current interaction to begin with
      // which means the outgoing lepton is the incoming neutrino
      MCParticle nu  = fPartList[0];
      MCParticle lep = fPartList[0];

      // start at i = 1 because i = 0 is the incoming neutrino
      for(unsigned int i = 1; i < fPartList.size(); ++i){
        if(fPartList[i].Mother() == nu.TrackId() &&
           (fPartList[i].PdgCode()  == nu.PdgCode() ||
            abs(fPartList[i].PdgCode()) == abs(nu.PdgCode())-1) ){
             lep = fPartList[i];
             break;
           }
      }//done looping over particles
    
      fMCNeutrino = simb::MCNeutrino(nu, lep, 
                                     CCNC, mode, interactionType,
                                     target, nucleon, quark,
                                     w, x, y, qsqr);
    } // end if MCNeutrino is not already set
    else
      mf::LogWarning("MCTruth") << "MCTruth - attempt to set neutrino when already set";
      
    return;
  }

  //......................................................................
  std::ostream& operator<< (std::ostream& o, simb::MCTruth const& a)
  {
    if(a.Origin() == kCosmicRay) 
      o << "This is a cosmic ray event" << std::endl;
    else if(a.Origin() == kBeamNeutrino){
      o << "This is a beam neutrino event" << std::endl;
      o << a.GetNeutrino();
    }
    else if(a.Origin() == kSuperNovaNeutrino){ 
      o << "This is a supernova neutrino event" << std::endl;
      o << a.GetNeutrino();
    }  

    for (int i = 0; i < a.NParticles(); ++i)
      o << i << " " << a.GetParticle(i) << std::endl;

    return o;
  }
}
////////////////////////////////////////////////////////////////////////
