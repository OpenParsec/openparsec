//-----------------------------------------------------------------------------
//	BSPLIB MODULE: SingleInput.cpp
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "SingleInput.h"
#include "BspObject.h"
#include "BspTool.h"


BSPLIB_NAMESPACE_BEGIN


// file is read and parsed immediately after construction of an object --------
//
SingleInput::SingleInput( BspObjectList objectlist, const char *filename ) :
	InputData3D( objectlist, filename, DONT_CREATE_OBJECT ),
	m_baseobject( objectlist.CreateNewObject() ),
	m_input( filename, "r" )
{
	m_parser_lineno = 0;
	ParseObjectData();
}


// destructor destroys only class members and the base classes ----------------
//
SingleInput::~SingleInput()
{
}


// convert color indexes to rgb values ----------------------------------------
//
int SingleInput::ConvertColIndxs()
{
	// do colorindex to rgb conversion
	if ( m_palette_fname != NULL ) {
		char *palette = new char[ 256 * 3 ];
		FileAccess palfile( m_palette_fname, "rb", 0 );
		if ( palfile.Status() == SYSTEM_IO_OK ) {
			if ( palfile.Read( palette, 3, 256 ) != SYSTEM_IO_OK ) {
				if ( getRGBConversionFlag() )
					ErrorMessage( "Specified palette invalid or read error.\n" );
				delete palette;
				return FALSE;
			} else if ( m_baseobject->ConvertColorIndexesToRGB( palette, getRGBConversionFlag() ) ) {
				if ( getRGBConversionFlag() )
					InfoMessage( "Color indexes converted to RGB triplets.\n" );
				delete palette;
			}
		} else {
			if ( getRGBConversionFlag() )
				ErrorMessage( "Specified palette not found.\n" );
			delete palette;
			return FALSE;
		}
	} else {
		if ( getRGBConversionFlag() )
			ErrorMessage( "Conversion col-indexes to RGB triplets desired, but no palette set!\n" );
		return FALSE;
	}

	return TRUE;
}


// apply translation specified as origin --------------------------------------
//
void SingleInput::ApplyOriginTranslation()
{
	//CAVEAT:
	// this will corrupt explicit separator planes and bounding boxes.
	// thus, it must only be used for files where these two data types
	// don't occur!

	// scan all vertices and translate them
	VertexChunk& vtxlist = m_baseobject->getVertexList();
	for ( int i = 0; i < vtxlist.getNumElements(); i++ ) {
		vtxlist[ i ].setX( vtxlist[ i ].getX() - NewOriginX );
		vtxlist[ i ].setY( vtxlist[ i ].getY() - NewOriginY );
		vtxlist[ i ].setZ( vtxlist[ i ].getZ() - NewOriginZ );
	}
	NewOriginX = NewOriginY = NewOriginZ = 0.0;
}


// filter direction switches for axes -----------------------------------------
//
void SingleInput::FilterAxesDirSwitch()
{
	//CAVEAT:
	// this will corrupt explicit separator planes, bounding boxes,
	// and affine mappings. thus, it must only be used for files
	// where these three data types don't occur!

	if ( PostProcessingFlags & FILTER_AXES_DIR_SWITCH ) {

		//NOTE:
		// axis direction switches (negative object scale factors)
		// are only filtered here. that is, they don't make it into
		// the output file. they are only applied to the object if
		// FILTER_AXES_EXCHANGE is also specified!

		// filter direction switches of axes
		double xc = ( ObjScaleX < 0 ) ? -1.0 : 1.0;
		double yc = ( ObjScaleY < 0 ) ? -1.0 : 1.0;
		double zc = ( ObjScaleZ < 0 ) ? -1.0 : 1.0;

		AxesDirSwitch = Vertex3( xc, yc, zc );

		ObjScaleX *= xc;
		ObjScaleY *= yc;
		ObjScaleZ *= zc;
	}
}


