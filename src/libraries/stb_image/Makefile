# this is a pretty bare bones make file with some magical trickery in it
# to make building object files in a separate directory work
#
include ../../../rules.make

SOURCES=stb_image.cpp stb_image_write.c
OBJ_DIR=../../../build/parsec
OBJECTS=$(OBJ_DIR)/stb_image.o $(OBJ_DIR)/stb_image_write.o
export CXXFLAGS:=$(CXXFLAGS) -DPARSEC_CLIENT

.PHONY: all clean

all: $(OBJECTS)


clean:
	rm -f $(OBJ_DIR)/stb_image*.o


$(OBJ_DIR)/stb_image.o: stb_image.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_DIR)/stb_image_write.o: stb_image_write.c
	$(CXX) $(CXXFLAGS) -c $< -o $@
