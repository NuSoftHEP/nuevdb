#
# $Id: GNUmakefile,v 1.3 2011-03-04 19:58:12 brebel Exp $
#
# messier@indiana.edu
#
include SoftRelTools/arch_spec_root.mk

PACKAGE     := SimulationBase
LIB_TYPE    := shared
LIB         := lib$(PACKAGE)
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)

LIBLINK     := -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -l$(PACKAGE)

########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk

override LIBLIBS += $(LOADLIBES) 