// filter object scale factors and apply them ---------------------------------
//
void SingleInput::FilterScaleFactors()
{
	//CAVEAT:
	// this will corrupt explicit separator planes, bounding boxes,
	// and affine mappings. thus, it must only be used for files
	// where these three data types don't occur!

	if ( PostProcessingFlags & FILTER_SCALE_FACTORS ) {
		// scan all vertices and apply scale factors
		VertexChunk& vtxlist = m_baseobject->getVertexList();
		for ( int i = 0; i < vtxlist.getNumElements(); i++ ) {
			vtxlist[ i ].setX( vtxlist[ i ].getX() * ObjScaleX );
			vtxlist[ i ].setY( vtxlist[ i ].getY() * ObjScaleY );
			vtxlist[ i ].setZ( vtxlist[ i ].getZ() * ObjScaleZ );
		}
		// reset (filter) scale factors
		ObjScaleX = ObjScaleY = ObjScaleZ = 1.0;
	}
}


// exchange axes according to specification -----------------------------------
//
void SingleInput::FilterAxesExchange()
{
	//CAVEAT:
	// this will corrupt explicit separator planes, bounding boxes,
	// and affine mappings. thus, it must only be used for files
	// where these three data types don't occur!

	if ( PostProcessingFlags & FILTER_AXES_EXCHANGE ) {

		// exchange axes by applying axes exchange command to every
		// vertex. axis direction switches are also applied if they
		// have been filtered previously (FILTER_AXES_DIR_SWITCH).
		VertexChunk& vtxlist = m_baseobject->getVertexList();
		for ( int i = 0; i < vtxlist.getNumElements(); i++ )
			vtxlist[ i ].ChangeAxes( xchangecmd, AxesDirSwitch );

		// reset exchange command
		strcpy( xchangecmd, "xyz" );
	}
}


// enforce maximum extents for all coordinates --------------------------------
//
void SingleInput::EnforceMaximumExtents()
{
	if ( PostProcessingFlags & FORCE_MAXIMUM_EXTENT ) {
		// calculate scale factor for entire object
		double fmext = getMaxExtents();
		double mext  = m_maximumextents.getX();
		if ( m_maximumextents.getY() > mext ) mext = m_maximumextents.getY();
		if ( m_maximumextents.getZ() > mext ) mext = m_maximumextents.getZ();
		double cfac = fmext / ( 2 * mext );
		// scan all vertices and apply scale factor
		VertexChunk& vtxlist = m_baseobject->getVertexList();
		for ( int i = 0; i < vtxlist.getNumElements(); i++ ) {
			vtxlist[ i ].setX( vtxlist[ i ].getX() * cfac );
			vtxlist[ i ].setY( vtxlist[ i ].getY() * cfac );
			vtxlist[ i ].setZ( vtxlist[ i ].getZ() * cfac );
		}
		// apply scale factor to separator planes and
		// bounding boxes of bsp tree nodes
		m_baseobject->getBSPTreeFlat().ApplyScaleFactor( cfac );
	}
}


// print error message if error in object data-file ---------------------------
//
void SingleInput::ParseError( int section )
{
	sprintf( line, "%sin section %s (line %d)", parser_err_str,
			 GetSectionName( section ), m_parser_lineno );
	ErrorMessage( line );
	HandleCriticalError();
}


// read single integer parameter ----------------------------------------------
//
void SingleInput::ReadIntParameter( int& param, int lowbound )
{
	char *remptr;
	int fno = strtol( m_scanptr, &remptr, 10 );
	if ( ( *remptr != '\0' ) || ( fno < lowbound ) )
		ParseError( m_section );
	param = fno - lowbound;
}


// read single floating point parameter ---------------------------------------
//
void SingleInput::ReadFloatParameter( double& param )
{
	char *remptr;
	param = strtod( m_scanptr, &remptr );
	if ( *remptr != '\0' )
		ParseError( m_section );
}


// read two adjacent double values --------------------------------------------
//
void SingleInput::ReadTwoDoubles( double& x, double& y )
{
	ReadFloatParameter( x );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( y );
}


// read three adjacent double values ------------------------------------------
//
void SingleInput::ReadThreeDoubles( double& x, double& y, double& z )
{
	ReadFloatParameter( x );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( y );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( z );
}


// read four adjacent double values -------------------------------------------
//
void SingleInput::ReadFourDoubles( double& x, double& y, double& z, double& d )
{
	ReadFloatParameter( x );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( y );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( z );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( d );
}


