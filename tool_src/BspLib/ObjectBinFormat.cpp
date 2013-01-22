//-----------------------------------------------------------------------------
//	BSPLIB MODULE: ObjectBinFormat.cpp
//
//  Copyright (c) 1998-1999 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "BspLibDefs.h"
#include "ObjectBinFormat.h"
#include "Transform2.h"

// parsec header files
#include "../../src/libparsec/include/od_geomv.h"
#include "../../src/libparsec/include/od_odt.h"


#define VERTEX_SCALE_FAC ( 1.0 / 20.0 )

// default to defining.
#ifndef BIG_ENDIAN
//#define BIG_ENDIAN
#endif

#ifdef BIG_ENDIAN

#define SWAP_16(s)         ( ((word)(s) >> 8) | ((word)(s) << 8) )
#define SWAP_32(l)         ( ( ((dword)(l) ) << 24 ) | \
                             ( ((dword)(l) ) >> 24 ) | \
                             ( ((dword)(l) & 0x0000ff00) << 8 )  | \
                             ( ((dword)(l) & 0x00ff0000) >> 8 ) )

#else

#define SWAP_16(s)         ( s )
#define SWAP_32(l)         ( l )

#endif // BIG_ENDIAN


void OD2_Geomv_out( float *value )
{

  dword tmp = SWAP_32( DW32( *value ) );
  *(dword *)value = tmp;

}


BSPLIB_NAMESPACE_BEGIN


// calculate affine mapping using face's mapping specification ----------------
//
void ObjectBinFormat::ODT_CalcAffineMapping( Face& face, dword *dmatrx )
{
	Vertex2	vtx;
	double	mat[3][3];
	int i = 0;
	// build xyw-matrix
	for (  i = 0; i < 3; i++ ) {
		vtx = face.MapXY( i );
		mat[ 0 ][ i ] = vtx.getX() *  VERTEX_SCALE_FAC;
		mat[ 1 ][ i ] = vtx.getY() * -VERTEX_SCALE_FAC;
		mat[ 2 ][ i ] = vtx.getW() * -VERTEX_SCALE_FAC;
	}
	Transform2 xyw( (const double(*)[3]) mat );

	// build uv1-matrix
	for ( i = 0; i < 3; i++ ) {
		vtx = face.MapUV( i );
		mat[ 0 ][ i ] = vtx.getX();
		mat[ 1 ][ i ] = vtx.getY();
		mat[ 2 ][ i ] = vtx.getW();
	}
	Transform2 uv1( (const double(*)[3]) mat );

	// invert uv1
	Transform2 uv1i;
	if ( !uv1.Inverse( uv1i ) ) {
		ErrorMessage( "ObjectBinFormat::ODT_CalcAffineMapping(): Collinear mapping coordinates encountered!" );
	}

	// calculate affine mapping
	Transform2 map( xyw );
	map.Concat( uv1i );

	// store result into destination structure (9 coefficients)
	double (*affinemap)[ 3 ] = ( double (*)[3] ) map.LinMatrixAccess();
	fixed_t (*dest)[ 4 ] = (fixed_t (*)[4]) dmatrx;
	for ( i = 0; i < 3; i++ ) {
		dest[ i ][ 0 ] = FLOAT_TO_FIXED( affinemap[ i ][ 0 ] );
		dest[ i ][ 1 ] = FLOAT_TO_FIXED( affinemap[ i ][ 1 ] );
		dest[ i ][ 2 ] = FLOAT_TO_FIXED( 0.0 ); // third column is zero
		dest[ i ][ 3 ] = FLOAT_TO_FIXED( affinemap[ i ][ 2 ] );
	}
}


