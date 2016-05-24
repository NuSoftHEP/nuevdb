///
/// \file   ParameterSetEditDialog.cxx
/// \brief  Pop-up window for editing parameter sets
/// \author messier@indiana.edu
///
#include "EventDisplayBase/ParameterSetEditDialog.h"
#include <iostream>
#include <sstream>
#include "TROOT.h"
#include "TGTab.h"
#include "TGButton.h"
#include "TGCanvas.h"
#include "TGTableLayout.h"
#include "TGLayout.h"
#include "TGFrame.h"
#include "TGTextEntry.h"
#include "TGListBox.h"
#include "TGDoubleSlider.h"
#include "fhiclcpp/ParameterSet.h"
#include "EventDisplayBase/NavState.h"
#include "EventDisplayBase/ServiceTable.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace evdb;

// Window and row sizes in units of pixels
static const unsigned int kWidth  = 500*11/10;
static const unsigned int kHeight = 500*11/10;
static const unsigned int kRowW   = kWidth-150;
static const unsigned int kRowH   = 18;

//
// Flags to help us decide what sort of parameter we need to build a
// GUI for.
//
static const int kSINGLE_VALUED_PARAM    = 1<<0; // Expect single value
static const int kVECTOR_PARAM           = 1<<1; // Expect multiple values
static const int kVECTOR_OF_VECTOR_PARAM = 1<<2; // Expect multiple values
static const int kHAVE_GUI_TAGS          = 1<<3; // GUI tags are present
static const int kNO_GUI_TAGS            = 1<<4; // GUI tags are not present
static const int kINTEGER_PARAM          = 1<<5; // Force the value to be int
static const int kPARAMETER_SET_PARAM    = 1<<6; // Value is a parameter set itself
//
// The short letter codes for the various GUI objects supported. Also
// provide a list of all possible tags.
//
#define GUITAG static const std::string
GUITAG kTEXT_ENTRY      = "te";  // A text edit box
GUITAG kLIST_BOX_SINGLE = "lbs"; // A list box, single choice allowed
GUITAG kLIST_BOX_MULTI  = "lbm"; // A list box, multuiple choices allowed
GUITAG kRADIO_BUTTONS   = "rb";  // Radio buttons
GUITAG kCHECK_BOX       = "cb";  // Check boxes
GUITAG kSLIDER          = "sl";  // Slider bar
GUITAG kSLIDER_INT      = "sli"; // Slider bar, limit to integers
#undef GUITAG
static const std::vector<std::string> gsGUITAG = {
  kTEXT_ENTRY,
  kLIST_BOX_SINGLE,
  kLIST_BOX_MULTI,
  kRADIO_BUTTONS,
  kCHECK_BOX,
  kSLIDER,
  kSLIDER_INT
};

//======================================================================
//
// ParameterSetEditRow methods
//
ParameterSetEditRow::ParameterSetEditRow(ParameterSetEditFrame* frame,
					 TGHorizontalFrame* lhs,
					 TGHorizontalFrame* rhs,
					 const fhicl::ParameterSet& ps,
					 const std::string& key) :
  fFrame(frame),
  fRightLH(0),
  fLeftLH(0),
  fLabel(0),
  fTextEntry(0),
  fListBox(0),
  fSlider(0),
  fKEY(key)
{
  //
  // Extract information about the parameter for which we are building
  // the GUI
  //
  std::string              tag;    // What sort of frame to build?
  std::vector<std::string> values; // What is the current value?
  this->UnpackParameter(ps, key, fParamFlags, tag, fChoice, values, fGUI, fDOC);
  if (values.empty()){
    // What happened here? We'll crash if we continue though, so bail out.
    return;
  }

  if (fParamFlags&kVECTOR_PARAM) {
    fValue = "[";
    for (unsigned int i=0; i<values.size(); ++i) {
      fValue += values[i];
      if (i+1<values.size()) fValue += ",";
      else                   fValue += "]";
    }
  }
  else if(fParamFlags&kPARAMETER_SET_PARAM){
    fValue  = "{";
    fValue += values[0];
    fValue += "}";
  }
  else {
    fValue = values[0];
  }

  fLeftLH  = new TGLayoutHints(kLHintsLeft, 1,1,0,0);
  fRightLH = new TGLayoutHints(kLHintsRight,1,1,0,0);
  
  fLabel = new TGTextButton(lhs,
			    key.c_str(),
			    -1,
			    TGButton::GetDefaultGC()(),
			    TGTextButton::GetDefaultFontStruct(),
			    0);
  lhs->AddFrame(fLabel);
  fLabel->SetToolTipText(fDOC.c_str());
  fLabel->SetTextJustify(kTextRight);
  
  if (tag==kTEXT_ENTRY) {
    this->SetupTextEntry(rhs, fParamFlags, values);
  }
  if (tag==kLIST_BOX_SINGLE) {
    this->SetupListBox(rhs, fChoice, values, false);
  }
  if (tag==kLIST_BOX_MULTI) {
    this->SetupListBox(rhs, fChoice, values, true);
  }
  if (tag==kRADIO_BUTTONS) {
    this->SetupRadioButtons(rhs, fChoice, values);
  }
  if (tag==kCHECK_BOX) {
    this->SetupCheckButton(rhs, fChoice, values);
  }
  if (tag==kSLIDER) {
    this->SetupSlider(rhs, fChoice, values);
  }
  if (tag==kSLIDER_INT) {
    fParamFlags |= kINTEGER_PARAM;
    this->SetupSlider(rhs, fChoice, values);
  }
}

