//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BoundingBox.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BOUNDINGBOX_H_
#define _BOUNDINGBOX_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObject.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// axial bounding box; can be part of singly linked list ----------------------
//
class BoundingBox {

	friend class ObjectBSPNode;

public:
	BoundingBox() { containedobject = NULL; nextbox = NULL; }
	BoundingBox( BspObject *cobj, BoundingBox *next = NULL );
	BoundingBox( const Vertex3& minvert, const Vertex3& maxvert, BoundingBox *next = NULL );
	~BoundingBox() { delete nextbox; }

	void			ApplyScaleFactor( double sfac );
	void			GrowBoundingBox( BoundingBox *otherbox );
	void			BoundingBoxListUnion( BoundingBox& unionbox );
	class ObjectBSPNode*	PartitionSpace();

	Vertex3			getMinVertex() const { return minvertex; }
	Vertex3			getMaxVertex() const { return maxvertex; }
	BspObject*		getContainedObject() const { return containedobject; }
	BoundingBox*	getNext() const { return nextbox; }

private:
	Vertex3			minvertex;
	Vertex3			maxvertex;
	BspObject*		containedobject;
	BoundingBox*	nextbox;
};

// construct bounding box by calculating its extents --------------------------
inline BoundingBox::BoundingBox( BspObject *cobj, BoundingBox *next )
{
	if ( ( containedobject = cobj ) != NULL ) {
		cobj->CalcBoundingBox( minvertex, maxvertex );
	}
	nextbox = next;
}

// construct bounding box directly --------------------------------------------
inline BoundingBox::BoundingBox( const Vertex3& minvert, const Vertex3& maxvert, BoundingBox *next )
{
	minvertex		= minvert;
	maxvertex		= maxvert;
	containedobject	= NULL;
	nextbox			= next;
}


BSPLIB_NAMESPACE_END


#endif // _BOUNDINGBOX_H_