// create object recognizeable by engine (ODT format) -------------------------
//
byte *ObjectBinFormat::ODT_CreateEngineObject( int& memblocksize )
{
	// fetch object data lists
	VertexChunk& vtxlist  = getVertexList();
	PolygonList& polylist = getPolygonList();
	FaceChunk& facelist   = getFaceList();
	TextureChunk& texlist = getTextureList();

	// calculate some basic numbers
	int numvertices	= getNumVertices();
	int numnormals	= getNumNormals();
	int numallvtxs	= numvertices + numnormals;
	int numpolygons	= BSPTreeAvailable() ? getBspPolygons() : getNumPolygons();
	int numfaces	= getNumFaces();
	int numbspnodes	= getBspPolygons();

	// count number of vertices of all polygons
	int numvertexindices = 0;
	if ( BSPTreeAvailable() )
		bsptree->SumVertexNums( numvertexindices );
	else
		numvertexindices = polylist.FetchHead()->SumVertexNumsEntireList();

	// calculate size of entire object structure
	size_t objectmemsize = sizeof( ODT_GenObject ) +									// generic object header
						sizeof( ODT_Vertex3 ) * numallvtxs +						// object space vertices
						sizeof( ODT_Vertex3 ) * numallvtxs +						// view space vertices
						sizeof( ODT_ProjPoint ) * numallvtxs +						// projected vertices
						sizeof( ODT_SPoint ) * numallvtxs +							// screen space vertices
						sizeof( ODT_Poly ) * numpolygons +							// polygon control data
						sizeof( dword ) * numvertexindices +						// polygon vertindx lists
						sizeof( ODT_Face ) * numfaces +								// face control data
						sizeof( ODT_VisPolys ) + sizeof( dword ) * numpolygons +	// vispolylist
						sizeof( ODT_BSPNode ) * ( numbspnodes + 1 );				// bsp tree

	// allocate memory for all object data (only excluding texturemaps)
	ODT_GenObject *binobj = (ODT_GenObject *) new char[ objectmemsize ];
	memset( binobj, 0x00, objectmemsize );

	// fill in objectheader
	binobj->NextObj			= NULL;
	binobj->PrevObj			= NULL;
	binobj->NextVisObj		= NULL;
	binobj->InstanceSize	= sizeof( ODT_GenObject );
	binobj->NumVerts		= numallvtxs;
	binobj->NumPolyVerts	= numvertices;
	binobj->NumNormals		= numnormals;
	binobj->VertexList		= (ODT_Vertex3 *) ( binobj + 1 );
	binobj->X_VertexList	= (ODT_Vertex3 *)
		( (char *) binobj->VertexList + sizeof( ODT_Vertex3 ) * numallvtxs );
	binobj->P_VertexList	= (ODT_ProjPoint *)
		( (char *) binobj->X_VertexList + sizeof( ODT_Vertex3 ) * numallvtxs );
	binobj->S_VertexList	= (ODT_SPoint *)
		( (char *) binobj->P_VertexList + sizeof( ODT_ProjPoint ) * numallvtxs );
	binobj->NumPolys		= numpolygons;
	binobj->PolyList		= (ODT_Poly *)
		( (char *) binobj->S_VertexList + sizeof( ODT_SPoint ) * numallvtxs );
	binobj->NumFaces		= numfaces;
	binobj->FaceList		= (ODT_Face *)
		( (char *) binobj->PolyList + sizeof( ODT_Poly ) * numpolygons + sizeof( dword ) * numvertexindices );
	binobj->VisPolyList		= (ODT_VisPolys *)
		( (char *) binobj->FaceList + sizeof( ODT_Face ) * numfaces );
	binobj->BSPTree			= (ODT_BSPNode *)
		( (char *) binobj->VisPolyList + sizeof( ODT_VisPolys ) + sizeof( dword ) * numpolygons );

	// calculate bounding sphere for object
	double maxx = -100000; double minx = 100000;
	double maxy = -100000; double miny = 100000;
	double maxz = -100000; double minz = 100000;
	double maxsphere = 0;
	int i = 0;
	for (  i = 0; i < numvertices; i++ ) {
		// calc bounding sphere
		double ctlength = ( (Vector3) vtxlist[ i ] ).VecLength();
		if ( ctlength > maxsphere )
			maxsphere = ctlength;
		// calc bounding box
		if ( vtxlist[ i ].getX() > maxx ) maxx = vtxlist[ i ].getX();
		if ( vtxlist[ i ].getY() > maxy ) maxy = vtxlist[ i ].getY();
		if ( vtxlist[ i ].getZ() > maxz ) maxz = vtxlist[ i ].getZ();
		if ( vtxlist[ i ].getX() < minx ) minx = vtxlist[ i ].getX();
		if ( vtxlist[ i ].getY() < miny ) miny = vtxlist[ i ].getY();
		if ( vtxlist[ i ].getZ() < minz ) minz = vtxlist[ i ].getZ();
	}

	maxsphere *= VERTEX_SCALE_FAC;
	binobj->BoundingSphere  = FLOAT_TO_FIXED( maxsphere );
	binobj->BoundingSphere2 = FLOAT_TO_FIXED( maxsphere * maxsphere );

	// fill vertex list -----------------------------------
	ODT_Vertex3 *vfillp = binobj->VertexList;
	// store face normals first
	for ( i = 0; i < numnormals; i++, vfillp++ ) {
		Vector3 normal( facelist[ i ].getPlaneNormal() );
		vfillp->X	  = FLOAT_TO_FIXED(  normal.getX() );
		vfillp->Y	  = FLOAT_TO_FIXED( -normal.getY() );
		vfillp->Z	  = FLOAT_TO_FIXED( -normal.getZ() );
		vfillp->Flags = 0x00000000L;
	}
	// store real vertices after face normals
	for ( i = 0; i < numvertices; i++, vfillp++ ) {
		vfillp->X	  = FLOAT_TO_FIXED(  vtxlist[ i ].getX() * VERTEX_SCALE_FAC );
		vfillp->Y	  = FLOAT_TO_FIXED( -vtxlist[ i ].getY() * VERTEX_SCALE_FAC );
		vfillp->Z	  = FLOAT_TO_FIXED( -vtxlist[ i ].getZ() * VERTEX_SCALE_FAC );
		vfillp->Flags = 0x00000000L;
	}

	// build flat bsp tree if not available
	if ( BSPTreeAvailable() && !BSPTreeFlatAvailable() ) {
		//TODO:
		// implement flat->linked
	}

	// fill polygon array and vertex index arrays----------
	ODT_Poly *pfillp = binobj->PolyList;
	char *vertbase = (char *) pfillp + sizeof( ODT_Poly ) * numpolygons;
	int  countofs  = 0;
	if ( BSPTreeFlatAvailable() ) {
		// scan polygons of flat bsp tree
		int numnodes = bsptreeflat.getNumNodes();
		for ( i = 1; i <= numnodes; i++ ) {
			// node zero does not correspond to any polygon
			// and is also not included in the number of nodes!
			BSPNodeFlat *node = bsptreeflat.FetchNodePerId( i );
			Polygon *polyscan = node->getPolygon();
			int polyno = polyscan->getId();
			pfillp[ polyno ].NumVerts  = polyscan->getNumVertices();
			pfillp[ polyno ].FaceIndx  = polyscan->getFaceId();
			pfillp[ polyno ].VertIndxs = (dword *) polyscan;
		}
		// scan polygons once again to assign vertex index lists
		// in order instead of in the order of bsp nodes
		for ( i = 1; i <= numnodes; i++, pfillp++ ) {
			Polygon *polyscan = (Polygon *) pfillp->VertIndxs;
			pfillp->VertIndxs = (dword *) ( vertbase + countofs );
			// fill in array of vertex indexes
			polyscan->FillVertexIndexArray( pfillp->VertIndxs );
			countofs += sizeof( dword ) * pfillp->NumVerts;
		}
	} else {
		// scan entire polygon list
		Polygon *polyscan = polylist.FetchHead();
		for ( i = 0; i < polylist.getNumElements(); i++, polyscan = polyscan->getNext(), pfillp++ ) {
			pfillp->NumVerts = polyscan->getNumVertices();
			pfillp->FaceIndx = polyscan->getFaceId();
			pfillp->VertIndxs = (dword *) ( vertbase + countofs );
			// fill in array of vertex indexes
			polyscan->FillVertexIndexArray( pfillp->VertIndxs );
			countofs += sizeof( dword ) * pfillp->NumVerts;
		}
	}
	// correct vertex indexes to take face normals into account
	dword *dfillp = (dword *) vertbase;
	for ( i = 0; i < numvertexindices; i++ )
		*dfillp++ += numnormals;

	//NOTE:
	// the object contains a pointer to the polygon list.
	// this list contains all the polygon structures (no vertex indexes!)
	// the vertex index lists for all the polygons of the object follow
	// contiguously after all the polygon structures.

	// fill face list (defines surface properties) --------
	ODT_Face *ffillp = binobj->FaceList;
	for ( i = 0; i < numfaces; i++, ffillp++ ) {
		ffillp->TexMap			= NULL;
		ffillp->TexEqui 		= NULL;
		ffillp->ColorRGB		= 0;
		ffillp->ColorIndx		= 0;
		ffillp->FaceNormalIndx	= i;
		ffillp->Shading 		= facelist[ i ].getShadingType() & Face::base_mask;

		// write color if any attached and valid
		if ( facelist[ i ].getShadingType() & Face::color_mask ) {
			int coltype = facelist[ i ].getColorType();
			if ( coltype == Face::indexed_col ) {
				dword colindx;
				facelist[ i ].getColorIndex( colindx );
				ffillp->ColorIndx = ( ( ( ( ( colindx << 8 ) + colindx ) << 8 ) + colindx ) << 8 ) + colindx;
			} else if ( coltype == Face::rgb_col ) {
				ColorRGBA coltuple;
				facelist[ i ].getColorRGBA( coltuple );
				dword colrgb = ( ( ( ( ( coltuple.A << 8 ) + coltuple.B ) << 8 ) + coltuple.G ) << 8 ) + coltuple.R;
				ffillp->ColorRGB = colrgb;
			}
		}

		// attach texture
		if ( facelist[ i ].getShadingType() & Face::texmap_mask ) {
			const char *texname = facelist[ i ].getTextureName();
			// simply store name and calc mapping
			ffillp->TexMap = (char *) texname;
			ODT_CalcAffineMapping( facelist[ i ], (dword *) ffillp->TexXmatrx );
		}
	}

	// create bsp tree in object structure
	if ( BSPTreeFlatAvailable() ) {
		ODT_BSPNode *curbspnode = binobj->BSPTree + 1; // skip node at pos zero
		for ( int i = 1; i <= bsptreeflat.getNumNodes(); i++, curbspnode++ ) {
			BSPNodeFlat *node = bsptreeflat.FetchNodePerId( i );
			curbspnode->Polygon   = node->getPolygon()->getId();
			curbspnode->Contained = node->getContainedList();
			//curbspnode->BackList= node->getBackList(); //NOTE: not implemented!
			curbspnode->FrontTree = node->getFrontSubTree();
			curbspnode->BackTree  = node->getBackSubTree();
		}
	}

	memblocksize = objectmemsize;
	return (byte *) binobj;
}


