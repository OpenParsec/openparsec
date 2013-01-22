//-----------------------------------------------------------------------------
//	BSPLIB MODULE: Polygon.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "BspObject.h"
#include "Polygon.h"
#include "BoundingBox.h"
#include "BSPNode.h"
#include "BspTool.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// construct a Polygon --------------------------------------------------------
//
Polygon::Polygon( BspObject *bobj, int pno, int fno, Polygon *next, int num )
{
	baseobject		= bobj;
	polygonno		= pno;
	faceno			= fno;
	nextpolygon		= next;
	numpolygons		= num;
	numvertindxs	= 0;
	vertindxs		= NULL;
	vindxinsertpos	= NULL;
}


// get vertexlist of object polygon belongs to --------------------------------
//
VertexChunk& Polygon::getVertexList()
{
	CHECK_DEREFERENCING(
		if ( baseobject == NULL )
			Error();
	);
	return baseobject->getVertexList();
}


// get facelist of object polygon belongs to ----------------------------------
//
FaceChunk& Polygon::getFaceList()
{
	CHECK_DEREFERENCING(
		if ( baseobject == NULL )
			Error();
	);
	return baseobject->getFaceList();
}


// create new polygon and prepend it to list (append list to new polygon) -----
//
Polygon *Polygon::NewPolygon()
{
	//CAVEAT:
	// the automatical assignment of polygon and face number
	// must be used judiciously! it's very easy to get the
	// numbering wrong. also, a single polygon cannot be
	// used as "polygon-factory" since it will always assign
	// the same numbers to the new polygons, based on its own.

	//NOTE:
	// normally, this function should only be used like this:
	// poly = poly->NewPolygon();

	// automatically assign sequential polygon- and face-number
	return new Polygon( baseobject, numpolygons, numpolygons, this, numpolygons + 1 );
}


// prepend existing polygon to list (polygon- and face id are not changed!) ---
//
Polygon *Polygon::InsertPolygon( Polygon *poly )
{
	poly->nextpolygon = this;
	poly->numpolygons = numpolygons + 1;
	poly->baseobject  = baseobject;
	return poly;
}


// delete head of polygon list; return rest of list ---------------------------
//
Polygon *Polygon::DeleteHead()
{
	Polygon *temp = nextpolygon;
	nextpolygon   = NULL;
	delete this;
	//CAVEAT:
	// this can create problems if function not used correctly!
	// INVOKE ONLY LIKE THIS: poly = poly->DeleteHead();
	// otherwise a dangling pointer will be created!
	// THIS FUNCTION MAY ALSO NEVER BE INVOKED ON STATICALLY
	// ALLOCATED POLYGONS!!
	return temp;
}


// find polygon with specific id ----------------------------------------------
//
Polygon *Polygon::FindPolygon( int id )
{
	// scan entire list and compare ids
	for ( Polygon *scanpo = this; scanpo; scanpo = scanpo->getNext() ) {
		if ( scanpo->getId() == id )
			return scanpo;
	}
	// no polygon with this id found
	return NULL;
}


// sum up vertex numbers for all polygons of list -----------------------------
//
int Polygon::SumVertexNumsEntireList()
{
	// scan entire list and compare ids
	int count = 0;
	for ( Polygon *scanpo = this; scanpo; scanpo = scanpo->getNext() )
		count += scanpo->getNumVertices();
	return count;
}


// correct relative values to refer to new base -------------------------------
//
void Polygon::CorrectBase( BspObject *newbaseobj, int vertexindxbase, int faceidbase, int polygonidbase )
{
	baseobject = newbaseobj;
	faceno    += faceidbase;
	polygonno += polygonidbase;
	// correct all vertex indexes by just adding offset value
	for ( VIndx *vertlist = vertindxs; vertlist; vertlist = vertlist->getNext() )
		vertlist->setIndx( vertlist->getIndx() + vertexindxbase );
}


// correct relative values to refer to new base using vertex index map --------
//
void Polygon::CorrectBaseByTable( BspObject *newbaseobj, int *vtxindxmap, int faceidbase, int polygonidbase )
{
	baseobject = newbaseobj;
	faceno    += faceidbase;
	polygonno += polygonidbase;
	// correct all vertex indexes using mapping table
	for ( VIndx *vertlist = vertindxs; vertlist; vertlist = vertlist->getNext() )
		vertlist->setIndx( vtxindxmap[ vertlist->getIndx() ] );
}


// write list of vertices to text-file ----------------------------------------
//
void Polygon::FillVertexIndexArray( dword *arr ) const
{
	// scan list of vertex indexes and write them into the int array
	for ( VIndx *vertlist = vertindxs; vertlist; vertlist = vertlist->getNext() )
		*arr++ = vertlist->getIndx();
}


