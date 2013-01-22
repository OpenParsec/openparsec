//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BoundingBox.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "BoundingBox.h"
#include "ObjectBSPNode.h"
#include "Plane.h"


BSPLIB_NAMESPACE_BEGIN


// apply scale factor to bounding box -----------------------------------------
//
void BoundingBox::ApplyScaleFactor( double sfac )
{
	//NOTE:
	// this will be needed if the attached object is scaled.
	// the scale factor is applied to the space containing the
	// bounding box, i.e. its midpoint will move.

	minvertex.setX( minvertex.getX() * sfac );
	minvertex.setY( minvertex.getY() * sfac );
	minvertex.setZ( minvertex.getZ() * sfac );

	maxvertex.setX( maxvertex.getX() * sfac );
	maxvertex.setY( maxvertex.getY() * sfac );
	maxvertex.setZ( maxvertex.getZ() * sfac );
}


// grow this bounding box by another one (maximum/minimum merge) --------------
//
void BoundingBox::GrowBoundingBox( BoundingBox *otherbox )
{
	if ( otherbox != NULL ) {
		if ( otherbox->minvertex.getX() < minvertex.getX() )
			minvertex.setX( otherbox->minvertex.getX() );
		if ( otherbox->minvertex.getY() < minvertex.getY() )
			minvertex.setY( otherbox->minvertex.getY() );
		if ( otherbox->minvertex.getZ() < minvertex.getZ() )
			minvertex.setZ( otherbox->minvertex.getZ() );

		if ( otherbox->maxvertex.getX() > maxvertex.getX() )
			maxvertex.setX( otherbox->maxvertex.getX() );
		if ( otherbox->maxvertex.getY() > maxvertex.getY() )
			maxvertex.setY( otherbox->maxvertex.getY() );
		if ( otherbox->maxvertex.getZ() > maxvertex.getZ() )
			maxvertex.setZ( otherbox->maxvertex.getZ() );
	}
}


// fill in bounding box as union of entire list -------------------------------
//
void BoundingBox::BoundingBoxListUnion( BoundingBox& unionbox )
{
	// init union box
	delete unionbox.nextbox;
	unionbox.nextbox		 = NULL;
	unionbox.containedobject = NULL;
	unionbox.minvertex		 = minvertex;
	unionbox.maxvertex		 = maxvertex;

	// build union of entire list
	for ( BoundingBox *curbox = nextbox; curbox; curbox = curbox->nextbox )
		unionbox.GrowBoundingBox( curbox );
}