// create object that can be saved to file as single block (ODT format) -------
//
byte *ObjectBinFormat::ODT_CreateFileObject( int& memblocksize, byte *engineobj )
{
	ODT_GenObject *binobj = (ODT_GenObject *) engineobj;
	TextureChunk& texlist = getTextureList();

	// create table of texture names
	char **texnameaddxs;
	char *texturenames, *nexttexname;
	int numtextures = texlist.getNumElements();
	int texnamesize = 0;
	int i = 0;
	if ( numtextures > 0 ) {
		for (  i = 0; i < numtextures; i++ )
			texnamesize += strlen( texlist[ i ].getName() ) + 1;
		texturenames = new char[ texnamesize ];
		nexttexname  = texturenames;
		texnameaddxs = new char*[ numtextures ];
		for ( i = 0; i < numtextures; i++ ) {
			strcpy( nexttexname, texlist[ i ].getName() );
			texnameaddxs[ i ] = nexttexname;
			nexttexname += strlen( nexttexname ) + 1;
		}
	}

	// correct texture pointers to point to texture names in block
	ODT_Face *facescan = binobj->FaceList;
	dword j = 0;
	for (  j = 0; j < binobj->NumFaces; j++, facescan++ )
		if ( facescan->TexMap != NULL )
			for ( int k = 0; k < numtextures; k++ )
				if ( strcmp( texnameaddxs[ k ], facescan->TexMap ) == 0 ) {
//					delete facescan->TexMap;	// legacy
					facescan->TexMap = (char *)
						( (ptrdiff_t) texnameaddxs[ k ] - (ptrdiff_t) texturenames + memblocksize );
					break;
				}

	// make absolute pointers to vertex index lists header relative
	ODT_Poly *polylist = binobj->PolyList;
	for ( j = 0; j < binobj->NumPolys; j++, polylist++ ) {
		polylist->VertIndxs = (dword *) ( (ptrdiff_t) polylist->VertIndxs - (ptrdiff_t) binobj );
	}

	// correct absolute pointers in object header to header-relative pointers
	binobj->VertexList	 = (ODT_Vertex3 *)		( (ptrdiff_t) binobj->VertexList	- (ptrdiff_t) binobj );
	binobj->X_VertexList = (ODT_Vertex3 *)		( (ptrdiff_t) binobj->X_VertexList	- (ptrdiff_t) binobj );
	binobj->P_VertexList = (ODT_ProjPoint *)	( (ptrdiff_t) binobj->P_VertexList	- (ptrdiff_t) binobj );
	binobj->S_VertexList = (ODT_SPoint *)		( (ptrdiff_t) binobj->S_VertexList	- (ptrdiff_t) binobj );
	binobj->PolyList	 = (ODT_Poly *)			( (ptrdiff_t) binobj->PolyList		- (ptrdiff_t) binobj );
	binobj->FaceList	 = (ODT_Face *)			( (ptrdiff_t) binobj->FaceList		- (ptrdiff_t) binobj );
	binobj->VisPolyList	 = (ODT_VisPolys *)		( (ptrdiff_t) binobj->VisPolyList	- (ptrdiff_t) binobj );
	binobj->BSPTree		 = (ODT_BSPNode *)		( (ptrdiff_t) binobj->BSPTree		- (ptrdiff_t) binobj );

	// create block
	byte *block = new byte[ memblocksize + texnamesize ];
	memcpy( block, binobj, memblocksize );
	if ( texnamesize > 0 ) {
		memcpy( block + memblocksize, texturenames, texnamesize );
		memblocksize += texnamesize;
		// free texture name table
		delete texturenames;
		delete texnameaddxs;
	}

	return block;
}


