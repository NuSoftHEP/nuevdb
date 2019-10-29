#
# $Id: GNUmakefile,v 1.3 2012-09-07 21:35:26 brebel Exp $
#
# messier@indiana.edu
#
include SoftRelTools/arch_spec_root.mk
include SoftRelTools/arch_spec_cern.mk
include SoftRelTools/arch_spec_cry.mk

#VERBOSE     := true
LIB_TYPE    := shared
LIB         := lib$(PACKAGE)CRY
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)

########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk

override LIBLIBS += -L$(ROOTSYS)/lib -lGeom -lGeomPainter -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -lEventGeneratorBase

override CXXFLAGS := $(filter-out -Woverloaded-virtual, $(CXXFLAGS))

