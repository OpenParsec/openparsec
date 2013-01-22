//-----------------------------------------------------------------------------
//	BSPLIB MODULE: BRep.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "BRep.h"

// qvlib headers
#include <QvCoordinate3.h>
#include <QvMaterial.h>
#include <QvMaterialBinding.h>
#include <QvMatrixTransform.h>
#include <QvNormal.h>
#include <QvNormalBinding.h>
#include <QvRotation.h>
#include <QvScale.h>
#include <QvShapeHints.h>
#include <QvTexture2.h>
#include <QvTextureCoordinate2.h>
#include <QvTexture2Transform.h>
#include <QvTranslation.h>
#include <QvTransform.h>

#ifdef USE_JPEG_LIBRARY
#include <jpeglib.h> 
#endif

BSPLIB_NAMESPACE_BEGIN


struct tga_header_s {

    byte IDLength;
    byte CMapType;
    byte ImgType;
    byte CMapStartLo;
    byte CMapStartHi;
    byte CMapLengthLo;
    byte CMapLengthHi;
    byte CMapDepth;
    byte XOffSetLo;
    byte XOffSetHi;
    byte YOffSetLo;
    byte YOffSetHi;
    byte WidthLo;
    byte WidthHi;
    byte HeightLo;
    byte HeightHi;
    byte PixelDepth;
    byte ImageDescriptor;
};


// constructor for b-rep object -----------------------------------------------
//
BRep::BRep( QvState *state )
{
	// save pointer to state
	m_state = state;

	// create new object and insert into list
	m_baseobject = m_state->vrmlfile_base->getObjectList().CreateNewObject();
}


// fetch pointer to Coordinate3 array and length ------------------------------
//
float *BRep::FetchCoordinate3State( int &num )
{
	num = 0;
	float *coarray = NULL;
	QvElement *elt = m_state->getTopElement( QvState::Coordinate3Index );

	if ( elt != NULL ) {
		QvCoordinate3 *c3 = (QvCoordinate3 *) elt->data;
		num		= c3->point.num;
		coarray	= c3->point.values;
	}

	return coarray;
}


// fetch pointer to texture coordinates array and length ----------------------
//
float *BRep::FetchTextureCoordinate2State( int &num )
{
	num = 0;
	float *coarray = NULL;
	QvElement *elt = m_state->getTopElement( QvState::TextureCoordinate2Index );

	if ( elt != NULL ) {
		QvTextureCoordinate2 *tc = (QvTextureCoordinate2 *) elt->data;
		num		= tc->point.num;
		coarray	= tc->point.values;
	}

	return coarray;
}


// fetch pointer to normals array and length ----------------------------------
//
float *BRep::FetchNormalState( int &num )
{
	num = 0;
	float *coarray = NULL;
	QvElement *elt = m_state->getTopElement( QvState::NormalIndex );

	if ( elt != NULL ) {
		QvNormal *nml = (QvNormal *) elt->data;
		num		= nml->vector.num;
		coarray	= nml->vector.values;
	}

	return coarray;
}


// accumulate current transformation stack and init Transform3 object ---------
//
void BRep::FetchTransformationState( Transform3& trafo )
{
	trafo.LoadIdentity();

	QvElement *trafoelt = NULL;
	trafoelt = m_state->getTopElement( QvState::TransformationIndex );
	for ( ; trafoelt; trafoelt = trafoelt->next ) {

		if ( trafoelt->type == QvElement::Translation ) {
			QvTranslation *tnode = (QvTranslation *) trafoelt->data;
			trafo.Translate( tnode->translation.value[ 0 ],
							 tnode->translation.value[ 1 ],
							 tnode->translation.value[ 2 ] );
		} else if ( trafoelt->type == QvElement::Rotation ) {
			QvRotation *tnode = (QvRotation *) trafoelt->data;
			trafo.Rotate( tnode->rotation.angle,
						  tnode->rotation.axis[ 0 ],
						  tnode->rotation.axis[ 1 ],
						  tnode->rotation.axis[ 2 ] );
		} else if ( trafoelt->type == QvElement::Scale ) {
			QvScale *tnode = (QvScale *) trafoelt->data;
			trafo.Scale( tnode->scaleFactor.value[ 0 ],
						 tnode->scaleFactor.value[ 1 ],
						 tnode->scaleFactor.value[ 2 ] );
		} else if ( trafoelt->type == QvElement::Transform ) {
			QvTransform *tnode = (QvTransform *) trafoelt->data;
			trafo.Translate( -tnode->center.value[ 0 ],
							 -tnode->center.value[ 1 ],
							 -tnode->center.value[ 2 ] );
			trafo.Rotate( -tnode->scaleOrientation.angle,
						  tnode->scaleOrientation.axis[ 0 ],
						  tnode->scaleOrientation.axis[ 1 ],
						  tnode->scaleOrientation.axis[ 2 ] );
			trafo.Scale( tnode->scaleFactor.value[ 0 ],
						 tnode->scaleFactor.value[ 1 ],
						 tnode->scaleFactor.value[ 2 ] );
			trafo.Rotate( tnode->scaleOrientation.angle,
						  tnode->scaleOrientation.axis[ 0 ],
						  tnode->scaleOrientation.axis[ 1 ],
						  tnode->scaleOrientation.axis[ 2 ] );
			trafo.Rotate( tnode->rotation.angle,
						  tnode->rotation.axis[ 0 ],
						  tnode->rotation.axis[ 1 ],
						  tnode->rotation.axis[ 2 ] );
			trafo.Translate( tnode->center.value[ 0 ],
							 tnode->center.value[ 1 ],
							 tnode->center.value[ 2 ] );
			trafo.Translate( tnode->translation.value[ 0 ],
							 tnode->translation.value[ 1 ],
							 tnode->translation.value[ 2 ] );
		} else if ( trafoelt->type == QvElement::MatrixTransform ) {
			QvMatrixTransform *tnode = (QvMatrixTransform *) trafoelt->data;
			Transform3 tmat( (const float(*)[4]) tnode->matrix.value );
			trafo.Concat( tmat );
		}
	}
}


// accumulate current texture transformation stack into Transform2 object -----
//
void BRep::FetchTextureTransformationState( Transform2& trafo )
{
	trafo.LoadIdentity();

	QvElement *trafoelt = m_state->getTopElement( QvState::Texture2TransformationIndex );
	for ( ; trafoelt; trafoelt = trafoelt->next ) {

		if ( trafoelt->type == QvElement::Unknown /*Transform*/ ) {

			//NOTE:
			// well, yes. QvLib really uses QvElement::Unknown as type
			// for texture transformations!!

			QvTexture2Transform *tnode = (QvTexture2Transform *) trafoelt->data;
			trafo.Translate( -tnode->center.value[ 0 ],
							 -tnode->center.value[ 1 ] );
			trafo.Scale( tnode->scaleFactor.value[ 0 ],
						 tnode->scaleFactor.value[ 1 ] );
			trafo.Rotate( tnode->rotation.value );
			trafo.Translate( tnode->center.value[ 0 ],
							 tnode->center.value[ 1 ] );
			trafo.Translate( tnode->translation.value[ 0 ],
							 tnode->translation.value[ 1 ] );
		}
	}
}


