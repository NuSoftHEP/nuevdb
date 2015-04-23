#
# $Id: GNUmakefile,v 1.2 2012-09-21 02:29:00 greenc Exp $
#
# brebel@fnal.gov
#
#

LIB_TYPE    := shared
LIB         := lib$(PACKAGE)
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)

LIBLINK    := -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -l$(PACKAGE)

########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk
include SoftRelTools/arch_spec_root.mk
include SoftRelTools/arch_spec_geant4.mk
include SoftRelTools/arch_spec_xercesc.mk

override CPPFLAGS += -I$(G4INCLUDE)
override LIBLIBS += $(LOADLIBES) -L$(CLHEP_BASE)/lib -lCLHEP -L${GEANT4_FQ_DIR}/lib




