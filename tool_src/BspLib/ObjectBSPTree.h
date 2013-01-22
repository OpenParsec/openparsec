//-----------------------------------------------------------------------------
//	BSPLIB HEADER: ObjectBSPTree.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _OBJECTBSPTREE_H_
#define _OBJECTBSPTREE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "ObjectBSPNode.h"


BSPLIB_NAMESPACE_BEGIN


// bsp tree of objects; basically pointer to root node (representation class) -
//
class ObjectBSPTreeRep {

	friend class ObjectBSPTree;

private:
	ObjectBSPTreeRep() : ref_count( 0 ) { root = NULL; }
	~ObjectBSPTreeRep() { delete root; }

	void			KillTree() { delete root; root = NULL; }
	ObjectBSPNode*	InitTree( ObjectBSPNode *rootnode );
	ObjectBSPNode*	getRoot() { return root; }

	int				TreeEmpty() const { return ( root == NULL ); }

private:
	int				ref_count;
	ObjectBSPNode*	root;
};

// init with entire tree of ObjectBSPNode objects -----------------------------
inline ObjectBSPNode *ObjectBSPTreeRep::InitTree( ObjectBSPNode *rootnode )
{
	delete root;
	return ( root = rootnode );
}


// bsp tree of objects; basically pointer to root node (handle class) ---------
//
class ObjectBSPTree {

public:
	ObjectBSPTree() { rep = new ObjectBSPTreeRep; rep->ref_count = 1; }
	~ObjectBSPTree() { if ( --rep->ref_count == 0 ) delete rep; }

	ObjectBSPTree( const ObjectBSPTree& copyobj );
	ObjectBSPTree& operator =( const ObjectBSPTree& copyobj );

	// pass most operations through to ObjectBSPNode
	ObjectBSPNode*	operator->() { return rep->getRoot(); }

	void			KillTree() { rep->KillTree(); }
	ObjectBSPNode*	InitTree( ObjectBSPNode *rootnode ) { return rep->InitTree( rootnode ); }
	ObjectBSPNode*	getRoot() { return rep->getRoot(); }

	int				TreeEmpty() const { return rep->TreeEmpty(); }

private:
	ObjectBSPTreeRep* rep;
};

// copy constructor -----------------------------------------------------------
inline ObjectBSPTree::ObjectBSPTree( const ObjectBSPTree& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

// assignment operator --------------------------------------------------------
inline ObjectBSPTree& ObjectBSPTree::operator =( const ObjectBSPTree& copyobj )
{
	if ( &copyobj != this ) {
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;
		rep->ref_count++;
	}
	return *this;
}


BSPLIB_NAMESPACE_END


#endif // _OBJECTBSPTREE_H_

