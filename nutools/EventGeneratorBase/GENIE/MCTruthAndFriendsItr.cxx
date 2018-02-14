#include "MCTruthAndFriendsItr.h"

#include <iostream>
#include <iomanip>

#ifndef ART_V1
  #include "canvas/Persistency/Common/FindOneP.h"
  #include "canvas/Utilities/InputTag.h"
#else
  #include "art/Framework/Core/FindOneP.h"
  #include "art/Utilities/InputTag.h"
#endif


evgb::MCTruthAndFriendsItr::MCTruthAndFriendsItr(art::Event const & evtIn,
                                                 std::vector<std::string> const & labels)
  : evt(evtIn)
  , fInputModuleLabels(labels)
  , indx_itr(indices.begin())
  , nmctruth(0)
  , imctruth(-1)
  , thisMCTruth(0)
  , thisGTruth(0)
  , thisMCFlux(0)
  , thisDk2Nu(0)
  , thisNuChoice(0)
  , thisLabel("")
{

  // look for any existing MCTruth info in this event
  mclists.clear();

  if ( fInputModuleLabels.size()==0) {
    evt.getManyByType(mclists);
    // std::cout << "evt.getManyByType" << std::endl;
  } else {
    mclists.resize(fInputModuleLabels.size());
    for (size_t i=0; i<fInputModuleLabels.size(); ++i) {
      evt.getByLabel(fInputModuleLabels[i],mclists[i]);
      //std::cout << "evt.getByLabel " << fInputModuleLabels[i] << std::endl;
    }
  }

  //std::cerr << "evg::GENIEDumper::analyze got stuff ---------------- "
  //          << evt.id() << std::endl;

  for (size_t mcl = 0; mcl < mclists.size(); ++mcl) {
    art::Handle< std::vector<simb::MCTruth> > mclistHandle = mclists[mcl];
    if ( ! mclistHandle.isValid() ) {
      // std::cerr << "not valid mcl=" <<  mcl << "---------------- " << std::endl;
      continue;
    }
    // processName, moduleLabel, instance (productInstanceName?)
    /*
    std::cout << " mclistHandle processName '"
              << mclistHandle.provenance()->processName()  // top of fcl file
              << "' moduleLabel '"
              << mclistHandle.provenance()->moduleLabel()
              << "' productInstanceName '"
              << mclistHandle.provenance()->productInstanceName()
              << "'"
              << std::endl;
    */

    std::string handleLabel = mclistHandle.provenance()->moduleLabel();
    outlabels.push_back(handleLabel);
    /*
    std::cerr << "mcl=" <<  mcl << " '" << handleLabel << "' ---------------- " << std::endl;
    */

    // loop over mctruths in a list
    for(size_t nmc = 0; nmc < mclistHandle->size(); ++nmc) {
      art::Ptr<simb::MCTruth> mct(mclistHandle, nmc);

      std::pair<int,int> ipair(mcl,nmc);
      indices.insert(ipair);

      /**
      std::cout << "+++ mcl " << mcl << "[" << mclists.size() << "] "
                << "nmc " << nmc << "[" << mclistHandle->size() << "] "
                << std::endl;
      std::cout << *(mct.get()) << std::endl;
      ***/
    }

  }

  indx_itr = indices.begin();
  nmctruth = (int)(indices.size());
  //std::cout << ".... found nmctruth " << nmctruth
  //          << " nlabels " << outlabels.size()
  //          << std::endl;

}

bool evgb::MCTruthAndFriendsItr::Next()
{
  ++imctruth;   // started at -1, so first call to Next() prepares us for indx=0
  thisMCTruth  = 0;
  thisGTruth   = 0;
  thisMCFlux   = 0;
  thisDk2Nu    = 0;
  thisNuChoice = 0;
  //std::cerr << "Next() called ... moved to imctruth " << imctruth << std::endl;
  if ( imctruth >= nmctruth ) return false;

  size_t indx_handle = (*indx_itr).first;
  size_t indx_within = (*indx_itr).second;

  thisLabel = outlabels[indx_handle];

  art::Handle< std::vector<simb::MCTruth> > hvMCTruth = mclists[indx_handle];

  /**
  std::cout << "imctruth " << std::setw(3) << imctruth
            << "  [" << indx_handle << "," << indx_within << "]"
            << "  hvMCTruth.isValid() " << hvMCTruth.isValid()
            << " '" << outlabels[indx_handle] << "' "
            << std::endl;
  **/

  thisMCTruth = &(hvMCTruth->at(indx_within));

  // inefficient ... should only need to do this bit for every new
  // Handle ...

  //art::FindOneP<recob::Hit> findSPToHits(fSpacePoints,evt,fSpacePointLabel);
  //const art::Ptr<recob::Hit> hit = findSPToHits.at(iSP);

  try {
    art::FindOneP<simb::GTruth> qgtruth(hvMCTruth,evt,outlabels[indx_handle]);
    const art::Ptr<simb::GTruth> gtruthptr = qgtruth.at(indx_within);
    thisGTruth = gtruthptr.get();
  }
  catch (...) {
    // std::cerr << "no GTruth for this handle" << std::endl;
  }

  try {
    art::FindOneP<simb::MCFlux> qmcflux(hvMCTruth,evt,outlabels[indx_handle]);
    const art::Ptr<simb::MCFlux> mcfluxptr = qmcflux.at(indx_within);
    thisMCFlux = mcfluxptr.get();
  }
  catch (...) {
    // std::cerr << "no MCFlux for this handle" << std::endl;
  }

  try {
    art::FindOneP<bsim::Dk2Nu> qdk2nu(hvMCTruth,evt,outlabels[indx_handle]);
    const art::Ptr<bsim::Dk2Nu> dk2nuptr = qdk2nu.at(indx_within);
    thisDk2Nu = dk2nuptr.get();
  }
  catch (...) {
    // std::cerr << "no bsim::Dk2NU for this handle" << std::endl;
  }

  try {
    art::FindOneP<bsim::NuChoice> qnuchoice(hvMCTruth,evt,outlabels[indx_handle]);
    const art::Ptr<bsim::NuChoice> nuchoiceptr = qnuchoice.at(indx_within);
    thisNuChoice = nuchoiceptr.get();
  }
  catch (...) {
    // std::cerr << "no bsim::NuChoice for this handle" << std::endl;
  }

  //std::cerr << "Next() called ... seeing " << thisMCTruth
  //          << " " << thisGTruth << " " << thisMCFlux << std::endl;

  // so user code looks like
  // evgb::MCTruthAndFriendsItr mcitr(evt,labels);
  // while ( mcitr.Next() ) {
  //    const simb::MCTruth* amctruth = mcitr.GetMCTruth();
  //    const simb::GTruth*  agtruth  = mcitr.GetGTruth();
  //...

  /*
  // loop over lists
  try {
    //art::FindOneP<simb::GTruth> QueryG(mclistHandle,evt,handleLabel);
  }
  catch (art::Exception) {
    //std::cerr << "no GTruth for " << handleLabel << std::endl;
  }
  */

  // move the iterator on
  ++indx_itr;
  return true;
}