// ----------------------------------------------------------------------------
//
#define DOUBLE_TO_OD2FLOAT(x)	(float)(x)


// calculate affine mapping using face's mapping specification ----------------
//
void ObjectBinFormat::OD2_CalcAffineMapping( Face& face, dword *dmatrx )
{
	Vertex2	vtx;
	double	mat[3][3];
	int i = 0;
	// build xyw-matrix
	for (  i = 0; i < 3; i++ ) {
		vtx = face.MapXY( i );
		mat[ 0 ][ i ] = vtx.getX() *  VERTEX_SCALE_FAC;
		mat[ 1 ][ i ] = vtx.getY() * -VERTEX_SCALE_FAC;
		mat[ 2 ][ i ] = vtx.getW() * -VERTEX_SCALE_FAC;
	}
	Transform2 xyw( (const double(*)[3]) mat );

	// build uv1-matrix
	for ( i = 0; i < 3; i++ ) {
		vtx = face.MapUV( i );
		mat[ 0 ][ i ] = vtx.getX();
		mat[ 1 ][ i ] = vtx.getY();
		mat[ 2 ][ i ] = vtx.getW();
	}
	Transform2 uv1( (const double(*)[3]) mat );

	// invert uv1
	Transform2 uv1i;
	if ( !uv1.Inverse( uv1i ) ) {
		ErrorMessage( "ObjectBinFormat::OD2_CalcAffineMapping(): Collinear mapping coordinates encountered!" );
	}

	// calculate affine mapping
	Transform2 map( xyw );
	map.Concat( uv1i );
	// store result into destination structure (9 coefficients)
	double (*affinemap)[ 3 ] = ( double (*)[3] ) map.LinMatrixAccess();
	float (*dest)[ 4 ] = (float (*)[4]) dmatrx;
	for ( i = 0; i < 3; i++ ) {

	    dest[ i ][ 0 ] = DOUBLE_TO_OD2FLOAT( affinemap[ i ][ 0 ] );
 	    dest[ i ][ 1 ] = DOUBLE_TO_OD2FLOAT( affinemap[ i ][ 1 ] );
 	    dest[ i ][ 2 ] = DOUBLE_TO_OD2FLOAT( 0.0 );
 	    dest[ i ][ 3 ] = DOUBLE_TO_OD2FLOAT( affinemap[ i ][ 2 ] );

		OD2_Geomv_out( &dest[ i ][ 0 ] );
		OD2_Geomv_out( &dest[ i ][ 1 ] );
		OD2_Geomv_out( &dest[ i ][ 2 ] );
		OD2_Geomv_out( &dest[ i ][ 3 ] );
	}
}


