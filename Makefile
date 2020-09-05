# Universal Makefile for Linux Targets

# filename extensions
CEXTS:=c
CXXEXTS:=cpp c++ cc CC C
HEXTS:=h hh hpp H

-include ./common.mk

ROOT=.
BINDIR=$(ROOT)/bin
SRCDIR=$(ROOT)/src
INCDIR=$(ROOT)/include

GCCFLAGS:=-Wall -ldl 
CPPFLAGS:=-iquote$(INCDIR) -ldl 
GCCFLAGS:=`pkg-config --cflags openssl` -g -ldl 

LDLIBS=`pkg-config --libs openssl` -ldl 

CFLAGS=$(CPPFLAGS) $(WARNFLAGS) $(GCCFLAGS) --std=gnu11 -ldl 
CXXFLAGS=$(CPPFLAGS) $(WARNFLAGS) $(GCCFLAGS)  --std=gnu++14  -ldl 
LDFLAGS=$(WARNFLAGS)  -pthread -ldl 

LINTER=./cpplint.py
LINTFLAGS:=--filter=-readability/todo,-legal/copyright --linelength=100 --headers=hh

CSRC=$(foreach cext,$(CEXTS),$(call rwildcard, $(SRCDIR),*.$(cext), $1))
COBJ=$(addprefix $(BINDIR)/,$(patsubst $(SRCDIR)/%,%.o,$(CSRC)))
CXXSRC=$(foreach cxxext,$(CXXEXTS),$(call rwildcard, $(SRCDIR),*.$(cxxext), $1))
CXXOBJ=$(addprefix $(BINDIR)/,$(patsubst $(SRCDIR)/%,%.o,$(CXXSRC)))

GETALLOBJ=$(sort $(call COBJ,$1) $(call CXXOBJ,$1))
ALLFILES=$(foreach cxxext, $(CXXEXTS), $(call rwildcard, $(SRCDIR), *.$(cxxext)))
ALLFILES+=$(foreach cext, $(CEXTS), $(call rwildcard, $(SRCDIR), *.$(cext)))
ALLFILES+=$(foreach hext, $(HEXTS), $(call rwildcard, $(INCDIR), *.$(hext)))
ALLFILES:=$(sort $(ALLFILES))

all: git-commit myhttpd

myhttpd: $(call GETALLOBJ)
	@echo -n "Linking $@ "
	$(call test_output,$D$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@,$(OK_STRING))


git-commit:
	git checkout master >> .local.git.out || echo
	git add auth.txt * */* */*/* */*/*/* >> .local.git.out  || echo
	git commit -a -m "Commit"  >> .local.git.out || echo
	git push origin master


clean:
	@echo Cleaning $(BINDIR), myhttpd
	-$(VV)rm -rf $(BINDIR)
	-$(VV)rm -f myhttpd

.PHONY: lint
lint:
	$(VV)$(LINTER) $(LINTFLAGS) $(ALLFILES)

define c_rule
$(BINDIR)/%.$1.o: $(SRCDIR)/%.$1
	@echo -n "Compiling $$< "
	$(VV)mkdir -p $$(dir $$@)
	$$(call test_output,$D$(CC) -c $(INCLUDE) -iquote$(INCDIR)/$$(dir $$*) $(CFLAGS) -o $$@ $$<,$(OK_STRING))
endef
$(foreach cext,$(CEXTS),$(eval $(call c_rule,$(cext))))

define cxx_rule
$(BINDIR)/%.$1.o: $(SRCDIR)/%.$1
	@echo -n "Compiling $$< "
	$(VV)mkdir -p $$(dir $$@)
	$$(call test_output,$D$(CXX) -c $(INCLUDE) -iquote$(INCDIR)/$$(dir $$*) $(CXXFLAGS) -o $$@ $$<,$(OK_STRING))
endef
$(foreach cxxext,$(CXXEXTS),$(eval $(call cxx_rule,$(cxxext))))

