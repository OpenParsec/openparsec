//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Polygon.h
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _POLYGON_H_
#define _POLYGON_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Chunk.h"
#include "Face.h"
#include "Plane.h"
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


class BoundingBox;


// vertex index (element of vertex index list) --------------------------------
//
class VIndx {

public:
	VIndx( int indx = -1, VIndx *next = NULL ) { vertindx = indx; nextvertindx = next; }
	~VIndx() { delete nextvertindx; }

	int		getIndx() const { return vertindx; }
	void	setIndx( int indx ) { vertindx = indx; }

	VIndx*	getNext() const { return nextvertindx; }
	void	setNext( VIndx *next ) { nextvertindx = next; }

private:
	int		vertindx;			// -1 means end of list, regardless of nextvertindx
	VIndx*	nextvertindx;		// pointer to next VIndx in singly linked list
};


class BSPNode;
class BspObject;


// single polygon (element of polygon list) -----------------------------------
//
class Polygon : public virtual SystemIO {

	friend class PolygonListRep;

	void	Error() const;

	// possible classifications of polygon with respect to splitting plane
	enum {
		POLY_IN_FRONT_SUBSPACE,
		POLY_IN_BACK_SUBSPACE,
		POLY_STRADDLES_SPLITTER,
		POLY_IN_SAME_PLANE,
	};

	// frontsubspace/backsubspace identifiers
	enum {
		FRONT_SUBSPACE	= 1,
		BACK_SUBSPACE	= -1
	};

public:

	// splitter selection criteria
	enum {
		SPLITTERCRIT_FIRST_POLY		= 0x0000,	// always choose first polygon in list
		SPLITTERCRIT_SAMPLE_FIRST_N	= 0x0101,	// test first n polygons in list
		SPLITTERCRIT_SAMPLE_ALL		= 0x0002,	// test entire list and choose best
		SPLITTERCRIT_RANDOM_SAMPLE	= 0x0103,	// sample n polygons randomly
		SPLITTERCRITMASK_SAMPLESIZ	= 0x0100,	// flagmask if samplesize (n) needed
	};

	// message flags
	enum {
		MESSAGE_SPLITTING_POLYGON				= 0x0001,
		MESSAGE_STARTVERTEX_IN_SPLITTER_PLANE	= 0x0002,
		MESSAGE_VERTEX_IN_SPLITTER_PLANE		= 0x0004,
		MESSAGE_NEW_SPLITVERTEX					= 0x0008,
		MESSAGE_REUSING_SPLITVERTEX				= 0x0010,
		MESSAGE_TRACEVERTEX_INSERTED			= 0x0020,
		MESSAGE_SPLITTING_QUADRILATERAL			= 0x0040,
		MESSAGE_INVOCATION						= 0x0080,
		MESSAGE_CHECKING_POLYGON_PLANES			= 0x0100,
		MESSAGEMASK_DISPLAY_ALL					= 0xffff
	};

public:
	Polygon( BspObject *bobj, int pno = 0, int fno = 0, Polygon *next = NULL, int num = 1 );
	~Polygon() { delete vertindxs; delete nextpolygon; }

	Polygon*	NewPolygon();						// create new polygon with sequential id as head of list
	Polygon*	InsertPolygon( Polygon *poly );		// insert existing polygon as head of list
	Polygon*	DeleteHead();						// delete this, return rest of list (DANGEROUS!)
	Polygon*	FindPolygon( int id );				// return polygon in list having specified id
	int			SumVertexNumsEntireList();			// sum up vertex numbers for all polygons of list

	void		PrependNewVIndx( int indx );		// create new VIndx and prepend it to list (vertindxs)
	void		AppendNewVIndx( int indx = -1 );	// create new VIndx and append it to list (vertindxs)
	void		AppendVIndx( VIndx *vindx );		// append existing VIndx to list (vertindxs)
	VIndx*		UnlinkLastVIndx();					// return last VIndx in list after unlinking it

	void		CalcPlaneNormals();					// calc normals for all polygons in list
	void		CheckEdges();						// check edges for contained vertices (scans list!)
	Polygon*	CheckPlanesAndMappings();			// check planes and mappings of all polygons in list

	// calculate bounding box encompassing all polygons in list
	void		CalcBoundingBox( BoundingBox* &boundingbox );

	// classify polygon with respect to this polygon
	int			CheckIntersection( Polygon *testpoly );
	// check if other polygon's normal is contained in this one's positive halfspace (predicate)
	int			NormalDirectionSimilar( Polygon *testpoly );
	// split other polygon along this polygon; insert split pieces into their respective halfspaces
	void		SplitPolygon( Polygon *poly, Polygon* &frontsubspace, Polygon* &backsubspace );
	// partition space encompassing entire polygon list; return created BSP tree's root
	BSPNode*	PartitionSpace();

	// correct numberings (polygon-id, face-id, and vertex indexes) to new base
	void		CorrectBase( BspObject *newbaseobj, int vertexindxbase, int faceidbase, int polygonidbase );
	void		CorrectBaseByTable( BspObject *newbaseobj, int *vtxindxmap, int faceidbase, int polygonidbase );

	int			getId() const { return polygonno; }
	void		setId( int pno ) { polygonno = pno; }

	int			getFaceId() const { return faceno; }
	void		setFaceId( int fno ) { faceno = fno; }

	int			getNumPolygons() const { return numpolygons; }
	void		setNumPolygons( int len ) { numpolygons = len; }

