# BspLib makefile

CFILES = AodFormat.cpp\
AodInput.cpp\
AodOutput.cpp\
BRep.cpp\
BSPNode.cpp\
BSPTree.cpp\
BoundingBox.cpp\
BspFormat.cpp\
BspInput.cpp\
BspObject.cpp\
BspObjectList.cpp\
BspOutput.cpp\
BspTool.cpp\
EpsAreas.cpp\
Face.cpp\
IOData3D.cpp\
InputData3D.cpp\
LineSeg.cpp\
Mapping.cpp\
ObjectAodFormat.cpp\
ObjectBSPNode.cpp\
ObjectBinFormat.cpp\
ObjectBspFormat.cpp\
OutputData3D.cpp\
Polygon.cpp\
PolygonList.cpp\
SingleFormat.cpp\
SingleInput.cpp\
SingleOutput.cpp\
SystemIO.cpp\
Texture.cpp\
Transform2.cpp\
Transform3.cpp\
TriMapping.cpp\
Vertex.cpp\
VertexChunk.cpp\
VrmlFile.cpp


CFLAGS = -g -O2 -I../QvLib -I. -fno-for-scope\
         -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp

# use this for JPEG support
#CFLAGS = -g -O2 -I../QvLib -I. -fno-for-scope\
#         -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -DUSE_JPEG_LIBRARY

all: libbsp.a

libbsp.a: $(CFILES:.cpp=.o)
	ar rc libbsp.a $(CFILES:.cpp=.o)
	ranlib libbsp.a

.cpp.o:
	gcc $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a *~



