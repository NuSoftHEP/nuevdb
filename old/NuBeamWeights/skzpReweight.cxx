////////////////////////////////////////////////////////////////////////
/// \file Conventions.h
///
/// \version $Id: skzpReweight.cxx,v 1.1.1.1 2011-01-27 19:06:32 p-nusoftart Exp $
/// \author  ???
////////////////////////////////////////////////////////////////////////
#include <cmath>
#include <iostream>
#include <cstdlib>
#include "TFile.h"
#include "TH2F.h"
#include "TSystem.h"
#include "nutools/NuBeamWeights/skzpReweight.h"

namespace nbw{

  //......................................................................
  skzpReweight::skzpReweight(std::string fpath, std::string bpath, int flag)
  {
    //default parameters specified by minos-doc-7146
    fFPar.push_back(1.56);
    fFPar.push_back(-6.42);
    fFPar.push_back(1.11);
    fFPar.push_back(0.13);
    fFPar.push_back(1.00);
    fFPar.push_back(1.25);
    fFPar.push_back(3.50);
    fFPar.push_back(4.83);
    fFPar.push_back(1.51);
    fFPar.push_back(0.29);
    fFPar.push_back(0.97);
    fFPar.push_back(2.16);
    fFPar.push_back(1.04);
    fFPar.push_back(-0.89);
    fFPar.push_back(0.88);
    fFPar.push_back(0.05);

    fBPar.push_back(-3.85);
    fBPar.push_back(1.39);

    fFpath = fpath;
    fBpath = bpath;
    fBflag = flag;
    FlukConfig();
    if (fBflag>0)
      BeamConfig();
  }

  //......................................................................
  skzpReweight::~skzpReweight()
  {
    //Deconstructs members for Fluk
    for (std::vector<Conventions::ParticleType_t>::iterator itPlist=fPlist.begin();itPlist!=fPlist.end(); itPlist++)
    {
      delete fWeightHist[*itPlist];
      delete fPTPZ[*itPlist];
      delete fWeightedPTPZ[*itPlist];
    }
    fWeightHist.clear();
    fPTPZ.clear();
    fWeightedPTPZ.clear();
    fMeanPT.clear();
    fWeightedMeanPT.clear();
    fN.clear();
    fNWeighted.clear();
    fPlist.clear();
    fFPar.clear();
    fNBinsX.clear();
    fNBinsY.clear();
  }

  //......................................................................
  double skzpReweight::GetFlukWeight(int ptype, double pT, double pz)
  {
    double weight=1.;
    double xF=pz/120.;
    double A,B,C;
    double Ap,Bp,Cp;
    Conventions::ParticleType_t eptype=GeantToEnum(ptype);

    // This is SKZP parameterization
    if (xF>1.||xF<0.) return weight;
    if (pT>1.||pT<0.) return weight;

    if (eptype==Conventions::kPiPlus || eptype==Conventions::kPiMinus)
    {
      //Calculate weight for pions
      if (pT<0.03) // for low pT
	pT=0.03;
      // calculate the A, B and C in SKZP parameterization
      // A, B and C are best fit to Fluka 05
      
      A=-0.00761*pow((1.-xF),4.045)*(1.+9620.*xF)*pow(xF,-2.975);
      B=0.05465*pow((1.-xF),2.675)*(1.+69590.*xF)*pow(xF,-3.144);
      
      if (xF<0.22)
	C=-7.058/pow(xF,-0.1419)+9.188;
      else
	C = (3.008/exp((xF-0.1984)*3.577)) + 2.616*xF+0.1225;
      
      // scale/skew A, B and C
      Ap=(fFPar[0]+fFPar[1]*xF)*A;
      Bp=(fFPar[2]+fFPar[3]*xF)*B;
      Cp=(fFPar[4]+fFPar[5]*xF)*C;
      
      weight=(Ap+Bp*pT)/(A+B*pT)*exp(-(Cp-C)*pow(pT,3./2.));
      if(eptype == Conventions::kPiMinus && pz > 4)
	weight *= ( fFPar[12] + fFPar[13] * xF );
    }
    else if (eptype==Conventions::kKPlus || eptype==Conventions::kKMinus)
    {
      //Calculate weight for kaons
      if (pT<0.05) // for low pT
	pT=0.05;
      // calculate the A, B and C in SKZP parameterization
      // A, B and C are best fit to Fluka 05
          
      A=-0.005187*pow((1.-xF),4.119)*(1.+2170.*xF)*pow(xF,-2.767);
      B=0.4918*pow((1.-xF),2.672)*(1.+1373.*xF)*pow(xF,-2.927);
      
      if (xF<0.22)
	C=-16.10/pow(xF,-0.04582)+17.92;
      else
	C = (6.905/exp((xF+0.163)*6.718)) - 0.4257*xF + 2.486;
	  
      // scale/skew A, B and C
      Ap=(fFPar[6]+fFPar[7]*xF)*A;
      Bp=(fFPar[8]+fFPar[9]*xF)*B;
      Cp=(fFPar[10]+fFPar[11]*xF)*C;
	  
      weight=(Ap+Bp*pT)/(A+B*pT)*exp(-(Cp-C)*pow(pT,3./2.));
      if(eptype==Conventions::kKMinus)
	weight*=(fFPar[14]+fFPar[15]*xF);
    }
    else if (eptype==Conventions::kK0L)
    {
      //N(K0L) approximately given with (N(K+)+3*N(K-))/4
      weight= ((fNWeighted[Conventions::kKPlus]+3.*fNWeighted[Conventions::kKMinus])/(fN[Conventions::kKPlus]+3.*fN[Conventions::kKMinus]));
    }
    
    if (weight>10.) weight=10.;
    return weight;
  }