//......................................................................

ParameterSetEditRow::~ParameterSetEditRow() 
{
  unsigned int i;
  for (i=0; i<fCheckButton.size(); ++i) {
    if (fCheckButton[i]) delete fCheckButton[i];
  }
  for (i=0; i<fRadioButton.size(); ++i) {
    if (fRadioButton[i]) delete fRadioButton[i];
  }
  if (fSlider)    delete fSlider;
  if (fListBox)   delete fListBox;
  if (fTextEntry) delete fTextEntry;
  if (fLeftLH)    delete fLeftLH;
  if (fRightLH)   delete fRightLH;
  if (fLabel)     delete fLabel;
}

//......................................................................

void ParameterSetEditRow::UnpackParameter(const fhicl::ParameterSet& p,
					  const std::string&         key,
					  unsigned int&              flag,
					  std::string&               tag,
					  std::vector<std::string>&  choice,
					  std::vector<std::string>&  value,
					  std::string&               gui,
					  std::string&               doc)
{
  std::string guikey = key; guikey += ".gui";
  std::string dockey = key; dockey += ".doc";
  
  flag = 0;
  
  //
  // Try to extract GUI tags
  //
  try {
    gui = p.get< std::string >(guikey);
    doc = p.get< std::string >(dockey);
    flag |= kHAVE_GUI_TAGS;
  }
  catch (...) { 
    //
    // If they aren't there, try extracting it as a normal
    // parameter. Default to providing the user with a text entry box.
    //
    gui = kTEXT_ENTRY;
    doc = "See .fcl file for documentation...";
    flag |= kNO_GUI_TAGS;
  }
  
  //
  // Parse out the GUI string to find out what type of frame to build
  // and the choices we should present to the user
  //
  ParseGUItag(gui, tag, choice);

  //
  // Now extract the assigned value(s) of the parameter
  //
  // The key is either just the key, or in the case of GUI-enabled
  // parameters the key name with ".val" appended
  //
  std::string valkey = key;
  if ( flag&kHAVE_GUI_TAGS ) valkey += ".val";
  //
  // Try first to extract a single value.
  //
  try {
    std::string v = p.get<std::string>(valkey);
    value.push_back(v);
    flag |= kSINGLE_VALUED_PARAM;
  }
  catch (...) {
    //
    // If that fails, try extracting multiple values
    //
    try {
      value = p.get< std::vector<std::string> >(valkey);
      flag |= kVECTOR_PARAM;
      if (value.size()==0) value.push_back("");
    }
    catch (...) {
      //
      // Yikes - vector of vectors, perhaps?
      //
      try {
	std::vector< std::vector <std::string> > vv;
	vv = p.get<std::vector<std::vector<std::string> > >(valkey);
	//
	// Vectors of vectors are treated as vectors of
	// std::strings. The strings assigned to the values are
	// strings that FHICL will parse as vectors. So, this:
	//
	// [ [0,0], [1,1] ]
	//
	// is represented as:
	//
	// value.size()=2, value[0]="[0,0]", value[1]="[1,1]"
	//
	unsigned int i, j;
	flag |= kVECTOR_PARAM;
	for (i=0; i<vv.size(); ++i) {
	  std::string s;
	  s += "[";
	  for (j=0; j<vv[i].size(); ++j) {
	    s += vv[i][j];
	    if (j+2<vv[i].size()) s += ",";
	    else s += "]";
	  }
	  value.push_back(s);
	}
	if (vv.size()==0) value.push_back("[[]]");
      }
      catch (...) {
	// what about another fhicl::ParameterSet?
	try{
	  fhicl::ParameterSet v = p.get< fhicl::ParameterSet >(valkey);
	  flag |= kPARAMETER_SET_PARAM;
	  value.push_back(v.to_string());
	}
	catch(...){
	  //
	  // If that fails we are very stuck. Print a message and fail.
	  //
	  LOG_ERROR("ParameterSetEditDialog") << "Failed to parse " << key
					      << "\n" << p.to_string();
	}
      }
    }
  }
}

