////////////////////////////////////////////////////////////////////////
/// \file  MCNeutrino.cxx
/// \brief Simple MC truth class, holds a vector of TParticles
///
/// \version $Id: MCNeutrino.cxx,v 1.5 2012-10-15 20:36:27 brebel Exp $
/// \author  jpaley@indiana.edu
////////////////////////////////////////////////////////////////////////
#include "SimulationBase/MCNeutrino.h"
#include "SimulationBase/MCParticle.h"
#include "TVector3.h"
#include <iostream>
#include <climits>

namespace simb{

  //......................................................................
  MCNeutrino::MCNeutrino()
    : fNu             ()
    , fLepton         ()
    , fMode           (std::numeric_limits<int>::min())
    , fInteractionType(std::numeric_limits<int>::min())
    , fCCNC           (std::numeric_limits<int>::min())
    , fTarget         (std::numeric_limits<int>::min())
    , fHitNuc	        (std::numeric_limits<int>::min())
    , fHitQuark       (std::numeric_limits<int>::min())
    , fW              (std::numeric_limits<double>::min())
    , fX	            (std::numeric_limits<double>::min())
    , fY	            (std::numeric_limits<double>::min())
    , fQSqr           (std::numeric_limits<double>::min())
  {
  }

  //......................................................................
  ///nu is the incoming neutrino and lep is the outgoing lepton
  MCNeutrino::MCNeutrino(MCParticle &nu, 
                         MCParticle &lep,
                         int CCNC,
                         int mode,
                         int interactionType,
                         int target,
                         int nucleon,
                         int quark, 
                         double w, 
                         double x, 
                         double y, 
                         double qsqr)
    : fNu(nu)
    , fLepton(lep)
    , fMode(mode)
    , fInteractionType(interactionType)
    , fCCNC(CCNC)
    , fTarget(target)
    , fHitNuc(nucleon)
    , fHitQuark(quark)
    , fW(w)
    , fX(x)
    , fY(y)
    , fQSqr(qsqr)
  { 
  }

  //......................................................................
  double MCNeutrino::Theta() const
  {
    ///make TVector3 objects for the momenta of the incoming neutrino
    ///and outgoing lepton
    TVector3 in(fNu.Px(), fNu.Py(), fNu.Pz());
    TVector3 out(fLepton.Px(), fLepton.Py(), fLepton.Pz());

    return in.Angle(out);
  }

  //......................................................................
  double MCNeutrino::Pt() const
  {
    return fNu.Pt();
  }

  //......................................................................
  std::ostream&  operator<< (std::ostream& output, const simb::MCNeutrino &mcnu)
  {
    output << " neutrino =         " << mcnu.Nu().PdgCode()    << std::endl
	   << " neutrino energy =  " << mcnu.Nu().E()          << std::endl
	   << " CCNC =             " << mcnu.CCNC()            << std::endl
	   << " mode =             " << mcnu.Mode()            << std::endl
	   << " interaction type = " << mcnu.InteractionType() << std::endl
	   << " target =           " << mcnu.Target()          << std::endl 
	   << " nucleon =          " << mcnu.HitNuc()          << std::endl 
	   << " quark =            " << mcnu.HitQuark()        << std::endl 
	   << " W =                " << mcnu.W()               << std::endl 
	   << " X =                " << mcnu.X()               << std::endl 
	   << " Y =                " << mcnu.Y()               << std::endl 
	   << " Q^2 =              " << mcnu.QSqr()            << std::endl;

    return output;
  }

}
////////////////////////////////////////////////////////////////////////