// create object recognizeable by engine (ODT format) -------------------------
//
byte *ObjectBinFormat::OD2_CreateEngineObject( int& memblocksize )
{
	// fetch object data lists
	VertexChunk& vtxlist  = getVertexList();
	PolygonList& polylist = getPolygonList();
	FaceChunk& facelist   = getFaceList();
	TextureChunk& texlist = getTextureList();

	// calculate some basic numbers
	int numvertices	= getNumVertices();
	int numnormals	= getNumNormals();
	int numallvtxs	= numvertices + numnormals;
	int numpolygons	= BSPTreeAvailable() ? getBspPolygons() : getNumPolygons();
	int numfaces	= getNumFaces();
	int numbspnodes	= getBspPolygons();

	// count number of vertices of all polygons
	int numvertexindices = 0;
	if ( BSPTreeAvailable() )
		bsptree->SumVertexNums( numvertexindices );
	else
		numvertexindices = polylist.FetchHead()->SumVertexNumsEntireList();

	// calculate size of entire object structure
	size_t objectmemsize = sizeof( OD2_Root ) +										// generic object header
						sizeof( ODT_Vertex3 ) * numallvtxs +						// object space vertices
						sizeof( ODT_Poly ) * numpolygons +							// polygon control data
						sizeof( dword ) * numvertexindices +						// polygon vertindx lists
						sizeof( OD2_Face ) * numfaces;								// face control data

	// allocate memory for all object data (only excluding texturemaps)
	OD2_Root *binobj = (OD2_Root *) new char[ objectmemsize ];
	memset( binobj, 0x00, objectmemsize );

	// fill in objectheader
	strcpy( binobj->odt2, "ODT2\0" );
	binobj->major			= 1;
	binobj->minor			= 0;
	binobj->rootflags		= 0x0000;
	binobj->rootflags2		= 0x0000;
	binobj->NodeList		= NULL;
	binobj->Children[ 0 ]	= NULL;
	binobj->Children[ 1 ]	= NULL;
	binobj->InstanceSize	= SWAP_32( sizeof( OD2_Root ) );
	binobj->NumVerts		= SWAP_32( numallvtxs );
	binobj->NumPolyVerts	= SWAP_32( numvertices );
	binobj->NumNormals		= SWAP_32( numnormals );
	binobj->VertexList		= (OD2_Vertex3 *) ( binobj + 1 );
	binobj->NumPolys		= numpolygons;
	binobj->PolyList		= (OD2_Poly *)
		( (char *) binobj->VertexList + sizeof( OD2_Vertex3 ) * numallvtxs );
	binobj->NumFaces		= numfaces;
	binobj->FaceList		= (OD2_Face *)
		( (char *) binobj->PolyList + sizeof( OD2_Poly ) * numpolygons + sizeof( dword ) * numvertexindices );
	binobj->NumTextures		= 0; // must be set later on

	// calculate bounding sphere for object
	double maxx = -100000; double minx = 100000;
	double maxy = -100000; double miny = 100000;
	double maxz = -100000; double minz = 100000;
	double maxsphere = 0;
	int i = 0;
	for (  i = 0; i < numvertices; i++ ) {
		// calc bounding sphere
		double ctlength = ( (Vector3) vtxlist[ i ] ).VecLength();
		if ( ctlength > maxsphere )
			maxsphere = ctlength;
		// calc bounding box
		if ( vtxlist[ i ].getX() > maxx ) maxx = vtxlist[ i ].getX();
		if ( vtxlist[ i ].getY() > maxy ) maxy = vtxlist[ i ].getY();
		if ( vtxlist[ i ].getZ() > maxz ) maxz = vtxlist[ i ].getZ();
		if ( vtxlist[ i ].getX() < minx ) minx = vtxlist[ i ].getX();
		if ( vtxlist[ i ].getY() < miny ) miny = vtxlist[ i ].getY();
		if ( vtxlist[ i ].getZ() < minz ) minz = vtxlist[ i ].getZ();
	}

	maxsphere *= VERTEX_SCALE_FAC;
	binobj->BoundingSphere = DOUBLE_TO_OD2FLOAT( maxsphere );
	OD2_Geomv_out( &binobj->BoundingSphere );

	// fill vertex list -----------------------------------
	OD2_Vertex3 *vfillp = binobj->VertexList;
	// store face normals first
	for ( i = 0; i < numnormals; i++, vfillp++ ) {
		Vector3 normal( facelist[ i ].getPlaneNormal() );

		vfillp->X = DOUBLE_TO_OD2FLOAT( normal.getX() );
		vfillp->Y = DOUBLE_TO_OD2FLOAT( -normal.getY() );
		vfillp->Z = DOUBLE_TO_OD2FLOAT( -normal.getZ() );

		OD2_Geomv_out( &vfillp->X );
		OD2_Geomv_out( &vfillp->Y );
		OD2_Geomv_out( &vfillp->Z );

		vfillp->Flags = SWAP_32( 0x00000000L );
	}
	// store real vertices after face normals
	for ( i = 0; i < numvertices; i++, vfillp++ ) {

	    vfillp->X = DOUBLE_TO_OD2FLOAT(  vtxlist[ i ].getX() * VERTEX_SCALE_FAC );
	    vfillp->Y = DOUBLE_TO_OD2FLOAT( -vtxlist[ i ].getY() * VERTEX_SCALE_FAC );
	    vfillp->Z = DOUBLE_TO_OD2FLOAT( -vtxlist[ i ].getZ() * VERTEX_SCALE_FAC );
		
		OD2_Geomv_out( &vfillp->X );
		OD2_Geomv_out( &vfillp->Y );
		OD2_Geomv_out( &vfillp->Z );
 
		vfillp->Flags = SWAP_32( 0x00000000L );
	}

	// fill polygon array and vertex index arrays----------
	OD2_Poly *pfillp = binobj->PolyList;
	char *vertbase = (char *) pfillp + sizeof( OD2_Poly ) * numpolygons;
	int  countofs  = 0;
	// scan entire polygon list
	Polygon *polyscan = polylist.FetchHead();
	for ( i = 0; i < polylist.getNumElements(); i++, polyscan = polyscan->getNext(), pfillp++ ) {
		pfillp->NumVerts = polyscan->getNumVertices();
		pfillp->FaceIndx = SWAP_32( polyscan->getFaceId() );
		pfillp->VertIndxs = (dword *) ( vertbase + countofs );
		// fill in array of vertex indexes
		polyscan->FillVertexIndexArray( pfillp->VertIndxs );

		countofs += sizeof( dword ) * pfillp->NumVerts;

		pfillp->NumVerts = SWAP_32( pfillp->NumVerts );
	}


	// correct vertex indexes to take face normals into account
	dword *dfillp = (dword *) vertbase;
	for ( i = 0; i < numvertexindices; i++ ) {
		*dfillp += numnormals;
		*dfillp = SWAP_32( *dfillp );
		dfillp++;
	}

	//NOTE:
	// the object contains a pointer to the polygon list.
	// this list contains all the polygon structures (no vertex indexes!)
	// the vertex index lists for all the polygons of the object follow
	// contiguously after all the polygon structures.

	// fill face list (defines surface properties) --------
	OD2_Face *ffillp = binobj->FaceList;
	for ( i = 0; i < numfaces; i++, ffillp++ ) {
		ffillp->TexMap			= NULL;
		ffillp->ColorRGB		= 0;
		ffillp->ColorIndx		= 0;
		ffillp->FaceNormalIndx	= SWAP_32( i );
		ffillp->Shading 		= SWAP_32( facelist[ i ].getShadingType() & Face::base_mask );

		// write color if any attached and valid
		if ( facelist[ i ].getShadingType() & Face::color_mask ) {
			int coltype = facelist[ i ].getColorType();
			if ( coltype == Face::indexed_col ) {
				dword colindx;
				facelist[ i ].getColorIndex( colindx );
				ffillp->ColorIndx = SWAP_32( ( ( ( ( ( colindx << 8 ) + colindx ) << 8 ) + colindx ) << 8 ) + colindx);
			} else if ( coltype == Face::rgb_col ) {
				ColorRGBA coltuple;
				facelist[ i ].getColorRGBA( coltuple );
				dword colrgb = ( ( ( ( ( coltuple.A << 8 ) + coltuple.B ) << 8 ) + coltuple.G ) << 8 ) + coltuple.R;
				ffillp->ColorRGB = SWAP_32( colrgb );
			}
		}

		// attach texture
		if ( facelist[ i ].getShadingType() & Face::texmap_mask ) {
			const char *texname = facelist[ i ].getTextureName();
			// simply store name and calc mapping
			ffillp->TexMap = (char *) texname;
			OD2_CalcAffineMapping( facelist[ i ], (dword *) ffillp->TexXmatrx );
		}
	}

	memblocksize = objectmemsize;
	return (byte *) binobj;
}