//......................................................................
//
// Parse out what we can from the "gui" tag. Expected format is:
// "frame_tag:choice1,choice2,choice3"
//
void ParameterSetEditRow::ParseGUItag(const std::string&        guitag,
				      std::string&              frame,
				      std::vector<std::string>& choice)
{
  //
  // Get the frame name. Should be piece just before the ":"
  //
  choice.clear();
  size_t icolon = guitag.find(':');
  if (icolon == std::string::npos) frame = guitag;
  else                             frame = guitag.substr(0,icolon);
  if (!IsLegalGUItag(frame))       frame = kTEXT_ENTRY;
  
  //
  // Get the list of choices. Should be comma separated.
  //
  size_t icomma = icolon;
  size_t spos, epos;
  while (icomma!=std::string::npos) {
    spos = icomma+1;
    epos = guitag.find(',',spos);
    std::string s = guitag.substr(spos,epos-spos);
    choice.push_back(s);
    icomma = epos;
  }
}

//......................................................................

bool ParameterSetEditRow::IsLegalGUItag(const std::string& s) 
{
  for(unsigned int i=0; i<gsGUITAG.size(); ++i) {
    if (s==gsGUITAG[i]) return true;
  }
  LOG_ERROR("ParameterSetEditDialog") << s << " is not a legal GUI tag.";
  return false;
}

//......................................................................

void ParameterSetEditRow::SetupTextEntry(TGCompositeFrame* f, 
                                        unsigned int flags,
                                        const std::vector<std::string>& value)
{
  static TColor* c = gROOT->GetColor(41);

  fTextEntry = new TGTextEntry(f);
  f->AddFrame(fTextEntry);
  fTextEntry->SetTextColor(c);

  fTextEntry->Connect("ReturnPressed()",
                      "evdb::ParameterSetEditRow",
                      this,
                      "TextEntryReturnPressed()");
  
  std::string buff;
  if (flags&kVECTOR_PARAM) buff += "[";
  if (flags&kPARAMETER_SET_PARAM) buff += "{";
  for (unsigned int i=0; i<value.size(); ++i) {
    buff += value[i];
    if ((i+1)!=value.size()) buff += ",";
  }
  if (flags&kVECTOR_PARAM) buff += "]";
  if (flags&kPARAMETER_SET_PARAM) buff += "}";
  fTextEntry->SetText(buff.c_str(), 0);
  fTextEntry->Resize(kRowW,kRowH);
}

//......................................................................

void ParameterSetEditRow::SetupListBox(TGCompositeFrame* f, 
                                      const std::vector<std::string>& choice,
                                      const std::vector<std::string>& value,
                                      bool ismulti)
{
  fListBox = new TGListBox(f);
  f->AddFrame(fListBox);
  if (ismulti) fListBox->SetMultipleSelections();
  
  for (size_t i=0; i<choice.size(); ++i) {
    fListBox->AddEntry(choice[i].c_str(), i);
    for (size_t j=0; j<value.size(); ++j) {
      if (value[j]==choice[i]) fListBox->Select(i);
    }
  }

  fListBox->Connect("SelectionChanged()",
                    "evdb::ParameterSetEditRow",
                    this,
                    "ListBoxSelectionChanged()");
  fListBox->Connect("Selected(Int_t)",
                    "evdb::ParameterSetEditRow",
                    this,
                    "ListBoxSelected(int)");

  size_t h = kRowH*choice.size();
  if (h>3*kRowH) h = 3*kRowH;
  fListBox->Resize(kRowW,h);
}

