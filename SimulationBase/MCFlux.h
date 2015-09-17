////////////////////////////////////////////////////////////////////////
/// \file  MCFlux.h
/// \brief object containing MC flux information
///
/// \version $Id: MCFlux.h,v 1.2 2012-10-15 20:36:27 brebel Exp $
/// \author  brebel@fnal.gov
////////////////////////////////////////////////////////////////////////
#ifndef SIMB_MCFLUX_H
#define SIMB_MCFLUX_H

#include <iostream>
#include <vector>

namespace simb{

  /// Which flux was used to generate this event?
  enum flux_code_{
    kHistPlusFocus  = +1, ///< Flux for positive horn focus
    kHistMinusFocus = -1, ///< Flux for negative horn focus
    kGenerator      =  0, ///< A bogus flux assumed by the generator
    kNtuple         =  2, ///< Full flux simulation ntuple
    kSimple_Flux    =  3  ///< A simplified flux ntuple for quick running
  };

  class MCFlux {

  public:

    MCFlux();

    // maintained variable names from gnumi ntuples
    // see http://www.hep.utexas.edu/~zarko/wwwgnumi/v19/[/v19/output_gnumi.html]

    int    frun;
    int    fevtno;
    double fndxdz;
    double fndydz;
    double fnpz;
    double fnenergy;
    double fndxdznea;
    double fndydznea;
    double fnenergyn;
    double fnwtnear;
    double fndxdzfar;
    double fndydzfar;
    double fnenergyf;
    double fnwtfar;
    int    fnorig;
    int    fndecay;
    int    fntype;
    double fvx;
    double fvy;
    double fvz;
    double fpdpx;
    double fpdpy;
    double fpdpz;
    double fppdxdz;
    double fppdydz;
    double fpppz;
    double fppenergy;
    int    fppmedium;
    int    fptype;     // converted to PDG
    double fppvx;
    double fppvy;
    double fppvz;
    double fmuparpx;
    double fmuparpy;
    double fmuparpz;
    double fmupare;
    double fnecm;
    double fnimpwt;
    double fxpoint;
    double fypoint;
    double fzpoint;
    double ftvx;
    double ftvy;
    double ftvz;
    double ftpx;
    double ftpy;
    double ftpz;
    int    ftptype;   // converted to PDG
    int    ftgen;
    int    ftgptype;  // converted to PDG
    double ftgppx;
    double ftgppy;
    double ftgppz;
    double ftprivx;
    double ftprivy;
    double ftprivz;
    double fbeamx;
    double fbeamy;
    double fbeamz;
    double fbeampx;
    double fbeampy;
    double fbeampz;    

    simb::flux_code_ fFluxType;

    double fgenx;     ///< origin of ray from flux generator 
    double fgeny;
    double fgenz;
    double fdk2gen;   ///< distance from decay to ray origin
    double fgen2vtx;  ///< distance from ray origin to event vtx

   private:

    float fFluxPos[6]; ///< e,ebar,mu,mubar,tau,taubar flux, +horn focus
    float fFluxNeg[6]; ///< Fluxes as aboce, for negative horn focus
    float fFluxGen[6]; ///< Fluxes as above, assumed by generator

#ifndef __GCCXML__

  public:
    
    void Reset();
    double Flux(int pdgcode, 
                int which=0) const;
    void SetFluxPos(double nue,  double nuebar,
                    double numu, double numubar,
                    double nutau,double nutaubar);
    void SetFluxNeg(double nue,  double nuebar,
                    double numu, double numubar,
                    double nutau,double nutaubar);
    void SetFluxGen(double nue,  double nuebar,
                    double numu, double numubar,
                    double nutau,double nutaubar);
    
    void ReDecay(double &newE, 
                 double &newW, 
                 double x, 
                 double y, 
                 double z);

    friend std::ostream& operator<< (std::ostream& output, const simb::MCFlux &mcflux);
    
#endif

  };
}

#endif //SIMB_MCFLUX_H
