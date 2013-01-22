//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BspObjectList.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspObjectList.h"


BSPLIB_NAMESPACE_BEGIN


// create new (default constructed) BspObject and prepend it to list ----------
//
BspObject *BspObjectListRep::CreateNewObject()
{
	BspObject *temp	= new BspObject;
	temp->next		= list;
	list			= temp;
	return list;
}


// insert existing BspObject at head of list ----------------------------------
//
BspObject *BspObjectListRep::InsertObject( BspObject *obj )
{
	if ( obj != NULL ) {
		obj->next = list;
		list      = obj;
	}
	return obj;
}


// build a list of bounding boxes containing all objects of list --------------
//
BoundingBox *BspObjectListRep::BuildBoundingBoxList()
{
	return ( list ? list->BuildBoundingBoxList() : NULL );
}


// count number of objects in list --------------------------------------------
//
int BspObjectListRep::CountListObjects()
{
	int count = 0;
	for ( BspObject *scan = list; scan; scan = scan->getNext() )
		count++;
	return count;
}


// prepare object bsp tree as secondary data structure ------------------------
//
int BspObjectListRep::PrepareObjectBSPTree( ObjectBSPTree& objbsptree )
{
	if ( list != NULL ) {
		// create list of bounding boxes with contained objects
		BoundingBox *bboxlist = BuildBoundingBoxList();
		// partition space containing bounding boxes (list becomes invalid!)
		objbsptree.InitTree( bboxlist->PartitionSpace() );
		// create new object list, object bsp tree becomes secondary data structure
		list = objbsptree->CreateObjectList();
	}
	return ( list != NULL );
}


// merge object bsp tree into a single object and attached bsp tree -----------
//
int BspObjectListRep::MergeObjectBSPTree( ObjectBSPTree& objbsptree )
{
	//NOTE:
	// it is imperative that the passed in ObjectBSPTree really
	// contains all BspObjects of this list! if this is not the
	// case the list will simply be overwritten with a single-element
	// list containing the aggregate object of all nodes of the
	// passed in ObjectBSPTree. the original list nodes will be lost!

	// create list consisting only of the merged object
	list = objbsptree->CreateMergedBSPTree();

	// delete all BspObjects contained in tree since they are now
	// unnecessary. (their data is contained in the merged object.)
	objbsptree->DeleteNodeBspObjects();
	objbsptree.KillTree();

	return ( list != NULL );
}


// collapse entire list of BspObjects into head of list -----------------------
//
int BspObjectListRep::CollapseObjectList()
{
	if ( list != NULL )
		list->CollapseObjectList();
	return ( list != NULL );
}


// process all objects contained in list --------------------------------------
//
int BspObjectListRep::ProcessObjects( int flags )
{
	// scan entire list
	for ( BspObject *bspobject = list; bspobject; bspobject = bspobject->getNext() ) {

		// apply transformations directly to vertices
		if ( flags & BspObjectList::APPLY_TRANSFORMATIONS ) {
			bspobject->ApplyTransformation();
		}

		// calculate plane normals for planes of all polygons
		if ( flags & BspObjectList::CALC_PLANE_NORMALS )
			bspobject->CalcPlaneNormals();

		// build linked bsp tree from flat representation
		if ( flags & BspObjectList::BUILD_FROM_FLAT ) {
			if ( bspobject->BuildBSPTreeFromFlat() )
				bspobject->bsptree->NumberBSPNodes( bspobject->numbsppolygons );
			continue;	// no other processing allowed
		}

		//TODO:
		// 1. vertices zusammenlegen: MergeVertices()
		// 2. ueberfluessige edges entfernen: CullNullEdges()
		// 3. t vertices entfernen: EliminateTVertices()
		// 4. faces zusammenlegen: MergeFaces()

		// MergeFaces(): nur fuer 3d-studio tri-mesh!!
		//   fuer jedes triangle werden alle anderen gescannt, ob sie eine
		//   edge teilen. falls ja, wird fuer den dritten punkt geprueft, ob er
		//   in der selben ebene liegt. ja --> triangle to quadrilateral merge.
		//   der umlaufsinn der vertex numerierung wird dahingehend geprueft, ob
		//   er konsistent mit dem ergebnis-quad ist.
		//   nein: warnmeldung und kein mergen!!

		if ( flags & BspObjectList::MERGE_VERTICES ) {
		}

		if ( flags & BspObjectList::CULL_NULL_EDGES ) {
		}

		if ( flags & BspObjectList::ELIMINATE_T_VERTICES ) {
		}

		if ( flags & BspObjectList::MERGE_FACES ) {
		}

		// split (possibly hand edited) faces with vertices not in same plane
		if ( flags & BspObjectList::CHECK_PLANES )
			bspobject->CheckPolygonPlanes();

		// compile bsp tree and number nodes
		if ( flags & BspObjectList::BUILD_BSP )
			if ( bspobject->BuildBSPTree() )
				bspobject->bsptree->NumberBSPNodes( bspobject->numbsppolygons );

		// check for vertices with exact same coordinates
		if ( flags & BspObjectList::CHECK_VERTICES )
			bspobject->CheckVertices( TRUE );

		// calc bounding boxes for all nodes
		// operates on bsp tree only, so one has to be present!
		if ( flags & BspObjectList::CALC_BOUNDING_BOXES )
			bspobject->CalcBoundingBoxes();

		// calc explicit separator planes for all nodes
		// operates on bsp tree only, so one has to be present!
		if ( flags & BspObjectList::CALC_SEPARATOR_PLANES )
			bspobject->CalcSeparatorPlanes();

		// check edges for t-vertices and eliminate them (insert trace vertices)
		// operates on bsp tree only, so one has to be present!
		if ( flags & BspObjectList::CHECK_EDGES )
			bspobject->CheckEdges();

		// display object and compilation statistics
		if ( flags & BspObjectList::DISPLAY_STATS )
			bspobject->DisplayStatistics();

		// update attribute numbers, even if no statistics desired
		bspobject->UpdateAttributeNumbers();
	}

	return TRUE;
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