// write list of vertices to text-file ----------------------------------------
//
void Polygon::WriteVertexList( FILE *fp, int cr ) const
{
	// scan list of vertex indexes and write them to text file
	for ( VIndx *vertlist = vertindxs; vertlist; vertlist = vertlist->getNext() )
		fprintf( fp, vertlist->getNext() ? "%d, " : "%d", vertlist->getIndx() + 1 );
	if ( cr )
		fprintf( fp, "\n" );
}


// write list of polygon numbers to text-file (reverse ordering) --------------
//
void Polygon::WritePolyList( FILE *fp ) const
{
	// recurse to achieve reverse ordering
	if ( nextpolygon )
		nextpolygon->WritePolyList( fp );

	fprintf( fp, "%d, ", getId() + 1 );
}


// add new VIndx for polygon at head of vertindxs -----------------------------
//
void Polygon::PrependNewVIndx( int indx )
{
	// prepend new VIndx
	VIndx *oldhead = vertindxs;
	vertindxs = new VIndx( indx, oldhead );
	// remember last node in list
	if ( vindxinsertpos == NULL )
		vindxinsertpos = vertindxs;
	// update number of vertex indexes
	numvertindxs++;
}


// add new VIndx for polygon at tail of vertindxs -----------------------------
//
void Polygon::AppendNewVIndx( int indx )
{
	// append new VIndx
	if ( vindxinsertpos != NULL ) {
		vindxinsertpos->setNext( new VIndx( indx ) );
		vindxinsertpos = vindxinsertpos->getNext();
	} else {
		vertindxs		= new VIndx( indx );
		vindxinsertpos	= vertindxs;
	}
	// update number of vertex indexes
	numvertindxs++;
}


// append existing VIndx at tail of vertindxs ---------------------------------
//
void Polygon::AppendVIndx( VIndx *vindx )
{
	// append passed in VIndx
	if ( vindxinsertpos ) {
		vindxinsertpos->setNext( vindx );
	} else {
		vertindxs = vindx;
	}

	vindxinsertpos = vindx;
	//CAVEAT:
	// if passed in VIndx is the head of a list
	// that list is unlinked when appending!
	vindx->setNext( NULL );
	// update number of vertex indexes
	numvertindxs++;
}


// unlink last node in list of vertex indexes ---------------------------------
//
VIndx *Polygon::UnlinkLastVIndx()
{
	VIndx *vlist = vertindxs;
	if ( vlist ) {
		numvertindxs--;
		if ( vlist->getNext() ) {
			for ( ; vlist->getNext()->getNext(); vlist = vlist->getNext() ) ;
			vindxinsertpos = vlist;
			VIndx *node = vlist->getNext();
			vlist->setNext( NULL );
			return node;
		} else {
			// return one and only node in list
			vindxinsertpos = NULL;
			vertindxs	   = NULL;
			return vlist;
		}
	} else {
		// list was already empty
		return NULL;
	}
}


// calculate bounding box encompassing all polygons in list -------------------
//
void Polygon::CalcBoundingBox( BoundingBox* &boundingbox )
{
	VertexChunk& vtxlist = getVertexList();
	VIndx *polyvertlist  = getVList();

	Vertex3 minvertex = vtxlist[ polyvertlist->getIndx() ];
	Vertex3 maxvertex = vtxlist[ polyvertlist->getIndx() ];
	polyvertlist = polyvertlist->getNext();

	for ( Polygon *curpoly = this; ; ) {

		for ( ; polyvertlist; polyvertlist = polyvertlist->getNext() ) {
			Vertex3& testvertex = vtxlist[ polyvertlist->getIndx() ];

			if ( testvertex.getX() < minvertex.getX() )
				minvertex.setX( testvertex.getX() );
			if ( testvertex.getY() < minvertex.getY() )
				minvertex.setY( testvertex.getY() );
			if ( testvertex.getZ() < minvertex.getZ() )
				minvertex.setZ( testvertex.getZ() );

			if ( testvertex.getX() > maxvertex.getX() )
				maxvertex.setX( testvertex.getX() );
			if ( testvertex.getY() > maxvertex.getY() )
				maxvertex.setY( testvertex.getY() );
			if ( testvertex.getZ() > maxvertex.getZ() )
				maxvertex.setZ( testvertex.getZ() );
		}

		if ( ( curpoly = curpoly->getNext() ) != NULL )
			polyvertlist = curpoly->getVList();
		else
			break;
	}

	//CAVEAT:
	// boundingbox MUST NOT be initialized, since it
	// is overwritten without deletion!
	boundingbox = new BoundingBox( minvertex, maxvertex );
}


