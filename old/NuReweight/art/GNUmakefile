#
# $Id: GNUmakefile,v 1.2 2012-09-19 18:12:09 brebel Exp $
#
# messier@indiana.edu
#
#include SoftRelTools/arch_spec_root.mk
#include SoftRelTools/arch_spec_genie.mk
#include SoftRelTools/arch_spec_cern.mk
#include SoftRelTools/arch_spec_cry.mk
#include SoftRelTools/standard.mk
#include SoftRelTools/arch_spec_art.mk
#include SoftRelTools/arch_spec_root.mk
#include SoftRelTools/arch_spec_genie.mk

PACKAGE     := NuReweight
LIB_TYPE    := shared
LIB         := lib$(PACKAGE)Art
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)

ifneq ($(shell grep KinePhaseSpace_t \${GENIE}/src/GHEP/GHepRecord.h 2> /dev/null ),)
  # genie post R-2_8_0 SetDiffXSec() takes 2 args
  # 2nd is a KinePhaseSpace_t 
  #$(warning GHepRecord SetDiffXSec takes two args)
else
  override CPPFLAGS += -DSETDIFFXSEC_1ARG
  #$(warning GHepRecord SetDiffXSec takes only one arg)
endif

LIBLINK    := -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -l$(PACKAGE)Art


########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk
include SoftRelTools/arch_spec_root.mk
include SoftRelTools/arch_spec_genie.mk
#-lEventGeneratorBase
override LIBLIBS += $(LOADLIBES)  -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -lSimulationBase -lNuReweight

override CXXFLAGS := $(filter-out -Woverloaded-virtual, $(CXXFLAGS))