//......................................................................

void ParameterSetEditRow::SetupRadioButtons(TGCompositeFrame* f, 
					    const std::vector<std::string>& choice,
					    const std::vector<std::string>& value)
{
  unsigned int v = atoi(value[0].c_str());
  
  for (size_t i=0; i<choice.size(); ++i) {
    TGRadioButton* b = new TGRadioButton(f, choice[i].c_str(), i);
    f->AddFrame(b);
    
    b->SetTextJustify(kTextLeft);
    b->Connect("Clicked()",
               "evdb::ParameterSetEditRow",
               this,
               "RadioButtonClicked()");
    
    if (i==v) b->SetState(kButtonDown);
    
    fRadioButton.push_back(b);
  }
}

//......................................................................

void ParameterSetEditRow::SetupCheckButton(TGCompositeFrame* f, 
					   const std::vector<std::string>& choice,
					   const std::vector<std::string>& value)
{
  unsigned int mask;
  unsigned int v = atoi(value[0].c_str());
  for (size_t i=0; i<choice.size(); ++i) {
    TGCheckButton* b = new TGCheckButton(f, choice[i].c_str(), i);
    f->AddFrame(b);
    b->Connect("Clicked()",
               "evdb::ParameterSetEditRow",
               this,
               "CheckButtonClicked()");
    
    mask = (0x1)<<i;
    if (v&mask) b->SetState(kButtonDown);

    fCheckButton.push_back(b);
  }
}
//......................................................................
void ParameterSetEditRow::SetupSlider(TGCompositeFrame* f, 
				      const std::vector<std::string>& choice,
				      const std::vector<std::string>& value)
{
  fTextEntry = new TGTextEntry(f);
  f->AddFrame(fTextEntry);
  
  std::string t;
  if (value.size()==1) { t = value[0]; }
  if (value.size()==2) { 
    t = "["; t += value[0]; t += ","; t += value[1]; t += "]";
  }
  fTextEntry->SetText(t.c_str());
  
  fTextEntry->Connect("ReturnPressed()",
                      "evdb::ParameterSetEditRow",
                      this,
                      "TextEntryReturnPressed()");

  fSlider = new TGDoubleHSlider(f, 100, kDoubleScaleBoth);
  f->AddFrame(fSlider);

  float min = atof(choice[0].c_str());
  float max = atof(choice[1].c_str());

  float pos1 = 0;
  float pos2 = 0;
  if (value.size()==1) {
    pos1 = atof(value[0].c_str());
    pos2 = pos1;
  }
  if (value.size()==2) {
    pos1 = atof(value[0].c_str());
    pos2 = atof(value[1].c_str());
  }

  fSlider->SetRange(min, max);
  fSlider->SetPosition(pos1, pos2);
  
  fSlider->Connect("PositionChanged()",
                   "evdb::ParameterSetEditRow",
                   this,
                   "SliderPositionChanged()");
  
  fTextEntry->Resize(kRowW*1/5,   kRowH);
  fSlider->   Resize(kRowW*4/5,10*kRowH);
}

//......................................................................

void ParameterSetEditRow::TextEntryReturnPressed()
{
  if (fTextEntry==0) return;
  
  const char* text = fTextEntry->GetBuffer()->GetString();
  
  static TColor* c = gROOT->GetColor(1);
  fTextEntry->SetTextColor(c);

  //
  // If we also have a slider connected to this frame, make sure its
  // state is updated
  //
  if (fSlider) {
    int   n=0;
    float f1=0, f2=0;
    n = sscanf(text, "[%f, %f]", &f1, &f2);
    if (n!=2) {
      n = sscanf(text, "%f", &f1);
      f2 = f1;
    }
    fSlider->SetPosition(f1, f2);
  }
  fValue = text;
  fFrame->Modified();
}