// check which class with respect to the splitter a given polygon belongs to --
//
int Polygon::CheckIntersection( Polygon *testpoly )
{
	VertexChunk& vtxlist = getVertexList();
	Plane splitterplane  = getPlane();

	int numneg = 0, numpos = 0, numzero = 0;

	// classify all vertices of polygon with respect to splitting plane
	VIndx *polyvertlist = testpoly->getVList();
	for ( ; polyvertlist; polyvertlist = polyvertlist->getNext() ) {
		Vertex3 curvertex = vtxlist[ polyvertlist->getIndx() ];

		if ( splitterplane.PointInPositiveHalfspace( curvertex ) )
			numpos++;
		else if ( splitterplane.PointInNegativeHalfspace( curvertex ) )
			numneg++;
		else
			numzero++;
	}

	// determine polygon's classification from vertex classifications
	if ( ( numneg == 0 ) && ( numpos == 0 ) )
		return POLY_IN_SAME_PLANE;
	else if ( numneg == 0 )
		return POLY_IN_FRONT_SUBSPACE;
	else if ( numpos == 0 )
		return POLY_IN_BACK_SUBSPACE;
	else
		return POLY_STRADDLES_SPLITTER;
}


// check which halfspace another polygon's normal's tip lies in ---------------
//
int Polygon::NormalDirectionSimilar( Polygon *testpoly )
{
	// calculate position of normal's tip
	Vertex3 normalstip = testpoly->getFirstVertex() + testpoly->getPlaneNormal();

	// check position of tip with respect to this polygon's plane
	return !getPlane().PointInNegativeHalfspace( normalstip );
}


