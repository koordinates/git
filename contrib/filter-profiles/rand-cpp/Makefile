# run this via `FILTER_PROFILES=contrib/profile-filters/rand make`
# from the main git directory. That way we inherit useful variables.

all:

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifneq ($(findstring s,$(MAKEFLAGS)),s)
ifndef V
	QUIET_CXX      = @echo '   ' CXX $@;
	QUIET_LINK     = @echo '   ' LINK $@;
endif
endif

CXX = c++
CFLAGS += -I$(PROFILE_DIR)
CFLAGS += -Wmissing-prototypes -Wstrict-prototypes
CPPFLAGS = --std=c++17

ifeq ($(uname_S),Darwin)
LDFLAGS += -bundle -bundle_loader '$(PROFILE_DIR)/git'
else
LDFLAGS += -shared --just-symbols='$(PROFILE_DIR)/git'
endif

FILTER_LIB = rand-cpp.so

all: $(FILTER_LIB)
ifeq ($(MAKELEVEL),0)
	$(error "Run via parent git make")
endif
	@:

$(FILTER_LIB): rand.o
	$(QUIET_LINK)$(CXX) $^ $(ALL_LDFLAGS) $(LDFLAGS) -o $@

rand.o: rand.cpp
	$(QUIET_CXX)$(CXX) -c $(ALL_CFLAGS) $(CPPFLAGS) $<

clean:
	$(RM) $(FILTER_LIB) rand.o

install: all
	$(INSTALL) -m 644 $(FILTER_LIB) '$(DESTDIR_SQ)$(gitexec_instdir_SQ)'

.PHONY: all clean install