  //......................................................................
  void skzpReweight::FlukConfig()
  {
    std::cout <<"FlukConfig reading data from: "<<fFpath<<std::endl;

    //Builds maps and histograms for Fluk from file
    //In fluka05ptxf.root, file and histogram names refer to xf, but they actually mean pz, which is proportional to xF:The conversion takes place in this code and not the histogram.

    TFile* hFile=new TFile(fFpath.c_str());
    
    fPlist.push_back(Conventions::kPiPlus);
    fPlist.push_back(Conventions::kPiMinus);
    fPlist.push_back(Conventions::kKPlus);
    fPlist.push_back(Conventions::kKMinus);
    fPlist.push_back(Conventions::kK0L);
  
    for (std::vector<Conventions::ParticleType_t>::iterator itPlist=fPlist.begin();itPlist!=fPlist.end(); itPlist++)
    {
      TH2F* hist=dynamic_cast <TH2F*> (hFile->Get(Form("hF05ptxf%s",PartEnumToString(*itPlist).c_str()))->Clone());
      hist->SetDirectory(0);
      TH2F* hist2=dynamic_cast <TH2F*> (hist->Clone(Form("hWeightedPTXF%s",PartEnumToString(*itPlist).c_str())));
      hist2->SetDirectory(0);
      hist2->SetTitle(Form("%s weighted pt-pz",PartEnumToString(*itPlist).c_str()));
      fWeightHist[*itPlist]=dynamic_cast<TH2F*> (hist->Clone(Form("hWeight%s",PartEnumToString(*itPlist).c_str())));

      fWeightHist[*itPlist]->Divide(hist2);
      fWeightHist[*itPlist]->SetDirectory(0);

      std::pair<Conventions::ParticleType_t, TH2F* > p(*itPlist, hist);
      std::pair<Conventions::ParticleType_t, TH2F* > p2(*itPlist, hist2);
      fPTPZ.insert(p);
      fWeightedPTPZ.insert(p2);

      fPTPZ[*itPlist]->SetDirectory(0);
      fWeightedPTPZ[*itPlist]->SetDirectory(0);

      fMeanPT[*itPlist]=fPTPZ[*itPlist]->ProjectionY()->GetMean()*1000.;
      fWeightedMeanPT[*itPlist]=fWeightedPTPZ[*itPlist]->ProjectionY()->GetMean()*1000.;
            
      double N=fPTPZ[*itPlist]->ProjectionY()->GetSumOfWeights();
      double wN=fWeightedPTPZ[*itPlist]->ProjectionY()->GetSumOfWeights();
    
      fN[*itPlist]=N;
      fNWeighted[*itPlist]=wN;
    }
    hFile->Close();
    delete hFile;
    hFile=0;
    //Updates maps and histograms for Fluk with GetFlukWeight (the weight for KOL depends on fN and fNWeighted for KPlus and KMinus).
    for (std::vector<Conventions::ParticleType_t>::iterator itPlist=fPlist.begin(); itPlist!=fPlist.end(); itPlist++)
    {
      double pt,pz;
      double meanpt(0.);
      double sum(0.);
      for (int i=0;i<fPTPZ[*itPlist]->GetNbinsX()+1;i++)
      {
	for (int j=0;j<fPTPZ[*itPlist]->GetNbinsY()+1;j++)
        {
	  pz=fPTPZ[*itPlist]->GetXaxis()->GetBinCenter(i);
	  pt=fPTPZ[*itPlist]->GetYaxis()->GetBinCenter(j);
	  fWeightedPTPZ[*itPlist]->SetBinContent(i,j,(fPTPZ[*itPlist]->GetBinContent(i,j)*GetFlukWeight(*itPlist,pt,pz)));
	  fWeightHist[*itPlist]->SetBinContent(i,j,GetFlukWeight(*itPlist,pt,pz));
	  meanpt+=fWeightedPTPZ[*itPlist]->GetBinContent(i,j)*pt;
	  sum+=fWeightedPTPZ[*itPlist]->GetBinContent(i,j);
	}
      }
      meanpt/=sum;
      fWeightedMeanPT[*itPlist]=meanpt*1000.; //GeV to MeV
      fNWeighted[*itPlist]=sum;
      meanpt=0.;
      sum=0.;
    }
    return;
  }

