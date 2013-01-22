//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BSPTree.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BSPTree.h"


BSPLIB_NAMESPACE_BEGIN


#define BLOCK_SIZE 4096


// append new node to flat bsp tree -------------------------------------------
//
BSPNodeFlat *BSPTreeFlatRep::AppendNode( int front, int back, Polygon *poly, int clist, int blist )
{
	// expand storage area (array) if no space available
	if ( nodestorage - numnodes == 0 ) {
		BSPNodeFlat *newroot = new BSPNodeFlat[ nodestorage + BLOCK_SIZE ];
		if ( root != NULL ) {
			memcpy( newroot, root, nodestorage * sizeof( BSPNodeFlat ) );
			delete[] root;
		}
		root = newroot;
		nodestorage += BLOCK_SIZE;
	}
	// init new node and return its address
	root[ numnodes ].InitNode( front, back, poly, clist, blist );
	return root + numnodes++;
}


// fetch node per id (node number) --------------------------------------------
//
BSPNodeFlat *BSPTreeFlatRep::FetchNodePerId( int id )
{
	// id has to be greater than 0 since 0 means empty halfspace
	return ( ( id > 0 ) && ( id <= numnodes ) ) ? ( root + id - 1 ) : NULL;
}


// apply scale factor to separator planes and bounding boxes of all nodes -----
//
void BSPTreeFlatRep::ApplyScaleFactor( double sfac )
{
	BSPNodeFlat *scan = root;
	for ( int i = 0; i < numnodes; i++, scan++ )
		scan->ApplyScaleFactor( sfac );
}


// build pointer-based bsp tree from flat representation ----------------------
//
BSPNode *BSPTreeFlatRep::BuildBSPTree( int nodenum )
{
	if ( nodenum == 0 )
		return NULL;

	BSPNodeFlat *node = FetchNodePerId( nodenum );
	if ( node == NULL )
		return NULL;

	// get node's polygon
	Polygon	*poly = node->getPolygon();

	// append contained list to polygon (frontfacing polygons)
	static int containedindx;
	static Polygon *precpoly;
	containedindx = node->getContainedList();
	precpoly	  = poly;
	while ( containedindx > 0 ) {
		static BSPNodeFlat *cnode;
		static Polygon *cpoly;
		cnode = FetchNodePerId( containedindx );
		cpoly = cnode ? cnode->getPolygon() : NULL;
		containedindx = cnode ? cnode->getContainedList() : 0;
		precpoly->setNext( cpoly );
		precpoly = cpoly;
	}
	if ( precpoly ) {
		// detach original polygon list
		precpoly->setNext( NULL );
	}

	// build list of backfacing polygons (backlist)
	Polygon *backlist = NULL;
	containedindx	  = node->getBackList();
	precpoly		  = NULL;
	while ( containedindx > 0 ) {
		static BSPNodeFlat *cnode;
		static Polygon *cpoly;
		cnode = FetchNodePerId( containedindx );
		cpoly = cnode ? cnode->getPolygon() : NULL;
		containedindx = cnode ? cnode->getBackList() : 0;
		if ( precpoly == NULL)
			backlist = cpoly;
		else
			precpoly->setNext( cpoly );
		precpoly = cpoly;
	}
	if ( precpoly ) {
		// detach original polygon list
		precpoly->setNext( NULL );
	}

	// recursively build front- and back-subtree
	BSPNode *front = BuildBSPTree( node->getFrontSubTree() );
	BSPNode *back  = BuildBSPTree( node->getBackSubTree() );

	return new BSPNode( front, back, poly, backlist, node->getSeparatorPlane(), node->getBoundingBox() );
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
