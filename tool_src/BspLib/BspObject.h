//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BspObject.h
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPOBJECT_H_
#define _BSPOBJECT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SystemIO.h"
#include "Chunk.h"
#include "Face.h"
#include "Mapping.h"
#include "PolygonList.h"
#include "BSPTree.h"
#include "Texture.h"
#include "Transform3.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


class BoundingBox;


// base class for BspObject providing data members ----------------------------
//
class BspObjectInfo {

	friend class BspObject;

private:
	BspObjectInfo();
	~BspObjectInfo() { }

public:
	int	getNumVertices() const		{ return numvertices; }
	int	getNumPolygons() const		{ return numpolygons; }
	int	getNumFaces() const			{ return numfaces; }

	int	getInputVertices() const	{ return numvertices_in; }
	int	getInputPolygons() const	{ return numpolygons_in; }
	int	getInputFaces() const		{ return numfaces_in; }

	int getBspPolygons() const		{ return numbsppolygons; }
	int getPreBspPolygons() const	{ return numpolygons_before_bsp; }

	int	getNumTextures() const		{ return numtextures; }
	int	getNumMappings() const		{ return numcorrespondences; }
	int	getNumNormals() const		{ return numnormals; }
	int	getNumTexturedFaces() const	{ return numtexmappedfaces; }

	int getNumTraceVertices() const	{ return numtracevertices; }
	int getNumMultiVertices() const	{ return nummultvertices; }
	int getNumSplitQuads() const	{ return numsplitquadrilaterals; }

protected:
	int numvertices;				// number of vertices in vertexlist
	int numpolygons;				// number of polygons in polygonlist
	int numfaces;					// number of faces in facelist
	int numtextures;				// number of textures in texturelist
	int numcorrespondences;			// number of correspondences in mappinglist

	int numnormals;					// number of faces with valid normals
	int numtexmappedfaces;			// number of texture mapped faces

	int numvertices_in;				// copy of numvertices directly after parse
	int numpolygons_in;				// copy of numpolygons directly after parse
	int numfaces_in;				// copy of numfaces directly after parse

	int numbsppolygons;				// number of polygons contained in bsp tree
	int numpolygons_before_bsp;		// copy of numpolygons before start of bsp compilation

	int numtracevertices;			// number of vertices for edge tracing
	int nummultvertices;			// number of multiply contained vertices
	int numsplitquadrilaterals;		// number of quadrilaterals split into triangles
};


// class describing a generic 3-D object in BspLib ----------------------------
//
class BspObject : public BspObjectInfo, public virtual SystemIO {

	friend class BoundingBox;
	friend class BspObjectListRep;
	friend class Face;
	friend class ObjectBSPNode;
	friend class Polygon;

	friend class VrmlFile;

public:
	BspObject();
	~BspObject() { delete objectname; delete next; }

	BspObject( const BspObject& copyobj );
	BspObject& operator =( const BspObject& copyobj );

	// merge another BspObject into this object. this renumbers the other
	// object's vertices, faces, polygons, and vertexindexes in polygons
	void			MergeObjects( BspObject *mergeobj );

	void			CollapseObjectList();		// collapse entire list into this object

	BoundingBox*	BuildBoundingBoxList();		// build a list of bounding boxes

	BSPNode*		BuildBSPTree();				// build bsp tree from polygon list
	BSPNode*		BuildBSPTreeFromFlat();		// build bsp tree from flat representation

	int				BspTreeAvailable();			// any bsp tree available?
	int				BSPTreeAvailable();			// linked bsp tree available?
	int				BSPTreeFlatAvailable();		// flat bsp tree available?

	// processing functions (typically passed through to primitives)
	void			CalcBoundingBoxes();		// operates on bsp tree only
	void			CalcSeparatorPlanes();		// operates on bsp tree only
	void			CheckEdges();				// operates on bsp tree only
	void			CheckPolygonPlanes();		// operates on polygon list only
	void			CheckVertices( int verbose ); // scans entire vertex list
	void			CalcPlaneNormals();			// operates on polygon list only
	void			CheckParsedData();			// some consistency checks
	void			UpdateAttributeNumbers();	// calc length of lists

	int				ConvertColorIndexesToRGB( char *palette, int changemode );

	// return bounding box extents for this object
	void			CalcBoundingBox( Vertex3& minvertex, Vertex3& maxvertex );

	// display some object statistics
	void			DisplayStatistics();

