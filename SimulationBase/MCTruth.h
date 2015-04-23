////////////////////////////////////////////////////////////////////////
/// \file  MCTruth.cxx
/// \brief Simple MC truth class, holds a vector of TParticles
///
/// \version $Id: MCTruth.h,v 1.5 2012-10-15 20:36:27 brebel Exp $
/// \author  jpaley@indiana.edu
////////////////////////////////////////////////////////////////////////
#ifndef SIMB_MCTRUTH_H
#define SIMB_MCTRUTH_H

#include <vector>
#include "SimulationBase/MCNeutrino.h"

namespace simb {

  class MCParticle;

  /// event origin types
  typedef enum _ev_origin{
    kUnknown,           ///< ???
    kBeamNeutrino,      ///< Beam neutrinos
    kCosmicRay,         ///< Cosmic rays
    kSuperNovaNeutrino, ///< Supernova neutrinos
    kSingleParticle     ///< single particles thrown at the detector
  } Origin_t;

  //......................................................................

  /// Event generator information
  class MCTruth {
  public:
    MCTruth();

  private:

    std::vector<simb::MCParticle> fPartList;    ///< list of particles in this event
    simb::MCNeutrino              fMCNeutrino;  ///< reference to neutrino info - null if not a neutrino
    simb::Origin_t                fOrigin;      ///< origin for this event
    bool                          fNeutrinoSet; ///< flag for whether the neutrino information has been set

#ifndef __GCCXML__
  public:

    simb::Origin_t          Origin()            const;
    int                     NParticles()        const;
    const simb::MCParticle& GetParticle(int i)  const;
    const simb::MCNeutrino& GetNeutrino()       const;
    bool                    NeutrinoSet()       const;
    
    void             Add(simb::MCParticle& part);           
    void             SetOrigin(simb::Origin_t origin);
    void             SetNeutrino(int CCNC, 
				 int mode, 
				 int interactionType,
				 int target, 
				 int nucleon,
				 int quark, 
				 double w, 
				 double x, 
				 double y, 
				 double qsqr);                      
 
    friend std::ostream&  operator<< (std::ostream& o, simb::MCTruth const& a);
#endif

  };
}

#ifndef __GCCXML__

inline simb::Origin_t          simb::MCTruth::Origin()            const { return fOrigin;               }
inline int                     simb::MCTruth::NParticles()        const { return (int)fPartList.size(); }
inline const simb::MCParticle& simb::MCTruth::GetParticle(int i)  const { return fPartList[i];          }
inline const simb::MCNeutrino& simb::MCTruth::GetNeutrino()       const { return fMCNeutrino;           }
inline bool                    simb::MCTruth::NeutrinoSet()       const { return fNeutrinoSet;          }

inline void                    simb::MCTruth::Add(simb::MCParticle& part)      { fPartList.push_back(part);    }
inline void                    simb::MCTruth::SetOrigin(simb::Origin_t origin) { fOrigin = origin;             }

#endif

#endif //SIMB_MCTRUTH_H
////////////////////////////////////////////////////////////////////////
