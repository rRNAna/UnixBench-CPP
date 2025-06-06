###########################################################################
# Makefile for UnixBench C++ version
#
# Version: 6.0.0
# Author: rRNA
# Date: 4/29/2025
#
# Description:
# This Makefile is used to build the UnixBench C++ version benchmarks.
# It automatically selects clang++ if available, otherwise falls back to g++.
# Static linking is enabled by default to ensure standalone executables.
# Optimizations are adjusted based on platform detection (Linux x86_64, ARM64, or macOS).
#
# Usage:
#   make all        - Build all benchmark binaries
#   make clean      - Remove all generated binaries
#   make spotless   - Remove binaries and temporary results
#
###########################################################################

##############################################################################
# CONFIGURATION
##############################################################################

SHELL = /bin/sh


# GRAPHICS TESTS: Uncomment the definition of "GRAPHIC_TESTS" to enable
# the building of the graphics benchmarks.  This will require the
# X11 libraries on your system.
#
# Comment the line out to disable these tests.
# GRAPHIC_TESTS = defined

# Set "GL_LIBS" to the libraries needed to link a GL program.
GL_LIBS = -lGL -lXext -lX11

# Automatically select clang++ if available, otherwise fall back to g++
# CXX := $(shell if command -v clang++ >/dev/null 2>&1; then echo clang++; else echo g++; fi)

CXX = clang++

# OPTIMISATION SETTINGS:
#
# Use static compilation only for systems other than MacOS
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  STATICOPTON =
else
  STATICOPTON = -static
endif
# Automatic platform detection for optimisation settings
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Linux)
  ifeq ($(UNAME_M),x86_64)
    OPTON = -O3 -march=native -fomit-frame-pointer -ffast-math
  else ifneq (,$(filter aarch64 arm64,$(UNAME_M)))
    OPTON = -O3 -mcpu=native -fomit-frame-pointer -ffast-math
  endif
else ifeq ($(UNAME_S),Darwin)
  OPTON = -target arm64-apple-macos -arch arm64 -O3 -fomit-frame-pointer -ffast-math
endif

# Generic C++ flags.  -DTIME must be included.
CXXFLAGS = -DTIME -Wall -pedantic -std=c++17

##############################################################################
# END CONFIGURATION
##############################################################################

# Local directories
PROGDIR = ./pgms
SRCDIR = ./src
TESTDIR = ./testdir
RESULTDIR = ./results
TMPDIR = ./tmp
# Other directories
INCLDIR = /usr/include
LIBDIR = /lib
SCRIPTS = unixbench.logo multi.sh tst.sh index.base
# Source files, now using .cpp extensions
SOURCES = arith.cpp big.cpp context1.cpp \
	dummy.cpp execl.cpp \
	fstime.cpp hanoi.cpp \
	pipe.cpp spawn.cpp \
	syscall.cpp looper.cpp timeit.cpp time-polling.cpp \
	dhry.cpp dhry.h whets.cpp ubgears.cpp
TESTS = sort.src cctest.cpp dc.dat large.txt

ifdef GRAPHIC_TESTS
GRAPHIC_BINS = $(PROGDIR)/ubgears
else
GRAPHIC_BINS =
endif

# Program binaries.
BINS = $(PROGDIR)/arithoh $(PROGDIR)/register $(PROGDIR)/short \
	$(PROGDIR)/int $(PROGDIR)/long $(PROGDIR)/float $(PROGDIR)/double \
	$(PROGDIR)/hanoi $(PROGDIR)/syscall $(PROGDIR)/context1 \
	$(PROGDIR)/pipe $(PROGDIR)/spawn $(PROGDIR)/execl \
	$(PROGDIR)/dhry $(PROGDIR)/dhry_reg  $(PROGDIR)/looper \
	$(PROGDIR)/fstime $(PROGDIR)/whetstone-double $(GRAPHIC_BINS)
## These compile only on some platforms...
# $(PROGDIR)/poll $(PROGDIR)/poll2 $(PROGDIR)/select

# Required non-binary files.
REQD = $(BINS) $(PROGDIR)/unixbench.logo \
	$(PROGDIR)/multi.sh $(PROGDIR)/tst.sh $(PROGDIR)/index.base \
	$(PROGDIR)/gfx-x11 \
	$(TESTDIR)/sort.src $(TESTDIR)/cctest.cpp $(TESTDIR)/dc.dat \
	$(TESTDIR)/large.txt

# ######################### the big ALL ############################
all: programs
# ####################### a check for Run ######################
check: $(REQD)
	make all