// split polygon straddling two subspaces -------------------------------------
//
void Polygon::SplitPolygon( Polygon *poly, Polygon* &frontsubspace, Polygon* &backsubspace )
{
	VertexChunk& vtxlist = getVertexList();
	FaceChunk& facelist  = getFaceList();

	if ( display_messages & MESSAGE_SPLITTING_POLYGON ) {
		sprintf( str_scratchpad, "--- Splitting polygon %d\n", poly->getId() + 1 );
		InfoMessage( str_scratchpad );
	}

	// create the two new polygons: one for each subspace.
	// the original polygon (this) will not be deleted by this function.
	Polygon *frontpoly = new Polygon( baseobject,
									  poly->getId(),
									  poly->getFaceId(),
									  frontsubspace );
	Polygon *backpoly  = new Polygon( baseobject,
									  poly->getId(),
									  poly->getFaceId(),
									  backsubspace );

	// fetch normal vector and arbitrary point on splitter plane
	Vector3 splitternormal = getPlaneNormal();
	Vertex3 splittervertex = getFirstVertex();

	// init pointer to vertex-list of source polygon
	VIndx *polyvertlist = poly->getVList();

	// check location of first vertex and set starting subspace accordingly
	Vector3 directionvec( splittervertex, poly->getFirstVertex() );
	if ( normalize_vectors )
		directionvec.Normalize();
	double scalprod( splitternormal.DotProduct( directionvec ) );

	// check if first vertex lies in splitter plane
	if ( fabs( scalprod ) < EPS_SCALAR ) {

		if ( display_messages & MESSAGE_STARTVERTEX_IN_SPLITTER_PLANE ) {
			sprintf( str_scratchpad, "Startvertex in splitter-plane: %d\n",
					 polyvertlist->getIndx() + 1 );
			InfoMessage( str_scratchpad );
		}
		VIndx *scan = NULL;
		// scan vertex list for vertex not contained in splitter plane
		for ( scan = polyvertlist->getNext(); scan; scan = scan->getNext() ) {
			directionvec.CreateDirVec( splittervertex, vtxlist[ scan->getIndx() ] );
			if ( normalize_vectors )
				directionvec.Normalize();
			// set scalprod such that the start-subspace is opposite
			// the first non-contained vertex's subspace
			scalprod = -splitternormal.DotProduct( directionvec );
			// exit loop if non-contained vertex found
			if ( fabs( scalprod ) >= EPS_SCALAR )
				break;
		}

		if ( display_messages & MESSAGE_STARTVERTEX_IN_SPLITTER_PLANE ) {
			if ( scan == NULL ) {
				sprintf( str_scratchpad, "WARNING: all vertices contained in splitter-plane!\n" );
			} else {
				sprintf( str_scratchpad, "Using vertex %d for halfspace classification.\n",
						 scan->getIndx() + 1 );
			}
			InfoMessage( str_scratchpad );
		}
	}
/*
	// check if polygon to split is non-convex
	if ( [non-convex polygon] ) {
		ErrorMessage( "***ERROR*** Non-convex polygon encountered while splitting." );
		HandleCriticalError();
	}
*/
	// select subspace to start in
	int cursubspace = ( scalprod > 0.0 ) ? FRONT_SUBSPACE : BACK_SUBSPACE;

	// split polygon into two polygons; one for each subspace ---------------
	int twas1 = 0;
	for ( ; polyvertlist; polyvertlist = polyvertlist->getNext() ) {

		// copy current vertex from source polygon to its respective subspace
		if ( cursubspace == FRONT_SUBSPACE )
			frontpoly->AppendNewVIndx( polyvertlist->getIndx() );
		else
			backpoly->AppendNewVIndx( polyvertlist->getIndx() );

		// check intersection of splitter plane with next lineseg -----
		int nextvertex = polyvertlist->getNext() ?
						 polyvertlist->getNext()->getIndx() : poly->getVList()->getIndx();

		// denominator is length of projection of lineseg onto plane normal
		Vector3 linevec( vtxlist[ polyvertlist->getIndx() ], vtxlist[ nextvertex ] );
		double denominator = splitternormal.DotProduct( linevec );

		// numerator is distance from splittervertex to startvertex of lineseg
		Vector3 directionvec( splittervertex, vtxlist[ polyvertlist->getIndx() ] );
		double numerator = splitternormal.DotProduct( directionvec );

		// if lineseg isn't parallel to splitter plane, check for intersection
		if ( fabs( denominator ) >= EPS_DENOM_ZERO ) {

			double t = - numerator / denominator;
			if ( twas1 || ( fabs( t ) < EPS_POINT_ON_LINESEG ) ) {

				if ( display_messages & MESSAGE_VERTEX_IN_SPLITTER_PLANE ) {
					sprintf( str_scratchpad, "Vertex in splitter-plane! (%d)\n", polyvertlist->getIndx() + 1 );
					InfoMessage( str_scratchpad );
				}

				// reset flag
				twas1 = 0;

				// switch subspace and insert same vertex again
				cursubspace = -cursubspace;
				if ( cursubspace == FRONT_SUBSPACE )
					frontpoly->AppendNewVIndx( polyvertlist->getIndx() );
				else
					backpoly->AppendNewVIndx( polyvertlist->getIndx() );

				// check if next vertex is contained in the other subspace
				Vector3 directionvec( splittervertex, vtxlist[ nextvertex ] );
				if ( normalize_vectors )
					directionvec.Normalize();
				double scalprod( splitternormal.DotProduct( directionvec ) );
				if ( ( ( scalprod < 0.0 ) && ( cursubspace == FRONT_SUBSPACE ) ) ||
					 ( ( scalprod > 0.0 ) && ( cursubspace == BACK_SUBSPACE ) ) ) {
					// switch back again
					cursubspace = -cursubspace;
					if ( display_messages & MESSAGE_VERTEX_IN_SPLITTER_PLANE ) {
						sprintf( str_scratchpad, "Vertex alone in %s halfspace!\n",
								 ( cursubspace == BACK_SUBSPACE ) ? "front" : "back" );
						InfoMessage( str_scratchpad );
					}
				}

				//NOTE:
				// the above check is necessary to avoid problems with polygons actually
				// entirely contained in a single subspace but nevertheless classified as
				// straddling. in this case the implicit subspace-switching for t=0 vertices
				// would cause these vertices to be inserted into the wrong subspace!

			} else if ( ( t >= EPS_POINT_ON_LINESEG ) && ( t <= 1.0 - EPS_POINT_ON_LINESEG ) ) {

				// insert intersection point into both subspaces and switch
				cursubspace = -cursubspace;
				Vertex3 newvertex( linevec * t + vtxlist[ polyvertlist->getIndx() ] );
				int newvertindx = vtxlist.FindCloseVertex( newvertex );
				if ( newvertindx == -1 ) {
					newvertindx = vtxlist.AddVertex( newvertex );
					if ( display_messages & MESSAGE_NEW_SPLITVERTEX ) {
						sprintf( str_scratchpad, "Inserting new split-vertex (%d)\n", vtxlist.getNumElements() );
						InfoMessage( str_scratchpad );
					}
				} else if ( display_messages & MESSAGE_REUSING_SPLITVERTEX ) {
					sprintf( str_scratchpad, "Reusing split-vertex (%d)\n", newvertindx + 1 );
					InfoMessage( str_scratchpad );
				}

				frontpoly->AppendNewVIndx( newvertindx );
				backpoly->AppendNewVIndx( newvertindx );

			} else if ( fabs( t - 1.0 ) < EPS_POINT_ON_LINESEG ) {

				// set flag
				twas1 = 1;

				//NOTE:
				// this flag is necessary to avoid numerical problems when
				// a vertex is classified as t=1 for one edge but is not
				// classified as t=0 for the next edge.

				// if this is not the last vertex it will be inserted
				// during the next iteration. if not, the insertion
				// must be done here, otherwise a vertex will be missing
				// for the current polygon!
				if ( polyvertlist->getNext() == NULL ) {
					// determine vertex's subspace
					Polygon *testpoly = ( cursubspace == FRONT_SUBSPACE ) ? frontpoly : backpoly;
					// check if not already inserted as first vertex
					if ( ( testpoly->getVList() != NULL ) &&
						 ( testpoly->getVList()->getIndx() == nextvertex ) )
						continue;
					// insert last vertex into its subspace
					testpoly->AppendNewVIndx( nextvertex );
				}
			}

		} else if ( polyvertlist->getNext() == NULL ) {

			// determine last vertex's subspace
			Polygon *testpoly = ( cursubspace == FRONT_SUBSPACE ) ? frontpoly : backpoly;
			// check if not already inserted as first vertex
			if ( ( testpoly->getVList() != NULL ) &&
				 ( testpoly->getVList()->getIndx() == nextvertex ) )
				continue;
			// insert last vertex into its subspace
			testpoly->AppendNewVIndx( nextvertex );
		}
	}

	// check generated polygons for degeneration
	if ( frontpoly->HasArea() ) {
		// insert front poly into front subspace
		frontsubspace = frontpoly;
		// check back poly
		if ( backpoly->HasArea() ) {
			// insert back poly into back subspace
			backsubspace = backpoly;
			// set new id in sequence
			backpoly->setId( baseobject->getNumPolygons() );
			// this poly has not been counted yet
			baseobject->numpolygons++;
		} else {
			// remove degenerated back poly
			backpoly->setNext( NULL );
			delete backpoly;
			if ( display_messages & MESSAGE_SPLITTING_POLYGON )
				InfoMessage( "Split not carried out due to boundary case: polygon in front halfspace.\n" );
		}
	} else {
		// remove degenerated front poly
		frontpoly->setNext( NULL );
		delete frontpoly;
		if ( display_messages & MESSAGE_SPLITTING_POLYGON )
			InfoMessage( "Split not carried out due to boundary case: polygon in back halfspace.\n" );
		// check back poly
		if ( backpoly->HasArea() ) {
			// insert back poly into back subspace
			backsubspace = backpoly;
		} else {
			// both polygons degenerated: should be impossible!
			Error();
		}
	}
}


