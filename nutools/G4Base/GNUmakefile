#
# $Id: GNUmakefile,v 1.6 2012-09-07 22:30:22 brebel Exp $
#
# messier@indiana.edu
#
include SoftRelTools/arch_spec_root.mk
include SoftRelTools/arch_spec_CLHEP.mk
include SoftRelTools/arch_spec_xercesc.mk
include SoftRelTools/arch_spec_geant4.mk

PACKAGE     := G4Base
LIB_TYPE    := shared
LIB         := lib$(PACKAGE)
LIBCXXFILES := $(wildcard *.cxx)
# alternative G4 code uses .cc/.hh extensions
LIBCCFILES  := $(wildcard *.cc)

CPPFLAGS     += -I$(G4INSTALL)/include

LIBLINK    := -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -l$(PACKAGE)


########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk
########################################################################

override LIBLIBS += $(LOADLIBES) -L$(CLHEP_BASE)/lib -lCLHEP -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -lMagneticField -lSimulationBase