// check current texture state; create new texture if necessary ---------------
//
Texture *BRep::CheckTexture2State()
{
	QvElement *elt = m_state->getTopElement( QvState::Texture2Index );

	if ( elt == NULL )
		return NULL;

	// fetch texture node
	QvTexture2 *tex = (QvTexture2 *) elt->data;
	const char *texfilename = tex->filename.value.getString();

	if ( ( texfilename == NULL ) || ( *texfilename == 0 ) )
		return NULL;

	// check if texture already exists
	TextureChunk& texlist = m_baseobject->getTextureList();
	for ( int i = 0; i < texlist.getNumElements(); i++ ) {
		if ( strcmp( texlist[ i ].getFile(), texfilename ) == 0 )
			return &texlist[ i ];
	}

	// default texture width and height are 1, yielding texture
	// coordinates between 0.0 and 1.0 when multiplied by vrml
	// texture coordinates later on.
	int width  = 1;
	int height = 1;

	// try to open texture file if it is in 3df format to read actual width and height
	int namelen = strlen( texfilename );
	if ( ( namelen > 4 ) && ( strcmp( texfilename + namelen - 4, ".3df" ) == 0 ) ) {

		// open 3df file
		FILE *fp = fopen( texfilename, "rb" );
		if ( fp != NULL ) {

			// header fields
			char version[ 7 ];
			char formatspec[ 11 ];
			int  lodsmall;
			int  lodlarge;
			int	 aspectw;
			int  aspecth;

			// scan out header
			if ( fscanf( fp, "3df v%6s %10s lod range: %i %i aspect ratio: %i %i",
				 version, formatspec, &lodsmall, &lodlarge, &aspectw, &aspecth ) == 6 ) {
				 
				fclose( fp );

				version[ 6 ]     = 0;
				formatspec[ 10 ] = 0;

				// determine geometry
				int aindx = ( aspectw << 4 ) | aspecth;
				switch ( aindx ) {

					case 0x81 :
						width  = lodlarge;
						height = lodlarge / 8;
						break;

					case 0x41 :
						width  = lodlarge;
						height = lodlarge / 4;
						break;

					case 0x21 :
						width  = lodlarge;
						height = lodlarge / 2;
						break;

					case 0x11 :
						width  = lodlarge;
						height = lodlarge;
						break;

					case 0x12 :
						width  = lodlarge / 2;
						height = lodlarge;
						break;

					case 0x14 :
						width  = lodlarge / 4;
						height = lodlarge;
						break;

					case 0x18 :
						width  = lodlarge / 8;
						height = lodlarge;
						break;

					default :
						break;
				}
			}
		}
		
	} else if ( ( namelen > 4 ) && ( strcmp( texfilename + namelen - 4, ".tga" ) == 0 ) ) {

		// try to get width/height from .tga header

		FILE *fp = fopen( texfilename, "rb" );
		if ( fp != NULL ) {
		
			tga_header_s header;
		
			// read header
			if ( fread( &header, 1, sizeof( tga_header_s ), fp ) == sizeof( tga_header_s ) ) {

				width  = ( header.WidthHi  << 8 ) | header.WidthLo;
				height = ( header.HeightHi << 8 ) | header.HeightLo;
			}
			
			fclose( fp );
		}
		
	} else if ( ( namelen > 4 ) && ( strcmp( texfilename + namelen - 4, ".jpg" ) == 0 ) ) {

#ifdef USE_JPEG_LIBRARY

		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr 		  jerr;
	
		FILE *fp = fopen( texfilename, "rb" );
		if ( fp != NULL ) {
	
			// specify error handler
			cinfo.err = jpeg_std_error( &jerr );
	
			// init decompressor
			jpeg_create_decompress( &cinfo );
	
			// specify the source for the compressed data
			jpeg_stdio_src( &cinfo, fp );
	
			// read jpeg header
			jpeg_read_header( &cinfo, TRUE );

			// de-init decompressor
			jpeg_destroy_decompress( &cinfo );

			fclose( fp );
			
			width  = cinfo.image_width;
			height = cinfo.image_height;
		}
#else

		fflush( stdout );
		fprintf( stderr, "\nJPEG files are not supported by this version of makeodt.\n" );
		fprintf( stderr, "If you need JPEG support, you can enable -DUSE_JPEG_LIBRARY\n" );
		fprintf( stderr, "in the BspLib Makefile and recompile BspLib and makeodt.\n" );
		fprintf( stderr, "This will require a working version of libjpeg 6.x\n" );
		fprintf( stderr, "to be installed on your system.\n\n" );
	
		exit( EXIT_FAILURE );
#endif
	
	}

	Texture temptex( width, height );
	temptex.setName( texfilename );
	temptex.setFile( texfilename );
	texlist.AddElement( temptex );

	return &texlist[ texlist.getNumElements() - 1 ];
}


// retrieve material corresponding to specified index -------------------------
//
int BRep::FetchMaterialState( Material& mat, int indx )
{
	QvElement *elt = m_state->getTopElement( QvState::MaterialIndex );

	if ( elt == NULL )
		return 0;

	// fetch material node
	QvMaterial *m = (QvMaterial *) elt->data;

	// determine period for binding
	int ambnum = m->ambientColor.num;
	int difnum = m->diffuseColor.num;
	int spcnum = m->specularColor.num;
	int eminum = m->emissiveColor.num;
	int shinum = m->shininess.num;
	int tranum = m->transparency.num;
	int period = ambnum;
	if ( difnum > period ) period = difnum;
	if ( spcnum > period ) period = spcnum;
	if ( eminum > period ) period = eminum;
	if ( shinum > period ) period = shinum;
	if ( tranum > period ) period = tranum;

	ColorRGBA col;
	col.A = 255;

	int modindx  = indx % period;
	int readindx = ( modindx < ambnum ) ? modindx : ambnum - 1;
	col.R = (byte)( m->ambientColor.values[ readindx * 3 + 0 ] * 255 );
	col.G = (byte)( m->ambientColor.values[ readindx * 3 + 1 ] * 255 );
	col.B = (byte)( m->ambientColor.values[ readindx * 3 + 2 ] * 255 );
	mat.setAmbientColor( col );

	readindx = ( modindx < difnum ) ? modindx : difnum - 1;
	col.R = (byte)( m->diffuseColor.values[ readindx * 3 + 0 ] * 255 );
	col.G = (byte)( m->diffuseColor.values[ readindx * 3 + 1 ] * 255 );
	col.B = (byte)( m->diffuseColor.values[ readindx * 3 + 2 ] * 255 );
	mat.setDiffuseColor( col );

	readindx = ( modindx < spcnum ) ? modindx : spcnum - 1;
	col.R = (byte)( m->specularColor.values[ readindx * 3 + 0 ] * 255 );
	col.G = (byte)( m->specularColor.values[ readindx * 3 + 1 ] * 255 );
	col.B = (byte)( m->specularColor.values[ readindx * 3 + 2 ] * 255 );
	mat.setSpecularColor( col );

	readindx = ( modindx < eminum ) ? modindx : eminum - 1;
	col.R = (byte)( m->emissiveColor.values[ readindx * 3 + 0 ] * 255 );
	col.G = (byte)( m->emissiveColor.values[ readindx * 3 + 1 ] * 255 );
	col.B = (byte)( m->emissiveColor.values[ readindx * 3 + 2 ] * 255 );
	mat.setEmissiveColor( col );

	readindx = ( modindx < shinum ) ? modindx : shinum - 1;
	mat.setShininess( m->shininess.values[ readindx ] );

	readindx = ( modindx < tranum ) ? modindx : tranum - 1;
	mat.setTransparency( m->transparency.values[ readindx ] );

	return 1;
}