//......................................................................

void ParameterSetEditRow::ListBoxSelectionChanged()
{
  //
  // Only need to handle list boxes where multiple selections are
  // allowed here.
  //
  if (fListBox->GetMultipleSelections()==0) return;
  
  fValue = "[";
  TList selections;
  fListBox->GetSelectedEntries(&selections);
  TGLBEntry* sel;
  bool isfirst = true;
  for (unsigned int i=0;;++i) {
    sel = (TGLBEntry*)selections.At(i);
    if (sel==0) break;
    if (!isfirst) fValue += ",";
    fValue += fChoice[sel->EntryId()];
    isfirst = false;
  }    
  fValue += "]";
  fFrame->Modified();
}

//......................................................................

void ParameterSetEditRow::ListBoxSelected(int id)
{
  //
  // Only handle single selection list boxes here
  //
  if (fListBox->GetMultipleSelections()) return;
  fValue = fChoice[id];
  fFrame->Modified();
}
//......................................................................

void ParameterSetEditRow::RadioButtonClicked() 
{
  unsigned int value = 0;
  TGButton* b = (TGButton*)gTQSender;
  int id = b->WidgetId();
  for (size_t i=0; i<fRadioButton.size(); ++i) {
    if (fRadioButton[i]->WidgetId() != id) {
      fRadioButton[i]->SetState(kButtonUp);
    }
    else value = i;
  }
  char buff[256];
  sprintf(buff, "%d", value);
  fValue = buff;
  fFrame->Modified();
}

//......................................................................

void ParameterSetEditRow::CheckButtonClicked() 
{
  int value = 0;
  for (unsigned int i=0; i<fCheckButton.size(); ++i) {
    if (fCheckButton[i]->IsDown()) value |= 1<<i;
  }
  char buff[256];
  sprintf(buff, "%d", value);
  fValue = buff;
  fFrame->Modified();
}

//......................................................................

void ParameterSetEditRow::SliderPositionChanged() 
{
  char buff[1024];
  float mn, mx, ave;
  fSlider->GetPosition(mn, mx);
  
  ave = 0.5*(mn+mx);

  if (fParamFlags & kINTEGER_PARAM) {  
    int mni  = rint(mn);
    int mxi  = rint(mx);
    int avei = rint(ave);
    if (fParamFlags & kVECTOR_PARAM) {
      sprintf(buff, "[%d, %d]",mni,mxi);
    }
    else {
      sprintf(buff, "%d",avei);
    }
  }
  else {
    if (fParamFlags & kVECTOR_PARAM) {
      sprintf(buff, "[%.1f, %.1f]",mn,mx);
    }
    else {
      sprintf(buff, "%.1f",ave);
    }
  }
  fTextEntry->SetText(buff);
  fValue = buff;
  fFrame->Modified();
}

//......................................................................

void ParameterSetEditRow::Finalize() 
{
  if (fTextEntry) {
    if (fValue != fTextEntry->GetBuffer()->GetString()) {
      this->TextEntryReturnPressed();
    }
  }
}

//......................................................................

std::string ParameterSetEditRow::AsFHICL() const
{
  std::ostringstream s;
  if (fParamFlags & kNO_GUI_TAGS) {
    s << fKEY << ":" << fValue << " ";
  }
  else {
    s << fKEY 
      << ": { "
      << "val:" << fValue << " "
      << "gui:\"" << fGUI << "\" "
      << "doc:\"" << fDOC << "\" "
      << "}";
  }
  return s.str();
}

