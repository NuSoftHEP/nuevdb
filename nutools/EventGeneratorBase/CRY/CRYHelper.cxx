////////////////////////////////////////////////////////////////////////
/// \file  CRYHelper.cxx
/// \brief Implementation of an interface to the CRY cosmic-ray generator.
///
/// \version $Id: CRYHelper.cxx,v 1.27 2012-10-15 20:46:42 brebel Exp $
/// \author messier@indiana.edu
////////////////////////////////////////////////////////////////////////
#include <cmath>
#include <iostream>

// CRY include files
#include "CRYSetup.h"
#include "CRYParticle.h"
#include "CRYGenerator.h"

// ROOT include files
#include "TDatabasePDG.h"
#include "TLorentzVector.h"
#include "TGeoManager.h"

// Framework includes
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

// NuTools include files
#include "nutools/EventGeneratorBase/evgenbase.h"
#include "nutools/EventGeneratorBase/CRY/CRYHelper.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"

namespace evgb{

  //......................................................................
  CRYHelper::CRYHelper() 
  {
  }

  //......................................................................
  CRYHelper::CRYHelper(fhicl::ParameterSet     const& pset, 
		       CLHEP::HepRandomEngine&        engine,
		       std::string             const& worldVol)
    : fSampleTime     (pset.get< double      >("SampleTime")            )
    , fToffset        (pset.get< double      >("TimeOffset")            )
    , fEthresh        (pset.get< double      >("EnergyThreshold")       )
    , fWorldVolume    (worldVol)
    , fLatitude       (pset.get< std::string >("Latitude")              )
    , fAltitude       (pset.get< std::string >("Altitude")              )
    , fSubBoxL        (pset.get< std::string >("SubBoxLength")          )
    , fBoxDelta       (pset.get< double      >("WorldBoxDelta", 1.e-5)  )
    , fSingleEventMode(pset.get< bool        >("GenSingleEvents", false))
  {    
    // Construct the CRY generator
    std::string config("date 1-1-2014 ");

    // all particles are turned on by default.  have to have trailing space if 
    // configured in .fcl file
    config += pset.get< std::string >("GammaSetting",    "returnGammas    1 ");
    config += pset.get< std::string >("ElectronSetting", "returnElectrons 1 ");
    config += pset.get< std::string >("MuonSetting",     "returnMuons     1 ");
    config += pset.get< std::string >("PionSetting",     "returnPions     1 ");
    config += pset.get< std::string >("NeutronSetting",  "returnNeutrons  1 ");
    config += pset.get< std::string >("ProtonSetting",   "returnProtons   1 ");
    config += fLatitude;
    config += fAltitude;
    config += fSubBoxL;

    // Find the pointer to the CRY data tables
    std::string crydatadir;
    const char* datapath = getenv("CRYDATAPATH");
    if( datapath != 0) crydatadir = datapath;
    else{
      mf::LogError("CRYHelper") << "no variable CRYDATAPATH set for cry data location, bail";
      exit(0);
    }
      
    // Construct the event generator object
    fSetup = new CRYSetup(config, crydatadir);

    RNGWrapper<CLHEP::HepRandomEngine>::set(&engine, &CLHEP::HepRandomEngine::flat);

    fSetup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);

