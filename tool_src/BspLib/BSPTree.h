//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BSPTree.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPTREE_H_
#define _BSPTREE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BSPNode.h"


BSPLIB_NAMESPACE_BEGIN


// bsp tree; basically pointer to root node (representation class) ------------
//
class BSPTreeRep {

	friend class BSPTree;

private:
	BSPTreeRep() : ref_count( 0 ) { root = NULL; }
	~BSPTreeRep() { delete root; }

	BSPNode*	InitTree( BSPNode *rootnode );
	BSPNode*	getRoot() { return root; }

	void		InvalidateTree();

	int			TreeEmpty() const { return ( root == NULL ); }

private:
	int			ref_count;
	BSPNode*	root;
};

// init with entire tree of BSPNode objects -----------------------------------
inline BSPNode *BSPTreeRep::InitTree( BSPNode *rootnode )
{
	delete root;
	return ( root = rootnode );
}

// set root to null without deleting tree first -------------------------------
inline void BSPTreeRep::InvalidateTree()
{
	root = NULL;
}


// bsp tree; basically pointer to root node (handle class) --------------------
//
class BSPTree {

public:
	BSPTree() {	rep = new BSPTreeRep; rep->ref_count = 1; }
	~BSPTree() { if ( --rep->ref_count == 0 ) delete rep; }

	BSPTree( const BSPTree& copyobj );
	BSPTree& operator =( const BSPTree& copyobj );

	// pass most operations through to BSPNode
	BSPNode*	operator->() { return rep->getRoot(); }

	BSPNode*	InitTree( BSPNode *rootnode ) { return rep->InitTree( rootnode ); }
	BSPNode*	getRoot() { return rep->getRoot(); }

	void		InvalidateTree() { rep->InvalidateTree(); }

	int			TreeEmpty() const { return rep->TreeEmpty(); }

private:
	BSPTreeRep*	rep;
};

// copy constructor -----------------------------------------------------------
inline BSPTree::BSPTree( const BSPTree& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

// assignment operator --------------------------------------------------------
inline BSPTree& BSPTree::operator =( const BSPTree& copyobj )
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


// flat bsp tree; basically pointer to array (representation class) -----------
//
class BSPTreeFlatRep {

	friend class BSPTreeFlat;

private:
	BSPTreeFlatRep() : ref_count( 0 ) { root = NULL; numnodes = 0; nodestorage = 0; }
	~BSPTreeFlatRep() { delete[] root; }

	BSPNodeFlat*	AppendNode( int front, int back, Polygon *poly, int clist, int blist );
	BSPNodeFlat*	FetchNodePerId( int id );
	BSPNode*		BuildBSPTree( int nodenum );
	BSPNodeFlat*	getRoot() { return root; }

	int				TreeEmpty() const { return ( root == NULL ); }
	int				getNumNodes() const { return numnodes; }

	void			ApplyScaleFactor( double sfac );
	void			DestroyTree() { delete[] root; root = NULL; numnodes = 0; nodestorage = 0; }

private:
	int				ref_count;
	int				numnodes;
	int				nodestorage;
	BSPNodeFlat*	root;
};


// flat bsp tree; basically pointer to array (handle class) -------------------
//
class BSPTreeFlat {

public:
	BSPTreeFlat() {	rep = new BSPTreeFlatRep; rep->ref_count = 1; }
	~BSPTreeFlat() { if ( --rep->ref_count == 0 ) delete rep; }

	BSPTreeFlat( const BSPTreeFlat& copyobj );
	BSPTreeFlat& operator =( const BSPTreeFlat& copyobj );

	BSPNodeFlat*	AppendNode( int front, int back, Polygon *poly, int clist, int blist )
		{ return rep->AppendNode( front, back, poly, clist, blist ); }
	BSPNodeFlat*	FetchNodePerId( int id ) { return rep->FetchNodePerId( id ); }
	BSPNode*		BuildBSPTree( int nodenum ) { return rep->BuildBSPTree( nodenum ); }
	BSPNodeFlat*	getRoot() { return rep->getRoot(); }

	int				TreeEmpty() const { return rep->TreeEmpty(); }
	int				getNumNodes() const { return rep->getNumNodes(); }

	void			ApplyScaleFactor( double sfac ) { rep->ApplyScaleFactor( sfac ); }
	void			DestroyTree() { rep->DestroyTree(); }

private:
	BSPTreeFlatRep*	rep;
};

// copy constructor -----------------------------------------------------------
inline BSPTreeFlat::BSPTreeFlat( const BSPTreeFlat& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

// assignment operator --------------------------------------------------------
inline BSPTreeFlat& BSPTreeFlat::operator =( const BSPTreeFlat& copyobj )
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


#endif // _BSPTREE_H_

