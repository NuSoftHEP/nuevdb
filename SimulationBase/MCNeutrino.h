////////////////////////////////////////////////////////////////////////
/// \file  MCNeutrino.cxx
/// \brief Simple MC truth class, holds a vector of TParticles
///
/// \version $Id: MCNeutrino.h,v 1.6 2012-11-20 17:39:38 brebel Exp $
/// \author  jpaley@indiana.edu
////////////////////////////////////////////////////////////////////////
#ifndef SIMB_MCNEUTRINO_H
#define SIMB_MCNEUTRINO_H

#include "SimulationBase/MCParticle.h"

namespace simb {
  
  //......................................................................

  /// Event generator information
  class MCNeutrino {
  public:

    MCNeutrino();

  private:
 
    simb::MCParticle fNu;              ///< the incoming neutrino			      
    simb::MCParticle fLepton;          ///< the outgoing lepton			      
    int              fMode;            ///< Interaction mode (QE/1-pi/DIS...) see enum list			
    int        	     fInteractionType; ///< More detailed interaction type, see enum list below kNuanceOffset	
    int        	     fCCNC;            ///< CC or NC interaction? see enum list	      			
    int        	     fTarget;          ///< Nuclear target, as PDG code				      	
    int        	     fHitNuc;          ///< Hit nucleon (2212 (proton) or 2112 (neutron))  			
    int        	     fHitQuark;        ///< For DIS events only, as PDG code			      		
    double     	     fW;               ///< Hadronic invariant mass, in GeV			      		
    double     	     fX;               ///< Bjorken x=Q^2/(2M*(E_neutrino-E_lepton)), unitless	      	
    double     	     fY;               ///< Inelasticity y=1-(E_lepton/E_neutrino), unitless	      		
    double           fQSqr;            ///< Momentum transfer Q^2, in GeV^2                                     

#ifndef __GCCXML__
  public:

    MCNeutrino(simb::MCParticle &nu, 
	       simb::MCParticle &lep, 
	       int CCNC, 
	       int mode, 
	       int interactionType,
	       int target, 
	       int nucleon,
	       int quark, 
	       double w, 
	       double x, 
	       double y, 
	       double qsqr);

    const  simb::MCParticle& Nu()              const;
    const  simb::MCParticle& Lepton()          const;
    int                      CCNC()            const;								    
    int                	     Mode()            const;								    
    int                	     InteractionType() const;								    
    int                	     Target()          const;								    
    int                	     HitNuc()          const;								    
    int                	     HitQuark()        const;								    
    double             	     W()               const;								    
    double             	     X()               const;								    
    double             	     Y()               const;								    
    double             	     QSqr()            const;								    
    double             	     Pt()              const; ///< transverse momentum of interaction, in GeV/c	    
    double             	     Theta()           const; ///< angle between incoming and outgoing leptons, in radians
    friend std::ostream&  operator<< (std::ostream& output, const simb::MCNeutrino &mcnu);
#endif

  };
}

#ifndef __GCCXML__
namespace simb{
  /// Neutrino interaction categories
  enum curr_type_{
    kCC,
    kNC
  };

