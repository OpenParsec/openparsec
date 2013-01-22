# this is a pretty bare bones make file with some magical trickery in it
# to make building object files in a separate directory work
#
include ../../rules.make

#CXXFLAGS=-Iinclude/ -I../parsec/include -g -m32 -I../libparsec/include -I/opt/include -Wno-write-strings -ggdb
#LDFLAGS=-L/usr/lib -L/usr/X11R6/lib -L/opt/lib -ldl -ggdb
SOURCES=$(shell ls -1 *.cpp)
OBJECTS=$(shell ls -1 *.cpp | sed 's/.cpp/.o/' |  sed 's!^!../../build/parsec_server/!')
PACKAGE=$(OBJ_DIR)/parsec_server
export CXXFLAGS:=$(CXXFLAGS) -DPARSEC_SERVER -Iinclude/ -I../libparsec/include/ -I../parsec_server/include/
export LIBPARSEC_LD_FLAGS
OBJ_DIR=../../build/parsec_server
LIBPARSEC_OBJ=../../build/libparsec/libparsec.o

.PHONY: all clean

all: $(PACKAGE)

$(LIBPARSEC_OBJ): 
	make -C ../libparsec/ -f ../libparsec/Makefile


clean: clean_server clean_lib

clean_server: 
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/parsec_server


clean_lib:
	rm -f ../../build/libparsec/*.o

$(PACKAGE): $(OBJECTS) $(LIBPARSEC_OBJ)
	$(CXX) $(CXXFLAGS)  -o $(PACKAGE) $(OBJECTS) $(LIBPARSEC_OBJ) -lncurses 
	@cp $(OBJ_DIR)/parsec_server ../../parsec_root/server/ -f

$(OBJ_DIR)/%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@