// partition list of polygons into two halfspaces (heavily recursive!) --------
//
BSPNode *Polygon::PartitionSpace()
{
	// display invocation message if flag set
	if ( display_messages & MESSAGE_INVOCATION ) {
		sprintf( str_scratchpad, "PartitionSpace() called: #%d polygon %d\n",
				 partition_callcount, getId() + 1 );
		InfoMessage( str_scratchpad );
		partition_callcount++;
	}

	// only one polygon in this halfspace?
	if ( nextpolygon == NULL ) {
		if ( display_messages & MESSAGE_INVOCATION ) {
			sprintf( str_scratchpad, "Leaf: polygon %d\n", getId() + 1 );
			InfoMessage( str_scratchpad );
		}
		return new BSPNode( NULL, NULL, this, NULL );
	}

	// used after recursive calls: has to be automatic!!
	Polygon *splitter = this;

	// take first polygon in list without testing?
	if ( splitter_crit == SPLITTERCRIT_FIRST_POLY )
		goto splitter_selected;

	// find best candidate for splitter -----------------------------
	static int bestnumsplitted;
	static int bestnumcontained;
	bestnumsplitted	 = INT_MAX;
	bestnumcontained = 0;

	static int samplecount;
	samplecount = 0;

	static Polygon *currentsplitter;
	static Polygon *precpolygon;
	static Polygon *splitterpred;
	currentsplitter	= this;
	precpolygon		= NULL;
	splitterpred	= NULL;

	// walk polygon list trying to find appropriate splitter
	for ( ; currentsplitter; precpolygon = currentsplitter, currentsplitter = currentsplitter->getNext() ) {

		if ( splitter_crit == SPLITTERCRIT_RANDOM_SAMPLE ) {
			//TODO: don't use rtl rand() as pseudo random number generator
			//NOTE: assumes that RAND_MAX is at least 10000
			if ( ( rand() % 10000 ) > test_probability )
				continue;
		}

		static int numsplitted;
		static int numcontained;
		numsplitted  = 0;
		numcontained = 0;

		// determine number of polygons splitted and contained by *currentsplitter, respectively
		static Polygon *scanpo;
		for ( scanpo = this; scanpo; scanpo = scanpo->getNext() ) {
			if ( scanpo != currentsplitter ) {
				switch ( currentsplitter->CheckIntersection( scanpo ) ) {
				case POLY_STRADDLES_SPLITTER:
					numsplitted++;
					break;
				case POLY_IN_SAME_PLANE:
					numcontained++;
					break;
				default:
					// *currentsplitter neither splits nor contains *scanpo
					break;
				}
			}
		}

		if ( numsplitted == 0 ) {
			// if nothing splitted at all, skip all other tests
			splitter	 = currentsplitter;
			splitterpred = precpolygon;
			break;
		} else if ( numsplitted < bestnumsplitted ) {
			splitter		 = currentsplitter;
			splitterpred	 = precpolygon;
			bestnumsplitted  = numsplitted;
			bestnumcontained = numcontained;
		} else if ( ( numsplitted == bestnumsplitted ) && ( numcontained > bestnumcontained ) ) {
			splitter		 = currentsplitter;
			splitterpred	 = precpolygon;
			bestnumcontained = numcontained;
		}

		// test sample size for SPLITTERCRIT_SAMPLE_FIRST_N and SPLITTERCRIT_RANDOM_SAMPLE
		if ( splitter_crit & SPLITTERCRITMASK_SAMPLESIZ ) {
			// desired sample size already reached?
			if ( ++samplecount >= sample_size )
				break;
		}
	}

splitter_selected:

	// build polygon list without splitter
	static Polygon *polylist;
	polylist = this;
	if ( splitter == polylist ) {
		polylist = splitter->getNext();
	} else {
		splitterpred->setNext( splitter->getNext() );
	}

	splitter->setNext( NULL );		// unlink splitter from rest of list
	splitter->setNumPolygons( 1 );	// splitter contains only itself

	// display splitter info
	if ( display_messages & MESSAGE_INVOCATION ) {
		sprintf( str_scratchpad, "Splitter: polygon %d\n", splitter->getId() + 1 );
		InfoMessage( str_scratchpad );
	}

	// partition space; create subspace lists -----------------------
	Polygon	*frontsubspace	= NULL;
	Polygon	*backsubspace	= NULL;
	Polygon *backlist		= NULL;	// list of backfacing polygons

	static Polygon *containlist;	// append position for contained polygons
	containlist	= splitter;
	while ( polylist != NULL ) {

		static Polygon *temppoly;
		switch ( splitter->CheckIntersection( polylist ) ) {

			// polygon completely contained in front-subspace
			case POLY_IN_FRONT_SUBSPACE:
				temppoly = polylist->getNext();
				polylist->setNext( frontsubspace );
				frontsubspace = polylist;
				polylist = temppoly;
				break;

			// polygon completely contained in back-subspace
			case POLY_IN_BACK_SUBSPACE:
				temppoly = polylist->getNext();
				polylist->setNext( backsubspace );
				backsubspace = polylist;
				polylist = temppoly;
				break;

			// polygon straddles plane of splitter
			case POLY_STRADDLES_SPLITTER:
				// split polygon and add split pieces to the front- and back-subspace, respectively
				splitter->SplitPolygon( polylist, frontsubspace, backsubspace );
				// delete split polygon
				polylist = polylist->DeleteHead();
				break;

			// polygon lies in same plane as splitter
			case POLY_IN_SAME_PLANE:
				temppoly = polylist->getNext();
				if ( splitter->NormalDirectionSimilar( polylist ) ) {
					// insert frontfacing polygon into contained list (at tail)
					containlist->setNext( polylist );
					containlist = polylist;
					containlist->setNext( NULL );
					containlist->setNumPolygons( 1 );
					//CAVEAT:
					// numpolygons is only correct for first polygon in contained-list (splitter)!!
					splitter->numpolygons++;
				} else {
					// insert backfacing polygon into backlist (at head)
					polylist->setNext( backlist );
					polylist->setNumPolygons( backlist ? backlist->getNumPolygons() + 1 : 1 );
					backlist = polylist;
				}
				polylist = temppoly;
				break;
		}
	}

	// allocate new root; partition halfspaces recursively and return root
	BSPNode	*front = frontsubspace ? frontsubspace->PartitionSpace() : NULL;
	BSPNode	*back  = backsubspace  ? backsubspace->PartitionSpace()  : NULL;
	return new BSPNode( front, back, splitter, backlist );
}


