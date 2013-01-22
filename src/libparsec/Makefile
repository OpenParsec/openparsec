# this is a pretty bare bones make file with some magical trickery in it
# to make building object files in a separate directory work
#
# TODO: need to add in the stuff to do the link of libparsec to a single .o and clean up everything else

#include ../../rules.make # this prevents -DPARSEC_CLIENT or -DPARSEC_SERVER from coming here...

SOURCES=$(shell ls -1 *.cpp)
OBJECTS=$(shell ls -1 *.cpp | sed 's/.cpp/.o/' |  sed 's!^!../../build/libparsec/!')
PACKAGE=$(OBJ_DIR)/libparsec.o
OBJ_DIR=../../build/libparsec
#CXXFLAGS:=$(LIBPARSEC_CXXFLAGS)
LDFLAGS:=$(LIBPARSEC_LD_FLAGS)
.PHONY: all clean

all: $(PACKAGE)
clean:
	rm -f $(OBJ_DIR)/*.o

$(PACKAGE): $(OBJECTS)
	ld -r -o $(PACKAGE) $(OBJECTS) $(LDFLAGS)
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(shell basename $*).cpp -o $@
