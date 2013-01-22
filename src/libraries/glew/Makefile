# this is a pretty bare bones make file with some magical trickery in it
# to make building object files in a separate directory work
#
include ../../../rules.make

SOURCES=glew.c
OBJ_DIR=../../../build/parsec
OBJECTS=$(OBJ_DIR)/glew.o
PACKAGE=$(OBJ_DIR)/glew.o
export CXXFLAGS:=$(CXXFLAGS) -DPARSEC_CLIENT

.PHONY: all clean

all: $(OBJECTS)


clean:
	rm -f $(OBJ_DIR)/glew.o


$(OBJ_DIR)/glew.o: glew.c 
	$(CXX) $(CXXFLAGS) -c $< -o $@