// calculate plane normals for planes of all polygons -------------------------
//
void Polygon::CalcPlaneNormals()
{
	// scan entire polygon list
	FaceChunk& facelist = getFaceList();
	for ( Polygon *polyscan = this; polyscan; polyscan = polyscan->getNext() ) {
		// calculate plane (and normal) if not valid yet
		Face& curface = facelist[ polyscan->getFaceId() ];
		if ( !curface.PlaneValid() )
			curface.CalcPlane( polyscan->getFirstVertex(),
							   polyscan->getSecondVertex(),
							   polyscan->getThirdVertex() );
	}
}


// check all edges for t-vertices and insert trace-vertices as appropriate ----
//
void Polygon::CheckEdges()
{
	//NOTE:
	// this function is terribly slow, because for every polygon
	// every single edge is checked whether it contains any vertices.
	// thus, overall every edge is checked twice for containment
	// of all vertices and no topological information is used at all!

	VertexChunk& vtxlist = getVertexList();

	// scan all polygons
	for ( Polygon *poly = this; poly; poly = poly->getNext() ) {
		// just to be sure
		int numvtxs = vtxlist.getNumElements();
		if ( numvtxs < 3 )
			continue;
		// create vertex mask array
		char *vmask = new char[ numvtxs ];
		memset( vmask, 0, sizeof( char ) * numvtxs );
		// scan all edges: init mask
		VIndx *polyvertlist = poly->getVList();
		for ( ; polyvertlist; polyvertlist = polyvertlist->getNext() )
			vmask[ polyvertlist->getIndx() ] = 1;
		// scan all edges: check t-junctions
		polyvertlist = poly->getVList();
		for ( ; polyvertlist; polyvertlist = polyvertlist->getNext() ) {
			// determine next vertex with wrap-around
			int nextvertex = polyvertlist->getNext() ?
							 polyvertlist->getNext()->getIndx() : poly->getVList()->getIndx();
			// create directed edge
			Vector3	linevec( vtxlist[ polyvertlist->getIndx() ], vtxlist[ nextvertex ] );
			// check edge against all vertices not masked out
			for ( int i = 0; i < numvtxs; i++ )
				if ( vmask[ i ] == 0 ) {
					LineSeg3 lineseg( vtxlist[ polyvertlist->getIndx() ], linevec );
					if ( lineseg.PointOnLineSeg( vtxlist[ i ] ) ) {

						// create new vertex-index
						VIndx *tempvindx = new VIndx( i, polyvertlist->getNext() );
						polyvertlist->setNext( tempvindx );
						if ( tempvindx->getNext() == NULL ) {

							//NOTE:
							// if tracevertex is inserted into last edge, AppendNewVIndx() and AppendVIndx()
							// wouldn't work anymore if the insertposition in the polygon isn't
							// updated correctly; normally this isn't done anyway, though

							vindxinsertpos = tempvindx;
						}

						// new endpoint for lineseg to check
						nextvertex = i;
						vmask[ i ] = 1;
						linevec.CreateDirVec( vtxlist[ polyvertlist->getIndx() ], vtxlist[ nextvertex ] );
						baseobject->numtracevertices++;

						if ( display_messages & MESSAGE_TRACEVERTEX_INSERTED ) {
							sprintf( str_scratchpad, "Trace vertex inserted: polygon %d, vertex %d\n",
									 poly->getId() + 1, i + 1 );
							InfoMessage( str_scratchpad );
						}
					}
				}
		}
		// remove mask array
		delete vmask;
	}
}


