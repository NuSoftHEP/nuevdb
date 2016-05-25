///
/// \file   EditMenu.cxx
/// \brief  The edit pull down menu
/// \author messier@indiana.edu
///
#include "nutools/EventDisplayBase/EditMenu.h"
#include "TGMenu.h"
#include "nutools/EventDisplayBase/ServiceTable.h"

namespace evdb 
{
  EditMenu::EditMenu(TGMenuBar* menubar, 
		     TGMainFrame* /*mf*/)
  {
    fEditMenu = new TGPopupMenu(gClient->GetRoot());

    fDrawingMenu = new TGPopupMenu();
    fExpMenu     = new TGPopupMenu();
    fARTMenu     = new TGPopupMenu();
    
    fEditMenu->AddPopup("Configure &Drawing",             fDrawingMenu);
    fEditMenu->AddPopup("Configure &Experiment Services", fExpMenu);
    fEditMenu->AddPopup("Configure &Art Services",        fARTMenu);

    fLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);

    menubar->AddPopup("&Edit",fEditMenu,fLayout);
  }

  //......................................................................

  EditMenu::~EditMenu() 
  {
    if (fLayout)      { delete fLayout;      fLayout      = 0; }
    if (fEditMenu)    { delete fEditMenu;    fEditMenu    = 0; }
    if (fARTMenu)     { delete fARTMenu;     fARTMenu     = 0; }
    if (fExpMenu)     { delete fExpMenu;     fExpMenu     = 0; }
    if (fDrawingMenu) { delete fDrawingMenu; fDrawingMenu = 0; }
  }

  //....................................................................
  
  void EditMenu::WipeMenu(TGPopupMenu* m) 
  {
    for (unsigned int i=0;;++i) {
      TGMenuEntry* me = m->GetEntry(i);
      if (me) m->DeleteEntry(i);
      else break;
    }
  }

  //....................................................................
  
  void EditMenu::SetServices()
  {
    this->WipeMenu(fDrawingMenu);
    this->WipeMenu(fExpMenu);
    this->WipeMenu(fARTMenu);
    
    const ServiceTable& st = ServiceTable::Instance();
    
    unsigned int i;
    for (i=0; i<st.fServices.size(); ++i) {
      const ServiceTableEntry& ste = st.fServices[i];
      if (ste.fCategory==kDRAWING_SERVICE) {
	fDrawingMenu->AddEntry(ste.fName.c_str(), i);
      }
      else if (ste.fCategory==kEXPERIMENT_SERVICE) {
	fExpMenu->AddEntry(ste.fName.c_str(), i);
      }
      else if (ste.fCategory==kART_SERVICE) {
	fARTMenu->AddEntry(ste.fName.c_str(), i);
      }
      
      fEditMenu->Connect("Activated(Int_t)",
			 "evdb::EditMenu",
			 this,
			 "MenuSelect(int)");
    }
  }

  //......................................................................

  void EditMenu::MenuSelect(int i) 
  {
    ServiceTable::Instance().Edit(i);
  }
  
}// namespace
////////////////////////////////////////////////////////////////////////
