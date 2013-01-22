CXXFLAGS=-g -m32 -march=i686 -I/opt/include -Wno-write-strings -ggdb -DSYSTEM_ENABLE_SDL -I/opt/include/
LDFLAGS=-L/usr/lib -L/usr/X11R6/lib -L/opt/lib -ldl -lSDL_mixer -lSDL -lGL -ljpeg -ggdb
LIBPARSEC_LD_FLAGS=-m elf_i386