// read six adjacent double values --------------------------------------------
//
void SingleInput::ReadSixDoubles( Vertex3& v1, Vertex3& v2 )
{
	double rdoub;

	ReadFloatParameter( rdoub );
	v1.setX( rdoub );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( rdoub );
	v1.setY( rdoub );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( rdoub );
	v1.setZ( rdoub );

	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( rdoub );
	v2.setX( rdoub );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( rdoub );
	v2.setY( rdoub );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFloatParameter( rdoub );
	v2.setZ( rdoub );
}


// read three vertices specified in x y z order -------------------------------
//
void SingleInput::ReadVertex()
{
	double x, y, z;
	ReadThreeDoubles( x, y, z );

	double absx = fabs( x );
	double absy = fabs( y );
	double absz = fabs( z );

	double mx = ( absx > m_maximumextents.getX() ) ? absx : m_maximumextents.getX();
	double my = ( absy > m_maximumextents.getY() ) ? absy : m_maximumextents.getY();
	double mz = ( absz > m_maximumextents.getZ() ) ? absz : m_maximumextents.getZ();

	m_maximumextents = Vertex3( mx, my, mz );
	m_baseobject->getVertexList().AddVertex( Vertex3( x, y, z ) );
}


// read face normal from file -------------------------------------------------
//
void SingleInput::ReadFaceNormal( int& normalsread )
{
	// read normal coordinates
	double x, y, z;
	ReadThreeDoubles( x, y, z );

	// build normal
	Vector3 normal( x, y, z );

	// insert face into facelist
	FaceChunk& facelist = m_baseobject->getFaceList();
	if ( facelist.getNumElements() > normalsread ) {
		// do nothing if normal already valid
		if ( !facelist[ normalsread ].NormalValid() ) {
			// attach normal to existing face
			facelist[ normalsread ].AttachNormal( normal );
		}
	} else {
		// create face containing only normal info
		Face tempface;
		tempface.AttachNormal( normal );
		tempface.setId( facelist.getNumElements() );
		facelist.AddElement( tempface );
	}
	normalsread++;
}


// read vertex indexes comprising a face --------------------------------------
//
void SingleInput::ReadFace( int aod_read )
{
	// add new polygon to list
	Polygon *poly = m_baseobject->getPolygonList().NewPolygon();

	// if not aod format, set face id to "invalid"
	if ( !aod_read )
		poly->setFaceId( -1 );

	// create vertexlist for this polygon
	while ( m_scanptr != NULL ) {
		if ( *m_scanptr == ';' )
			break;
		char *remptr;
		int fno = strtol( m_scanptr, &remptr, 10 );
		if ( ( *remptr != '\0') || ( fno < 1 ) )
			ParseError( m_section );

		// store vertex index (converted to 0-base)
		m_baseobject->getPolygonList().AppendNewVIndx( fno - 1 );
		m_scanptr = strtok( NULL, ",/ \t\n\r" );
	}

	// check if face is either triangle or quadrilateral
	if ( aod_read && !getAllowNGonFlag() ) {
		int vnum = m_baseobject->getPolygonList().getNumVertices();
		if ( ( vnum < 3 ) || ( vnum > 4 ) ) {
			sprintf( line, "%s[Face must be triangle or quadrilateral]: %d",
					 parser_err_str, m_baseobject->getPolygonList().getNumElements() );
			ErrorMessage( line );
			HandleCriticalError();
		}
	}
}


// read RGBA quadruplet for material ------------------------------------------
//
void SingleInput::ReadMaterialRGBA( ColorRGBA &col, int indx )
{
	double c_r, c_g, c_b, c_a;
	if ( strcmp( m_scanptr, Face::material_strings[ indx ] ) != 0 )
		ParseError( m_section );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	ReadFourDoubles( c_r, c_g, c_b, c_a );
	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );

	col.R = (byte)( c_r * 255 );
	col.G = (byte)( c_g * 255 );
	col.B = (byte)( c_b * 255 );
	col.A = (byte)( c_a * 255 );
}