    fGen = new CRYGenerator(fSetup);

  }  

  //......................................................................
  CRYHelper::~CRYHelper() 
  {
    delete fGen;
    delete fSetup;
  }

  //......................................................................
  double CRYHelper::Sample(simb::MCTruth&      mctruth, 
			   double      const& surfaceY,
			   double      const& detectorLength,
			   double*            w, 
			   double             rantime)
  {
    // Generator time at start of sample
    double tstart = fGen->timeSimulated();
    int    idctr = 1;
    bool particlespushed = false;
    while (1) {
      std::vector<CRYParticle*> parts;
      fGen->genEvent(&parts);
      for (unsigned int i=0; i<parts.size(); ++i) {
	
	// Take ownership of the particle from the vector
	std::unique_ptr<CRYParticle> cryp(parts[i]);
	
	// Pull out the PDG code
	int pdg = cryp->PDGid();
	
	// Get the energies of the particles
	double ke = cryp->ke()*1.0E-3; // MeV to GeV conversion
	if (ke<fEthresh) continue;
	
	double m    = 0.; // in GeV
	
	static TDatabasePDG*  pdgt = TDatabasePDG::Instance();
	TParticlePDG* pdgp = pdgt->GetParticle(pdg);
	if (pdgp) m = pdgp->Mass();
	
	double etot = ke + m;
	double ptot = etot*etot-m*m;
	if (ptot>0.0) ptot = sqrt(ptot);
	else          ptot = 0.0;
	
	// Sort out the momentum components. Remember that the NOvA
	// frame has y up and z along the beam. So uvw -> zxy
	double px = ptot * cryp->v();
	double py = ptot * cryp->w();
	double pz = ptot * cryp->u();
	
	// Particle start position. CRY distributes uniformly in x-y
	// plane at fixed z, where z is the vertical direction. This
	// requires some offsets and rotations to put the particles at
	// the surface in the geometry as well as some rotations
	// since the coordinate frame has y up and z along the
	// beam.
	double vx = cryp->y()*100.0;
	double vy = cryp->z()*100.0 + surfaceY;
	double vz = cryp->x()*100.0 + 0.5*detectorLength;
	double t  = cryp->t()-tstart + fToffset; // seconds
	if(fSingleEventMode) t  = fSampleTime*rantime; // seconds

	// Project backward to edge of world volume
	double xyz[3]  = { vx,  vy,  vz};
	double xyzo[3];
	double dxyz[3] = {-px, -py, -pz};
	double x1 = 0.;
	double x2 = 0.;
	double y1 = 0.;
	double y2 = 0.;
	double z1 = 0.;
	double z2 = 0.;
	this->WorldBox(&x1, &x2, &y1, &y2, &z1, &z2);
	
	LOG_DEBUG("CRYHelper") << xyz[0] << " " << xyz[1] << " " << xyz[2] << " " 
			       << x1 << " " << x2 << " " 
			       << y1 << " " << y2 << " " 
			       << z1 << " " << z2;
	
	this->ProjectToBoxEdge(xyz, dxyz, x1, x2, y1, y2, z1, z2, xyzo);
	
	vx = xyzo[0];
	vy = xyzo[1];
	vz = xyzo[2];
	
	// Boiler plate...
	int istatus    =  1;
	int imother1   = kCosmicRayGenerator;
	
	// Push the particle onto the stack
	std::string primary("primary");
	
	particlespushed=true;
	simb::MCParticle p(idctr,
			   pdg,
			   primary,
			   imother1,
			   m,
			   istatus);
	TLorentzVector pos(vx,vy,vz,t*1e9);// time needs to be in ns to match GENIE, etc
	TLorentzVector mom(px,py,pz,etot);
	p.AddTrajectoryPoint(pos,mom);
	
	mctruth.Add(p);
	++idctr;
      } // Loop on particles in event

      // Check if we're done with this time sample
      // note that now requiring npart==1 in singlevent mode.
      
      if (fGen->timeSimulated()-tstart > fSampleTime || 
	  (fSingleEventMode && particlespushed ) 
	  ) break;    
    } // Loop on events simulated
    
    mctruth.SetOrigin(simb::kCosmicRay);

    /// \todo Check if this time slice passes selection criteria
    if (w) *w = 1.0;
    return fGen->timeSimulated()-tstart;

  }

  ///----------------------------------------------------------------
  ///
  /// Return the ranges of x,y and z for the "world volume" that the
  /// entire geometry lives in. If any pointers are 0, then those
  /// coordinates are ignored.
  ///
  /// \param xlo : On return, lower bound on x positions
  /// \param xhi : On return, upper bound on x positions
  /// \param ylo : On return, lower bound on y positions
  /// \param yhi : On return, upper bound on y positions
  /// \param zlo : On return, lower bound on z positions
  /// \param zhi : On return, upper bound on z positions
  ///
  void CRYHelper::WorldBox(double* xlo, double* xhi,
			   double* ylo, double* yhi,
			   double* zlo, double* zhi) const  
  {
    const TGeoShape* s = gGeoManager->GetVolume(fWorldVolume.c_str())->GetShape();
    if(!s)
      throw cet::exception("CRYHelper") << "No TGeoShape found for world volume";
    
    if (xlo || xhi) {
      double x1, x2;
      s->GetAxisRange(1,x1,x2); if (xlo) *xlo = x1; if (xhi) *xhi = x2;
    }
    if (ylo || yhi) {
      double y1, y2;
      s->GetAxisRange(2,y1,y2); if (ylo) *ylo = y1; if (yhi) *yhi = y2;
    }
    if (zlo || zhi) {
      double z1, z2;
      s->GetAxisRange(3,z1,z2); if (zlo) *zlo = z1; if (zhi) *zhi = z2;
    }
  }// end of WorldBox

  ///----------------------------------------------------------------
  /// Project along a direction from a particular starting point to the
  /// edge of a box
  ///
  /// \param xyz    - The starting x,y,z location. Must be inside box.
  /// \param dxyz   - Direction vector
  /// \param xlo    - Low edge of box in x
  /// \param xhi    - Low edge of box in x
  /// \param ylo    - Low edge of box in y
  /// \param yhi    - Low edge of box in y
  /// \param zlo    - Low edge of box in z
  /// \param zhi    - Low edge of box in z
  /// \param xyzout - On output, the position at the box edge
  ///
  /// Note: It should be safe to use the same array for input and
  /// output.
  ///
  void CRYHelper::ProjectToBoxEdge(const double xyz[],
				   const double dxyz[],
				   double &xlo, double &xhi,
				   double &ylo, double &yhi,
				   double &zlo, double &zhi,
				   double xyzout[])
  {
    // make the world box slightly smaller so that the projection to 
    // the edge avoids possible rounding errors later on with Geant4
    xlo += fBoxDelta;
    xhi -= fBoxDelta;
    ylo += fBoxDelta;
    yhi -= fBoxDelta;
    zlo += fBoxDelta;
    zhi -= fBoxDelta;

    // Make sure we're inside the box!
    if(xyz[0] < xlo || xyz[0] > xhi ||
       xyz[1] < ylo || xyz[1] > yhi || 
       xyz[2] < zlo || xyz[2] > zhi)
      throw cet::exception("CRYHelper") << "Projection to edge is outside"
					<< " bounds of world box:\n "
					<< "\tx: " << xyz[0] << " ("
					<< xlo << "," << xhi << ")\n"
					<< "\ty: " << xyz[1] << " ("
					<< ylo << "," << yhi << ")\n"
					<< "\tz: " << xyz[2] << " ("
					<< zlo << "," << zhi << ")";

    // Compute the distances to the x/y/z walls
    double dx = 99.E99;
    double dy = 99.E99;
    double dz = 99.E99;
    if      (dxyz[0] > 0.0) { dx = (xhi-xyz[0])/dxyz[0]; }
    else if (dxyz[0] < 0.0) { dx = (xlo-xyz[0])/dxyz[0]; }
    if      (dxyz[1] > 0.0) { dy = (yhi-xyz[1])/dxyz[1]; }
    else if (dxyz[1] < 0.0) { dy = (ylo-xyz[1])/dxyz[1]; }
    if      (dxyz[2] > 0.0) { dz = (zhi-xyz[2])/dxyz[2]; }
    else if (dxyz[2] < 0.0) { dz = (zlo-xyz[2])/dxyz[2]; }
    
    // Choose the shortest distance
    double d = 0.0;
    if      (dx < dy && dx < dz) d = dx;
    else if (dy < dz && dy < dx) d = dy;
    else if (dz < dx && dz < dy) d = dz;
    
    // Make the step
    for (int i = 0; i < 3; ++i) {
      xyzout[i] = xyz[i] + dxyz[i]*d;
    }
  }
}
////////////////////////////////////////////////////////////////////////