  /// Neutrino interaction categories
  enum int_type_{
    kQE                        = 0,
    kRes                       = 1,
    kDIS		       = 2,
    kCoh		       = 3,
    kNuanceOffset              = 1000,                ///< offset to account for adding in Nuance codes to this enum
    kCCQE                      = kNuanceOffset + 1,   ///< charged current quasi-elastic	      	
    kNCQE                      = kNuanceOffset + 2,   ///< neutral current quasi-elastic	      	
    kResCCNuProtonPiPlus       = kNuanceOffset + 3,   ///< resonant charged current, nu p -> l- p pi+
    kResCCNuNeutronPi0         = kNuanceOffset + 4,   ///< resonant charged current, nu n -> l- n pi0
    kResCCNuNeutronPiPlus      = kNuanceOffset + 5,   ///< resonant charged current, nu n -> l- n pi+
    kResNCNuProtonPi0          = kNuanceOffset + 6,   ///< resonant neutral current, nu p -> nu p pi0
    kResNCNuProtonPiPlus       = kNuanceOffset + 7,   ///< resonant neutral current, nu p -> nu p pi+
    kResNCNuNeutronPi0         = kNuanceOffset + 8,   ///< resonant neutral current, nu n -> nu n pi0
    kResNCNuNeutronPiMinus     = kNuanceOffset + 9,   ///< resonant neutral current, nu n -> nu p pi-
    kResCCNuBarNeutronPiMinus  = kNuanceOffset + 10,  ///< resonant charged current, nubar n -> l+ n pi-
    kResCCNuBarProtonPi0       = kNuanceOffset + 11,  ///< resonant charged current, nubar p -> l+ n pi0
    kResCCNuBarProtonPiMinus   = kNuanceOffset + 12,  ///< resonant charged current, nubar p -> l+ p pi-
    kResNCNuBarProtonPi0       = kNuanceOffset + 13,  ///< resonant charged current, nubar p -> nubar p pi0
    kResNCNuBarProtonPiPlus    = kNuanceOffset + 14,  ///< resonant charged current, nubar p -> nubar n pi+
    kResNCNuBarNeutronPi0      = kNuanceOffset + 15,  ///< resonant charged current, nubar n -> nubar n pi0
    kResNCNuBarNeutronPiMinus  = kNuanceOffset + 16,  ///< resonant charged current, nubar n -> nubar p pi-  
    kResCCNuDeltaPlusPiPlus    = kNuanceOffset + 17,
    kResCCNuDelta2PlusPiMinus  = kNuanceOffset + 21,
    kResCCNuBarDelta0PiMinus   = kNuanceOffset + 28,
    kResCCNuBarDeltaMinusPiPlus= kNuanceOffset + 32,
    kResCCNuProtonRhoPlus      = kNuanceOffset + 39,
    kResCCNuNeutronRhoPlus     = kNuanceOffset + 41,
    kResCCNuBarNeutronRhoMinus = kNuanceOffset + 46,
    kResCCNuBarNeutronRho0     = kNuanceOffset + 48,
    kResCCNuSigmaPlusKaonPlus  = kNuanceOffset + 53,
    kResCCNuSigmaPlusKaon0     = kNuanceOffset + 55,
    kResCCNuBarSigmaMinusKaon0 = kNuanceOffset + 60,
    kResCCNuBarSigma0Kaon0     = kNuanceOffset + 62,
    kResCCNuProtonEta          = kNuanceOffset + 67,
    kResCCNuBarNeutronEta      = kNuanceOffset + 70,
    kResCCNuKaonPlusLambda0    = kNuanceOffset + 73,
    kResCCNuBarKaon0Lambda0    = kNuanceOffset + 76,
    kResCCNuProtonPiPlusPiMinus= kNuanceOffset + 79,
    kResCCNuProtonPi0Pi0       = kNuanceOffset + 80,
    kResCCNuBarNeutronPiPlusPiMinus = kNuanceOffset + 85,
    kResCCNuBarNeutronPi0Pi0   = kNuanceOffset + 86,
    kResCCNuBarProtonPi0Pi0    = kNuanceOffset + 90,
    kCCDIS                     = kNuanceOffset + 91,  ///< charged current deep inelastic scatter
    kNCDIS                     = kNuanceOffset + 92,  ///< charged current deep inelastic scatter
    kUnUsed1                   = kNuanceOffset + 93,
    kUnUsed2                   = kNuanceOffset + 94,
    kCCQEHyperon               = kNuanceOffset + 95,
    kNCCOH                     = kNuanceOffset + 96,
    kCCCOH                     = kNuanceOffset + 97,  ///< charged current coherent pion         
    kNuElectronElastic         = kNuanceOffset + 98,  ///< neutrino electron elastic scatter	
    kInverseMuDecay            = kNuanceOffset + 99   ///< inverse muon decay			  
  };
}

inline const  simb::MCParticle& simb::MCNeutrino::Nu()              const { return fNu;              }
inline const  simb::MCParticle& simb::MCNeutrino::Lepton()   	    const { return fLepton;  	     }
inline        int               simb::MCNeutrino::CCNC()     	    const { return fCCNC;    	     }
inline        int         	simb::MCNeutrino::Mode()     	    const { return fMode;    	     }
inline        int         	simb::MCNeutrino::InteractionType() const { return fInteractionType; }
inline        int         	simb::MCNeutrino::Target()          const { return fTarget;          }
inline        int         	simb::MCNeutrino::HitNuc()   	    const { return fHitNuc;  	     }  
inline        int         	simb::MCNeutrino::HitQuark()        const { return fHitQuark;	     }  
inline        double      	simb::MCNeutrino::W()               const { return fW;       	     }  
inline        double      	simb::MCNeutrino::X()               const { return fX;       	     }  
inline        double      	simb::MCNeutrino::Y()               const { return fY;       	     }  
inline        double      	simb::MCNeutrino::QSqr()            const { return fQSqr;    	     }  
#endif

#endif //SIMB_MCNEUTRINO_H
////////////////////////////////////////////////////////////////////////
