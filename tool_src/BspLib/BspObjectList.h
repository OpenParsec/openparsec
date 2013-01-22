//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BspObjectList.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPOBJECTLIST_H_
#define _BSPOBJECTLIST_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObject.h"
#include "ObjectBSPTree.h"


BSPLIB_NAMESPACE_BEGIN


// class for list of BspObject items (representation class) -------------------
//
class BspObjectListRep {

	friend class BspObjectList;

public:
	BspObjectListRep() : ref_count( 0 ) { list = NULL; }
	~BspObjectListRep() { delete list; }

	BspObject*		CreateNewObject();				// prepend empty object to list
	BspObject*		InsertObject( BspObject *obj );	// insert existing object at head

	BoundingBox*	BuildBoundingBoxList();			// build a list of bounding boxes

	int				CountListObjects();				// count list objects

	int				PrepareObjectBSPTree( ObjectBSPTree& objbsptree );
	int				MergeObjectBSPTree( ObjectBSPTree& objbsptree );

	int				CollapseObjectList();			// collapse list into its head object

	int				ProcessObjects( int flags );	// various processing functions

	int				BspTreeAvailable() { return list ? list->BspTreeAvailable() : FALSE; }
	int				BSPTreeAvailable() { return list ? list->BSPTreeAvailable() : FALSE; }
	int				BSPTreeFlatAvailable() { return list ? list->BSPTreeFlatAvailable() : FALSE; }

	BspObject*		getListHead() const { return list; }

private:
	int				ref_count;	// number of references to this list
	BspObject*		list;		// first element in list
};


// class for list of BspObject items (handle class) ---------------------------
//
class BspObjectList {

public:

	// object processing flags
	enum {
		CHECK_PLANES			= 0x0001,
		BUILD_BSP				= 0x0002,
		CHECK_VERTICES			= 0x0004,
		CHECK_EDGES				= 0x0008,
		BUILD_BSP_WITH_CHECKS	= 0x000F,
		DISPLAY_STATS			= 0x0010,
		BUILD_FROM_FLAT			= 0x0020,
		MERGE_VERTICES			= 0x0040,
		CULL_NULL_EDGES			= 0x0080,
		ELIMINATE_T_VERTICES	= 0x0100,
		MERGE_FACES				= 0x0200,
		CALC_PLANE_NORMALS		= 0x0400,
		CALC_BOUNDING_BOXES		= 0x0800,
		CALC_SEPARATOR_PLANES	= 0x1000,
		APPLY_TRANSFORMATIONS	= 0x2000,
	};

public:
	BspObjectList() { rep = new BspObjectListRep(); rep->ref_count = 1; }
	~BspObjectList() { if ( --rep->ref_count == 0 ) delete rep; }

	BspObjectList( const BspObjectList& copyobj );
	BspObjectList& operator =( const BspObjectList& copyobj );

	BspObject*		CreateNewObject() { return rep->CreateNewObject(); }
	BspObject*		InsertObject( BspObject *obj ) { return rep->InsertObject( obj ); }

	// create a list of bounding boxes containing a bounding box for each
	// node in the list. user has to keep track of the list's head!
	BoundingBox*	BuildBoundingBoxList() { return rep->BuildBoundingBoxList(); }

	int				CountListObjects() { return rep->CountListObjects(); }

	int				PrepareObjectBSPTree( ObjectBSPTree& objbsptree ) { return rep->PrepareObjectBSPTree( objbsptree ); }
	int				MergeObjectBSPTree( ObjectBSPTree& objbsptree ) { return rep->MergeObjectBSPTree( objbsptree ); }

	int				CollapseObjectList() { return rep->CollapseObjectList(); }

	// process entire list according to bitfield specifying desired processing
	int				ProcessObjects( int flags ) { return rep->ProcessObjects( flags ); }

	// BSP tree of any representation available (linked or flat)?
	int				BspTreeAvailable() { return rep->BspTreeAvailable(); }
	// linked BSP tree available?
	int				BSPTreeAvailable() { return rep->BSPTreeAvailable(); }
	// flat BSP tree available?
	int				BSPTreeFlatAvailable() { return rep->BSPTreeFlatAvailable(); }

	//NOTE:
	// the previous functions actually return the state of the first BspObject
	// in the list. if bsp trees have not been built through ProcessObjects()
	// it is not guaranteed that the returned state holds for every object in
	// the list! normally, list nodes should not be accessed separately, though.

	// return pointer to first BspObject in list
	BspObject*		getListHead() { return rep->getListHead(); }

private:
	BspObjectListRep *rep;
};

// copy constructor for BspObjectList -----------------------------------------
inline BspObjectList::BspObjectList( const BspObjectList& copyobj )
{
	rep = copyobj.rep;	// shallow copy
	rep->ref_count++;	// with reference counting
}

// assignment operator for BspObjectList --------------------------------------
inline BspObjectList& BspObjectList::operator =( const BspObjectList& copyobj )
{
	if ( &copyobj != this ) {
		// old reference is overwritten
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;	// shallow copy
		rep->ref_count++;	// with reference counting
	}
	return *this;
}


BSPLIB_NAMESPACE_END


#endif // _BSPOBJECTLIST_H_

