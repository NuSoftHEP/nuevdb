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
#SUBDIRS     := art
LIB_TYPE    := shared
LIB         := lib$(PACKAGE)
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)
SUBDIRS     := art

LIBLINK    := -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -l$(PACKAGE)


########################################################################
include SoftRelTools/standard.mk
#include SoftRelTools/arch_spec_art.mk
include SoftRelTools/arch_spec_root.mk
include SoftRelTools/arch_spec_genie.mk
#-lEventGeneratorBase
override LIBLIBS += $(LOADLIBES)  -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) 

override CXXFLAGS := $(filter-out -Woverloaded-virtual, $(CXXFLAGS))