// split polygons with vertices not contained in the same plane ---------------
//
Polygon *Polygon::CheckPlanesAndMappings()
{
	//NOTE:
	// for historical reasons this function checks only quadrilaterals
	// for planarity! i.e., if a polygon has more than 4 vertices no
	// check is done at all whether all vertices are contained in the
	// same plane. quadrilaterals, however, are checked and split into
	// two triangles if nonplanar.
	// explicit triangulation of quadrilaterals can be forced by using
	// the static Polygon::triangulate_all flag.

	// fetch vertex and face lists
	VertexChunk& vtxlist = getVertexList();
	FaceChunk& facelist  = getFaceList();

	if ( display_messages & MESSAGE_CHECKING_POLYGON_PLANES )
		InfoMessage( "\nChecking polygon planes and mappings...\n" );

	// scan entire polygon list
	Polygon	*polylist = this;
	for ( Polygon *polyscan = polylist; polyscan; polyscan = polyscan->getNext() ) {

		Face& curface = facelist[ polyscan->getFaceId() ];
		int vertindx1 = polyscan->getFirstVertexIndx();
		int vertindx2 = polyscan->getSecondVertexIndx();
		int vertindx3 = polyscan->getThirdVertexIndx();

		// calculate face plane/normal if not already done
		if ( !curface.PlaneValid() )
			curface.CalcPlane( vtxlist[ vertindx1 ], vtxlist[ vertindx2 ], vtxlist[ vertindx3 ] );

		// convert correspondences to face mapping if not already done
		if ( curface.FaceTexMapped() && !curface.MappingAttached() ) {
			// calc correspondence number
			int corrno = 0;
			for ( int i = 0; i < polyscan->getFaceId(); i++ )
				if ( facelist[ i ].FaceTexMapped() )
					corrno++;
			// set projective space coordinates
			curface.MapXY( 0 ).InitFromVertex3( vtxlist[ vertindx1 ] );
			curface.MapXY( 1 ).InitFromVertex3( vtxlist[ vertindx2 ] );
			curface.MapXY( 2 ).InitFromVertex3( vtxlist[ vertindx3 ] );
			// set (u,v)-space coordinates
			Vertex2	corrpoint;
			corrpoint = baseobject->mappinglist[ corrno ].FetchMapPoint( vertindx1 );
			curface.MapUV( 0 ) = corrpoint;
			corrpoint = baseobject->mappinglist[ corrno ].FetchMapPoint( vertindx2 );
			curface.MapUV( 1 ) = corrpoint;
			corrpoint = baseobject->mappinglist[ corrno ].FetchMapPoint( vertindx3 );
			curface.MapUV( 2 ) = corrpoint;
		}

		// check fourth vertex if face is quadrilateral
		if ( polyscan->getNumVertices() == 4 ) {

			VIndx *vindxscan = polyscan->getVList()->getNext()->getNext();	// pointer to third VIndx
			int testvertindx = vindxscan->getNext()->getIndx();				// fourth vertindx
			if ( !curface.getPlane().PointContained( vtxlist[ testvertindx ] ) || triangulate_all ) {

				// split quadrilateral into triangles
				polylist = polylist->NewPolygon();						// create second triangle
				polylist->AppendNewVIndx( vertindx3 );					// append VIndx for third vertex
				polylist->AppendVIndx( polyscan->UnlinkLastVIndx() );	// append existing VIndx for fourth vertex
				polylist->AppendNewVIndx( vertindx1 );					// append VIndx for first vertex

				if ( display_messages & MESSAGE_SPLITTING_QUADRILATERAL ) {
					sprintf( str_scratchpad, "Splitting quadrilateral: polygon %d\n",
							 polyscan->getId() + 1 );
					InfoMessage( str_scratchpad );
					sprintf( str_scratchpad, "-->Creating new triangle: polygon %d\n",
							 polylist->getId() + 1 );
					InfoMessage( str_scratchpad );
				}
				// update count of split quadrilaterals
				baseobject->numsplitquadrilaterals++;
				// create new face
				int newfaceno = facelist.AddElement( curface );
				Face& newface = facelist[ newfaceno ];
				// update face id of new polygon
				polylist->setFaceId( newfaceno );
				// fetch first three vertices
				vertindx1 = polylist->getFirstVertexIndx();
				vertindx2 = polylist->getSecondVertexIndx();
				vertindx3 = polylist->getThirdVertexIndx();
				// calculate face normal
				newface.CalcPlane( vtxlist[ vertindx1 ], vtxlist[ vertindx2 ], vtxlist[ vertindx3 ] );
				// transfer mapping into face
				if ( curface.FaceTexMapped() ) {
					// calc correspondence number
					int corrno = 0;
					for ( int i = 0; i < polyscan->getFaceId(); i++ )
						if( facelist[ i ].FaceTexMapped() )
							corrno++;
					// set projective space coordinates
					newface.MapXY( 0 ).InitFromVertex3( vtxlist[ vertindx1 ] );
					newface.MapXY( 1 ).InitFromVertex3( vtxlist[ vertindx2 ] );
					newface.MapXY( 2 ).InitFromVertex3( vtxlist[ vertindx3 ] );
					// set (u,v)-space coordinates
					Vertex2	corrpoint;
					corrpoint = baseobject->mappinglist[ corrno ].FetchMapPoint( vertindx1 );
					newface.MapUV( 0 ) = corrpoint;
					corrpoint = baseobject->mappinglist[ corrno ].FetchMapPoint( vertindx2 );
					newface.MapUV( 1 ) = corrpoint;
					corrpoint = baseobject->mappinglist[ corrno ].FetchMapPoint( vertindx3 );
					newface.MapUV( 2 ) = corrpoint;
				}
			}
		}
	}

	return polylist;
}