// partition list of bounding boxing boxes and build tree structure -----------
//
ObjectBSPNode *BoundingBox::PartitionSpace()
{
	//NOTE:
	// this function alters the link fields of the objects attached
	// to bounding boxes. therefore, the original linked list of
	// objects is invalid after this function! the objects themselves
	// are only accessible via tree nodes afterwards.

	//NOTE:
	// analogously to polygon bsp trees where the polygon list is dissolved
	// in the process of bsp compilation, the list of bounding boxes is
	// completely dissolved by this function! that is, the bounding box
	// for which PartitionSpace() has been invoked should not be accessed
	// directly anymore. all bounding boxes can then be accessed via the
	// generated object-bsp-tree!

	// only one box in this subspace?
	if ( nextbox == NULL ) {
		// unlink tail of object list
		containedobject->next = NULL;
		// create ObjectBSPNode for this box (leaf!)
		return new ObjectBSPNode( NULL, NULL, NULL, this );
	}

	// select a separating plane
	Vector3	separatornormal( 1.0, 0.0, 0.0 );
	double	separatoroffset = maxvertex.getX();
	int		currentboundary	= 0;
	Plane *separator = new Plane( separatornormal, separatoroffset );
	BoundingBox *currentbox = this;
	// scan all bounding boxes to find a suitable separating plane
	for ( currentbox = this; ; ) {

		// check if plane separates bounding boxes cleanly
		int cleanseparation = 1;
		int membersofpos	= 0;
		int membersofneg	= 0;
		for ( BoundingBox *scan = this; scan; scan = scan->getNext() ) {
			if ( !separator->PointInNegativeHalfspace( scan->getMinVertex() ) &&
				 !separator->PointInNegativeHalfspace( scan->getMaxVertex() ) ) {
				membersofpos++;
			} else if ( !separator->PointInPositiveHalfspace( scan->getMinVertex() ) &&
						!separator->PointInPositiveHalfspace( scan->getMaxVertex() ) ) {
				membersofneg++;
			} else {
				cleanseparation = 0;
				break;
			}
		}
		//NOTE:
		// both halfspaces have to contain bounding boxes; separation of vacant
		// space from inhabited space just makes the tree unnecessarily large.
		// this does not influence if there is a clean separating plane or not!
		if ( ( membersofpos > 0 ) && ( membersofneg > 0 ) && cleanseparation )
			break;

		// try next boundary plane
		currentboundary = ( currentboundary + 1 ) % 6;
		if ( currentboundary == 0 )
			currentbox = currentbox->getNext();

		if ( currentbox != NULL ) {
			switch ( currentboundary ) {
			case 0:
				separatornormal = Vector3( 1.0, 0.0, 0.0 );
				separatoroffset = currentbox->maxvertex.getX();
				break;
			case 1:
				separatornormal = Vector3( 0.0, 1.0, 0.0 );
				separatoroffset = currentbox->maxvertex.getY();
				break;
			case 2:
				separatornormal = Vector3( 0.0, 0.0, 1.0 );
				separatoroffset = currentbox->maxvertex.getZ();
				break;
			case 3:
				separatornormal = Vector3( -1.0, 0.0, 0.0 );
				separatoroffset = -currentbox->minvertex.getX();
				break;
			case 4:
				separatornormal = Vector3( 0.0, -1.0, 0.0 );
				separatoroffset = -currentbox->minvertex.getY();
				break;
			case 5:
				separatornormal = Vector3( 0.0, 0.0, -1.0 );
				separatoroffset = -currentbox->minvertex.getZ();
				break;
			}
			separator->setPlaneNormal( separatornormal );
			separator->setPlaneOffset( separatoroffset );	
		} else {
			break;
		}
	}

	BoundingBox	*frontsubspace	= NULL;
	BoundingBox	*backsubspace	= NULL;

	if ( currentbox != NULL ) {
		// partition space into two halfspaces (walk list of bounding boxes)
		for ( currentbox = this; currentbox; ) {
			if ( !separator->PointInNegativeHalfspace( currentbox->getMinVertex() ) &&
				 !separator->PointInNegativeHalfspace( currentbox->getMaxVertex() ) ) {
				// box contained in positive halfspace
				BoundingBox	*tmpbox	= currentbox->getNext();
				currentbox->nextbox	= frontsubspace;
				frontsubspace		= currentbox;
				currentbox			= tmpbox;
			} else {
				// box contained in negative halfspace
				BoundingBox	*tmpbox	= currentbox->getNext();
				currentbox->nextbox	= backsubspace;
				backsubspace		= currentbox;
				currentbox			= tmpbox;
			}
		}
	} else {
		// no suitable separating plane found: create list of unseparable objects
		for ( currentbox = this; currentbox; currentbox = currentbox->getNext() ) {
			currentbox->containedobject->next =
				currentbox->getNext() ? currentbox->getNext()->containedobject : NULL;
		}
		// delete legacy bounding boxes
		delete nextbox;
		nextbox = NULL;
		// create ObjectBSPNode for this box (leaf!)
		return new ObjectBSPNode( NULL, NULL, NULL, this );

		//NOTE:
		// if bounding boxes cannot be separated cleanly, their corresponding objects
		// are inserted into a linear list attached to a single bounding box. this
		// bounding box, however, encompasses only the head of this list!
		// for bsp compilation, the objects in these lists have to be explicitly merged
		// into a single object. this is not done automatically!
		// currently, only ObjectBSPNode::CreateObjectList() merges these objects.
	}

	// allocate new root; partition halfspaces recursively and return root
	ObjectBSPNode *front = frontsubspace ? frontsubspace->PartitionSpace() : NULL;
	ObjectBSPNode *back  = backsubspace  ? backsubspace->PartitionSpace()  : NULL;
	return new ObjectBSPNode( front, back, separator, NULL );
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