// fetch material binding -----------------------------------------------------
//
void BRep::FetchMaterialBindingState( int& binding )
{
	binding = QvMaterialBinding::DEFAULT;
	QvElement *elt = m_state->getTopElement( QvState::MaterialBindingIndex );
	if ( elt != NULL ) {
		QvMaterialBinding *mb = (QvMaterialBinding *) elt->data;
		binding = mb->value.value;
	}
}


// fetch normal binding -------------------------------------------------------
//
void BRep::FetchNormalBindingState( int& binding )
{
	binding = QvMaterialBinding::DEFAULT;
	QvElement *elt = m_state->getTopElement( QvState::NormalBindingIndex );
	if ( elt != NULL ) {
		QvNormalBinding *mb = (QvNormalBinding *) elt->data;
		binding = mb->value.value;
	}
}


// fetch shape hints information into static members --------------------------
//
void BRep::FetchShapeHintsState()
{
	QvElement *elt = m_state->getTopElement( QvState::ShapeHintsIndex );
	if ( elt != NULL ) {
		QvShapeHints *sh = (QvShapeHints *) elt->data;
		shapehint_vertexOrdering = sh->vertexOrdering.value;
		shapehint_shapeType		 = sh->shapeType.value;
		shapehint_faceType		 = sh->faceType.value;
		shapehint_creaseAngle	 = sh->creaseAngle.value;
	}
}


// do b-rep construction post-processing --------------------------------------
//
void BRep::PostProcessObject()
{
	VertexChunk& vtxlist = m_baseobject->getVertexList();

	int numvtxs = vtxlist.getNumElements();
	if ( numvtxs > 0 ) {

		// apply transformation matrix
		// otherwise bounding box will not be correct!
		m_baseobject->ApplyTransformation();

		// determine object bounding box
		Vertex3 minvertex;
		Vertex3 maxvertex;
		m_baseobject->CalcBoundingBox( minvertex, maxvertex );

		// prepend bounding box to list of scene object bounding boxes
		m_state->vrmlfile_base->m_bboxlist = new BoundingBox( minvertex, maxvertex, m_state->vrmlfile_base->m_bboxlist );

		// set actual vertex correspondences (needs to be done after transformation!)
		FaceChunk& facelist = m_baseobject->getFaceList();
		for ( int i = 0; i < facelist.getNumElements(); i++ ) {
			if ( facelist[ i ].FaceTexMapped() ) {
				// retrieve fake vertex containing corresponding vertex indexes
				Vertex2 fake;
				fake = facelist[ i ].MapXY( 0 );
				// use vertex indexes to store actual correspondence coordinates
				facelist[ i ].MapXY( 0 ).InitFromVertex3( vtxlist[ (int) fake.getX() ] );
				facelist[ i ].MapXY( 1 ).InitFromVertex3( vtxlist[ (int) fake.getY() ] );
				facelist[ i ].MapXY( 2 ).InitFromVertex3( vtxlist[ (int) fake.getW() ] );
			}
		}
	}

	m_baseobject->CheckParsedData();
}


