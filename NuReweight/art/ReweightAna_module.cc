////////////////////////////////////////////////////////////////////////
/// \file ReweightAna_module.cc Analyzer module for art framework 
/// cross section reweighting
//
/// \version $Id: ReweightAna.h,v 1.2 2012-09-19 18:12:09 nsmayer Exp $
/// \author  nsmayer@fnal.gov
////////////////////////////////////////////////////////////////////////
#include <vector>
#include <cmath>
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TDatabasePDG.h"
#include "TSystem.h"
#include "SimulationBase/MCTruth.h"
#include "SimulationBase/GTruth.h"
#include "SimulationBase/MCNeutrino.h"
#include "SimulationBase/MCFlux.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "NuReweight/art/NuReweight.h"

#include "art/Framework/Core/EDAnalyzer.h"

#include "art/Framework/Core/ModuleMacros.h"


namespace simb  { class MCTruth;       }
namespace simb  { class GTruth;        }
namespace rwgt  {class NuReweight;  }

class TH1F;
class TH2F;

namespace rwgt {

  //class GENIEReweight;
  /// A module to check the results from the Monte Carlo generator
  
  class ReweightAna : public art::EDAnalyzer {
    
  public:
    explicit ReweightAna(fhicl::ParameterSet const& pset);
    virtual ~ReweightAna();

    void analyze(art::Event const& evt);
    void beginSubRun(art::SubRun const& sr);
    void beginJob();
    void endJob();
    void endSubRun(art::SubRun const& sr);
    void reconfigure(const fhicl::ParameterSet& p);

  private:
    void LoadMCInfo(art::Event const& evt);

  private:
    TH1F* fEnergyNeutrino;           ///< Total number of events 
    TH1F* fNeventsSubrun;            ///< Total number of events per subrun
    TH1F* fWgtQE[3];
    TH1F* fWgtRES[3];
    TH1F* fWgtDIS[3];
    rwgt::NuReweight* fGrwgt[3];  ///< X-sec weight calculator

    std::string fMCTruthModuleLabel; ///< label for module producing mc truth information
    std::string fPotLabel;           ///< Module that produced the POTSum object

  };
}

namespace rwgt{

  static int cntEvent = 0; //event number

  //......................................................................  
  ReweightAna::ReweightAna(fhicl::ParameterSet const& p)
    : EDAnalyzer(p)
  {
    this->reconfigure(p);
  }
  

  //......................................................................
  ReweightAna::~ReweightAna() { }
  
  //......................................................................
  void ReweightAna::beginJob()
  {
    art::ServiceHandle<art::TFileService> tfs;
   
    mf::LogVerbatim("ReweightAna") << "make histograms" ;

    fEnergyNeutrino   = tfs->make<TH1F>("fEnergyneutrino", "Total number of events", 50, 0., 25); 
    fNeventsSubrun    = tfs->make<TH1F>("fNeventsSubrun", "Total number of events", 1, 0., 1.); 

    char name[300];
    for(int i = 0; i < 3; i++) {
      sprintf(name, "fWgtQE_%dsigma", i+1);
      fWgtQE[i] = tfs->make<TH1F>(name, "Evt Wgts", 100, 0., 2.0); 
      sprintf(name, "fWgtRES_%dsigma", i+1);
      fWgtRES[i] = tfs->make<TH1F>(name, "Evt Wgts", 100, 0., 2.0); 
      sprintf(name, "fWgtDIS_%dsigma", i+1);
      fWgtDIS[i] = tfs->make<TH1F>(name, "Evt Wgts", 100, 0., 2.0); 

      double sigma = (double)(i+1);
      fGrwgt[i] = new rwgt::NuReweight();
      fGrwgt[i]->AddReweightValue(rwgt::fReweightMaCCQE, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightMaCCRES, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightMaNCRES, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvpCC1pi, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvnCC1pi, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvpCC2pi, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvnCC2pi, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvpNC1pi, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvnNC1pi, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvpNC2pi, sigma);
      fGrwgt[i]->AddReweightValue(rwgt::fReweightRvnNC2pi, sigma);
      
      fGrwgt[i]->Configure();
    }
  }
  
  //......................................................................
  void ReweightAna::beginSubRun(art::SubRun const& sr) {
    
  }

  //......................................................................
  void ReweightAna::analyze(art::Event const& evt)
  { 

    // // Pull the MC generator information out of the event
    mf::LogVerbatim("ReweightAna") << "Start analyze" ;
    art::Handle< std::vector<simb::MCTruth> > mclist;
    evt.getByLabel(fMCTruthModuleLabel, mclist);
    if (mclist->empty()) {
      mf::LogWarning("ReweightAna") << "Error retrieving MCTruth list" ;
      return;
    }
    
    art::Handle< std::vector<simb::GTruth> > gtlist;
    evt.getByLabel(fMCTruthModuleLabel, gtlist);
    if (gtlist->empty()) {
      mf::LogWarning("ReweightAna") << "Error retrieving GTruth list" ;
      return;
    }
    
   LOG_DEBUG("ReweightAna")<<"MC List sizes:" << mclist->size() << " " << gtlist->size() << "\n";
    
    // // Loop over neutrino interactions
    for(size_t i_intx = 0; i_intx < mclist->size(); ++i_intx){
      LOG_DEBUG("ReweightAna") << "start loop";
      
      //   // Link to the MCNeutrino class.
      //   // The class contains information not only about
      //   // the incoming neutrino, but about the products of the decay
      simb::MCTruth    const& truth       = mclist->at(i_intx);
      simb::GTruth     const& gtruth      = gtlist->at(i_intx);
      simb::MCNeutrino const& mc_neutrino = truth.GetNeutrino();

      fEnergyNeutrino->Fill(mc_neutrino.Nu().E());
      for(int i = 0; i < 3; i++) {
	double wgt = fGrwgt[i]->CalcWeight(truth, gtruth);
	//double wgt = 1.;
	if(mc_neutrino.Mode()==0 && mc_neutrino.CCNC()==0) {
	  fWgtQE[i]->Fill(wgt);
	}
	else if(mc_neutrino.Mode()==1 && mc_neutrino.CCNC()==0) {
	  fWgtRES[i]->Fill(wgt);
	}
	else if(mc_neutrino.Mode()==2 && mc_neutrino.CCNC()==0) {
	  fWgtDIS[i]->Fill(wgt);
	}
      }

      LOG_DEBUG("ReweightAna") << "end loop" ;
    }//end loop over interactions

 
    return;
  }
  

  //......................................................................  
  void ReweightAna::LoadMCInfo(art::Event const& evt)
  {


  }
    
  
  //......................................................................
  void ReweightAna::reconfigure(const fhicl::ParameterSet& p)
  {    
    fMCTruthModuleLabel = p.get< std::string>("MCTruthModuleLabel");
  }
  
  //......................................................................
  void ReweightAna::endSubRun(art::SubRun const& sr)
  {
    fNeventsSubrun->Fill(sr.subRun(), cntEvent);
    cntEvent = 0;

  }

  //......................................................................
  void ReweightAna::endJob()
  {

  }
  
  
  ////////////////////////////////////////////////////////////////////////
}
//Module Definition

namespace rwgt{

  DEFINE_ART_MODULE(ReweightAna)

}