//======================================================================
//
// ParameterSetEditFrame methods
//
ParameterSetEditFrame::ParameterSetEditFrame(TGCompositeFrame* mother,
					     unsigned int psetid) :
  fParameterSetID(psetid),
  fIsModified(false)
{
  unsigned int i, j;

  fCanvas  = new TGCanvas(mother, kWidth-6, kHeight-50);
  fCanvasH = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY);
  mother->AddFrame(fCanvas, fCanvasH);
  
  fContainer = new TGCompositeFrame(fCanvas->GetViewPort());
  fCanvas->SetContainer(fContainer);
  
  //
  // Locate the parameter set connected to this frame
  //
  const ServiceTable& st = ServiceTable::Instance();
  const fhicl::ParameterSet& pset = st.GetParameterSet(psetid);
  std::vector<std::string>   key  = pset.get_names();
  unsigned int               nkey = key.size();
  
  //
  // Count the number of "non system" parameters - each of these will
  // need an row in the dialog window.
  //
  unsigned int nparam = 0;
  for (i=0; i<nkey; ++i) {
    if (!((key[i]=="service_type") ||
	  (key[i]=="module_type")  ||
	  (key[i]=="module_label"))) {
      ++nparam;
    }
  }
  
  //
  // Build the layout
  //
  fLayout = new TGTableLayout(fContainer, nparam, 2);
  fContainer->SetLayoutManager(fLayout);

  for (i=0, j=0; i<nkey; ++i) {
    if (!((key[i]=="service_type") ||
	  (key[i]=="module_type")  ||
	  (key[i]=="module_label"))) {
      
      TGHorizontalFrame*  lhs  = new TGHorizontalFrame(fContainer);
      TGHorizontalFrame*  rhs  = new TGHorizontalFrame(fContainer);

      TGTableLayoutHints* lhsh = new TGTableLayoutHints(0,1,j,j+1);
      TGTableLayoutHints* rhsh = new TGTableLayoutHints(1,2,j,j+1);
      
      fContainer->AddFrame(lhs, lhsh);
      fContainer->AddFrame(rhs, rhsh);
      
      fLHS.     push_back(lhs);
      fRHS.     push_back(rhs);
      fLHSHints.push_back(lhsh);
      fRHSHints.push_back(rhsh);
      
      fRow.push_back(new ParameterSetEditRow(this, lhs, rhs, pset, key[i]));
      ++j;
    }
  }

  fCanvas->Connect("ProcessedEvent(Event_t*)", "evdb::ParameterSetEditFrame", 
		   this,
		   "HandleMouseWheel(Event_t*)");

  fCanvas->Resize();
}

//......................................................................

ParameterSetEditFrame::~ParameterSetEditFrame() 
{
  unsigned int i;
  for (i=0; i<fRow.size(); ++i)      delete fRow[i];
  for (i=0; i<fRHSHints.size(); ++i) delete fRHSHints[i];
  for (i=0; i<fLHSHints.size(); ++i) delete fLHSHints[i];
  for (i=0; i<fRHS.size(); ++i)      delete fRHS[i];
  for (i=0; i<fLHS.size(); ++i)      delete fLHS[i];
  delete fLayout;
  //
  // Parent takes care of delete for fContainer, I think. Anyhow,
  // trying to delete it causes a seg fault.
  //
  // delete fContainer;
  delete fCanvasH;
  delete fCanvas;
}

//......................................................................
void ParameterSetEditFrame::HandleMouseWheel(Event_t *event)
{
  // Handle mouse wheel to scroll.                                      
  if (event->fType != kButtonPress && event->fType != kButtonRelease)
    return;
  
  Int_t page = 0;
  if (event->fCode == kButton4 || event->fCode == kButton5) {
    if (!fCanvas) return;
    if (fCanvas->GetContainer()->GetHeight())
      page = Int_t(Float_t(fCanvas->GetViewPort()->GetHeight() *
			   fCanvas->GetViewPort()->GetHeight()) /
		   fCanvas->GetContainer()->GetHeight());
  }

  if (event->fCode == kButton4) {
    //scroll up
    Int_t newpos = fCanvas->GetVsbPosition() - page;
    if (newpos < 0) newpos = 0;
    fCanvas->SetVsbPosition(newpos);
  }
  if (event->fCode == kButton5) {
    // scroll down
    Int_t newpos = fCanvas->GetVsbPosition() + page;
    fCanvas->SetVsbPosition(newpos);
  }

  return;
}

//......................................................................
void ParameterSetEditFrame::Modified() { fIsModified = true; }

//......................................................................
void ParameterSetEditFrame::Finalize() 
{
  unsigned int i;
  for (i=0; i<fRow.size(); ++i) fRow[i]->Finalize();
}

//......................................................................

