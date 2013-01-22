CFILES = makeodt.c getopt.c

CFLAGS = -g -O2 -x c++ -fno-for-scope -I../BspLib -I../QvLib

all: makeodt

clean:
	rm -f *.o *~ makeodt

makeodt: $(CFILES:.c=.o) ../BspLib/libbsp.a ../QvLib/libqv.a
	$(CC) -o makeodt $(CFILES:.c=.o) ../BspLib/libbsp.a ../QvLib/libqv.a -lstdc++

# use this if you've compiled BspLib with JPEG support
#makeodt: $(CFILES:.c=.o) ../BspLib/libbsp.a ../QvLib/libqv.a
#	$(CC) -o makeodt $(CFILES:.c=.o) ../BspLib/libbsp.a ../QvLib/libqv.a -lstdc++ -ljpeg



