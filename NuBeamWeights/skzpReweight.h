////////////////////////////////////////////////////////////////////////
/// \file skzpReweight.h
///
/// \version $Id: skzpReweight.h,v 1.2 2011-01-29 19:44:13 p-nusoftart Exp $
/// \author  ???
////////////////////////////////////////////////////////////////////////
#ifndef NBW_SKZPREWEIGHT_H
#define NBW_SKZPREWEIGHT_H

#include <string>
#include <map>
#include <vector>
#include "SimulationBase/MCFlux.h"
#include "NuBeamWeights/Conventions.h"

class TFile;
class TH2F;
class TH2D;

///reweighting utility for NuMI beam
namespace nbw{
  
  class skzpReweight{

    public:

    skzpReweight(std::string fpath="/nova/data/flux/SKZPdata/fluka05ptxf.root", std::string bpath="/nova/data/flux/SKZPdata/IPNDhists.root", int flag=2);
    //Bpath="/nova/data/flux/SKZPdata/beamsys_Dogwood1_Daikon07_v2.root ", flag=1 is another working configuration.
    ~skzpReweight();
    void SetParams(std::vector<double> fpar, std::vector<double> bpar) {
      fFPar=fpar; FlukConfig();
      fBPar=bpar; BeamConfig();
      return; };
    void SetFlukParams(std::vector<double> fpar) {
      fFPar=fpar; FlukConfig();
      return; };
    void SetBeamParams(std::vector<double> bpar) {
      fBPar=bpar; BeamConfig();
      return; };
    
    double GetWeight(const simb::MCFlux *mcf, double Enu, int det, int beam){      
      double pt = sqrt(mcf->ftpx*mcf->ftpx + mcf->ftpy*mcf->ftpy);
      return GetFlukWeight(mcf->ftptype,pt,mcf->ftpz)*GetBeamWeight(mcf->fntype,Enu,beam,det); };
    
    double GetFlukWeight(const simb::MCFlux *mcf){
      double pt = sqrt(mcf->ftpx*mcf->ftpx + mcf->ftpy*mcf->ftpy);
      return GetFlukWeight(mcf->ftptype,pt,mcf->ftpz); };

    double GetFlukWeight(int ptype, double pT, double pz);

    double GetBeamWeight(int ntype, double Enu, int det=1, int beam=2);

  private:
    //methods for Fluk
    void FlukConfig();
    Conventions::ParticleType_t GeantToEnum(int ptype);
    std::string PartEnumToString(Conventions::ParticleType_t ptype);
    //members for Fluk
    std::vector<double> fFPar;
    std::string fFpath;
    std::vector<Conventions::ParticleType_t> fPlist;
    std::map<Conventions::ParticleType_t, TH2F* > fPTPZ;
    std::map<Conventions::ParticleType_t, TH2F* > fWeightedPTPZ;
    std::map<Conventions::ParticleType_t, TH2F* > fWeightHist;
    std::map<Conventions::ParticleType_t, double > fMeanPT;
    std::map<Conventions::ParticleType_t, double > fN;
    std::map<Conventions::ParticleType_t, double > fNWeighted;
    std::map<Conventions::ParticleType_t, double > fWeightedMeanPT;
    std::map<Conventions::ParticleType_t, int > fNBinsY,fNBinsX;

    //methods for Beam
    void BeamConfig();
    //Each flag (path location) is a different formatting to histogram names
    std::string GetHname(int inu, int eff, int beam, int det);
    void FillVector(TH1D* hist, int ntype, int eff, int beam, int det);
    void FillVector(TH1F* hist, int ntype, int eff, int beam, int det);
    std::string BeamSysToString(Conventions::BeamSys_t bstype);
    std::string BeamTypeToString(Conventions::BeamType_t btype);
    std::string DetTypeToString(Conventions::DetType_t dtype);
    //mapkey structure for Beam
    //Each combination that makes up a mapkey has a different weight histogram.
    struct mapkey
    {
      int NuDex;
      int DetDex;
      int BeamDex;
      int EffDex;
    };

    struct LessThan
    {
      bool operator()(const mapkey lhs,const mapkey rhs) const
      {
	if (lhs.NuDex < rhs.NuDex)
	  return true;
	else if (lhs.NuDex == rhs.NuDex)
	{
	  if(lhs.DetDex < rhs.DetDex)
	    return true;
	  else if (lhs.DetDex == rhs.DetDex)
	  {
	    if(lhs.BeamDex < rhs.BeamDex)
	      return true;
	    else if (lhs.BeamDex == rhs.BeamDex)
	    {
	      if(lhs.EffDex < rhs.EffDex)
		return true; //(else return false)
	    }
	  }
	}
	return false;
      }
    };
    //members for Beam
    std::vector<double> fBPar;
    std::string fBpath;
    TFile* fBeamSysFile;
    //key for WeightMap_t represents the upper edge of the energy of each bin, the mapped value is the corresponding weight.
    typedef std::map<double, double> WeightMap_t;
    //see struct mapkey above
    std::map<mapkey, WeightMap_t, LessThan > fBeamSysMap;
    //Each flag (path location) is a different formatting to histogram names
    //flag = 1: formatting based on "beamsys_Dogwood1_Daikon07_v2.root"
    //flag = 2: formatting based on "IPNDhists.root" - histograms from ipndfluxerr and histograms names from enum names located in Conventions.
    //See GetHname and *ToString methods for details.
    int fBflag;
  };
}
#endif //NBW_SKZPREWEIGHT_H
