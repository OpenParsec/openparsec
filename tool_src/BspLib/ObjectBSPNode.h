//-----------------------------------------------------------------------------
//	BSPLIB HEADER: ObjectBSPNode.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _OBJECTBSPNODE_H_
#define _OBJECTBSPNODE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BoundingBox.h"
#include "BspObject.h"
#include "Plane.h"


BSPLIB_NAMESPACE_BEGIN


// node of object bsp tree (a bsp tree of whole objects instead of polygons) --
//
class ObjectBSPNode {

public:
	ObjectBSPNode( ObjectBSPNode *front = NULL, ObjectBSPNode *back = NULL, Plane *sep = NULL, BoundingBox *bbox = NULL, int id = -1 );
	~ObjectBSPNode() { delete separatorplane; delete boundingbox; delete frontsubtree; delete backsubtree; }

	void			NumberBSPNodes( int& curno );
	void			WriteBSPTree( FILE *fp ) const;

	BspObject*		CreateObjectList();
	BspObject*		CreateMergedBSPTree();

	void			DeleteNodeBspObjects();

	int				getNodeNumber() const { return nodenumber; }
	Plane*			getSeparatorPlane() const { return separatorplane; }
	BoundingBox*	getBoundingBox() const { return boundingbox; }
	ObjectBSPNode*	getFrontSubtree() const { return frontsubtree; }
	ObjectBSPNode*	getBackSubtree() const { return backsubtree; }

private:
	void			MergeTreeNodeObjects( BspObject *newobject );
	BSPNode*		CreateUnifiedBSPTree();

private:
	int				nodenumber;		// this node's id
	Plane*			separatorplane;	// plane inducing two halfspaces; NULL for leaves
	BoundingBox*	boundingbox;	// attached bounding box; NULL for internal nodes
	ObjectBSPNode*	frontsubtree;	// tree in front halfspace
	ObjectBSPNode*	backsubtree;	// tree in back halfspace
};

// construct node -------------------------------------------------------------
inline ObjectBSPNode::ObjectBSPNode( ObjectBSPNode *front, ObjectBSPNode *back, Plane *sep, BoundingBox *bbox, int id )
{
	frontsubtree	= front;
	backsubtree		= back;
	separatorplane	= sep;
	boundingbox		= bbox;
	nodenumber		= id;
}


BSPLIB_NAMESPACE_END


#endif // _OBJECTBSPNODE_H_

