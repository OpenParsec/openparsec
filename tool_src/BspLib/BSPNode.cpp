//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BSPNode.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BSPNode.h"
#include "BoundingBox.h"
#include "BspFormat.h"


BSPLIB_NAMESPACE_BEGIN


// construct BSPNode ----------------------------------------------------------
//
BSPNode::BSPNode( BSPNode *front, BSPNode *back, Polygon *poly, Polygon *backpoly, Plane *sep, BoundingBox *box, int num )
{
	frontsubtree	= front;
	backsubtree		= back;
	polygon			= poly;
	backpolygon		= backpoly;
	separatorplane	= sep;
	boundingbox		= box;
	nodenumber		= num;
}


// destroy BSPNode ------------------------------------------------------------
//
BSPNode::~BSPNode()
{
	delete polygon;
	delete backpolygon;
	delete separatorplane;
	delete boundingbox;
	delete frontsubtree;
	delete backsubtree;
}


// traverse bsp tree (preorder) and number nodes as encountered ---------------
//
void BSPNode::NumberBSPNodes( int& curno )
{
	// store nodenumber
	nodenumber = ++curno;

	// assign node numbers to all polygons contained in splitter plane
	static Polygon *polylist;
	for ( polylist = polygon ? polygon->getNext() : NULL; polylist; polylist = polylist->getNext() ) {
		// only increment numbering, numbers don't get stored in polygons!
		++curno;
	}
	for ( polylist = backpolygon; polylist; polylist = polylist->getNext() ) {
		// only increment numbering, numbers don't get stored in polygons!
		++curno;
	}

	// do numbering recursively for front- and backsubtree
	if ( frontsubtree ) frontsubtree->NumberBSPNodes( curno );
	if ( backsubtree ) backsubtree->NumberBSPNodes( curno );
}


// sum number of vertices of all polygon in tree ------------------------------
//
void BSPNode::SumVertexNums( int& vtxnum )
{
	// sum number of vertices of polygons contained in this plane
	static Polygon *polylist;
	for ( polylist = polygon; polylist; polylist = polylist->getNext() )
		vtxnum += polylist->getNumVertices();
	for ( polylist = backpolygon; polylist; polylist = polylist->getNext() )
		vtxnum += polylist->getNumVertices();

	// do sum recursively for front- and backsubtree
	if ( frontsubtree ) frontsubtree->SumVertexNums( vtxnum );
	if ( backsubtree ) backsubtree->SumVertexNums( vtxnum );
}


// traverse entire tree and correct relative polygon info to new base values --
//
void BSPNode::CorrectPolygonBases( BspObject *newbaseobj, int vertexindxbase, int faceidbase, int polygonidbase )
{
	static Polygon *polylist;
	for ( polylist = polygon; polylist; polylist = polylist->getNext() )
		polylist->CorrectBase( newbaseobj, vertexindxbase, faceidbase, polygonidbase );
	for ( polylist = backpolygon; polylist; polylist = polylist->getNext() )
		polylist->CorrectBase( newbaseobj, vertexindxbase, faceidbase, polygonidbase );

	if ( frontsubtree ) frontsubtree->CorrectPolygonBases( newbaseobj, vertexindxbase, faceidbase, polygonidbase );
	if ( backsubtree ) backsubtree->CorrectPolygonBases( newbaseobj, vertexindxbase, faceidbase, polygonidbase );
}


// traverse entire tree and correct relative polygon info to new base values --
//
void BSPNode::CorrectPolygonBasesByTable( BspObject *newbaseobj, int *vtxindxmap, int faceidbase, int polygonidbase )
{
	static Polygon *polylist;
	for ( polylist = polygon; polylist; polylist = polylist->getNext() )
		polylist->CorrectBaseByTable( newbaseobj, vtxindxmap, faceidbase, polygonidbase );
	for ( polylist = backpolygon; polylist; polylist = polylist->getNext() )
		polylist->CorrectBaseByTable( newbaseobj, vtxindxmap, faceidbase, polygonidbase );

	if ( frontsubtree ) frontsubtree->CorrectPolygonBasesByTable( newbaseobj, vtxindxmap, faceidbase, polygonidbase );
	if ( backsubtree ) backsubtree->CorrectPolygonBasesByTable( newbaseobj, vtxindxmap, faceidbase, polygonidbase );
}