  //......................................................................
  double skzpReweight::GetBeamWeight(int ntype, double Enu, int det, int beam)
  {
    double weight=1.;
    
    if (ntype == 14) ntype = 56;
    else if (ntype == -14) ntype = 55;
    else if (ntype == 12) ntype = 53;
    else if (ntype == -12) ntype = 52;

    struct mapkey dex;
    dex.NuDex=ntype;
    dex.BeamDex=beam;
    dex.DetDex=det;

    for(int sys = 1; sys <= 2; ++sys){//Loops through each beam-focusing correction applied
      double w = 0.;
      dex.EffDex=sys;
      
      std::map<mapkey, WeightMap_t, LessThan>::iterator dexit = fBeamSysMap.find(dex);
      if (dexit != fBeamSysMap.end()){
	for (WeightMap_t::iterator EnDex = (dexit->second).begin();  EnDex!=(dexit->second).end(); EnDex++){
	  if(EnDex->first > Enu){//the first EnDex that goes over is mapped to the weight value
	    w = EnDex->second;
	    break;
	  }
	}
      }
      weight *= std::abs(w)*fBPar[sys-1]+1.;
    }    
    return weight;
  }

  //......................................................................
  void skzpReweight::BeamConfig()
  {
    std::cout <<"BeamConfig reading data from: "<<fBpath<<std::endl;
  
    bool FoundHist = false;
    TDirectory *save = gDirectory;
    fBeamSysFile = new TFile(fBpath.c_str());
    if (fBeamSysFile->IsZombie()){
      std::cout << "Don't recognize path: " << fBpath << std::endl;
      return;
    }

    int ntype[]    ={    56,       55  , 53  , 52};
    for (int inu = 0; inu < 4; ++inu) {
      //'End' enums are there so one can just change conventions when needed.
      for (int eff=1;eff<Conventions::kBeamSysEnd;eff++) {
	for (int beam=1;beam<Conventions::kBeamEnd;beam++) {
	  for (int det=1;det<Conventions::kDetEnd;det++) {
	    std::string hname = GetHname(inu,eff,beam,det);
	    //I know this is ugly, but it works and I don't know why the dynamic_cast does not work like it is supposed to. I'll fix it later.
	    TH1D* hist=dynamic_cast<TH1D*> (fBeamSysFile->Get(hname.c_str()));
	    if (hist)
	    {
	      FoundHist = true;
	      if (det!=Conventions::kMINOSrat || det!=Conventions::kNOvArat)
		FillVector(hist,ntype[inu],eff,beam,det);
	      else //for far/near error bar internally use kUnknownDet
		FillVector(hist,ntype[inu],eff,beam,Conventions::kUnknownDet);
	    }
	    TH1F* hist2=dynamic_cast<TH1F*> (fBeamSysFile->Get(hname.c_str()));
	    if (hist2)
	    {
	      FoundHist = true;
	      if (det!=Conventions::kMINOSrat || det!=Conventions::kNOvArat)
		FillVector(hist2,ntype[inu],eff,beam,det);
	      else //for far/near error bar internally use kUnknownDet
		FillVector(hist2,ntype[inu],eff,beam,Conventions::kUnknownDet);
	    }
	  }
	}
      }
    }
    if (!FoundHist)
      std::cout<<"No histograms found in " << fBpath <<  " with name formatted as expected by flag: " << fBflag << std::endl;
    fBeamSysFile->Close();
    delete fBeamSysFile;
    fBeamSysFile = 0;
    save->cd();
    return;
  }

