//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BspObject.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspObject.h"
#include "BoundingBox.h"
#include "IOData3D.h"


BSPLIB_NAMESPACE_BEGIN

// flags
#define NO_CONSISTENCY_CHECKS


// constructor for BspObjectInfo ----------------------------------------------
//
BspObjectInfo::BspObjectInfo()
{
	numvertices				= 0;
	numpolygons 			= 0;
	numfaces				= 0;
	numtextures 			= 0;
	numcorrespondences		= 0;

	numnormals				= 0;
	numtexmappedfaces		= 0;

	numvertices_in			= 0;
	numpolygons_in			= 0;
	numfaces_in 			= 0;

	numbsppolygons			= 0;
	numpolygons_before_bsp	= 0;

	numtracevertices		= 0;
	nummultvertices			= 0;
	numsplitquadrilaterals	= 0;
}


// constructor for BspObject --------------------------------------------------
//
BspObject::BspObject() : polygonlist( this )
{
	objectname	= NULL;			// no object name attached
	next		= NULL;			// empty list
	objecttrafo.LoadIdentity();	// load identity transformation
}


// set object's name (string is copied into the object!) ----------------------
//
void BspObject::setObjectName( char *name )
{
	delete objectname;
	if ( name != NULL ) {
		objectname = new char[ strlen( name ) + 1 ];
		strcpy( objectname, name );
	} else {
		objectname = NULL;
	}
}


// convert all faces with colorindexes to rgb triplets ------------------------
//
int BspObject::ConvertColorIndexesToRGB( char *palette, int changemode )
{
	int did_convert = FALSE;
	for ( int i = 0; i < facelist.getNumElements(); i++ )
		if ( facelist[ i ].ConvertColorIndexToRGB( palette, changemode ) )
			did_convert = TRUE;
	return did_convert;
}


// print inconsistency error message ------------------------------------------
//
void BspObject::InconsistencyError( const char *err )
{
	if ( err != NULL ) {
		StrScratch message;
		sprintf( message, "**ERROR** [Inconsistencies]: %s", err );
		ErrorMessage( message );
	}
	HandleCriticalError();
}


// update entity counts of contained attribute lists --------------------------
//
void BspObject::UpdateAttributeNumbers()
{
	numvertices			= vertexlist.getNumElements();
	numpolygons			= polygonlist.getNumElements();
	numfaces			= facelist.getNumElements();
	numtextures			= texturelist.getNumElements();
	numcorrespondences	= mappinglist.getNumElements();

	numnormals			= 0;
	numtexmappedfaces	= 0;

	for ( int i = 0; i < numfaces; i++ ) {
		// count number of faces with already calculated normals
		if ( facelist[ i ].NormalValid() )
			numnormals++;
		// count number of texture mapped faces
		if ( facelist[ i ].FaceTexMapped() )
			numtexmappedfaces++;
	}
}


// process data immediately after parse was done ------------------------------
//
void BspObject::CheckParsedData()
{
	// calculate entity counts
	UpdateAttributeNumbers();

	// remember number of input entities
	numvertices_in	= numvertices;
	numpolygons_in	= numpolygons;
	numfaces_in		= numfaces;

#if !defined( NO_CONSISTENCY_CHECKS ) && !defined( VIEW_BSP )

	// consistency checks
	if ( numpolygons != numfaces )
		InconsistencyError( "properties must be defined for every face!" );
	if ( numcorrespondences != numtexmappedfaces )
		InconsistencyError( "mapping must be specified for every texturemapped face!" );
#endif

}