// read material properties of a face -----------------------------------------
//
void SingleInput::ReadFaceProperties( int& facepropsread )
{
	// face to create
	Face tempface;

	// convert shading type string to type id
	int shadingtypeid = tempface.GetTypeIndex( m_scanptr );
	if ( shadingtypeid < 0 ) {
		sprintf( line, "%s[Undefined face-property]: %s (line %d)",
				 parser_err_str, m_scanptr, m_parser_lineno );
		ErrorMessage( line );
		HandleCriticalError();
	}

	// set shading type for face
	tempface.setShadingType( shadingtypeid );

	// read color if necessary for shading type
	if ( shadingtypeid & Face::color_mask ) {
		if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
			ParseError( m_section );

		// determine how color is specified
		int readrgb = 0;
		if ( isalpha( *m_scanptr ) ) {
			int colormodel = tempface.GetColorModelIndex( m_scanptr );
			switch ( colormodel ) {
			case Face::rgb_col:
				readrgb = 1;
				break;
			case Face::indexed_col:
				readrgb = 0;
				break;
			case Face::material_col:
				readrgb = 2;
				break;
			default:
				ParseError( m_section );
			}

			if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
				ParseError( m_section );
		}

		// read color in specified (detected) model
		if ( readrgb == 1 ) {
			// read (r,g,b) triplet
			double col_r, col_g, col_b;
			ReadThreeDoubles( col_r, col_g, col_b );
			ColorRGBA rgba_col;
			rgba_col.R = (byte) ( col_r * 255 );
			rgba_col.G = (byte) ( col_g * 255 );
			rgba_col.B = (byte) ( col_b * 255 );
			rgba_col.A = 255;
			tempface.setFaceColor( rgba_col );
		} else if ( readrgb == 2 ) {
			// read material specification
			Material *mat = new Material;
			ColorRGBA colquad;
			ReadMaterialRGBA( colquad, 0 );
			mat->setAmbientColor( colquad );
			ReadMaterialRGBA( colquad, 1 );
			mat->setDiffuseColor( colquad );
			ReadMaterialRGBA( colquad, 2 );
			mat->setSpecularColor( colquad );
			ReadMaterialRGBA( colquad, 3 );
			mat->setEmissiveColor( colquad );
			if ( strcmp( m_scanptr, Face::material_strings[ 4 ] ) != 0 )
				ParseError( m_section );
			if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
				ParseError( m_section );
			double floatpara;
			ReadFloatParameter( floatpara );
			mat->setShininess( (float) floatpara );
			if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
				ParseError( m_section );
			if ( strcmp( m_scanptr, Face::material_strings[ 5 ] ) != 0 )
				ParseError( m_section );
			if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
				ParseError( m_section );
			ReadFloatParameter( floatpara );
			mat->setTransparency( (float) floatpara );
			// attach material
			tempface.AttachMaterial( mat );
		} else {
			// read color index
			char *remptr;
			int fno = strtol( m_scanptr, &remptr, 10 );
			if ( *remptr != '\0' )
				ParseError( m_section );
			tempface.setFaceColor( fno );
		}

	}

	// read texture name if necessary for shading type
	if ( shadingtypeid & Face::texmap_mask ) {
		while ( *m_scanptr++ != '\0' ) ;
		while ( ( *m_scanptr == ' ' ) || ( *m_scanptr == '\t' ) )
			m_scanptr++;
		if ( ( m_scanptr = strtok( m_scanptr, "\"" ) ) == NULL )
			ParseError( m_section );
		tempface.setTextureName( m_scanptr );
	}

	// insert face into facelist
	FaceChunk& facelist = m_baseobject->getFaceList();
	if ( facelist.getNumElements() > facepropsread ) {
		if ( facelist[ facepropsread ].NormalValid() ) {
			// take over normal if one has already been calculated
			Vector3 normal = facelist[ facepropsread ].getPlaneNormal();
			tempface.AttachNormal( normal );
		}
		tempface.setId( facelist[ facepropsread ].getId() );
		facelist[ facepropsread ] = tempface;
	} else {
		tempface.setId( facelist.getNumElements() );
		facelist.AddElement( tempface );
	}
	facepropsread++;
}


