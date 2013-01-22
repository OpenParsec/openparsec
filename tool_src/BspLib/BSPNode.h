//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BSPNode.h
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPNODE_H_
#define _BSPNODE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "PolygonList.h"
#include "Plane.h"


BSPLIB_NAMESPACE_BEGIN


class BoundingBox;
class BspObject;


// node of bsp tree -----------------------------------------------------------
//
class BSPNode {

public:

	// output formats
	enum {
		OUTPUT_OLD_STYLE,
		OUTPUT_KEY_VALUE_STYLE
	};

public:
	BSPNode( BSPNode *front = NULL, BSPNode *back = NULL,
			 Polygon *poly = NULL, Polygon *backpoly = NULL,
			 Plane *sep = NULL, BoundingBox *box = NULL, int num = -1 );
	~BSPNode();

public:
	void		NumberBSPNodes( int& curno );
	void		SumVertexNums( int& vtxnum );
	void		CorrectPolygonBases( BspObject *newbaseobj, int vertexindxbase, int faceidbase, int polygonidbase );
	void		CorrectPolygonBasesByTable( BspObject *newbaseobj, int *vtxindxmap, int faceidbase, int polygonidbase );
	void		WriteBSPTree( FILE *fp );
	void		CheckEdges();
	void		FetchFacePolygons( int facno, PolygonList& facepolylist );
	Polygon*	FetchBSPPolygon( int polyno );
	void		CalcBoundingBoxes();
	void		CalcSeparatorPlanes();

	int			getNodeNumber() const { return nodenumber; }
	Polygon*	getPolygon() { return polygon; }
	Polygon*	getBackPolygon() { return backpolygon; }
	BSPNode*	getFrontSubtree() const { return frontsubtree; }
	BSPNode*	getBackSubtree() const { return backsubtree; }

	Plane*		getSeparatorPlane() { return separatorplane; }
	void		setSeparatorPlane( Plane *sep ) { separatorplane = sep; }

	BoundingBox*getBoundingBox() { return boundingbox; }
	void		setBoundingBox( BoundingBox *box ) { boundingbox = box; }

private:
	void		GrowBoundingBox( BSPNode *othernode );

public:
	static int	getOutputFormat() { return outputformat; }
	static void	setOutputFormat( int format ) { outputformat = format; }

private:
	static int	outputformat;	// format used to write bsp nodes to files

private:
	int			nodenumber;		// node id
	Polygon*	polygon;		// list of frontfacing polygons in splitting plane
	Polygon*	backpolygon;	// list of backfacing polygons in splitting plane
	Plane*		separatorplane;	// plane if node is only separator (no polygons!)
	BoundingBox*boundingbox;	// bounding box containing node and all children
	BSPNode*	frontsubtree;	// tree partitioning front halfspace
	BSPNode*	backsubtree;	// tree partitioning back halfspace
};


// node of flat bsp tree ------------------------------------------------------
//
class BSPNodeFlat {

public:
	BSPNodeFlat( int front = 0, int back = 0,
				 Polygon *poly = NULL, int clist = 0, int blist = 0,
				 Plane *sep = NULL, BoundingBox *box = NULL, int num = -1 );
	~BSPNodeFlat() { /* don't delete polygon, separatorplane, and boundingbox!! */ }

public:
	void		InitNode( int front, int back, Polygon *poly, int clist, int blist,
						  Plane *sep = NULL, BoundingBox *box = NULL, int num = -1 );

	void		ApplyScaleFactor( double sfac );

	int			getNodeNumber() const { return nodenumber; }
	Polygon*	getPolygon() { return polygon; }
	int			getContainedList() const { return containedlistindx; }
	int			getBackList() const { return backlistindx; }
	int			getFrontSubTree() const { return frontsubtreeindx; }
	int			getBackSubTree() const { return backsubtreeindx; }

	Plane*		getSeparatorPlane() { return separatorplane; }
	void		setSeparatorPlane( Plane *sep ) { separatorplane = sep; }

	BoundingBox*getBoundingBox() { return boundingbox; }
	void		setBoundingBox( BoundingBox *box ) { boundingbox = box; }

private:
	int			nodenumber;			// node id (may be different than array index!!)
	Polygon*	polygon;			// this node's polygon
	Plane*		separatorplane;		// plane if node is only separator (no polygons!)
	BoundingBox*boundingbox;		// bounding box containing node and all children
	int			containedlistindx;	// contained frontfacing polygons
	int			backlistindx;		// contained backfacing polygons
	int			frontsubtreeindx;	// front subtree
	int			backsubtreeindx;	// back subtree
};


BSPLIB_NAMESPACE_END


#endif // _BSPNODE_H_

