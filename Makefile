DEBUG = FALSE

SUBDIRS = tests
SUBDIRSCLEAN = $(addsuffix .clean, $(SUBDIRS))

E = @echo
GXX = g++
LD  = g++

GXXFLAGS = -Wall -Wextra -Werror
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system

ifeq ($(DEBUG), FALSE)
	GXXFLAGS += -Os
else
	GXXFLAGS += -O0 -g
endif

ENGINESRCS = $(wildcard src/engine/*.cpp)
SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(ENGINESRCS))
OBJS += $(patsubst %.cpp, %.o, $(SRCS))
BINDIR = .
EXE = chess

.PHONY: all
all: $(EXE) $(SUBDIRS)

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.o: %.cpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

$(EXE): $(OBJS)
	mkdir -p $(BINDIR)
	$(LD) $^ -o $(BINDIR)/$@ $(LDFLAGS)

.PHONY: clean
clean: $(SUBDIRSCLEAN)
	$(E) "cleaning complete: $(SUBDIRS)"
	rm -f $(OBJS) $(BINDIR)/$(EXE)

.PHONY: $(SUBDIRSCLEAN)
$(SUBDIRSCLEAN):
	$(E) "cleaning $(basename $@)"
	if [ -e $(basename $@)/Makefile ] ; then \
		$(MAKE) -C $(basename $@) clean ; \
	fi
