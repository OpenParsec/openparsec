//-----------------------------------------------------------------------------
//	BSPLIB MODULE: PolygonList.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "PolygonList.h"
#include "BspObject.h"
#include "BspTool.h"
#include "BSPNode.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// init with already existing list of polygons --------------------------------
//
Polygon *PolygonListRep::InitList( Polygon *listhead )
{
	delete list;
	list = listhead;
	numpolygons	= list ? list->getNumPolygons() : 0;

	return list;
}


// merge another polygon list into this list ----------------------------------
//
void PolygonListRep::MergeLists( PolygonListRep *mergelist )
{
	if ( list != NULL ) {
		// scan to end of local list
		Polygon *poly = NULL;
		for (poly = list; poly->getNext(); poly = poly->getNext() )
			poly->setNumPolygons( poly->getNumPolygons() + mergelist->numpolygons );
		// append mergelist
		poly->setNext( mergelist->list );
		poly->setNumPolygons( poly->getNumPolygons() + mergelist->numpolygons );
		numpolygons += mergelist->numpolygons;
	} else {
		// local list is empty: simply take over mergelist
		list		= mergelist->list;
		numpolygons	= mergelist->numpolygons;
	}
}


// create new polygon and add at head of list ---------------------------------
//
Polygon *PolygonListRep::NewPolygon()
{
	list = list ? list->NewPolygon() : new Polygon( baseobject );
	numpolygons = list->getNumPolygons();

	return list;
}


// insert existing polygon at head of list ------------------------------------
//
Polygon *PolygonListRep::InsertPolygon( Polygon *poly )
{
	if ( poly != NULL ) {

		if ( list != NULL ) {
			list = list->InsertPolygon( poly );
		} else {
			poly->setNext( NULL );
			poly->setNumPolygons( 1 );
			poly->baseobject = baseobject;
			list = poly;
		}
		numpolygons = list->getNumPolygons();
	}

	return list;
}


// unlink head from list and return pointer to it -----------------------------
//
Polygon *PolygonListRep::UnlinkHead()
{
	Polygon *tmp = list;

	if ( tmp != NULL ) {
		list = tmp->getNext();
		tmp->setNumPolygons( 1 );
		tmp->setNext( NULL );
		numpolygons = list ? list->getNumPolygons() : 0;
	}

	return tmp;
}


// delete head of list and return pointer to rest of list ---------------------
//
Polygon *PolygonListRep::DeleteHead()
{
	if ( list != NULL )	{
		list = list->DeleteHead();
		numpolygons = list ? list->getNumPolygons() : 0;
	}

	return list;
}


// calculate plane normals for planes of all polygons -------------------------
//
Polygon *PolygonListRep::CalcPlaneNormals()
{
	if ( list != NULL ) {
		list->CalcPlaneNormals();
	}

	return list;
}


// check polygon planes for all polygons contained in list --------------------
//
Polygon *PolygonListRep::CheckPolygonPlanes()
{
	if ( list != NULL ) {
		// list possibly grows due to checking planes!!
		list = list->CheckPlanesAndMappings();
		numpolygons = list->getNumPolygons();
	}

	return list;
}


// build bsp tree; polygon list is resolved in the process --------------------
//
BSPNode *PolygonListRep::PartitionSpace()
{
	BSPNode *bspnode = NULL;

	if ( list != NULL ) {

		if ( list->getSplitterSelection() == Polygon::SPLITTERCRIT_RANDOM_SAMPLE ) {
			// calculate random sample probability
			list->CalcSplitterTestProbability();
		}

		bspnode		= list->PartitionSpace();
		list		= NULL;
		numpolygons = 0;
		//NOTE:
		// the polygon list is resolved in the process of
		// bsp compilation! polygons are only accessible as
		// elements of the newly created bsp tree afterwards.
	}

	return bspnode;
}


// write list of polygon numbers to text-file (reverse ordering) --------------
//
void PolygonListRep::WritePolyList( FILE *fp, int no ) const
{
	if ( list != NULL ) {
		if ( list->nextpolygon )
			list->nextpolygon->WritePolyList( fp );
		fprintf( fp, "%d\t\t; %d\n", list->getId() + 1, no );
	}
}


// class PolygonList error message handler ------------------------------------
//
void PolygonListRep::Error( int err ) const
{
	{
	StrScratch line;

	switch ( err ) {

	case E_NEWVINDX:
		strcpy( line, "***ERROR*** in PolygonList::(Append|Prepend)NewVIndx()\n" );
		break;

	case E_APPENDVINDX:
		strcpy( line, "***ERROR*** in PolygonList::AppendVIndx()\n" );
		break;

	case E_GETNUMVERTXS:
		strcpy( line, "***ERROR*** in PolygonList::getNumElements()\n" );
		break;

	default:
		strcpy( line, "***ERROR*** in object of class BspLib::PolygonList\n" );
		break;
	}

	ErrorMessage( line );
	}

	HandleCriticalError();
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