// create object that can be saved to file as single block (OD2 format) -------
//
byte *ObjectBinFormat::OD2_CreateFileObject( int& memblocksize, byte *engineobj )
{
	OD2_Root *binobj = (OD2_Root *) engineobj;
	TextureChunk& texlist = getTextureList();

	// create table of texture names
	char **texnameaddxs;
	char *texturenames, *nexttexname;
	int numtextures = texlist.getNumElements();
	int texnamesize = 0;
	int i = 0;
	if ( numtextures > 0 ) {
		for (  i = 0; i < numtextures; i++ )
			texnamesize += strlen( texlist[ i ].getName() ) + 1;
		texturenames = new char[ texnamesize ];
		nexttexname  = texturenames;
		texnameaddxs = new char*[ numtextures ];
		for ( i = 0; i < numtextures; i++ ) {
			strcpy( nexttexname, texlist[ i ].getName() );
			texnameaddxs[ i ] = nexttexname;
			nexttexname += strlen( nexttexname ) + 1;
		}
	}

	// store number of textures
	binobj->NumTextures = SWAP_32( numtextures );

	// correct texture pointers to point to texture names in block
	OD2_Face *facescan = binobj->FaceList;
	dword j = 0;
	for (  j = 0; j < binobj->NumFaces; j++, facescan++ )
		if ( facescan->TexMap != NULL )
			for ( int k = 0; k < numtextures; k++ )
				if ( strcmp( texnameaddxs[ k ], facescan->TexMap ) == 0 ) {
//					delete facescan->TexMap;	// legacy
					facescan->TexMap = (char *) SWAP_32( ( (ptrdiff_t) texnameaddxs[ k ] - (ptrdiff_t) texturenames + memblocksize ) );
					break;
				}

	binobj->NumFaces = SWAP_32( binobj->NumFaces );

	// make absolute pointers to vertex index lists header relative
	OD2_Poly *polylist = binobj->PolyList;
	for ( j = 0; j < binobj->NumPolys; j++, polylist++ ) {
		polylist->VertIndxs = (dword *) SWAP_32( ( (ptrdiff_t) polylist->VertIndxs - (ptrdiff_t) binobj ) );
	}

	binobj->NumPolys = SWAP_32( binobj->NumPolys );

	// correct absolute pointers in object header to header-relative pointers
//	binobj->NodeList	 = (OD2_Node *)			SWAP_32( (ptrdiff_t) binobj->NodeList		- (ptrdiff_t) binobj );
//	binobj->Children[0]	 = (OD2_Child *)		SWAP_32( (ptrdiff_t) binobj->Children[0]	- (ptrdiff_t) binobj );
//	binobj->Children[1]	 = (OD2_Child *)		SWAP_32( (ptrdiff_t) binobj->Children[1]	- (ptrdiff_t) binobj );
	binobj->VertexList	 = (OD2_Vertex3 *)		SWAP_32( (ptrdiff_t) binobj->VertexList		- (ptrdiff_t) binobj );
	binobj->PolyList	 = (OD2_Poly *)			SWAP_32( (ptrdiff_t) binobj->PolyList		- (ptrdiff_t) binobj );
	binobj->FaceList	 = (OD2_Face *)			SWAP_32( (ptrdiff_t) binobj->FaceList		- (ptrdiff_t) binobj );

	// create block
	byte *block = new byte[ memblocksize + texnamesize ];
	memcpy( block, binobj, memblocksize );
	if ( texnamesize > 0 ) {
		memcpy( block + memblocksize, texturenames, texnamesize );
		memblocksize += texnamesize;
		// free texture name table
		delete texturenames;
		delete texnameaddxs;
	}

	return block;
}


