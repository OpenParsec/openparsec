//-----------------------------------------------------------------------------
//	BSPLIB MODULE: ObjectBSPNode.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "ObjectBSPNode.h"


BSPLIB_NAMESPACE_BEGIN


// create linear object list from objects attached to nodes -------------------
//
BspObject *ObjectBSPNode::CreateObjectList()
{
	// process tree from its leaves upwards
	BspObject *front = frontsubtree ? frontsubtree->CreateObjectList() : NULL;
	BspObject *back  = backsubtree ? backsubtree->CreateObjectList() : NULL;
	BspObject *scan = NULL;
	if ( front != NULL ) {
		// append back-list to front-list if front-list exists

		for ( scan = front; scan->next; scan = scan->next ) ;
		scan->next = back;
	}

	if ( boundingbox != NULL ) {
		static BspObject *headobject;
		if ( ( headobject = boundingbox->containedobject ) != NULL ) {
			// scan list and merge all subsequent objects into head object
			static BspObject *scan;
			for ( scan = headobject->next; scan; scan = scan->next ) {
				headobject->MergeObjects( scan );
				// invalidate other object's polygonlist
				scan->getPolygonList().InvalidateList();
			}
			// delete all objects that have been merged into head
			delete headobject->next;
			// single object, no list
			headobject->next = NULL;
			// return head object now encompassing entire list
			return headobject;
		} else {
			// return contatenated list for internal nodes with bounding box
			return front ? front : back;
		}
	} else {
		// simply return contatenated list for nodes without bounding box
		return front ? front : back;
	}
}


// merge all objects contained in tree into passed object ---------------------
//
void ObjectBSPNode::MergeTreeNodeObjects( BspObject *newobject )
{
	if ( separatorplane == NULL ) {
		// merge leaf into already existing cumulative object
		newobject->MergeObjects( boundingbox->containedobject );
		// unlink list from already merged object
		boundingbox->containedobject->next = NULL;

		//NOTE:
		// the merged objects must not be freed at this point,
		// since they are still needed for the subsequent merging
		// of bsp trees! nevertheless, their next field must be
		// set to null, to prevent crosslinking of leaves.

	} else {
		// process subtrees; order not relevant in any way
		if ( frontsubtree ) frontsubtree->MergeTreeNodeObjects( newobject );
		if ( backsubtree ) backsubtree->MergeTreeNodeObjects( newobject );
	}
}


// create unified (separator plane/polygon bsp) from object bsp tree ----------
//
BSPNode *ObjectBSPNode::CreateUnifiedBSPTree()
{
	//NOTE:
	// the BSPTrees of the BspObjects are invalidated after they
	// have been integrated into the unified BSPTree. this is done
	// to prevent accidental deletion at the time when the underlying
	// objects will be deleted. (which will normally be done in
	// DeleteNodeBspObjects() later on.)

	if ( separatorplane == NULL ) {
		// node is leaf: return object's bsp tree
		static BSPNode *root;
		root = boundingbox->containedobject->getBSPTree().getRoot();
		boundingbox->containedobject->getBSPTree().InvalidateTree();
		return root;
	} else {
		// node is separator (internal node): create separator bsp node
		BSPNode *front = frontsubtree ? frontsubtree->CreateUnifiedBSPTree() : NULL;
		BSPNode *back  = backsubtree ? backsubtree->CreateUnifiedBSPTree() : NULL;
		return new BSPNode( front, back, NULL, NULL, separatorplane, NULL );
	}
}


// merge all objects contained in object bsp tree into single object and tree -
//
BspObject *ObjectBSPNode::CreateMergedBSPTree()
{
	//NOTE:
	// this function may only be invoked on ObjectBSPNodes which
	// actually contain objects with valid BSPTrees! so, it may only
	// be invoked after successful bsp compilation.

	// create new object to encompass all other objects
	BspObject *newobject = new BspObject;

	// merge objects of nodes into one big object
	MergeTreeNodeObjects( newobject );

	// create unified bsp tree for all contained objects and their respective separators
	newobject->getBSPTree().InitTree( CreateUnifiedBSPTree() );
	newobject->numbsppolygons = 0;
	newobject->getBSPTree()->NumberBSPNodes( newobject->numbsppolygons ); //CAVEAT: sind nicht alles Polygone!!! //TODO

	newobject->UpdateAttributeNumbers();
	return newobject;
}


// delete all BSPObjects attached to bounding boxes contained in tree ---------
//
void ObjectBSPNode::DeleteNodeBspObjects()
{
	if ( separatorplane == NULL ) {
		// delete leaf
		delete boundingbox->containedobject;
		boundingbox->containedobject = NULL;
	} else {
		// detach separator plane from node to prevent accidental
		// deletion later on. (planes are still used by unified
		// bsp tree.)
		separatorplane = NULL;
		// process subtrees; order not relevant in any way
		if ( frontsubtree ) frontsubtree->DeleteNodeBspObjects();
		if ( backsubtree ) backsubtree->DeleteNodeBspObjects();
	}
}


// traverse bsp tree (preorder) and number nodes as encountered ---------------
//
void ObjectBSPNode::NumberBSPNodes( int& curno )
{
	// never used
}


// write bsp tree structure to output file (preorder traversal) ---------------
//
void ObjectBSPNode::WriteBSPTree( FILE *fp ) const
{
	// never used
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