	Polygon*	getNext() const { return nextpolygon; }
	void		setNext( Polygon *next ) { nextpolygon = next; }

	int			getNumVertices() const { return numvertindxs; }

	VIndx*		getVList() const { return vertindxs; }
	BspObject*	getBaseObject() const { return baseobject; }

	VertexChunk& getVertexList();
	FaceChunk&	getFaceList();

	int			HasArea() const;

	int			getFirstVertexIndx() const;
	int			getSecondVertexIndx() const;
	int			getThirdVertexIndx() const;

	Vertex3		getFirstVertex() const;
	Vertex3		getSecondVertex() const;
	Vertex3		getThirdVertex() const;

	Plane		getPlane() const { return ((Polygon *const)this)->getFaceList()[ faceno ].getPlane(); }
	Vector3		getPlaneNormal() const { return ((Polygon *const)this)->getFaceList()[ faceno ].getPlaneNormal(); }

	void		FillVertexIndexArray( dword *arr ) const;		// write list of vertices into array
	void		WriteVertexList( FILE *fp, int cr ) const;		// write list of vertices to file
	void		WritePolyList( FILE *fp ) const;				// write list of polygon ids to file

	int			CalcSplitterTestProbability();

public:
	static int	getSplitterSelection() { return splitter_crit; }
	static void	setSplitterSelection( int criterion ) { splitter_crit = criterion; }

	static int	getSampleSize() { return sample_size; }
	static void	setSampleSize( int siz ) { sample_size = siz; }

	static int	getTriangulationFlag() { return triangulate_all; }
	static void setTriangulationFlag( int flag ) { triangulate_all = flag; }

	static int	getNormalizeVectorsFlag() { return normalize_vectors; }
	static void setNormalizeVectorsFlag( int flag ) { normalize_vectors = flag; }

	static int	getDisplayMessagesFlag() { return display_messages; }
	static void setDisplayMessagesFlag( int flag ) { display_messages = flag; }

	static void	ResetCallCount() { partition_callcount = 0; }

private:
	static int	splitter_crit;		// splitter selection criterion to use
	static int	sample_size;		// sample size for splitter sampling
	static int	triangulate_all;	// triangulate polygons with more than three vertices
	static int	normalize_vectors;	// always normalize direction vectors
	static int	display_messages;	// display a message every time PartitionSpace() is invoked
	static int	partition_callcount;// call counter for PartitionSpace()
	static int	test_probability;	// probability to test single polygon if SPLITTERCRIT_RANDOM_SAMPLE
	static char	str_scratchpad[];	// string scratch pad

private:
	int			polygonno;			// global number of this polygon
	int			faceno;				// global number of face this polygon is contained in
	int			numpolygons;		// length of singly linked polygon list
	int			numvertindxs;		// number of entries in vertex index list attached to this polygon
	VIndx*		vertindxs;			// pointer to first VIndx
	VIndx*		vindxinsertpos;		// pointer to last VIndx (NULL means insert-position is at head)
	BspObject*	baseobject;			// pointer to object this polygon belongs to
	Polygon*	nextpolygon;		// pointer to next polygon in list
};

// determine if the polygon has area ------------------------------------------
inline int Polygon::HasArea() const
{
	// collinear vertices are not checked here, the polygon need only have
	// at least three vertices to count as having area!
	return ( vertindxs && vertindxs->getNext() && vertindxs->getNext()->getNext() );
}

// get first vertex of polygon ------------------------------------------------
inline int Polygon::getFirstVertexIndx() const
{
	CHECK_DEREFERENCING(
		if ( vertindxs == NULL )
			Error();
	);
	return vertindxs->getIndx();
}
inline Vertex3 Polygon::getFirstVertex() const
{
	CHECK_DEREFERENCING(
		if ( vertindxs == NULL )
			Error();
	);
	return ((Polygon *const)this)->getVertexList()[ vertindxs->getIndx() ];
}

// get second vertex of polygon -----------------------------------------------
inline int Polygon::getSecondVertexIndx() const
{
	CHECK_DEREFERENCING(
		if ( ( vertindxs == NULL ) || ( vertindxs->getNext() == NULL ) )
			Error();
	);
	return vertindxs->getNext()->getIndx();
}
inline Vertex3 Polygon::getSecondVertex() const
{
	CHECK_DEREFERENCING(
		if ( ( vertindxs == NULL ) || ( vertindxs->getNext() == NULL ) )
			Error();
	);
	return ((Polygon *const)this)->getVertexList()[ vertindxs->getNext()->getIndx() ];
}

// get third vertex of polygon ------------------------------------------------
inline int Polygon::getThirdVertexIndx() const
{
	CHECK_DEREFERENCING(
		if ( ( vertindxs == NULL ) ||
			 ( vertindxs->getNext() == NULL ) ||
			 ( vertindxs->getNext()->getNext() == NULL ) )
			Error();
	);
	return vertindxs->getNext()->getNext()->getIndx();
}
inline Vertex3 Polygon::getThirdVertex() const
{
	CHECK_DEREFERENCING(
		if ( ( vertindxs == NULL ) ||
			 ( vertindxs->getNext() == NULL ) ||
			 ( vertindxs->getNext()->getNext() == NULL ) )
			Error();
	);
	return ((Polygon *const)this)->getVertexList()[ vertindxs->getNext()->getNext()->getIndx() ];
}


BSPLIB_NAMESPACE_END


#endif // _POLYGON_H_