// display statistics (number of vertices, faces, polygons, etc.) -------------
//
void BspObject::DisplayStatistics()
{
	UpdateAttributeNumbers();

	StrScratch message;
	sprintf( message, "\n--- Statistics ---------------------\n" );

#define sprintf InfoMessage( message ); sprintf

	sprintf( message, "\nINPUT\n" );
	sprintf( message, " Vertices  = %d\n", numvertices_in );
	sprintf( message, " Faces     = %d\n", numfaces_in );
	sprintf( message, " Polygons  = %d\n", numpolygons_in );

	sprintf( message, "\nOUTPUT\n" );
	sprintf( message, " Vertices  = %d\n", numvertices );
	sprintf( message, " Faces     = %d\n", numfaces );
	sprintf( message, " Polygons  = %d\n", numbsppolygons );

	sprintf( message, "\nGENERAL\n" );
	sprintf( message, " Textures  = %d\n", numtextures );
	sprintf( message, " TexFaces  = %d\n", numtexmappedfaces );

	sprintf( message, "\nANALYSIS\n" );
	sprintf( message, " Inserted vertices = %d\n", numvertices - numvertices_in );
	sprintf( message, " Inserted polygons = %d (%d split by bsp; %d split as quadrilateral)\n",
			 numbsppolygons - numpolygons_in,
			 numbsppolygons - numpolygons_in - numsplitquadrilaterals,
			 numsplitquadrilaterals );
	sprintf( message, " Tracevertices     = %d\n", numtracevertices );
	sprintf( message, " Multiple vertices = %d\n", nummultvertices );

	sprintf( message, "\n------------------------------------\n\n" );
	InfoMessage( message );

#undef sprintf

}


// scale entire object (correct vertices and mappings) ------------------------
//
void BspObject::ApplyScale( double scalefac )
{
	// scale entire vertex list
	numvertices = vertexlist.getNumElements();
	int i = 0;
	for (  i = 0; i < numvertices; i++ ) {
		vertexlist[ i ].setX( vertexlist[ i ].getX() * scalefac );
		vertexlist[ i ].setY( vertexlist[ i ].getY() * scalefac );
		vertexlist[ i ].setZ( vertexlist[ i ].getZ() * scalefac );
	}
	// scale (x,y,z) part of mappings
	numfaces = facelist.getNumElements();
	for ( i = 0; i < numfaces; i++ ) {
		if ( facelist[ i ].MappingAttached() ) {
			for ( int j = 0; j < 3; j++ ) {
				Vertex2 vtx = facelist[ i ].MapXY( j );
				vtx.setX( vtx.getX() * scalefac );
				vtx.setY( vtx.getY() * scalefac );
				vtx.setW( vtx.getW() * scalefac );
				facelist[ i ].MapXY( j ) = vtx;
			}
		}
	}
}


// apply translation to all coordinates and set center to (0,0,0) -------------
//
void BspObject::ApplyCenter()
{
	// get center
	Vector3 cvec = objecttrafo.ExtractTranslation();
	double cx = cvec.getX();
	double cy = cvec.getY();
	double cz = cvec.getZ();

	// translate all vertices
	numvertices = vertexlist.getNumElements();
	for ( int i = 0; i < numvertices; i++ ) {
		vertexlist[ i ].setX( vertexlist[ i ].getX() + cx );
		vertexlist[ i ].setY( vertexlist[ i ].getY() + cy );
		vertexlist[ i ].setZ( vertexlist[ i ].getZ() + cz );
	}
}


// apply transformation matrix and set to identity afterwards -----------------
//
void BspObject::ApplyTransformation()
{
	// transform all vertices
	numvertices = vertexlist.getNumElements();
	for ( int i = 0; i < numvertices; i++ ) {
		vertexlist[ i ] = objecttrafo.TransformVector3( vertexlist[ i ] );
	}

	objecttrafo.LoadIdentity();
}