// write entire object as binary file -----------------------------------------
//
int ObjectBinFormat::WriteDataToFile( const char *filename, int format )
{
	// to be filled
	int		memblocksize;
	byte*	engineobj = NULL;
	byte*	fileobj   = NULL;

	if ( format == BINFORMAT_ODT ) {

		sprintf( line, "Writing object data to ODT file: \"%s\"...\n", filename );
		InfoMessage( line );

		// create object as binary block
		engineobj = ODT_CreateEngineObject( memblocksize );

		// convert object to destination file format
		fileobj = ODT_CreateFileObject( memblocksize, engineobj );

	} else if ( format == BINFORMAT_OD2 ) {

		sprintf( line, "Writing object data to OD2 file: \"%s\"...\n", filename );
		InfoMessage( line );

		// create object as binary block
		engineobj = OD2_CreateEngineObject( memblocksize );

		// convert object to destination file format
		fileobj = OD2_CreateFileObject( memblocksize, engineobj );

	} else {
		return FALSE;
	}

	// write binary object representation to file
	int wstat = 0;
	{
	FileAccess ofile( filename, "wb" );
	ofile.Write( fileobj, 1, memblocksize );
	wstat = ofile.Status();
	}

	// free binary object memory blocks
	delete fileobj;
	delete engineobj;

	return ( wstat == SYSTEM_IO_OK );
}


// string scratchpad ----------------------------------------------------------
//
char ObjectBinFormat::line[ 128 ] = "";


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