// write bsp tree structure to output file (preorder traversal) ---------------
//
void BSPNode::WriteBSPTree( FILE *fp )
{
	static int lnodnum, rnodnum, cnodnum, bnodnum;
	static int curnodnum;
	static Polygon *polylist;

	lnodnum = frontsubtree ? frontsubtree->nodenumber : 0;
	rnodnum = backsubtree  ? backsubtree->nodenumber  : 0;

	if ( polygon != NULL ) {
		cnodnum = polygon->getNext() ? nodenumber + 1 : 0;
		bnodnum = backpolygon ? nodenumber + polygon->getNumPolygons() : 0;
	} else {
		cnodnum = 0;
		bnodnum = 0;
	}

	if ( outputformat == OUTPUT_OLD_STYLE ) {

		if ( polygon != NULL ) {
			// print head node
			fprintf( fp, "%d: %d |%d|%d|%d-%d|\n",
					 nodenumber, polygon->getId() + 1, cnodnum, bnodnum, lnodnum, rnodnum );

			// print list of contained nodes (frontfacing)
			curnodnum = cnodnum;
			polylist  = polygon;
			while ( ( polylist = polylist->getNext() ) != NULL ) {
				cnodnum = polylist->getNext() ? ( curnodnum + 1 ) : 0;
				fprintf( fp, "%d: %d |%d|0|\n", curnodnum++, polylist->getId() + 1, cnodnum );
			}

			// print list of contained nodes (backfacing)
			curnodnum = bnodnum;
			for ( polylist = backpolygon; polylist; polylist = polylist->getNext() ) {
				bnodnum = polylist->getNext() ? ( curnodnum + 1 ) : 0;
				fprintf( fp, "%d: %d |0|%d|\n", curnodnum++, polylist->getId() + 1, bnodnum );
			}
		} else {
			//NOTE:
			// nodes not containing polygons are not supported
			// by the old ouput format!
			fprintf( fp, "no polygon contained in node %d.", nodenumber );
		}

	} else {

		// print head node
		fprintf( fp, "%d: ", nodenumber );
		if ( polygon != NULL )
			fprintf( fp, "%s %d ", BspFormat::_bspspec_polygon_str, polygon->getId() + 1 );
		if ( cnodnum != 0 )
			fprintf( fp, "%s %d ", BspFormat::_bspspec_frontlist_str, cnodnum );
		if ( bnodnum != 0 )
			fprintf( fp, "%s %d ", BspFormat::_bspspec_backlist_str, bnodnum );
		if ( lnodnum != 0 )
			fprintf( fp, "%s %d ", BspFormat::_bspspec_fronttree_str, lnodnum );
		if ( rnodnum != 0 )
			fprintf( fp, "%s %d ", BspFormat::_bspspec_backtree_str, rnodnum );
		if ( separatorplane != NULL ) {
			Vector3 normal = separatorplane->getPlaneNormal();
			fprintf( fp, "%s %f %f %f %f ", BspFormat::_bspspec_plane_str,
					 normal.getX(), normal.getY(), normal.getZ(),
					 separatorplane->getPlaneOffset() );
		}
		if ( boundingbox != NULL ) {
			Vertex3 minvert = boundingbox->getMinVertex();
			Vertex3 maxvert = boundingbox->getMaxVertex();
			fprintf( fp, "%s %f %f %f %f %f %f ", BspFormat::_bspspec_boundingbox_str, 
					 minvert.getX(), minvert.getY(), minvert.getZ(),
					 maxvert.getX(), maxvert.getY(), maxvert.getZ()	);
		}
		fprintf( fp, "\n" );

		// print list of contained nodes (frontfacing)
		if ( polygon != NULL ) {
			curnodnum = cnodnum;
			polylist  = polygon;
			while ( ( polylist = polylist->getNext() ) != NULL ) {
				fprintf( fp, "%d: ", curnodnum );
				cnodnum = polylist->getNext() ? ++curnodnum : 0;
				fprintf( fp, "%s %d ", BspFormat::_bspspec_polygon_str, polylist->getId() + 1 );
				if ( cnodnum != 0 )
					fprintf( fp, "%s %d", BspFormat::_bspspec_frontlist_str, cnodnum );
				fprintf( fp, "\n" );
			}
		}

		// print list of contained nodes (backfacing)
		curnodnum = bnodnum;
		for ( polylist = backpolygon; polylist; polylist = polylist->getNext() ) {
			fprintf( fp, "%d: ", curnodnum );
			bnodnum = polylist->getNext() ? ++curnodnum : 0;
			fprintf( fp, "%s %d ", BspFormat::_bspspec_polygon_str, polylist->getId() + 1 );
			if ( bnodnum != 0 )
				fprintf( fp, "%s %d", BspFormat::_bspspec_backlist_str, bnodnum );
			fprintf( fp, "\n" );
		}

	}

	// write out front- and backsubtree recursively
	if ( frontsubtree ) frontsubtree->WriteBSPTree( fp );
	if ( backsubtree ) backsubtree->WriteBSPTree( fp );
}


// fetch pointer to polygon with given number contained in bsp tree -----------
//
Polygon *BSPNode::FetchBSPPolygon( int polyno )
{
	// search local node and contained list (frontfacing)
	static Polygon *clist;
	for ( clist = polygon; clist; clist = clist->getNext() ) {
		if ( clist->getId() == polyno )
			return clist;
	}

	// search list of contained backfacing polygons
	for ( clist = backpolygon; clist; clist = clist->getNext() ) {
		if ( clist->getId() == polyno )
			return clist;
	}

	Polygon *search = NULL;

	if ( frontsubtree )
		search = frontsubtree->FetchBSPPolygon( polyno );
	if ( ( search == NULL ) && backsubtree )
		search = backsubtree->FetchBSPPolygon( polyno );

	return search;
}


