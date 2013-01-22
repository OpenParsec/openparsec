//-----------------------------------------------------------------------------
//	BSPLIB HEADER: PolygonList.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _POLYGONLIST_H_
#define _POLYGONLIST_H_

// bsplib header files
#include "BspLibDefs.h"
#include "Polygon.h"
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


class BSPNode;
class BspObject;


// singly linked polygon list (representation class) --------------------------
//
class PolygonListRep : public virtual SystemIO {

	friend class PolygonList;

	// error codes
	enum {
		E_NEWVINDX,
		E_APPENDVINDX,
		E_GETNUMVERTXS
	};

	// error handler for class PolygonList
	void		Error( int err ) const;

private:
	PolygonListRep( BspObject *bobj );
	~PolygonListRep() { delete list; }

	void		MergeLists( PolygonListRep *mergelist );

	Polygon*	InitList( Polygon *listhead );

	// invalidate list without freeing dynamic storage
	void		InvalidateList() { list = NULL; numpolygons = 0; }

	Polygon*	FetchHead() { return list; }
	Polygon*	UnlinkHead();
	Polygon*	DeleteHead();
	Polygon*	FindPolygon( int id ) { return list ? list->FindPolygon( id ) : NULL; }

	Polygon*	NewPolygon();
	Polygon*	InsertPolygon( Polygon *poly );

	void		PrependNewVIndx( int indx = -1 ) { if ( list == NULL ) Error( E_NEWVINDX ); list->PrependNewVIndx( indx ); }
	void		AppendNewVIndx( int indx = -1 ) { if ( list == NULL ) Error( E_NEWVINDX ); list->AppendNewVIndx( indx ); }
	void		AppendVIndx( VIndx *vindx ) { if ( list == NULL ) Error( E_APPENDVINDX ); list->AppendVIndx( vindx ); }
	Polygon*	CalcPlaneNormals();
	Polygon*	CheckPolygonPlanes();
	BSPNode*	PartitionSpace();

	void		WritePolyList( FILE *fp, int no ) const;

	int			getNumElements() const { return numpolygons; }
	int			getNumVertices() const { if( list == NULL ) Error( E_GETNUMVERTXS ); return list->getNumVertices(); }

private:
	int			ref_count;			// number of references to this list
	int			numpolygons;		// length of singly linked polygon list (not counting this head)
	BspObject*	baseobject;			// this list contains polygons belonging to *baseobject
	Polygon*	list;				// pointer to first polygon in list
};

// constructor without preexisting list ---------------------------------------
inline PolygonListRep::PolygonListRep( BspObject *bobj ) : ref_count( 0 )
{
	baseobject	= bobj;
	list		= NULL;
	numpolygons	= 0;
}


// singly linked polygon list (handle class) ----------------------------------
//
class PolygonList {

public:
	PolygonList( BspObject *bobj ) { rep = new PolygonListRep( bobj ); rep->ref_count = 1; }
	~PolygonList() { if ( --rep->ref_count == 0 ) delete rep; }

	PolygonList( const PolygonList& copyobj );
	PolygonList& operator =( const PolygonList& copyobj );

	void		MergeLists( PolygonList *mergelist ) { if ( mergelist ) rep->MergeLists( mergelist->rep ); }

	Polygon*	InitList( Polygon *listhead ) { return rep->InitList( listhead ); }
	void		InvalidateList() { rep->InvalidateList(); }

	Polygon*	FetchHead() { return rep->FetchHead(); }
	Polygon*	UnlinkHead() { return rep->UnlinkHead(); }
	Polygon*	DeleteHead() { return rep->DeleteHead(); }
	Polygon*	FindPolygon( int id ) { return rep->FindPolygon( id ); }

	Polygon*	NewPolygon() { return rep->NewPolygon(); }
	Polygon*	InsertPolygon( Polygon *poly ) { return rep->InsertPolygon( poly ); }

	void		PrependNewVIndx( int indx = -1 ) { rep->PrependNewVIndx( indx ); }
	void		AppendNewVIndx( int indx = -1 ) { rep->AppendNewVIndx( indx ); }
	void		AppendVIndx( VIndx *vindx ) { rep->AppendVIndx( vindx ); }
	Polygon*	CalcPlaneNormals() { return rep->CalcPlaneNormals(); }
	Polygon*	CheckPolygonPlanes() { return rep->CheckPolygonPlanes(); }
	BSPNode*	PartitionSpace() { return rep->PartitionSpace(); }

	void		WritePolyList( FILE *fp, int no ) const { rep->WritePolyList( fp, no ); }

	int			getNumElements() const { return rep->getNumElements(); }
	int			getNumVertices() const { return rep->getNumVertices(); }

private:
	PolygonListRep*	rep;
};

// copy constructor for PolygonList -------------------------------------------
inline PolygonList::PolygonList( const PolygonList& copyobj )
{
	rep = copyobj.rep;	// shallow copy
	rep->ref_count++;	// with reference counting
}

// assignment operator for PolygonList ----------------------------------------
inline PolygonList& PolygonList::operator =( const PolygonList& copyobj )
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


#endif // _POLYGONLIST_H_

