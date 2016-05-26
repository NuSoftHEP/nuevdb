#
# $Id: GNUmakefile,v 1.11 2012-09-07 21:35:25 brebel Exp $
#
# messier@indiana.edu
#
include SoftRelTools/arch_spec_root.mk
include SoftRelTools/arch_spec_cern.mk

#VERBOSE     := true
PACKAGE     := EventGeneratorBase
SUBDIRS     := test GENIE CRY
LIB_TYPE    := shared
LIB         := lib$(PACKAGE)
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)

########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk

override LIBLIBS += -L$(ROOTSYS)/lib -lGeom -lGeomPainter

override CXXFLAGS := $(filter-out -Woverloaded-virtual, $(CXXFLAGS))