// fetch all polygon numbers contained in specific face -----------------------
//
void BSPNode::FetchFacePolygons( int faceno, PolygonList& facepolylist )
{
	// search local node and contained list (frontfacing)
	static Polygon *clist;
	for ( clist = polygon; clist; clist = clist->getNext() ) {
		if ( clist->getFaceId() == faceno )
			facepolylist.InsertPolygon( new Polygon( NULL, clist->getId(), clist->getFaceId() ) );
	}

	// search list of contained backfacing polygons
	for ( clist = backpolygon; clist; clist = clist->getNext() ) {
		if ( clist->getFaceId() == faceno )
			facepolylist.InsertPolygon( new Polygon( NULL, clist->getId(), clist->getFaceId() ) );
	}

	// check children (subtrees)
	if ( frontsubtree ) frontsubtree->FetchFacePolygons( faceno, facepolylist );
	if ( backsubtree ) backsubtree->FetchFacePolygons( faceno, facepolylist );
}


// check edges for contained vertices and insert them as trace vertices -------
//
void BSPNode::CheckEdges()
{
	// check list of frontfacing polygons
	if ( polygon ) polygon->CheckEdges();

	// check list of backfacing polygons
	if ( backpolygon ) backpolygon->CheckEdges();

	// check children (subtrees)
	if ( frontsubtree ) frontsubtree->CheckEdges();
	if ( backsubtree ) backsubtree->CheckEdges();
}


// grow this node's bounding box by another node's ----------------------------
//
void BSPNode::GrowBoundingBox( BSPNode *othernode )
{
	if ( othernode != NULL ) {
		// ensure other node has a bounding box (calculate if not)
		if ( othernode->boundingbox == NULL )
			othernode->CalcBoundingBoxes();
		static BoundingBox *otherbox;
		otherbox = othernode->boundingbox;

		if ( boundingbox == NULL ) {
			// if this node has no bounding box copy other node's
			boundingbox = new BoundingBox( otherbox->getMinVertex(), otherbox->getMaxVertex() );
		} else {
			// merge the two bounding boxes
			boundingbox->GrowBoundingBox( otherbox );
		}
	}
}


// calculate axial bounding box for this node and all children ----------------
//
void BSPNode::CalcBoundingBoxes()
{
	if ( boundingbox == NULL ) {

		// if polygon(s) contained in this node calculate their bounding box
		if ( polygon != NULL ) {
			polygon->CalcBoundingBox( boundingbox );
			if ( backpolygon != NULL ) {
				static BoundingBox *backbox;
				backpolygon->CalcBoundingBox( backbox );
				boundingbox->GrowBoundingBox( backbox );
				delete backbox;
			}
		}

		// grow local bounding box by children's
		GrowBoundingBox( frontsubtree );
		GrowBoundingBox( backsubtree );
	}
}


// calculate separator planes from polygons for entire bsp tree ---------------
//
void BSPNode::CalcSeparatorPlanes()
{
	if ( ( separatorplane == NULL ) && ( polygon != NULL ) ) {
		separatorplane = new Plane( polygon->getFirstVertex(),
									polygon->getSecondVertex(),
									polygon->getThirdVertex() );
	}

	// check children (subtrees)
	if ( frontsubtree ) frontsubtree->CalcSeparatorPlanes();
	if ( backsubtree ) backsubtree->CalcSeparatorPlanes();
}


// format to use when writing bsp nodes to files ------------------------------
//
int BSPNode::outputformat = BSPNode::OUTPUT_KEY_VALUE_STYLE;


// construct BSPNodeFlat ------------------------------------------------------
//
BSPNodeFlat::BSPNodeFlat( int front, int back, Polygon *poly, int clist, int blist, Plane *sep, BoundingBox *box, int num )
{
	frontsubtreeindx	= front;
	backsubtreeindx		= back;
	polygon				= poly;
	containedlistindx	= clist;
	backlistindx		= blist;
	separatorplane		= sep;
	boundingbox			= box;
	nodenumber			= num;
}


// init BSPNodeFlat members ---------------------------------------------------
//
void BSPNodeFlat::InitNode( int front, int back, Polygon *poly, int clist, int blist, Plane *sep, BoundingBox *box, int num )
{
	frontsubtreeindx	= front;
	backsubtreeindx		= back;
	polygon				= poly;
	containedlistindx	= clist;
	backlistindx		= blist;
	separatorplane		= sep;
	boundingbox			= box;
	nodenumber			= num;
}


// apply scale factor to separator plane and bounding box if attached ---------
//
void BSPNodeFlat::ApplyScaleFactor( double sfac )
{
	if ( separatorplane != NULL )
		separatorplane->ApplyScaleFactor( sfac );
	if ( boundingbox != NULL )
		boundingbox->ApplyScaleFactor( sfac );
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
