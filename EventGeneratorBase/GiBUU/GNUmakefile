#
# $Id: GNUmakefile,v 1.4 2012-09-07 21:35:26 brebel Exp $
#
# messier@indiana.edu
#
include SoftRelTools/arch_spec_root.mk
include SoftRelTools/arch_spec_genie.mk
include SoftRelTools/arch_spec_cern.mk
include SoftRelTools/arch_spec_ifdhc.mk

#VERBOSE     := true
LIB_TYPE    := shared
LIB         := lib$(PACKAGE)GIBUU
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)

#
# this makefile (and package code) assumes GIBUU r3208 or later
#


########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk

override LIBLIBS += -L$(ROOTSYS)/lib -lGeom -lGeomPainter -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -lEventGeneratorBase -lEventGeneratorBaseGENIE -L$(IFDHC_FQ_DIR)/lib/ -lifdh

override CXXFLAGS := $(filter-out -Woverloaded-virtual, $(CXXFLAGS))