  //......................................................................
  std::string skzpReweight::GetHname(int inu, int eff, int beam, int det)
  {
    std::string hname;
    if (fBflag == 1 || fBflag == 2)
    {
      std::string nus[]={"NuMu","NuMuBar","NuE","NuEBar"};
      hname=nus[inu]+"_";
    }
    hname=hname + BeamSysToString(Conventions::BeamSys_t(eff));
    if (fBflag == 1 || fBflag == 2) hname = hname + "_";
    hname=hname + BeamTypeToString(Conventions::BeamType_t(beam));
    if (fBflag == 1 || fBflag == 2) hname = hname + "_";
    hname=hname + DetTypeToString(Conventions::DetType_t(det));
    return hname;
  }

  //......................................................................
  void skzpReweight::FillVector(TH1D* hist, int ntype, int eff, int beam, int det)
  { 
    mapkey dex;
    dex.NuDex=ntype;
    dex.EffDex=eff;
    dex.BeamDex=beam;
    dex.DetDex=det;

    std::map<mapkey, WeightMap_t, LessThan>::iterator dexit = fBeamSysMap.find(dex);
    if (dexit == fBeamSysMap.end())
    {
      WeightMap_t wmap;
      //Endex is the upper-edge for the energy of each bin
      double EnDex=0;
      for (int i=1; i<=hist->GetNbinsX();i++)
      {
	EnDex+=hist->GetBinWidth(i);
	wmap.insert(WeightMap_t::value_type(EnDex,hist->GetBinContent(i)));
      }
      fBeamSysMap.insert(std::map<mapkey, WeightMap_t, LessThan >::value_type(dex, wmap));
    }
    else
      std::cout << "Already loaded this histogram!" << std::endl;

    return;
  }

  //......................................................................
  void skzpReweight::FillVector(TH1F* hist, int ntype, int eff, int beam, int det)
  {
    mapkey dex;
    dex.NuDex=ntype;
    dex.EffDex=eff;
    dex.BeamDex=beam;
    dex.DetDex=det;

    std::map<mapkey, WeightMap_t, LessThan>::iterator dexit = fBeamSysMap.find(dex);
    if (dexit == fBeamSysMap.end())
    {
      WeightMap_t wmap;
      //Endex is the upper-edge for the energy of each bin
      double EnDex=0;
      for (int i=1; i<hist->GetNbinsX()+1;i++)
      {
	EnDex+=hist->GetBinWidth(i);
	wmap.insert(WeightMap_t::value_type(EnDex,hist->GetBinContent(i)));
      }
      fBeamSysMap.insert(std::map<mapkey, WeightMap_t, LessThan >::value_type(dex, wmap));
    }
    else
      std::cout << "Already loaded this histogram!" << std::endl;

    return;
  }

  //......................................................................
  Conventions::ParticleType_t skzpReweight::GeantToEnum(int ptype)
  {
    switch (ptype)
      {
      case 8   : return Conventions::kPiPlus;  break;
      case 211 : return Conventions::kPiPlus;  break;    
      case 9   : return Conventions::kPiMinus; break;
      case -211: return Conventions::kPiMinus; break;
      case 11  : return Conventions::kKPlus;   break;
      case 321 : return Conventions::kKPlus;   break;
      case 12  : return Conventions::kKMinus;  break;
      case -321: return Conventions::kKMinus;  break;
      case 10  : return Conventions::kK0L;     break;
      case 130 : return Conventions::kK0L;     break;
      default  : return Conventions::kUnknown; break;
      }
    return Conventions::kUnknown;
  }

