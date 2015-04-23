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
LIB         := lib$(PACKAGE)GENIE
LIBCXXFILES := $(wildcard *.cxx)
JOBFILES    := $(wildcard *.fcl)

#
# this makefile (and package code) assumes GENIE r3208 or later
#

# older versions of GENIE use $ENV variables to set parameters
ifeq ($(shell /bin/ls \${GENIE}/src/Utils/RunOpt.h 2> /dev/null ),)
  override CPPFLAGS += -DGENIE_USE_ENVVAR
  $(warning GENIE still uses environment variables)
#else
## genie is has the new code (circa r3747) ... no longer use $ENV variables
#  $(warning GENIE/RunOpt.h means GENIE no longer uses environment variables)
endif

# take advantage of new GENIE flux driver interfaces
ifeq ($(shell /bin/grep \<set\> \${GENIE}/src/FluxDrivers/GSimpleNtpFlux.h 2> /dev/null ),)
  $(warning GENIE/ROOTGeomAnalyzer missing LoadBeamSimData set/vector methods)
  override CPPFLAGS += -DGFLUX_MISSING_SETORVECTOR
#else
## genie has new code (r3669) ... use the method
#  $(warning GENIE/ROOTGeomAnalyzer use LoadBeamSimData set/vector methods)
endif

########################################################################
include SoftRelTools/standard.mk
include SoftRelTools/arch_spec_art.mk

override LIBLIBS += -L$(ROOTSYS)/lib -lGeom -lGeomPainter -L$(SRT_PRIVATE_CONTEXT)/lib/$(SRT_SUBDIR) -L$(SRT_PUBLIC_CONTEXT)/lib/$(SRT_SUBDIR) -lEventGeneratorBase -L$(IFDHC_FQ_DIR)/lib/ -lifdh

override CXXFLAGS := $(filter-out -Woverloaded-virtual, $(CXXFLAGS))