std::string ParameterSetEditFrame::AsFHICL() const
{
  unsigned int i;
  std::ostringstream s;
  for (i=0; i<fRow.size(); ++i) {
    s << fRow[i]->AsFHICL() << "\n";
  }
  return s.str();
}

//======================================================================
//
// ParameterSetEditDialog methods
//
ParameterSetEditDialog::ParameterSetEditDialog(unsigned int psetid) :
  TGTransientFrame(gClient->GetRoot(), gClient->GetRoot(), 4, 4)
{
  fTGTab = new TGTab(this);
  this->AddFrame(fTGTab);
  
  fButtons = new TGHorizontalFrame(this);
  this->AddFrame(fButtons);
  
  fApply  = new TGTextButton(fButtons, " Apply  ");
  fCancel = new TGTextButton(fButtons, " Cancel ");
  fDone   = new TGTextButton(fButtons, " Done   ");
  
  fButtons->AddFrame(fApply);
  fButtons->AddFrame(fCancel);
  fButtons->AddFrame(fDone);
  
  fApply-> Connect("Clicked()","evdb::ParameterSetEditDialog",this,"Apply()");
  fCancel->Connect("Clicked()","evdb::ParameterSetEditDialog",this,"Cancel()");
  fDone->  Connect("Clicked()","evdb::ParameterSetEditDialog",this,"Done()");
  
  //
  // Loop over all the parameter sets and build tabs for them
  //
  const ServiceTable& st = ServiceTable::Instance();
  int which = st.fServices[psetid].fCategory;

  unsigned int i;
  unsigned int top=0, indx=0;
  for (i=0; i<st.fServices.size(); ++i) {
    if (st.fServices[i].fCategory==which) {
      if (i==psetid) top = indx;
      std::string tabnm = this->TabName(st.fServices[i].fName);
      TGCompositeFrame* f = fTGTab->AddTab(tabnm.c_str());
      fFrames.push_back(new ParameterSetEditFrame(f, i));
      ++indx;
    }
  }
  fTGTab->SetTab(top);
  
  switch (which) {
  case kDRAWING_SERVICE:
    this->SetWindowName("Drawing Services");
    break;
  case kEXPERIMENT_SERVICE:
    this->SetWindowName("Experiment Services");
    break;
  case kART_SERVICE:
    this->SetWindowName("ART Services");
    break;
  default:
    this->SetWindowName("Services Configuration");
  }

  this->MapSubwindows();
  this->Resize(kWidth,kHeight);
  this->MapWindow();
}

//......................................................................

ParameterSetEditDialog::~ParameterSetEditDialog() 
{
  unsigned int i;
  for (i=0; i<fFrames.size(); ++i) delete fFrames[i];
  delete fDone;
  delete fCancel;
  delete fApply;
  delete fButtons;
  delete fTGTab;
}

//......................................................................

void ParameterSetEditDialog::Apply() 
{
  //
  // We're not in control of the event loop so what we can do is write
  // the new configuration to the ServiceTable. The main driver will
  // pick it up, apply it, and wipe it clean when a reload / next
  // event is triggered.
  //
  unsigned int i;
  ServiceTable& st = ServiceTable::Instance();
  for (i=0; i<fFrames.size(); ++i) {
    if (fFrames[i]->fIsModified) {
      unsigned int psetid = fFrames[i]->fParameterSetID;

      fFrames[i]->Finalize();
      std::string p = fFrames[i]->AsFHICL();
      
      p += "service_type:";
      p += st.fServices[psetid].fName;

      st.fServices[psetid].fParamSet = p;

    }
  }
  NavState::Set(kRELOAD_EVENT);
}

//......................................................................

void ParameterSetEditDialog::Cancel() { this->SendCloseMessage(); }

//......................................................................

void ParameterSetEditDialog::Done() 
{
  this->Apply();
  this->SendCloseMessage();
}

//......................................................................

void ParameterSetEditDialog::CloseWindow() { delete this; }

//......................................................................
//
// Remove any redundant text from the tab name
//
std::string ParameterSetEditDialog::TabName(const std::string& s)
{
  size_t n = 0;
  
  n = s.find("DrawingOptions");
  if (n!=std::string::npos) return s.substr(0,n);
  
  return s;
}

////////////////////////////////////////////////////////////////////////
