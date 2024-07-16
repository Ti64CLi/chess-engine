DEBUG = FALSE

GXX = g++
LD  = g++

GXXFLAGS = -Wall -Wextra -Werror
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system

ifeq ($(DEBUG), FALSE)
	GXXFLAGS += -Os
else
	GXXFLAGS += -O0 -g
endif

ENGINESRCS = $(wildcard ../src/engine/*.cpp)
TESTS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(ENGINESRCS))
OBJS += $(patsubst %.cpp, %.o, $(TESTS))
EXES = $(patsubst %.cpp, %, $(TESTS))
EXESCLEAN = $(addsuffix .clean, $(EXES))
BINDIR = ../bin

.PHONY: all clean $(EXESCLEAN)
all: $(EXES)

%.o: %.cpp
	$(GXX) $(GXXFLAGS) -c $< -o $@

$(EXES): $(OBJS)
	mkdir -p $(BINDIR)
	$(LD) $^ -o $(BINDIR)/$@ $(LDFLAGS)

$(EXESCLEAN):
	rm -f $(BINDIR)/$(basename $@)

clean: $(EXESCLEAN)
	rm -f $(OBJS)