// construct b-rep from vrml sphere primitive ---------------------------------
//
void BRep::BuildFromSpherePrimitive( const QvSphere& node )
{
	// base object
	VertexChunk& vtxlist  = m_baseobject->getVertexList();
	FaceChunk&   facelist = m_baseobject->getFaceList();
	PolygonList& polylist = m_baseobject->getPolygonList();

	// set possible transformation as object's local transformation
	Transform3 trafo;
	FetchTransformationState( trafo );
	m_baseobject->setObjectTransformation( trafo );

	int slices4 = tessellation_slices;

	double radius	= node.radius.value;

	double phi = 0.0;
	double phiinc = 1.570796327 / (double) slices4; // PI/2

	// create circumference vertices
	Vector3 basevec( radius, 0.0, 0.0 );
	Vector3 scancircle( basevec );
	int i = 0;
	for (  i = 0; i < slices4; i++ ) {
		vtxlist.AddVertex( scancircle );
		phi += phiinc;
		double cosphi = cos( phi );
		double sinphi = sin( phi );
		double nextx  = cosphi * basevec.getX() + sinphi * basevec.getZ();
		double nextz  = sinphi * basevec.getX() + cosphi * basevec.getZ();
		scancircle.setX( nextx );
		scancircle.setZ( -nextz );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX(  vtxlist[ i ].getZ() );
		nv.setY(  vtxlist[ i ].getY() );
		nv.setZ( -vtxlist[ i ].getX() );
		vtxlist.AddVertex( nv );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX( -vtxlist[ i ].getX() );
		nv.setY(  vtxlist[ i ].getY() );
		nv.setZ( -vtxlist[ i ].getZ() );
		vtxlist.AddVertex( nv );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX( -vtxlist[ i ].getZ() );
		nv.setY(  vtxlist[ i ].getY() );
		nv.setZ(  vtxlist[ i ].getX() );
		vtxlist.AddVertex( nv );
	}

	// create northern hemisphere
	basevec = Vector3( radius, 0.0, 0.0 );
	phi     = 0.0;
	for ( i = 1; i < slices4; i++ ) {
		phi += phiinc;
		double cosphi = cos( phi );
		double sinphi = sin( phi );
		double nextx  = cosphi * basevec.getX() - sinphi * basevec.getY();
		double nexty  = sinphi * basevec.getX() + cosphi * basevec.getY();
		double rfac   = nextx / radius;
		for ( int j = 0; j < slices4 * 4; j++ ) {
			Vertex3 nv;
			nv.setX( vtxlist[ j ].getX() * rfac );
			nv.setY( nexty );
			nv.setZ( vtxlist[ j ].getZ() * rfac );
			vtxlist.AddVertex( nv );
		}
	}

	// create southern hemisphere
	basevec = Vector3( radius, 0.0, 0.0 );
	phi     = 0.0;
	for ( i = 1; i < slices4; i++ ) {
		phi += phiinc;
		double cosphi = cos( phi );
		double sinphi = sin( phi );
		double nextx  = cosphi * basevec.getX() - sinphi * basevec.getY();
		double nexty  = sinphi * basevec.getX() + cosphi * basevec.getY();
		double rfac   = nextx / radius;
		for ( int j = 0; j < slices4 * 4; j++ ) {
			Vertex3 nv;
			nv.setX( vtxlist[ j ].getX() * rfac );
			nv.setY( -nexty );
			nv.setZ( vtxlist[ j ].getZ() * rfac );
			vtxlist.AddVertex( nv );
		}
	}

	// create pole vertices
	int p1indx = vtxlist.AddVertex( Vertex3( 0.0, radius, 0.0 ) );
	int p2indx = vtxlist.AddVertex( Vertex3( 0.0, -radius, 0.0 ) );

	// create polygons for side
	int faceid = 0;
	int j = 0;
	for (  j = 0; j < slices4 - 1; j++ ) {
		for ( i = 0; i < slices4 * 4 - 1; i++ ) {
			// create side polygon
			Polygon *sidepoly = polylist.NewPolygon();
			sidepoly->setFaceId( faceid++ );
			sidepoly->AppendNewVIndx( i + j * slices4 * 4 );
			sidepoly->AppendNewVIndx( i + ( j + 1 ) * slices4 * 4 );
			sidepoly->AppendNewVIndx( i + ( j + 1 ) * slices4 * 4 + 1 );
			sidepoly = polylist.NewPolygon();
			sidepoly->setFaceId( faceid++ );
			sidepoly->AppendNewVIndx( i + ( j + 1 ) * slices4 * 4 + 1 );
			sidepoly->AppendNewVIndx( i + j * slices4 * 4 + 1 );
			sidepoly->AppendNewVIndx( i + j * slices4 * 4 );
		}

		// create last side polygon
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + j * slices4 * 4 );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + ( j + 1 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( ( j + 1 ) * slices4 * 4 );
		sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( ( j + 1 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( j * slices4 * 4 );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + j * slices4 * 4 );
	}

	for ( i = 0; i < slices4 * 4 - 1; i++ ) {
		// create side polygon
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( i );
		sidepoly->AppendNewVIndx( i + 1 );
		sidepoly->AppendNewVIndx( i + slices4 * slices4 * 4 + 1 );
		sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( i + slices4 * slices4 * 4 + 1 );
		sidepoly->AppendNewVIndx( i + slices4 * slices4 * 4 );
		sidepoly->AppendNewVIndx( i );
	}

	// create last side polygon
	Polygon *sidepoly = polylist.NewPolygon();
	sidepoly->setFaceId( faceid++ );
	sidepoly->AppendNewVIndx( slices4 * 4 - 1 );
	sidepoly->AppendNewVIndx( 0 );
	sidepoly->AppendNewVIndx( slices4 * slices4 * 4 );
	sidepoly = polylist.NewPolygon();
	sidepoly->setFaceId( faceid++ );
	sidepoly->AppendNewVIndx( slices4 * slices4 * 4 );
	sidepoly->AppendNewVIndx( slices4 * 4 - 1 + slices4 * slices4 * 4 );
	sidepoly->AppendNewVIndx( slices4 * 4 - 1 );


	for ( j = slices4; j < 2 * slices4 - 2; j++ ) {
		for ( i = 0; i < slices4 * 4 - 1; i++ ) {
			// create side polygon
			Polygon *sidepoly = polylist.NewPolygon();
			sidepoly->setFaceId( faceid++ );
			sidepoly->AppendNewVIndx( i + j * slices4 * 4 );
			sidepoly->AppendNewVIndx( i + j * slices4 * 4 + 1 );
			sidepoly->AppendNewVIndx( i + ( j + 1 ) * slices4 * 4 + 1 );
			sidepoly = polylist.NewPolygon();
			sidepoly->setFaceId( faceid++ );
			sidepoly->AppendNewVIndx( i + ( j + 1 ) * slices4 * 4 + 1 );
			sidepoly->AppendNewVIndx( i + ( j + 1 ) * slices4 * 4 );
			sidepoly->AppendNewVIndx( i + j * slices4 * 4 );
		}

		// create last side polygon
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + j * slices4 * 4 );
		sidepoly->AppendNewVIndx( j * slices4 * 4 );
		sidepoly->AppendNewVIndx( ( j + 1 ) * slices4 * 4 );
		sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( ( j + 1 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + ( j + 1 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + j * slices4 * 4 );
	}

	// create polygons for poles
	for ( i = 0; i < slices4 * 4 - 1; i++ ) {
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( i + ( slices4 - 1 ) * slices4 * 4 + 1 );
		sidepoly->AppendNewVIndx( i + ( slices4 - 1 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( p1indx );
	}
	{
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( ( slices4 - 1 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + ( slices4 - 1 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( p1indx );
	}
	for ( i = 0; i < slices4 * 4 - 1; i++ ) {
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( i + ( 2 * slices4 - 2 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( i + ( 2 * slices4 - 2 ) * slices4 * 4 + 1 );
		sidepoly->AppendNewVIndx( p2indx );
	}
	{
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( faceid++ );
		sidepoly->AppendNewVIndx( slices4 * 4 - 1 + ( 2 * slices4 - 2 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( ( 2 * slices4 - 2 ) * slices4 * 4 );
		sidepoly->AppendNewVIndx( p2indx );
	}

	// the sphere primitive ignores the current material binding!
	// therefore fetch only the first material
	Material mat;
	int matvalid = FetchMaterialState( mat, 0 );

	// create faces
	for ( i = 0; i < polylist.getNumElements(); i++ ) {
		Face tempface;
		if ( matvalid ) {
			if ( use_material_spec ) {
				// attach material specification
				tempface.setShadingType( Face::material_shad );
				tempface.AttachMaterial( new Material( mat ) );
			} else {
				// use gouraud_shad with diffuse color
				tempface.setShadingType( Face::gouraud_shad );
				tempface.setFaceColor( mat.getDiffuseColor() );
			}
		} else {
			// use no_shad with index 255 if no material in state
			tempface.setShadingType( Face::no_shad );
			tempface.setFaceColor( 255 );
		}

		tempface.setId( facelist.getNumElements() );
		facelist.AddElement( tempface );
	}

	// do post-processing after object has been built
	PostProcessObject();
}


// construct b-rep from vrml cone primitive -----------------------------------
//
void BRep::BuildFromConePrimitive( const QvCone& node )
{
	// base object
	VertexChunk& vtxlist  = m_baseobject->getVertexList();
	FaceChunk&   facelist = m_baseobject->getFaceList();
	PolygonList& polylist = m_baseobject->getPolygonList();

	// set possible transformation as object's local transformation
	Transform3 trafo;
	FetchTransformationState( trafo );
	m_baseobject->setObjectTransformation( trafo );

	int slices4 = tessellation_slices;

	double radius	= node.bottomRadius.value;
	double top_y	= node.height.value / 2;
	double bottom_y	= - node.height.value / 2;

	double phi = 0.0;
	double phiinc = 1.570796327 / (double) slices4; // PI/2

	// create bottom plate vertices
	Vector3 basevec( radius, bottom_y, 0.0 );
	Vector3 scancircle( basevec );
	vtxlist.AddVertex( Vertex3( 0.0, bottom_y, 0.0 ) );
	int i = 0;
	for (  i = 0; i < slices4; i++ ) {
		vtxlist.AddVertex( scancircle );
		phi += phiinc;
		double cosphi = cos( phi );
		double sinphi = sin( phi );
		double nextx  = cosphi * basevec.getX() + sinphi * basevec.getZ();
		double nextz  = sinphi * basevec.getX() + cosphi * basevec.getZ();
		scancircle.setX( nextx );
		scancircle.setZ( -nextz );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX(  vtxlist[ i + 1 ].getZ() );
		nv.setY(  vtxlist[ i + 1 ].getY() );
		nv.setZ( -vtxlist[ i + 1 ].getX() );
		vtxlist.AddVertex( nv );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX( -vtxlist[ i + 1 ].getX() );
		nv.setY(  vtxlist[ i + 1 ].getY() );
		nv.setZ( -vtxlist[ i + 1 ].getZ() );
		vtxlist.AddVertex( nv );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX( -vtxlist[ i + 1 ].getZ() );
		nv.setY(  vtxlist[ i + 1 ].getY() );
		nv.setZ(  vtxlist[ i + 1 ].getX() );
		vtxlist.AddVertex( nv );
	}

	// create top vertex
	vtxlist.AddVertex( Vertex3( 0.0, top_y, 0.0 ) );

	// create polygons
	for ( i = 0; i < slices4 * 4 - 1; i++ ) {
		// create bottom plate polygon
		Polygon *bottompoly = polylist.NewPolygon();
		bottompoly->setFaceId( 0 );
		bottompoly->AppendNewVIndx( i + 1 );
		bottompoly->AppendNewVIndx( i + 2 );
		bottompoly->AppendNewVIndx( 0 );
		// create side polygon
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( i + 1 );
		sidepoly->AppendNewVIndx( i + 1 );
		sidepoly->AppendNewVIndx( slices4 * 4 + 1 );
		sidepoly->AppendNewVIndx( i + 2 );
	}

	{
		// create last bottom plate polygon
		Polygon *bottompoly = polylist.NewPolygon();
		bottompoly->setFaceId( 0 );
		bottompoly->AppendNewVIndx( slices4 * 4 );
		bottompoly->AppendNewVIndx( 1 );
		bottompoly->AppendNewVIndx( 0 );
		// create last side polygon
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( slices4 * 4 );
		sidepoly->AppendNewVIndx( slices4 * 4 );
		sidepoly->AppendNewVIndx( slices4 * 4 + 1 );
		sidepoly->AppendNewVIndx( 1 );
	}

	// fetch the first two materials
	Material mat1, mat2;
	int mat1valid = FetchMaterialState( mat1, 0 );
	int mat2valid = FetchMaterialState( mat2, 1 );

	// fetch the current material binding
	int currentbinding;
	FetchMaterialBindingState( currentbinding );

	// apply material binding
	if ( currentbinding != QvMaterialBinding::PER_PART &&
		 currentbinding != QvMaterialBinding::PER_PART_INDEXED ) {
		mat2      = mat1;
		mat2valid = mat1valid;
	}

	// create faces
	for ( i = 0; i < slices4 * 4 + 1; i++ ) {

		//NOTE:
		// there are ( slices4 * 4 + 1 ) faces.
		// the bottom plate is a single face although it consists
		// of ( slices4 * 4 ) polygons.

		Face tempface;
		if ( mat1valid && mat2valid ) {
			if ( use_material_spec ) {
				// attach material specification
				tempface.setShadingType( Face::material_shad );
				if ( i == 0 )
					tempface.AttachMaterial( new Material( mat2 ) );
				else
					tempface.AttachMaterial( new Material( mat1 ) );
			} else {
				// use gouraud_shad with diffuse color
				tempface.setShadingType( Face::gouraud_shad );
				if ( i == 0 )
					tempface.setFaceColor( mat2.getDiffuseColor() );
				else
					tempface.setFaceColor( mat1.getDiffuseColor() );
			}
		} else {
			// use no_shad with index 255 if no material in state
			tempface.setShadingType( Face::no_shad );
			tempface.setFaceColor( 255 );
		}

		tempface.setId( facelist.getNumElements() );
		facelist.AddElement( tempface );
	}

	// do post-processing after object has been built
	PostProcessObject();
}


// construct b-rep from vrml cylinder primitive -------------------------------
//
void BRep::BuildFromCylinderPrimitive( const QvCylinder& node )
{
	// base object
	VertexChunk& vtxlist  = m_baseobject->getVertexList();
	FaceChunk&   facelist = m_baseobject->getFaceList();
	PolygonList& polylist = m_baseobject->getPolygonList();

	// set possible transformation as object's local transformation
	Transform3 trafo;
	FetchTransformationState( trafo );
	m_baseobject->setObjectTransformation( trafo );

	int slices4 = tessellation_slices;

	double radius	= node.radius.value;
	double top_y	= node.height.value / 2;
	double bottom_y	= - node.height.value / 2;

	double phi = 0.0;
	double phiinc = 1.570796327 / (double) slices4; // PI/2

	// create bottom plate vertices
	Vector3 basevec( radius, bottom_y, 0.0 );
	Vector3 scancircle( basevec );
	vtxlist.AddVertex( Vertex3( 0.0, bottom_y, 0.0 ) );
	int i = 0;
	for (  i = 0; i < slices4; i++ ) {
		vtxlist.AddVertex( scancircle );
		phi += phiinc;
		double cosphi = cos( phi );
		double sinphi = sin( phi );
		double nextx  = cosphi * basevec.getX() + sinphi * basevec.getZ();
		double nextz  = sinphi * basevec.getX() + cosphi * basevec.getZ();
		scancircle.setX( nextx );
		scancircle.setZ( -nextz );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX(  vtxlist[ i + 1 ].getZ() );
		nv.setY(  vtxlist[ i + 1 ].getY() );
		nv.setZ( -vtxlist[ i + 1 ].getX() );
		vtxlist.AddVertex( nv );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX( -vtxlist[ i + 1 ].getX() );
		nv.setY(  vtxlist[ i + 1 ].getY() );
		nv.setZ( -vtxlist[ i + 1 ].getZ() );
		vtxlist.AddVertex( nv );
	}
	for ( i = 0; i < slices4; i++ ) {
		Vertex3 nv;
		nv.setX( -vtxlist[ i + 1 ].getZ() );
		nv.setY(  vtxlist[ i + 1 ].getY() );
		nv.setZ(  vtxlist[ i + 1 ].getX() );
		vtxlist.AddVertex( nv );
	}

	// create top plate vertices
	vtxlist.AddVertex( Vertex3( 0.0, top_y, 0.0 ) );
	for ( i = 0; i < slices4 * 4; i++ ) {
		Vertex3 nv( vtxlist[ i + 1 ] );
		nv.setY( top_y );
		vtxlist.AddVertex( nv );
	}

	// create polygons
	for ( i = 0; i < slices4 * 4 - 1; i++ ) {
		// create bottom plate polygon
		Polygon *bottompoly = polylist.NewPolygon();
		bottompoly->setFaceId( 0 );
		bottompoly->AppendNewVIndx( i + 1 );
		bottompoly->AppendNewVIndx( i + 2 );
		bottompoly->AppendNewVIndx( 0 );
		// create top plate polygon
		Polygon *toppoly = polylist.NewPolygon();
		toppoly->setFaceId( 1 );
		toppoly->AppendNewVIndx( i + slices4 * 4 + 3 );
		toppoly->AppendNewVIndx( i + slices4 * 4 + 2 );
		toppoly->AppendNewVIndx( slices4 * 4 + 1 );
		// create side polygon
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( i + 2 );
		sidepoly->AppendNewVIndx( i + 1 );
		sidepoly->AppendNewVIndx( i + slices4 * 4 + 2 );
		sidepoly->AppendNewVIndx( i + slices4 * 4 + 3 );
		sidepoly->AppendNewVIndx( i + 2 );
	}

	{
		// create last bottom plate polygon
		Polygon *bottompoly = polylist.NewPolygon();
		bottompoly->setFaceId( 0 );
		bottompoly->AppendNewVIndx( slices4 * 4 );
		bottompoly->AppendNewVIndx( 1 );
		bottompoly->AppendNewVIndx( 0 );
		// create last top plate polygon
		Polygon *toppoly = polylist.NewPolygon();
		toppoly->setFaceId( 1 );
		toppoly->AppendNewVIndx( slices4 * 4 + 2 );
		toppoly->AppendNewVIndx( 2 * slices4 * 4 + 1 );
		toppoly->AppendNewVIndx( slices4 * 4 + 1 );
		// create last side polygon
		Polygon *sidepoly = polylist.NewPolygon();
		sidepoly->setFaceId( slices4 * 4 + 1 );
		sidepoly->AppendNewVIndx( slices4 * 4 );
		sidepoly->AppendNewVIndx( 2 * slices4 * 4 + 1 );
		sidepoly->AppendNewVIndx( slices4 * 4 + 2 );
		sidepoly->AppendNewVIndx( 1 );
	}

	// fetch the first three materials
	Material mat1, mat2, mat3;
	int mat1valid = FetchMaterialState( mat1, 0 );
	int mat2valid = FetchMaterialState( mat2, 1 );
	int mat3valid = FetchMaterialState( mat3, 2 );

	// fetch the current material binding
	int currentbinding;
	FetchMaterialBindingState( currentbinding );

	// apply material binding
	if ( currentbinding != QvMaterialBinding::PER_PART &&
		 currentbinding != QvMaterialBinding::PER_PART_INDEXED ) {
		mat2      = mat1;
		mat3      = mat1;
		mat2valid = mat1valid;
		mat3valid = mat1valid;
	}

	// create faces
	for ( i = 0; i < slices4 * 4 + 2; i++ ) {
		Face tempface;

		//NOTE:
		// there are ( slices4 * 4 + 2 ) faces.
		// the bottom and top plates are each a single face although
		// they each consist of ( slices4 * 4 ) polygons.

		if ( mat1valid && mat2valid && mat3valid ) {
			if ( use_material_spec ) {
				// attach material specification
				tempface.setShadingType( Face::material_shad );
				if ( i == 0 )
					tempface.AttachMaterial( new Material( mat3 ) );
				else if ( i == 1 )
					tempface.AttachMaterial( new Material( mat2 ) );
				else
					tempface.AttachMaterial( new Material( mat1 ) );
			} else {
				// use gouraud_shad with diffuse color
				tempface.setShadingType( Face::gouraud_shad );
				if ( i == 0 )
					tempface.setFaceColor( mat3.getDiffuseColor() );
				else if ( i == 1 )
					tempface.setFaceColor( mat2.getDiffuseColor() );
				else
					tempface.setFaceColor( mat1.getDiffuseColor() );
			}
		} else {
			// use no_shad with index 255 if no material in state
			tempface.setShadingType( Face::no_shad );
			tempface.setFaceColor( 255 );
		}

		tempface.setId( facelist.getNumElements() );
		facelist.AddElement( tempface );
	}

	// do post-processing after object has been built
	PostProcessObject();
}


// construct b-rep from vrml cube primitive -----------------------------------
//
void BRep::BuildFromCubePrimitive( const QvCube& node )
{
	// base object
	VertexChunk& vtxlist  = m_baseobject->getVertexList();
	FaceChunk&   facelist = m_baseobject->getFaceList();
	PolygonList& polylist = m_baseobject->getPolygonList();

	// set possible transformation as object's local transformation
	Transform3 trafo;
	FetchTransformationState( trafo );
	m_baseobject->setObjectTransformation( trafo );

	double max_x = node.width.value / 2;
	double max_y = node.height.value / 2;
	double max_z = node.depth.value / 2;

	vtxlist.AddVertex( Vertex3( -max_x,  max_y,  max_z ) );
	vtxlist.AddVertex( Vertex3(  max_x,  max_y,  max_z ) );
	vtxlist.AddVertex( Vertex3(  max_x, -max_y,  max_z ) );
	vtxlist.AddVertex( Vertex3( -max_x, -max_y,  max_z ) );

	vtxlist.AddVertex( Vertex3( -max_x,  max_y, -max_z ) );
	vtxlist.AddVertex( Vertex3( -max_x, -max_y, -max_z ) );
	vtxlist.AddVertex( Vertex3(  max_x, -max_y, -max_z ) );
	vtxlist.AddVertex( Vertex3(  max_x,  max_y, -max_z ) );

	Polygon *sidepoly = polylist.NewPolygon();
	sidepoly->AppendNewVIndx( 0 ); sidepoly->AppendNewVIndx( 1 ); sidepoly->AppendNewVIndx( 2 ); sidepoly->AppendNewVIndx( 3 );
	sidepoly = polylist.NewPolygon();
	sidepoly->AppendNewVIndx( 1 ); sidepoly->AppendNewVIndx( 7 ); sidepoly->AppendNewVIndx( 6 ); sidepoly->AppendNewVIndx( 2 );
	sidepoly = polylist.NewPolygon();
	sidepoly->AppendNewVIndx( 7 ); sidepoly->AppendNewVIndx( 4 ); sidepoly->AppendNewVIndx( 5 ); sidepoly->AppendNewVIndx( 6 );
	sidepoly = polylist.NewPolygon();
	sidepoly->AppendNewVIndx( 4 ); sidepoly->AppendNewVIndx( 0 ); sidepoly->AppendNewVIndx( 3 ); sidepoly->AppendNewVIndx( 5 );
	sidepoly = polylist.NewPolygon();
	sidepoly->AppendNewVIndx( 3 ); sidepoly->AppendNewVIndx( 2 ); sidepoly->AppendNewVIndx( 6 ); sidepoly->AppendNewVIndx( 5 );
	sidepoly = polylist.NewPolygon();
	sidepoly->AppendNewVIndx( 1 ); sidepoly->AppendNewVIndx( 0 ); sidepoly->AppendNewVIndx( 4 ); sidepoly->AppendNewVIndx( 7 );

	//TODO:
	// polygons have to be created in the order prescribed by
	// material application!

	// fetch the first six materials
	Material mats[ 6 ];
	int matsvalid[ 6 ];
	int i = 0;
	for (  i = 0; i < 6; i++ )
		matsvalid[ i ] = FetchMaterialState( mats[ i ], 0 );

	// fetch the current material binding
	int currentbinding;
	FetchMaterialBindingState( currentbinding );

	// apply material binding
	if ( currentbinding != QvMaterialBinding::PER_PART			&&
		 currentbinding != QvMaterialBinding::PER_PART_INDEXED	&&
		 currentbinding != QvMaterialBinding::PER_FACE			&&
		 currentbinding != QvMaterialBinding::PER_FACE_INDEXED )
		for ( i = 1; i < 6; i++ ) {
			mats[ i ]      = mats[ 0 ];
			matsvalid[ i ] = matsvalid[ 0 ];
		}

	// check validity of all materials
	int allmatsvalid = 1;
	for ( i = 0; i < 6; i++ )
		if ( !matsvalid[ i ] )
			allmatsvalid = 0;

	// create faces
	for ( i = 0; i < 6; i++ ) {
		Face tempface;

		if ( allmatsvalid ) {
			if ( use_material_spec ) {
				// attach material specification
				tempface.setShadingType( Face::material_shad );
				tempface.AttachMaterial( new Material( mats[ i ] ) );
			} else {
				// use gouraud_shad with diffuse color
				tempface.setShadingType( Face::gouraud_shad );
				tempface.setFaceColor( mats[ i ].getDiffuseColor() );
			}
		} else {
			// use no_shad with index 255 if no material in state
			tempface.setShadingType( Face::no_shad );
			tempface.setFaceColor( 255 );
		}

		tempface.setId( facelist.getNumElements() );
		facelist.AddElement( tempface );
	}

	// do post-processing after object has been built
	PostProcessObject();
}


// create single face for specification in indexed face set -------------------
//
void BRep::CreateIndexedFace( Texture *texture, int curindex, int numtexindexs, long *texindexs, int v1, int v2, int v3 )
{
	// base object
	VertexChunk& vtxlist  = m_baseobject->getVertexList();
	FaceChunk&   facelist = m_baseobject->getFaceList();

	// face to be created
	Face tempface;

	// fetch material
	Material mat;
	int matvalid = FetchMaterialState( mat, 0 );

	if ( matvalid ) {
		if ( use_material_spec ) {
			// attach material specification
			tempface.setShadingType( Face::material_shad );
			tempface.AttachMaterial( new Material( mat ) );
		} else {
			// use gouraud_shad with diffuse color
			tempface.setShadingType( Face::gouraud_shad );
			tempface.setFaceColor( mat.getDiffuseColor() );
		}
	} else {
		// use no_shad with index 255 if no material in state
		tempface.setShadingType( Face::no_shad );
		tempface.setFaceColor( 255 );
	}

	// check if face is textured
	if ( texture != NULL ) {
		int coordsnum;
		float *texcoords = FetchTextureCoordinate2State( coordsnum );

		// fetch (u,v) transformation
		Transform2 trafo;
		FetchTextureTransformationState( trafo );

		// store fake vertex containing corresponding vertex indexes
		Vertex2 fake( v1, v2, v3 );
		tempface.MapXY( 0 ) = fake;

		int mappingvalid = 0;

		int coordindx = ( curindex - 3  < numtexindexs ) ? texindexs[ curindex - 3 ] : -1;
		if ( ( coordindx >= 0 ) && ( coordindx < coordsnum ) ) {
			tempface.MapUV( 0 ).setX( texcoords[ coordindx * 2 + 0 ] );
			tempface.MapUV( 0 ).setY( texcoords[ coordindx * 2 + 1 ] );
			tempface.MapUV( 0 ).setW( 1.0 );
			Vertex2 tv = trafo.TransformVector2( tempface.MapUV( 0 ) );
			tv.setX( tv.getX() * texture->getWidth() );
			double ucoord = mirror_v_axis ? ( 1.0 - tv.getY() ) : tv.getY();
			tv.setY( ucoord * texture->getHeight() );
			tempface.MapUV( 0 ) = tv;
			mappingvalid++;
		}

		coordindx = ( curindex - 2  < numtexindexs ) ? texindexs[ curindex - 2 ] : -1;
		if ( ( coordindx >= 0 ) && ( coordindx < coordsnum ) ) {
			tempface.MapUV( 1 ).setX( texcoords[ coordindx * 2 + 0 ] );
			tempface.MapUV( 1 ).setY( texcoords[ coordindx * 2 + 1 ] );
			tempface.MapUV( 1 ).setW( 1.0 );
			Vertex2 tv = trafo.TransformVector2( tempface.MapUV( 1 ) );
			tv.setX( tv.getX() * texture->getWidth() );
			double ucoord = mirror_v_axis ? ( 1.0 - tv.getY() ) : tv.getY();
			tv.setY( ucoord * texture->getHeight() );
			tempface.MapUV( 1 ) = tv;
			mappingvalid++;
		}

		coordindx = ( curindex - 1  < numtexindexs ) ? texindexs[ curindex - 1 ] : -1;
		if ( ( coordindx >= 0 ) && ( coordindx < coordsnum ) ) {
			tempface.MapUV( 2 ).setX( texcoords[ coordindx * 2 + 0 ] );
			tempface.MapUV( 2 ).setY( texcoords[ coordindx * 2 + 1 ] );
			tempface.MapUV( 2 ).setW( 1.0 );
			Vertex2 tv = trafo.TransformVector2( tempface.MapUV( 2 ) );
			tv.setX( tv.getX() * texture->getWidth() );
			double ucoord = mirror_v_axis ? ( 1.0 - tv.getY() ) : tv.getY();
			tv.setY( ucoord * texture->getHeight() );
			tempface.MapUV( 2 ) = tv;
			mappingvalid++;
		}

		// only set shading type to textured if all three
		// mapping coordinates valid
		if ( mappingvalid == 3 ) {
			tempface.setShadingType( Face::ipol1tex_shad );
			tempface.setTextureName( texture->getName() );
		}
	}

	tempface.setId( facelist.getNumElements() );
	facelist.AddElement( tempface );
}


// construct b-rep from vrml indexed face set ---------------------------------
//
void BRep::BuildFromIndexedFaceSet( const QvIndexedFaceSet& faceset )
{
	// check current shape hints
	FetchShapeHintsState();

	// check for texture mapping
	Texture *texture = CheckTexture2State();

	// determine number and address of vertices in current state
	int numvertices;
	float *vertices = FetchCoordinate3State( numvertices );

	// base object
	VertexChunk& vtxlist  = m_baseobject->getVertexList();
	FaceChunk&   facelist = m_baseobject->getFaceList();
	PolygonList& polylist = m_baseobject->getPolygonList();

	// set possible transformation as object's local transformation
	Transform3 trafo;
	FetchTransformationState( trafo );
	m_baseobject->setObjectTransformation( trafo );

	// create vertices (regardless of usage)
	for ( int i = 0; i < numvertices; i++ ) {
		double x = vertices[ i * 3 + 0 ];
		double y = vertices[ i * 3 + 1 ];
		double z = vertices[ i * 3 + 2 ];
		vtxlist.AddVertex( Vertex3( x, y, z ) );
	}

	//TODO:
	// all vertices are included in every object, regardless of usage!

	// fetch the current material binding
	int currentbinding;
	FetchMaterialBindingState( currentbinding );

	// determine number and address of vertex indexes
	int numvtxindexs = faceset.coordIndex.num;
	long *vtxindexs  = faceset.coordIndex.values;

	// determine number and address of material indexes
	int nummatindexs = faceset.materialIndex.num;
	long *matindexs  = faceset.materialIndex.values;

	// determine number and address of normal indexes
	int numnmlindexs = faceset.normalIndex.num;
	long *nmlindexs  = faceset.normalIndex.values;

	// determine number and address of texture coordinate indexes
	int numtexindexs = faceset.textureCoordIndex.num;
	long *texindexs  = faceset.textureCoordIndex.values;

	// create polygons and faces
	int   vtxsread  = 0;
	int   polysread = 0; // this also counts polygons removed due to degeneracy!
	int   v1, v2, v3;
	Plane curplane;
	while ( numvtxindexs > 0 ) {

		// prepend new polygon to list
		Polygon *poly = polylist.NewPolygon();

		// create vertexlist for this polygon
		int vtxcount = 0;
		while ( numvtxindexs-- > 0 ) {
			// read next vertex index
			int vindx = *vtxindexs++;
			vtxsread++;
			vtxcount++;
			// -1 is end marker for face
			if ( vindx == -1 )
				break;
			if ( vindx >= numvertices ) {
				{
				StrScratch message;
				sprintf( message, "**ERROR** [Specified vertex-index invalid]: %d\n", vindx );
				ErrorMessage( message );
				}
				HandleCriticalError();
			}

			if ( vtxcount == 1 ) {
				// store first vertex index
				v1 = vindx;
			} else if ( vtxcount == 2 ) {
				// store second vertex index
				v2 = vindx;
			} else if ( vtxcount == 3 ) {
				// store third vertex index
				v3 = vindx;
				// create plane for polygon (sidedness irrelevant!)
				curplane.InitPlane( vtxlist[ v1 ], vtxlist[ v2 ], vtxlist[ v3 ] );
				// count original polygons (including degenerate ones!)
				polysread++;
				// check if plane valid
				if ( !curplane.PlaneValid() ) {
					// remove polygon if invalid
					polylist.DeleteHead();
					// skip possibly remaining vertexindexes
					while ( ( numvtxindexs-- > 0 ) && ( *vtxindexs++ != -1 ) )
						vtxsread++;
					vtxindexs--;
					numvtxindexs++;
					continue;
				}
				// create face
				CreateIndexedFace( texture, vtxsread, numtexindexs, texindexs, v1, v2, v3 );
//			} else if ( vtxcount == 4 ) {
			} else {
				// create new polygon and face if next point not contained in
				// previous triangle's plane or triangulation explicitly desired
				if ( !curplane.PointContained( vtxlist[ vindx ] ) || do_triangulation ) {
					// add new polygon to list
					poly = polylist.NewPolygon();
					if ( shapehint_vertexOrdering == QvShapeHints::CLOCKWISE ) {
						polylist.AppendNewVIndx( v1 );
						polylist.AppendNewVIndx( v3 );
					} else {
						polylist.PrependNewVIndx( v1 );
						polylist.PrependNewVIndx( v3 );
					}
					// create plane for polygon
					curplane.InitPlane( vtxlist[ v1 ], vtxlist[ v3 ], vtxlist[ vindx ] );
					// check if plane valid
					if ( !curplane.PlaneValid() ) {
						// remove polygon if invalid
						polylist.DeleteHead();
						// skip possibly remaining vertexindexes
						while ( ( numvtxindexs-- > 0 ) && ( *vtxindexs++ != -1 ) )
							vtxsread++;
						vtxindexs--;
						numvtxindexs++;
						continue;
					}
					// create face
					CreateIndexedFace( texture, vtxsread, numtexindexs, texindexs, v1, v2, v3 );
				}
				// advance triangle fan
				v3 = vindx;
			}

			// insert current vertex index into polygon
			if ( shapehint_vertexOrdering == QvShapeHints::CLOCKWISE )
				polylist.AppendNewVIndx( vindx );
			else
				polylist.PrependNewVIndx( vindx );
		}

		if ( polylist.FetchHead() != NULL ) {

			//NOTE:
			// if all polygons up to now have been removed due to degeneracy,
			// polylist might contain not a single element! therefore,
			// getNumVertices() cannot be used!

			// check if polygon is at least triangle
			if ( polylist.getNumVertices() < 3 ) {
				{
				StrScratch message;
				sprintf( message, "**ERROR** [Face must have at least 3 vertices]: %d\n",
						 polylist.getNumElements() );
				ErrorMessage( message );
				}
				HandleCriticalError();
			}

		} else {
			StrScratch message;
			sprintf( message, "**ERROR** [Polygon is degenerate]: %d\n", polysread );
			ErrorMessage( message );
		}
	}

	// do post-processing after object has been built
	PostProcessObject();
}


// shape hints ----------------------------------------------------------------
//
int		BRep::shapehint_vertexOrdering	= QvShapeHints::UNKNOWN_ORDERING;
int		BRep::shapehint_shapeType		= QvShapeHints::UNKNOWN_SHAPE_TYPE;
int		BRep::shapehint_faceType		= QvShapeHints::CONVEX;
float	BRep::shapehint_creaseAngle		= 0.5f;


// full material specification usage control ----------------------------------
//
int		BRep::use_material_spec			= FALSE;


// flag if v axis should be mirrored for texture coordinates ------------------
//
int		BRep::mirror_v_axis				= TRUE;


// triangulation control ------------------------------------------------------
//
int		BRep::do_triangulation			= FALSE;


// tessellation resolution in slices per PI/2 ---------------------------------
//
int		BRep::tessellation_slices		= 4;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