// calculate probability with which to test a single polygon ------------------
//
int Polygon::CalcSplitterTestProbability()
{
	double numerator   = sample_size;
	double denominator = numpolygons;
	double prob	= ( denominator > 0.0 ) ? numerator / denominator : 1.0;
	prob = ( prob < 1.0 ) ? prob * 10000.0 : 10000.0;
	test_probability = (int) floor( prob + 0.5 );
	if ( test_probability < 1 ) test_probability = 1;
	return ( numpolygons > 0 );
}


// unspecified error encountered ----------------------------------------------
//
void Polygon::Error() const
{
	ErrorMessage( "\n***ERROR*** in object of class BspLib::Polygon." );
	HandleCriticalError();
}


// splitter selection ---------------------------------------------------------
//
int		Polygon::splitter_crit			= Polygon::SPLITTERCRIT_SAMPLE_ALL;
int		Polygon::sample_size			= 20;
int		Polygon::test_probability		= 10000; // means 100.00%


// static flags ---------------------------------------------------------------
//
int		Polygon::triangulate_all		= FALSE;
int		Polygon::normalize_vectors		= FALSE;
int		Polygon::display_messages		= Polygon::MESSAGEMASK_DISPLAY_ALL;


// counter for invocations of PartitionSpace() --------------------------------
//
int		Polygon::partition_callcount	= 0;


// string scratch pad ---------------------------------------------------------
//
char	Polygon::str_scratchpad[ 256 ]	= "";


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
