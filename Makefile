# ------------------------------------------------
# Generic Makefile
#
# Author: Yanick Rochon
# Date  : 2011-08-10
#
# Changelog :
#   2010-11-05 - first version
#   2011-08-10 - added structure : sources, objects, binaries
#                thanks to http://stackoverflow.com/users/128940/beta
# ------------------------------------------------

# project name (generate executable with this name)
TARGET = test

CPP = g++
# compiling flags here
CFLAGS = --std=c++11  -Wall -O3

LINKER = g++ -o
# linking flags here
LFLAGS = -ltfhe-spqlios-fma -pthread

# change these to set the proper directories where each files shoould be
SRCDIR = src
OBJDIR = obj
BINDIR = bin

.SECONDEXPANSION:

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.hpp)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm = rm -f

%/.:
		mkdir -p $(dir $@)
		touch $@

.PRECIOUS: %/.

$(BINDIR)/$(TARGET): $(OBJECTS) | $$(@D)/.
	@$(LINKER) $@ $(OBJECTS) $(LFLAGS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp | $$(@D)/.
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"



.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