// calculate axial bounding box -----------------------------------------------
//
void BspObject::CalcBoundingBox( Vertex3& minvertex, Vertex3& maxvertex )
{
	//NOTE:
	// if a transformation is attached, only the translation is
	// taken into account. so, the bounding box is not correct if,
	// for instance, a rotation is attached.
	// thus, ApplyTransformation() is normally invoked before
	// bounding boxes are calculated.

	// fetch object's location in world space
	Vector3 worldcenter = objecttrafo.FetchTranslation();

	if ( ( numvertices = vertexlist.getNumElements() ) < 1 ) {
		minvertex = worldcenter;
		maxvertex = worldcenter;
		return;
	}

	// start with first vertex
	minvertex = vertexlist[ 0 ];
	maxvertex = vertexlist[ 0 ];

	// test all other vertices
	for ( int i = 1; i < numvertices; i++ ) {
		Vertex3& testvertex = vertexlist[ i ];

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

	minvertex += worldcenter;
	maxvertex += worldcenter;
}


// check planes of all polygons contained in polygonlist ----------------------
//
void BspObject::CheckPolygonPlanes()
{
	// scan linear polygon list and check planes of all polygons
	polygonlist.CheckPolygonPlanes();

	// counts have to be updated; CheckPolygonPlanes() may have
	// created new vertices, polygons, and faces
	UpdateAttributeNumbers();

	if( numnormals != numfaces ) {
		ErrorMessage( "***ERROR*** Inconsistencies in normals!" );
		HandleCriticalError();
	}

	if ( numsplitquadrilaterals > 0 ) {
		StrScratch message;
		sprintf( message, "\n%d quadrilaterals were split into triangles.\n",
				 numsplitquadrilaterals );
		InfoMessage( message );
	}
}


// check for multiple vertices in vertexlist ----------------------------------
//
void BspObject::CheckVertices( int verbose )
{
	// determine number of equal vertices in vertexlist
	nummultvertices = vertexlist.CheckVertices( verbose );
}


// calculate bounding boxes for all nodes of bsp tree -------------------------
//
void BspObject::CalcBoundingBoxes()
{
	if ( !bsptree.TreeEmpty() ) {
		InfoMessage( "\nCalculating bounding boxes for bsp-tree nodes...\n" );
		bsptree->CalcBoundingBoxes();
	}
}


// calculate separator planes for all polygon-nodes of bsp tree ---------------
//
void BspObject::CalcSeparatorPlanes()
{
	if ( !bsptree.TreeEmpty() ) {
		InfoMessage( "\nCalculating explicit separator planes for bsp-tree nodes...\n" );
		bsptree->CalcSeparatorPlanes();
	}
}


// check edges of polygons contained in bsp tree for t-vertices ---------------
//
void BspObject::CheckEdges()
{
	if ( !bsptree.TreeEmpty() ) {
		InfoMessage( "\nChecking edges of bsp-tree polygons...\n" );
		bsptree->CheckEdges();
	}
}


// calculate plane normals for all polygons -----------------------------------
//
void BspObject::CalcPlaneNormals()
{
	// scan linear polygon list and calculate normals for all polygons
	polygonlist.CalcPlaneNormals();
}


// build a list of bounding boxes containing all objects of list --------------
//
BoundingBox *BspObject::BuildBoundingBoxList()
{
	// build bounding box list as secondary data structure.
	// there is a bounding box for every object; order is reversed!
	BoundingBox *headbox = NULL;
	for ( BspObject *obj = this; obj; obj = obj->getNext() ) {
		headbox = new BoundingBox( obj, headbox );
	}

	return headbox;
}


// build bsp tree and store pointer into object structure ---------------------
//
BSPNode *BspObject::BuildBSPTree()
{
	if ( bsptree.TreeEmpty() ) {
		InfoMessage( "\nCompiling bsp tree...\n" );
		numpolygons_before_bsp = numpolygons;
		bsptree.InitTree( polygonlist.PartitionSpace() );
	}

	return bsptree.getRoot();
}


// build linked bsp tree from flat bsp tree -----------------------------------
//
BSPNode *BspObject::BuildBSPTreeFromFlat()
{
	if ( bsptree.TreeEmpty() ) {
		InfoMessage( "\nBuilding linked bsp-tree from flat representation...\n" );
		bsptree.InitTree( bsptreeflat.BuildBSPTree( 1 ) );
		// linear list of polygons is not valid anymore!
		polygonlist.InvalidateList();
	}

	return bsptree.getRoot();
}


// determine if linked bsp tree is available (non-empty) ----------------------
//
int BspObject::BSPTreeAvailable()
{
	return !bsptree.TreeEmpty();
}


// determine if flat bsp tree is available (non-empty) ------------------------
//
int BspObject::BSPTreeFlatAvailable()
{
	return !bsptreeflat.TreeEmpty();
}


// determine if any bsp tree is available (non-empty) -------------------------
//
int BspObject::BspTreeAvailable()
{
	return ( BSPTreeAvailable() || BSPTreeFlatAvailable() );
}


// merge another BspObject into this object (corrects everything necessary) ---
//
void BspObject::MergeObjects( BspObject *mergeobj )
{
	if ( mergeobj != NULL ) {

		if ( !check_vertex_doublets_on_merge ) {

			// merge vertex lists
			int numsourcevertices = vertexlist.getNumElements();
			int nummergevertices  = mergeobj->vertexlist.getNumElements();
			int i = 0; 
			for (  i = 0; i < nummergevertices; i++ ) {
				vertexlist.AddVertex( mergeobj->vertexlist[ i ] );
			}
			numvertices		+= nummergevertices;
			numvertices_in	+= mergeobj->numvertices_in;

			// merge face lists
			int numsourcefaces = facelist.getNumElements();
			int nummergefaces  = mergeobj->facelist.getNumElements();
			for ( i = 0; i < nummergefaces; i++ ) {
				Face& mergeface = mergeobj->facelist[ i ];
				mergeface.setId( mergeface.getId() + numsourcefaces );
				facelist.AddElement( mergeface );
			}
			numfaces	+= nummergefaces;
			numfaces_in	+= mergeobj->numfaces_in;

			// correct all polygons
			if ( !mergeobj->BSPTreeAvailable() ) {
				// correct entire polygon list
				Polygon *polylist = mergeobj->polygonlist.FetchHead();
				for ( ; polylist; polylist = polylist->getNext() )
					polylist->CorrectBase( this, numsourcevertices, numsourcefaces, numpolygons );
				numpolygons		+= mergeobj->numpolygons;
				numpolygons_in	+= mergeobj->numpolygons_in;
				// merge polygon lists
				polygonlist.MergeLists( &mergeobj->polygonlist );
			} else {
				// correct all polygons contained in bsp tree
				mergeobj->bsptree->CorrectPolygonBases( this, numsourcevertices, numsourcefaces, numbsppolygons );
				numbsppolygons	+= mergeobj->numbsppolygons;
				numpolygons_in	+= mergeobj->numpolygons_in;

				//NOTE:
				// the bsp trees themselves are not merged! no polygonlist is
				// copied over, since it is empty. the bsp trees have to be
				// merged somewhere else, to take over the polygons and the
				// bsp tree structure!
			}

		} else {

			//NOTE:
			// this is the merging code that assumes that many vertices
			// in the two objects are potentially exactly identical.
			// therefore, the vertexlists are checked for doublets and
			// vertex indexes contained in polygons are corrected thusly.
			// since this normally is not necessary, the code is brute
			// force, yielding quadratic performance!!
			// however, in vrml files this may be a frequent case! if
			// a connected object is saved as separate objects (e.g., to
			// use more than one texture) the entire vertex list is often
			// instanced via USE at each of these objects. this yields
			// many objects with identical vertex lists.

			// merge vertex lists and alloc index mapping table
			int numsourcevertices = vertexlist.getNumElements();
			int nummergevertices  = mergeobj->vertexlist.getNumElements();
			int numinsertvertices = 0;
			int numinsertoriginal = 0;
			int *indxmap = new int[ nummergevertices ];
			int i = 0;
			int j = 0;
			for (  i = 0; i < nummergevertices; i++ ) {
				for (  j = 0; j < numsourcevertices; j++ )
					if ( vertexlist[ j ] == mergeobj->vertexlist[ i ] ) {
						indxmap[ i ] = j;
						break;
					}
				if ( j == numsourcevertices ) {
					indxmap[ i ] = vertexlist.getNumElements();
					vertexlist.AddVertex( mergeobj->vertexlist[ i ] );
					numinsertvertices++;
					if ( i < mergeobj->numvertices_in )
						numinsertoriginal++;
				}
			}
			numvertices		+= numinsertvertices;
			numvertices_in	+= numinsertoriginal;

			// merge face lists
			int numsourcefaces = facelist.getNumElements();
			int nummergefaces  = mergeobj->facelist.getNumElements();
			for ( i = 0; i < nummergefaces; i++ ) {
				Face& mergeface = mergeobj->facelist[ i ];
				mergeface.setId( mergeface.getId() + numsourcefaces );
				facelist.AddElement( mergeface );
			}
			numfaces	+= nummergefaces;
			numfaces_in	+= mergeobj->numfaces_in;

			// correct all polygons
			if ( !mergeobj->BSPTreeAvailable() ) {
				// correct entire polygon list
				Polygon *polylist = mergeobj->polygonlist.FetchHead();
				for ( ; polylist; polylist = polylist->getNext() )
					polylist->CorrectBaseByTable( this, indxmap, numsourcefaces, numpolygons );
				numpolygons		+= mergeobj->numpolygons;
				numpolygons_in	+= mergeobj->numpolygons_in;
				// merge polygon lists
				polygonlist.MergeLists( &mergeobj->polygonlist );
			} else {
				// correct all polygons contained in bsp tree
				mergeobj->bsptree->CorrectPolygonBasesByTable( this, indxmap, numsourcefaces, numbsppolygons );
				numbsppolygons	+= mergeobj->numbsppolygons;
				numpolygons_in	+= mergeobj->numpolygons_in;
			}

			// free vertex index map
			delete indxmap;
		}

		// merge texture lists
		int numsourcetextures = texturelist.getNumElements();
		int nummergetextures  = mergeobj->texturelist.getNumElements();
		int i = 0;
		int j = 0;
		for (  i = 0; i < nummergetextures; i++ ) {
			//NOTE:
			// terribly inefficient implementation. normally, number of
			// textures should be very low. if this is not the case this
			// may take considerable processing time!
			for (  j = 0; j < numsourcetextures; j++ )
				if ( strcmp( texturelist[ j ].getName(), mergeobj->texturelist[ i ].getName() ) == 0 )
					break;
			if ( j == numsourcetextures )
				texturelist.AddElement( mergeobj->texturelist[ i ] );
		}
		numtextures += nummergetextures;

		//NOTE:
		// mappinglists need not be merged, since they are only temporarily
		// used before complete face definitions are built. actual texture
		// parameterization is a part of each face!

		// add up other counts
		numcorrespondences		+= mergeobj->numcorrespondences;
		numnormals				+= mergeobj->numnormals;
		numtexmappedfaces		+= mergeobj->numtexmappedfaces;
		numtracevertices		+= mergeobj->numtracevertices;
		nummultvertices			+= mergeobj->nummultvertices;
		numsplitquadrilaterals	+= mergeobj->numsplitquadrilaterals;
		numpolygons_before_bsp	+= mergeobj->numpolygons_before_bsp;
	}

}


// collapse entire list of BspObjects into this object ------------------------
//
void BspObject::CollapseObjectList()
{
	//NOTE:
	// this function only works if no bsp trees have been built yet!
	// if bsp trees have already been built, ObjectBSPNode::CreateMergedBSPTree()
	// may be used to merge all objects and bsp trees into one.

	// exit if bsp tree available
	if ( BspTreeAvailable() )
		return;
	BspObject *curobj = NULL;
	curobj = getNext();
	// merge all subsequent objects into this object
	for ( curobj = curobj; curobj; curobj = curobj->getNext() ) {
		MergeObjects( curobj );

		// invalidate other object's polygonlist
		curobj->getPolygonList().InvalidateList();

		//NOTE:
		// the polygon list must be invalidated prior to deletion
		// because the actual polygons are now part of this object
		// and therefore still in use!
	}

	// delete all objects that have been merged into this object
	delete next;
	next = NULL;

	UpdateAttributeNumbers();
}


// static flags ---------------------------------------------------------------
//
int		BspObject::check_vertex_doublets_on_merge	= TRUE;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