  std::string skzpReweight::PartEnumToString(Conventions::ParticleType_t ptype)
  {
    switch (ptype)
      {
      case Conventions::kPiPlus : return "PiPlus";  break;
      case Conventions::kPiMinus: return "PiMinus"; break;
      case Conventions::kKPlus  : return "KPlus";   break;
      case Conventions::kKMinus : return "KMinus";  break;
      case Conventions::kK0L    : return "K0L";     break;
      case Conventions::kUnknown: return "Unknown"; break;
      default                   : return "Unknown"; break;
      }
    return "Unknown";
  }

  std::string skzpReweight::BeamSysToString(Conventions::BeamSys_t bstype)
  {
    if(fBflag>=0 || fBflag<=2)
    {
      switch (bstype)
	{
	case Conventions::kHornIMiscal: return "HornIMiscal"; break;
	case Conventions::kHornIDist  : return "HornIDist";   break;
	case Conventions::kUnknownEff : return "Unknown";     break;
	default                       : return "Unknown";     break;
	}
    }
    return "Unknown";
  }

  std::string skzpReweight::BeamTypeToString(Conventions::BeamType_t btype)
  {
    if(fBflag == 0)
    {
      switch (btype)
	{
	case Conventions::kLE         : return "LE";          break;
	case Conventions::kLE010z185i : return "LE010z185i";  break;
	case Conventions::kLE100z200i : return "LE100z200i";  break;
	case Conventions::kLE250z200i : return "LE250z200i";  break;
	case Conventions::kLE010z185iL: return "LE010z185iL"; break;
	case Conventions::kLE010z170i : return "LE010z170i";  break;
	case Conventions::kLE010z200i : return "LE010z200i";  break;
	case Conventions::kLE010z000i : return "LE010z000i";  break;
	case Conventions::kLE150z200i : return "LE150z200i";  break;
	case Conventions::kUnknownBeam: return "Unknown";     break;
	default          : return "Unknown";     break;
	}
    }
    else if (fBflag == 1 || fBflag == 2)
    {
      switch (btype)
	{
	case Conventions::kLE         : return "L";                break;
	case Conventions::kLE010z185i : return "L010z185i";        break;
	case Conventions::kLE100z200i : return "L100z200i";        break;
	case Conventions::kLE250z200i : return "L250z200i";        break;
	case Conventions::kLE010z185iL: return "L010z185i_lowint"; break;
	case Conventions::kLE010z170i : return "L010z170i";        break;
	case Conventions::kLE010z200i : return "L010z200i";        break;
	case Conventions::kLE010z000i : return "L010z000i";        break;
	case Conventions::kLE150z200i : return "L150z200i";        break;
	case Conventions::kUnknownBeam: return "Unknown";          break;
	default                       : return "Unknown";          break;
	}
    }
    return "Unknown";
  }

  std::string skzpReweight::DetTypeToString(Conventions::DetType_t dtype)
  {
    if(fBflag == 1)
    {
      switch (dtype)
	{
	case Conventions::kMINOSnd   : return "Near";    break;
	case Conventions::kMINOSfd   : return "Far";     break;
	case Conventions::kMINOSrat  : return "FN";      break;
	case Conventions::kUnknownDet: return "Unknown"; break;
	default                      : return "Unknown"; break;
	}
    }
    else if (fBflag == 0 || fBflag == 2)
    {
      switch (dtype)
	{
	case Conventions::kNOvAnd    : return "NOvAnd";   break;
	case Conventions::kNOvAfd    : return "NOvAfd";   break;
	case Conventions::kIPND      : return "IPND";     break;
	case Conventions::kMINOSnd   : return "MINOSnd";  break;
	case Conventions::kMINOSfd   : return "MINOSfd";  break;
	case Conventions::kNOvArat   : return "NOvArat";  break;
	case Conventions::kMINOSrat  : return "MINOSrat"; break;
	case Conventions::kUnknownDet: return "Unknown";  break;
	default                      : return "Unknown";  break;
	}
    }
    return "Unknown";
  }


}