// read vertex correspondences specifying a mapping ---------------------------
//
void SingleInput::ReadCorrespondences()
{
	Mapping tempmapping;

	// read vertexindexes for this face mapping (one line!)
	while ( m_scanptr != NULL ) {
		if ( *m_scanptr == ';' )
			break;
		int vertexno;
		ReadIntParameter( vertexno, 1 );
		tempmapping.InsertFaceVertex( vertexno );
		m_scanptr = strtok( NULL, ",/ \t\n\r" );
	}

	// read correspondences for all vertexindexes
	// (each correspondence has to be specified on a separate line!)
	for ( int k = 0; k < tempmapping.getNumVertices(); k++ ) {
		if ( m_input.ReadLine( line, TEXTLINE_MAX ) == NULL )
			ParseError( m_section );

		++m_parser_lineno;
		if ( ( m_scanptr = strtok( line, ",/ \t\n\r" ) ) == NULL )
			continue;

		double u, v;
		ReadTwoDoubles( u, v );
		tempmapping.setMappingCoordinates( k, Vertex2( u, v ) );
	}

	m_baseobject->getMappingList().AddElement( tempmapping );
}


// read texture definitions ---------------------------------------------------
//
void SingleInput::ReadTextures()
{
	char *remptr;

	int txwidth = strtol( m_scanptr, &remptr, 10 );
	if ( *remptr != '\0' ) ParseError( m_section );

	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );

	int txheight = strtol( m_scanptr, &remptr, 10 );
	if ( *remptr != '\0' ) ParseError( m_section );

	Texture temptex( txwidth, txheight );

	while ( *m_scanptr++ != '\0' ) ;
	while ( ( *m_scanptr == ' ' ) || ( *m_scanptr == '\t' ) )
		m_scanptr++;
	if ( ( m_scanptr = strtok( m_scanptr, "\"" ) ) == NULL )
		ParseError( m_section );
	temptex.setName( m_scanptr );

	if ( ( m_scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
		ParseError( m_section );
	temptex.setFile( m_scanptr );

	m_baseobject->getTextureList().AddElement( temptex );
}


// read initial world location for viewer -------------------------------------
//
void SingleInput::ReadWorldLocation()
{
	if ( worldlocation_set++ )
		ParseError( m_section );
	int i = 0;
	while ( m_scanptr != NULL ) {
		if ( *m_scanptr == ';' )
			break;
		if ( i > 15 ) // read 16 matrix elements
			ParseError( m_section );
		double fval;
		ReadFloatParameter( fval );
		wm[ i++ ] = fval;
		m_scanptr = strtok( NULL, ",/ \t\n\r" );
	}
	if ( i != 16 )
		ParseError( m_section );
}


// read initial camera location for viewer ------------------------------------
//
void SingleInput::ReadCameraLocation()
{
	if ( camera_set++ )
		ParseError( m_section );
	int i = 0;
	while ( m_scanptr != NULL ) {
		if ( *m_scanptr == ';' )
			break;
		if ( i > 15 ) // read 16 matrix elements
			ParseError( m_section );
		double fval;
		ReadFloatParameter( fval );
		vm[ i++ ] = fval;
		m_scanptr = strtok( NULL, ",/ \t\n\r" );
	}
	if ( i != 16 )
		ParseError( m_section );
}


// read filename of palette data ----------------------------------------------
//
void SingleInput::ReadPaletteFilename()
{
	if ( m_palette_fname != NULL )
		ParseError( m_section );
	m_palette_fname = new char[ strlen( m_scanptr ) + 1 ];
	strcpy( m_palette_fname, m_scanptr );
}


// read object scale factors --------------------------------------------------
//
void SingleInput::ReadScaleFactors()
{
	if ( scalefactors_set++ )
		ParseError( m_section );
	ReadThreeDoubles( ObjScaleX, ObjScaleY, ObjScaleZ );
}


// read axes exchange command -------------------------------------------------
//
void SingleInput::ReadXChangeCommand()
{
	if ( xchange_set++ )
		ParseError( m_section );
	if ( strlen( m_scanptr ) > 3 )
		ParseError( m_section );
	strcpy( xchangecmd, m_scanptr );
}


// read origin translation ----------------------------------------------------
//
void SingleInput::ReadOrigin()
{
	if ( setorigin_set++ )
		ParseError( m_section );
	ReadThreeDoubles( NewOriginX, NewOriginY, NewOriginZ );
	NewOriginX += PREMOVEORIGIN_X;
	NewOriginY += PREMOVEORIGIN_Y;
	NewOriginZ += PREMOVEORIGIN_Z;
}


// every PARSER_DOT_SIZE + 1 parsed lines a dot is printed (bitmask!!) --------
//
const int SingleInput::PARSER_DOT_SIZE = 0x1f;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
