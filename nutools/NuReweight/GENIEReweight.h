////////////////////////////////////////////////////////////////////////
/// \file  GENIEReweight.h
/// \brief Wrapper for reweight neutrino interactions with GENIE base class
///
/// \author  nathan.mayer@tufts.edu
////////////////////////////////////////////////////////////////////////
#ifndef RWGT_GENIEREWEIGHT_H
#define RWGT_GENIEREWEIGHT_H

#include <vector>
#include <map>
#include <set>
#include <fstream>
#include "NuReweight/ReweightLabels.h"

//namespace simb  { class MCTruth;      }
//namespace simb  { class GTruth;       }

///GENIE neutrino interaction simulation
namespace genie { class EventRecord; }
namespace genie { class AlgFactory;  }
namespace genie{namespace rew   { class GReWeight; }}

namespace rwgt{

  class GENIEReweight {

  public:
    GENIEReweight();
    ~GENIEReweight();

#ifndef __GCCXML__
    
    void AddReweightValue(ReweightLabel_t rLabel, double value);
    void ChangeParameterValue(ReweightLabel_t rLabel, double value);

    double NominalParameterValue(ReweightLabel_t rLabel);
    double ReweightParameterValue(ReweightLabel_t rLabel);
    
    genie::rew::GReWeight* WeightCalculator() {return fWcalc;}
    
    void Configure();
    void Reconfigure();

    //Simple Configuration Functions. Only one of these should be called per instance of GENIEReweight
    void ReweightNCEL(double ma, double eta);

    void ReweightQEMA(double ma);
    void ReweightQEVec(double mv);

    void ReweightResGanged(double ma, double mv=0.0);
    void ReweightCCRes(double ma, double mv=0.0);
    void ReweightNCRes(double ma, double mv=0.0);
    
    void ReweightCoh(double ma, double r0);
    
    void ReweightNonResRvp1pi(double sigma);
    void ReweightNonResRvbarp1pi(double sigma);
    void ReweightNonResRvp2pi(double sigma);
    void ReweightNonResRvbarp2pi(double sigma);

    void ReweightResDecay(double gamma, double eta, double theta);
    void ReweightNC(double norm);
    void ReweightDIS(double aht, double bht, double cv1u, double cv2u);
    void ReweightDISnucl(bool mode);
    void ReweightAGKY(double xF, double pT);

    void ReweightFormZone(double sigma);
    void ReweightFGM(double kF, double sf);

    void ReweightIntraNuke(ReweightLabel_t name, double sigma);
    void ReweightIntraNuke(int name, double sigma);
    
    //General Reweight Configurations
    void MaQEshape() {fMaQEshape=true;}
    void MaQErate()  {fMaQEshape=false;}
    
    void CCRESshape() {fMaCCResShape=true;}
    void CCRESrate()  {fMaCCResShape=false;}
    
    void NCRESshape() {fMaNCResShape=true;}
    void NCRESrate()  {fMaNCResShape=false;}
    
    void DIS_BYshape() {fDISshape=true;}
    void DIS_BYrate()  {fDISshape=false;}
    
    void UseSigmaDef()    {fUseSigmaDef=true;}
    void UseStandardDef() {fUseSigmaDef=false;}

    void SetNominalValues();
    double CalculateSigma(ReweightLabel_t label, double value);

    double CalculateWeight(const genie::EventRecord& evr);
      
    //genie::EventRecord RetrieveGHEP(simb::MCTruth truth, simb::GTruth gtruth);
    
    //Functions to configure individual weight calculators
    void ConfigureNCEL();
    void ConfigureQEMA();
    void ConfigureQEVec();
    void ConfigureCCRes();
    void ConfigureNCRes();
    void ConfigureResBkg();
    void ConfgureResDecay();
    void ConfigureNC();
    void ConfigureDIS();
    void ConfigureCoh();
    void ConfigureAGKY();
    void ConfigureDISNucMod();
    void ConfigureFGM();
    void ConfigureFZone();
    void ConfigureINuke();
    void ConfigureParameters();
#endif

  protected:

    //Reweight configuration bools it is possible to use all simultaneously 
    bool fReweightNCEL;
    bool fReweightQEMA;
    bool fReweightQEVec;
    bool fReweightCCRes;
    bool fReweightNCRes;
    bool fReweightResBkg;
    bool fReweightResDecay;
    bool fReweightNC;
    bool fReweightDIS;
    bool fReweightCoh;
    bool fReweightAGKY;
    bool fReweightDISNucMod;
    bool fReweightFGM;
    bool fReweightFZone;
    bool fReweightINuke; 
    bool fReweightMEC;   //Not used. Reserved for future addition to GENIE

    bool fMaQEshape;
    bool fMaCCResShape;
    bool fMaNCResShape;
    bool fDISshape;

    bool fUseSigmaDef;
       
    std::vector<int> fReWgtParameterName;
    std::vector<double> fReWgtParameterValue;

    std::map<int, double> fNominalParameters;

    genie::rew::GReWeight* fWcalc;
    

  };
}
#endif //RWGT_GENIEREWEIGHT_H