# ##############################################################
# Distribute the files to subdirectories if they are in this one
distr:
	@echo "Checking distribution of files"
	@if  test ! -d $(PROGDIR) ; then mkdir $(PROGDIR) ; mv $(SCRIPTS) $(PROGDIR) ; else echo "$(PROGDIR) exists" ; fi
	@if  test ! -d $(SRCDIR) ; then mkdir $(SRCDIR) ; mv $(SOURCES) $(SRCDIR) ; else echo "$(SRCDIR) exists" ; fi
	@if  test ! -d $(TESTDIR) ; then mkdir $(TESTDIR) ; mv $(TESTS) $(TESTDIR) ; else echo "$(TESTDIR) exists" ; fi
	@if  test ! -d $(TMPDIR) ; then mkdir $(TMPDIR) ; else echo "$(TMPDIR) exists" ; fi
	@if  test ! -d $(RESULTDIR) ; then mkdir $(RESULTDIR) ; else echo "$(RESULTDIR) exists" ; fi

programs: \
  $(PROGDIR)/arithoh \
  $(PROGDIR)/register \
  $(PROGDIR)/short \
  $(PROGDIR)/int \
  $(PROGDIR)/long \
  $(PROGDIR)/float \
  $(PROGDIR)/double \
  $(PROGDIR)/hanoi \
  $(PROGDIR)/syscall \
  $(PROGDIR)/context1 \
  $(PROGDIR)/pipe \
  $(PROGDIR)/spawn \
  $(PROGDIR)/execl \
  $(PROGDIR)/dhry \
  $(PROGDIR)/dhry_reg \
  $(PROGDIR)/looper \
  $(PROGDIR)/fstime \
  $(PROGDIR)/whetstone-double \
  $(GRAPHIC_BINS)

# Individual program rules (静态构建规则，覆盖原动态版本)
$(PROGDIR)/arithoh: $(SRCDIR)/arith.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -Darithoh $<

$(PROGDIR)/register: $(SRCDIR)/arith.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -Ddatum='register int' $<

$(PROGDIR)/short: $(SRCDIR)/arith.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -Ddatum=short $<

$(PROGDIR)/int: $(SRCDIR)/arith.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -Ddatum=int $<

$(PROGDIR)/long: $(SRCDIR)/arith.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -Ddatum=long $<

$(PROGDIR)/float: $(SRCDIR)/arith.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -Ddatum=float $<

$(PROGDIR)/double: $(SRCDIR)/arith.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -Ddatum=double $<

$(PROGDIR)/hanoi: $(SRCDIR)/hanoi.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $<

$(PROGDIR)/syscall: $(SRCDIR)/syscall.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $<

$(PROGDIR)/context1: $(SRCDIR)/context1.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $<

$(PROGDIR)/pipe: $(SRCDIR)/pipe.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $<

$(PROGDIR)/spawn: $(SRCDIR)/spawn.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $<

# Special slim build for execl: remove C++ RTTI and exception handling, only link libc
$(PROGDIR)/execl: $(SRCDIR)/execl.cpp $(SRCDIR)/big.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) -fno-exceptions -fno-rtti $(STATICOPTON) $(SRCDIR)/execl.cpp

$(PROGDIR)/dhry: $(SRCDIR)/dhry.cpp $(SRCDIR)/dhry.h
	cd $(SRCDIR); $(CXX) -c $(CXXFLAGS) -DHZ=${HZ} $(OPTON) $(STATICOPTON) dhry.cpp -o dhry.o
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $(SRCDIR)/dhry.o
	cd $(SRCDIR); rm -f dhry.o

$(PROGDIR)/dhry_reg: $(SRCDIR)/dhry.cpp $(SRCDIR)/dhry.h
	cd $(SRCDIR); $(CXX) -c $(CXXFLAGS) -DREG=register -DHZ=${HZ} $(OPTON) $(STATICOPTON) dhry.cpp -o dhry_reg.o
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $(SRCDIR)/dhry_reg.o
	cd $(SRCDIR); rm -f dhry_reg.o

$(PROGDIR)/looper: $(SRCDIR)/looper.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $<

$(PROGDIR)/fstime: $(SRCDIR)/fstime.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $<

$(PROGDIR)/whetstone-double: $(SRCDIR)/whets.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) -DDP -DUNIX -DUNIXBENCH $< -lm

$(PROGDIR)/ubgears: $(SRCDIR)/ubgears.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(OPTON) $(STATICOPTON) $(SRCDIR)/ubgears.cpp $(GL_LIBS)

# Run the benchmarks and create the reports
run:
	sh ./Run

clean:
	rm -f $(BINS) core *~ */*~

spotless: clean
	rm -f $(RESULTDIR)/* $(TMPDIR)/*

## END ##
