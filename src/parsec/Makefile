# this is a pretty bare bones make file with some magical trickery in it
# to make building object files in a separate directory work
#
include ../../rules.make

OBJ_DIR=../../build/parsec
SOURCES=$(shell ls -1 *.cpp)
OBJECTS=$(shell ls -1 *.cpp | sed 's/.cpp/.o/' |  sed 's!^!../../build/parsec/!')
PACKAGE=$(OBJ_DIR)/parsec
export CXXFLAGS:=$(CXXFLAGS) -DPARSEC_CLIENT -I ../libraries/ -I../libparsec/include/ -I../parsec/include/
export LIBPARSEC_LD_FLAGS
LIBPARSEC_OBJ:=../../build/libparsec/libparsec.o

.PHONY: all clean

all: $(PACKAGE)

#../../build/libparsec.o: 
$(LIBPARSEC_OBJ):
	make -C ../libparsec -f ../libparsec/Makefile 


clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/parsec
	rm -f ../../build/libparsec/*.o

$(PACKAGE): $(OBJECTS) $(OBJ_DIR)/glew.o $(OBJ_DIR)/stb_image.o $(OBJ_DIR)/stb_image_write.o $(LIBPARSEC_OBJ)
	$(CXX) $(CXXFLAGS)  -o $(PACKAGE) $(OBJECTS)  $(OBJ_DIR)/glew.o $(OBJ_DIR)/stb_image.o $(OBJ_DIR)/stb_image_write.o $(LIBPARSEC_OBJ) $(LDFLAGS)
	@cp $(OBJ_DIR)/parsec ../../parsec_root/client/ -f

$(OBJ_DIR)/glew.o: ../libraries/glew/glew.c
	make -C ../libraries/glew/ 

$(OBJ_DIR)/stb_image.o: ../libraries/stb_image/stb_image.cpp
	make -C ../libraries/stb_image/

$(OBJ_DIR)/stb_image_write.o: ../libraries/stb_image/stb_image_write.c
	make -C ../libraries/stb_image/ 

$(OBJ_DIR)/%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@
