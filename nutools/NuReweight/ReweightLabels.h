////////////////////////////////////////////////////////////////////////
///\file ReweightLabel.h
///\brief typedef defining all of the available GENIE reweighting parameters
///
///\author  nathan.mayer@tufts.edu 
////////////////////////////////////////////////////////////////////////
#ifndef RWGT_REWEIGHTLABEL_H
#define RWGT_REWEIGHTLABEL_H

#include "ReWeight/GSyst.h"

namespace rwgt {
  
  typedef enum EReweightLabel {
	  
    kReweightNull = genie::rew::kNullSystematic,
    
    // NCEL tweaking parameters:
    fReweightMaNCEL = genie::rew::kXSecTwkDial_MaNCEL,              ///< tweak Ma NCEL, affects dsigma(NCEL)/dQ2 both in shape and normalization
    fReweightEtaNCEL = genie::rew::kXSecTwkDial_EtaNCEL,            ///< tweak NCEL strange axial form factor eta, affects dsigma(NCEL)/dQ2 both in shape and normalization
    // CCQE tweaking parameters:
    fReweightNormCCQE = genie::rew::kXSecTwkDial_NormCCQE,          ///< tweak CCQE normalization (energy independent)
    fReweightNormCCQEenu = genie::rew::kXSecTwkDial_NormCCQEenu,    ///< tweak CCQE normalization (maintains dependence on neutrino energy)
    fReweightMaCCQEshape = genie::rew::kXSecTwkDial_MaCCQEshape,    ///< tweak Ma CCQE, affects dsigma(CCQE)/dQ2 in shape only (normalized to constant integral)
    fReweightMaCCQE = genie::rew::kXSecTwkDial_MaCCQE,              ///< tweak Ma CCQE, affects dsigma(CCQE)/dQ2 both in shape and normalization
    fReweightVecCCQEshape = genie::rew::kXSecTwkDial_VecFFCCQEshape,///< tweak elastic nucleon form factors (BBA/default -> dipole) - shape only effect of dsigma(CCQE)/dQ2
    // Resonance neutrino-production tweaking parameters:
    fReweightNormCCRES = genie::rew::kXSecTwkDial_NormCCRES,         ///< tweak CCRES normalization
    fReweightMaCCRESshape = genie::rew::kXSecTwkDial_MaCCRESshape,   ///< tweak Ma CCRES, affects d2sigma(CCRES)/dWdQ2 in shape only (normalized to constant integral)
    fReweightMvCCRESshape = genie::rew::kXSecTwkDial_MvCCRESshape,   ///< tweak Mv CCRES, affects d2sigma(CCRES)/dWdQ2 in shape only (normalized to constant integral)
    fReweightMaCCRES = genie::rew::kXSecTwkDial_MaCCRES,             ///< tweak Ma CCRES, affects d2sigma(CCRES)/dWdQ2 both in shape and normalization
    fReweightMvCCRES = genie::rew::kXSecTwkDial_MvCCRES,             ///< tweak Mv CCRES, affects d2sigma(CCRES)/dWdQ2 both in shape and normalization
    
    fReweightNormNCRES = genie::rew::kXSecTwkDial_NormNCRES,         ///< tweak NCRES normalization
    fReweightMaNCRESshape = genie::rew::kXSecTwkDial_MaNCRESshape,   ///< tweak Ma NCRES, affects d2sigma(NCRES)/dWdQ2 in shape only (normalized to constant integral)
    fReweightMvNCRESshape = genie::rew::kXSecTwkDial_MvNCRESshape,   ///< tweak Mv NCRES, affects d2sigma(NCRES)/dWdQ2 in shape only (normalized to constant integral)
    fReweightMaNCRES = genie::rew::kXSecTwkDial_MaNCRES,             ///< tweak Ma NCRES, affects d2sigma(NCRES)/dWdQ2 both in shape and normalization
    fReweightMvNCRES = genie::rew::kXSecTwkDial_MvNCRES,             ///< tweak Mv NCRES, affects d2sigma(NCRES)/dWdQ2 both in shape and normalization
    
    // Coherent pion production tweaking parameters:
    fReweightMaCOHpi = genie::rew::kXSecTwkDial_MaCOHpi,             ///< tweak Ma for COH pion production
    fReweightR0COHpi = genie::rew::kXSecTwkDial_R0COHpi,             ///< tweak R0 for COH pion production
    // Non-resonance background tweaking parameters:
    fReweightRvpCC1pi = genie::rew::kXSecTwkDial_RvpCC1pi,           ///< tweak the 1pi non-RES bkg in the RES region, for v+p CC
    fReweightRvpCC2pi = genie::rew::kXSecTwkDial_RvpCC2pi,           ///< tweak the 2pi non-RES bkg in the RES region, for v+p CC
    fReweightRvpNC1pi = genie::rew::kXSecTwkDial_RvpNC1pi,           ///< tweak the 1pi non-RES bkg in the RES region, for v+p NC
    fReweightRvpNC2pi = genie::rew::kXSecTwkDial_RvpNC2pi,           ///< tweak the 2pi non-RES bkg in the RES region, for v+p NC
    fReweightRvnCC1pi = genie::rew::kXSecTwkDial_RvnCC1pi,           ///< tweak the 1pi non-RES bkg in the RES region, for v+n CC
    fReweightRvnCC2pi = genie::rew::kXSecTwkDial_RvnCC2pi,           ///< tweak the 2pi non-RES bkg in the RES region, for v+n CC
    fReweightRvnNC1pi = genie::rew::kXSecTwkDial_RvnNC1pi,           ///< tweak the 1pi non-RES bkg in the RES region, for v+n NC
    fReweightRvnNC2pi = genie::rew::kXSecTwkDial_RvnNC2pi,           ///< tweak the 2pi non-RES bkg in the RES region, for v+n NC
    fReweightRvbarpCC1pi = genie::rew::kXSecTwkDial_RvbarpCC1pi,     ///< tweak the 1pi non-RES bkg in the RES region, for vbar+p CC
    fReweightRvbarpCC2pi = genie::rew::kXSecTwkDial_RvbarpCC2pi,     ///< tweak the 2pi non-RES bkg in the RES region, for vbar+p CC
    fReweightRvbarpNC1pi = genie::rew::kXSecTwkDial_RvbarpNC1pi,     ///< tweak the 1pi non-RES bkg in the RES region, for vbar+p NC
    fReweightRvbarpNC2pi = genie::rew::kXSecTwkDial_RvbarpNC2pi,     ///< tweak the 2pi non-RES bkg in the RES region, for vbar+p NC
    fReweightRvbarnCC1pi = genie::rew::kXSecTwkDial_RvbarnCC1pi,     ///< tweak the 1pi non-RES bkg in the RES region, for vbar+n CC
    fReweightRvbarnCC2pi = genie::rew::kXSecTwkDial_RvbarnCC2pi,     ///< tweak the 2pi non-RES bkg in the RES region, for vbar+n CC
    fReweightRvbarnNC1pi = genie::rew::kXSecTwkDial_RvbarnNC1pi,     ///< tweak the 1pi non-RES bkg in the RES region, for vbar+n NC
    fReweightRvbarnNC2pi = genie::rew::kXSecTwkDial_RvbarnNC2pi,     ///< tweak the 2pi non-RES bkg in the RES region, for vbar+n NC
    // DIS tweaking parameters - applied for DIS events with (Q2>Q2o, W>Wo), 
    // typically Q2okReweight =1GeV^2, WokReweight =1.7-2.0GeV
    fReweightAhtBY = genie::rew::kXSecTwkDial_AhtBY,                 ///< tweak the Bodek-Yang model parameter A_{ht} - incl. both shape and normalization effect
    fReweightBhtBY = genie::rew::kXSecTwkDial_BhtBY,                 ///< tweak the Bodek-Yang model parameter B_{ht} - incl. both shape and normalization effect
    fReweightCV1uBY = genie::rew::kXSecTwkDial_CV1uBY,               ///< tweak the Bodek-Yang model parameter CV1u - incl. both shape and normalization effect
    fReweightCV2uBY = genie::rew::kXSecTwkDial_CV2uBY,               ///< tweak the Bodek-Yang model parameter CV2u - incl. both shape and normalization effect
    fReweightAhtBYshape = genie::rew::kXSecTwkDial_AhtBYshape,       ///< tweak the Bodek-Yang model parameter A_{ht} - shape only effect to d2sigma(DIS)/dxdy
    fReweightBhtBYshape = genie::rew::kXSecTwkDial_BhtBYshape,       ///< tweak the Bodek-Yang model parameter B_{ht} - shape only effect to d2sigma(DIS)/dxdy
    fReweightCV1uBYshape = genie::rew::kXSecTwkDial_CV1uBYshape,     ///< tweak the Bodek-Yang model parameter CV1u - shape only effect to d2sigma(DIS)/dxdy
    fReweightCV2uBYshape = genie::rew::kXSecTwkDial_CV2uBYshape,     ///< tweak the Bodek-Yang model parameter CV2u - shape only effect to d2sigma(DIS)/dxdy
    fReweightNormDISCC = genie::rew::kXSecTwkDial_NormDISCC,         ///< tweak the inclusive DIS CC normalization (not currently working in genie)
    fReweightRnubarnuCC = genie::rew::kXSecTwkDial_RnubarnuCC,       ///< tweak the ratio of \sigma(\bar\nu CC) / \sigma(\nu CC) (not currently working in genie)
    fReweightDISNuclMod = genie::rew::kXSecTwkDial_DISNuclMod,       ///< tweak DIS nuclear modification (shadowing, anti-shadowing, EMC).  Does not appear to be working in GENIE at the moment
    //
    fReweightNC = genie::rew::kXSecTwkDial_NC,                ///<
    
    
    //
    // Hadronization (free nucleon target)
    // 
    
    fReweightAGKY_xF1pi = genie::rew::kHadrAGKYTwkDial_xF1pi,         ///< tweak xF distribution for low multiplicity (N + pi) DIS f/s produced by AGKY
    fReweightAGKY_pT1pi = genie::rew::kHadrAGKYTwkDial_pT1pi,         ///< tweak pT distribution for low multiplicity (N + pi) DIS f/s produced by AGKY
    
    
    //
    // Medium-effects to hadronization
    // 
    
    fReweightFormZone = genie::rew::kHadrNuclTwkDial_FormZone,         ///< tweak formation zone
    
    
    //
    // Intranuclear rescattering systematics.
    // There are 2 sets of parameters:
    // - parameters that control the total rescattering probability, P(total)
    // - parameters that control the fraction of each process (`fate'), given a total rescat. prob., P(fate|total)
    // These parameters are considered separately for pions and nucleons.
    //
    
    fReweightMFP_pi = genie::rew::kINukeTwkDial_MFP_pi,           ///< tweak mean free path for pions
    fReweightMFP_N = genie::rew::kINukeTwkDial_MFP_N,             ///< tweak mean free path for nucleons
    fReweightFrCEx_pi = genie::rew::kINukeTwkDial_FrCEx_pi,       ///< tweak charge exchange probability for pions, for given total rescattering probability
    fReweightFrElas_pi = genie::rew::kINukeTwkDial_FrElas_pi,     ///< tweak elastic   probability for pions, for given total rescattering probability
    fReweightFrInel_pi = genie::rew::kINukeTwkDial_FrInel_pi,     ///< tweak inelastic probability for pions, for given total rescattering probability
    fReweightFrAbs_pi = genie::rew::kINukeTwkDial_FrAbs_pi,       ///< tweak absorption probability for pions, for given total rescattering probability
    fReweightFrPiProd_pi = genie::rew::kINukeTwkDial_FrPiProd_pi, ///< tweak pion production probability for pions, for given total rescattering probability
    fReweightFrCEx_N = genie::rew::kINukeTwkDial_FrCEx_N,         ///< tweak charge exchange probability for nucleons, for given total rescattering probability
    fReweightFrElas_N = genie::rew::kINukeTwkDial_FrElas_N,       ///< tweak elastic    probability for nucleons, for given total rescattering probability
    fReweightFrInel_N = genie::rew::kINukeTwkDial_FrInel_N,       ///< tweak inelastic  probability for nucleons, for given total rescattering probability
    fReweightFrAbs_N = genie::rew::kINukeTwkDial_FrAbs_N,         ///< tweak absorption probability for nucleons, for given total rescattering probability
    fReweightFrPiProd_N = genie::rew::kINukeTwkDial_FrPiProd_N,   ///< tweak pion production probability for nucleons, for given total rescattering probability
    
    //
    // Nuclear model
    // 
    
    fReweightCCQEPauliSupViaKF = genie::rew::kSystNucl_CCQEPauliSupViaKF,   ///<
    fReweightCCQEMomDistroFGtoSF = genie::rew::kSystNucl_CCQEMomDistroFGtoSF, ///<
    
    //
    // Resonance decays
    // 
    
    fReweightBR1gamma = genie::rew::kRDcyTwkDial_BR1gamma,               ///< tweak Resonance -> X + gamma branching ratio, eg Delta+(1232) -> p gamma
    fReweightBR1eta = genie::rew::kRDcyTwkDial_BR1eta,                   ///< tweak Resonance -> X + eta   branching ratio, eg N+(1440) -> p eta
    fReweightTheta_Delta2Npi = genie::rew::kRDcyTwkDial_Theta_Delta2Npi,  ///< distort pi angular distribution in Delta -> N + pi

  } ReweightLabel_t;

} // end rwgt namespace
#endif //RWGT_REWEIGHTLABEL_H