	// ASCII output functions that have to be implemented by
	// derived ObjectXXXFormat classes.
	// must not be pure virtual!
	virtual int		WriteVertexList( FileAccess& fp ) { return FALSE; }
	virtual int		WritePolygonList( FileAccess& fp ) { return FALSE; }
	virtual int		WriteFaceList( FileAccess& fp ) { return FALSE; }
	virtual int		WriteFaceProperties( FileAccess& fp ) { return FALSE; }
	virtual int		WriteTextureList( FileAccess& fp ) { return FALSE; }
	virtual int		WriteMappingList( FileAccess& fp ) { return FALSE; }
	virtual int		WriteNormals( FileAccess& fp ) { return FALSE; }
	virtual int		WriteBSPTree( FileAccess& fp ) { return FALSE; }

	// return contained objects
	VertexChunk&	getVertexList()		{ return vertexlist; }
	PolygonList&	getPolygonList()	{ return polygonlist; }
	FaceChunk&		getFaceList()		{ return facelist; }
	TextureChunk&	getTextureList()	{ return texturelist; }
	MappingChunk&	getMappingList()	{ return mappinglist; }
	BSPTree&		getBSPTree()		{ return bsptree; }
	BSPTreeFlat&	getBSPTreeFlat()	{ return bsptreeflat; }

	// get/set object's name-string
	char*			getObjectName()	const { return objectname; }
	void			setObjectName( char *name );

	// get/set object's local transformation
	Vertex3			getCenterInWorldSpace() const { return objecttrafo.FetchTranslation(); }
	Transform3		getObjectTransformation() const { return objecttrafo; }
	void			setObjectTransformation( const Transform3& ot ) { objecttrafo = ot; }

	// scale entire object (all vertices)
	void			ApplyScale( double scalefac );

	// apply translation to all coordinates and set center to (0,0,0)
	void			ApplyCenter();

	// apply transformation matrix and set to identity afterwards
	void			ApplyTransformation();

	// return next object in list
	BspObject*		getNext() const { return next; }

private:
	void			InconsistencyError( const char *err );

public:
	static int		getEliminateDoubletsOnMergeFlag() { return check_vertex_doublets_on_merge; }
	static void		setEliminateDoubletsOnMergeFlag( int flag ) { check_vertex_doublets_on_merge = flag; }

private:
	static int		check_vertex_doublets_on_merge;

protected:
	// contained objects
	VertexChunk		vertexlist;		// list of vertices
	PolygonList		polygonlist;	// list of polygons
	FaceChunk		facelist;		// list of faces
	TextureChunk	texturelist;	// list of textures
	MappingChunk	mappinglist;	// list of mappings
	BSPTree			bsptree;		// linked bsp tree
	BSPTreeFlat		bsptreeflat;	// flat bsp tree
	Transform3		objecttrafo;	// attached transformation

	// object's name if any defined
	char*			objectname;

	// pointer to next object in list
	BspObject*		next;
};

// copy constructor -----------------------------------------------------------
inline BspObject::BspObject( const BspObject& copyobj ) :
	BspObjectInfo( copyobj ),
	vertexlist( copyobj.vertexlist ),
	polygonlist( copyobj.polygonlist ),
	facelist( copyobj.facelist ),
	texturelist( copyobj.texturelist ),
	mappinglist( copyobj.mappinglist ),
	bsptree( copyobj.bsptree ),
	bsptreeflat( copyobj.bsptreeflat ),
	objecttrafo( copyobj.objecttrafo )
{
	// copy object's name
	objectname = NULL;
	setObjectName( copyobj.objectname );

	// unlink tail of list
	next = NULL;
}

// assignment operator --------------------------------------------------------
inline BspObject& BspObject::operator =( const BspObject& copyobj )
{
	if ( &copyobj != this ) {

		// copy info part
		*(BspObjectInfo *)this = copyobj;

		// copy contained objects
		vertexlist	= copyobj.vertexlist;
		polygonlist	= copyobj.polygonlist;
		facelist	= copyobj.facelist;
		texturelist	= copyobj.texturelist;
		mappinglist	= copyobj.mappinglist;
		bsptree		= copyobj.bsptree;
		bsptreeflat	= copyobj.bsptreeflat;
		objecttrafo	= copyobj.objecttrafo;

		// copy object's name
		setObjectName( copyobj.objectname );

		// unlink tail of list
		next = NULL;
	}
	return *this;
}


BSPLIB_NAMESPACE_END


#endif // _BSPOBJECT_H_

