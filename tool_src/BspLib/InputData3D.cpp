//-----------------------------------------------------------------------------
//	BSPLIB MODULE: InputData3D.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "InputData3D.h"
#include "AodInput.h"
#include "BspInput.h"
#include "VrmlFile.h"
//ADD_FORMAT:


BSPLIB_NAMESPACE_BEGIN


// "virtual" constructor for different input format objects -------------------
//
InputData3D::InputData3D( BspObjectList objectlist, const char *filename, int format ) :
	IOData3D( objectlist, String( filename ) ),
	m_data( NULL )
{
	// create "real" input object according to format of input file
	if ( format != DONT_CREATE_OBJECT ) {
		switch ( m_objectformat = ReadFileSignature() ) {

		case AOD_FORMAT_1_1:
			m_data = new AodInput( objectlist, filename );
			break;

		case BSP_FORMAT_1_1:
			m_data = new BspInput( objectlist, filename );
			break;

		case VRML_FORMAT_1_0:
			m_data = new VrmlFile( objectlist, filename );
			break;

		case _3DX_FORMAT_1_0:
			//m_data = new ThreeDXInput( objectlist, filename );
			//TODO: add 3dx support
			break;

		//ADD_FORMAT:

		default:
			ErrorMessage( "[InputData3D]: Unrecognized input file format." );
			m_objectformat = UNKNOWN_FORMAT;
		}
	} else {
		m_objectformat = format;
	}

	m_inputok = m_data ? m_data->InputDataValid() : FALSE;
}


// virtual destructor ---------------------------------------------------------
//
InputData3D::~InputData3D()
{
	// delete "real" object
	delete m_data;
}


// redirection to ParseObjectData() of "real" object --------------------------
//
int InputData3D::ParseObjectData()
{
	return m_data ? m_data->ParseObjectData() : FALSE;
}


// determine file type according to signature in first line -------------------
//
int InputData3D::ReadFileSignature()
{
	FileAccess sig( m_filename, "r" );
	sig.ReadLine( line, TEXTLINE_MAX, CHECK_ERRORS );

	int filetype = UNKNOWN_FORMAT;

	if ( strncmp( line, AOD_SIGNATURE_1_1, strlen( AOD_SIGNATURE_1_1 ) ) == 0 )
		filetype = AOD_FORMAT_1_1;
	else if ( strncmp( line, BSP_SIGNATURE_1_1, strlen( BSP_SIGNATURE_1_1 ) ) == 0 )
		filetype = BSP_FORMAT_1_1;
	else if ( strncmp( line, VRML_SIGNATURE_1_0, strlen( VRML_SIGNATURE_1_0 ) ) == 0 )
		filetype = VRML_FORMAT_1_0;
	else if ( strncmp( line, _3DX_SIGNATURE_1_0, strlen( _3DX_SIGNATURE_1_0 ) ) == 0 )
		filetype = _3DX_FORMAT_1_0;
	//ADD_FORMAT:

	return filetype;
}


// set flag controlling color index to RGB conversion -------------------------
//
int InputData3D::EnableRGBConversion( int enable )
{
	int precstate = getRGBConversionFlag();
	if ( enable )
		PostProcessingFlags |= CONVERT_COLINDXS_TO_RGB;
	else
		PostProcessingFlags &= ~CONVERT_COLINDXS_TO_RGB;
	return precstate;
}


// set flag controlling application of object scale factors -------------------
//
int InputData3D::EnableScaleFactors( int enable )
{
	int precstate = getScaleFactorsFlag();
	if ( enable )
		PostProcessingFlags |= FILTER_SCALE_FACTORS;
	else
		PostProcessingFlags &= ~FILTER_SCALE_FACTORS;
	return precstate;
}


// set flag controlling change of axes per command ----------------------------
//
int InputData3D::EnableAxesChange( int enable )
{
	int precstate = getAxesChangeFlag();
	if ( enable )
		PostProcessingFlags |= DO_AXES_EXCHANGE;
	else
		PostProcessingFlags &= ~DO_AXES_EXCHANGE;
	return precstate;
}


// set flag controlling enforcement of maximum coordinate extent --------------
//
int InputData3D::EnableMaximumExtent( int enable, double extent )
{
	int precstate		 = getEnforceExtentsFlag();
	MaximumExtentToForce = extent;
	if ( enable )
		PostProcessingFlags |= FORCE_MAXIMUM_EXTENT;
	else
		PostProcessingFlags &= ~FORCE_MAXIMUM_EXTENT;
	return precstate;
}


// set flag controlling n-gon enabling/disabling -------------------------------
//
int InputData3D::EnableAllowNGons( int enable )
{
	int precstate = getAllowNGonFlag();
	if ( enable )
		PostProcessingFlags |= ALLOW_N_GONS;
	else
		PostProcessingFlags &= ~ALLOW_N_GONS;
	return precstate;
}


// InputData3D specific static variables --------------------------------------
//
dword		InputData3D::PostProcessingFlags		= InputData3D::FILTER_SCALE_FACTORS | InputData3D::DO_AXES_EXCHANGE;
double		InputData3D::MaximumExtentToForce		= 100.0;
const char	InputData3D::parser_err_str[]			= "\nObject parser: **ERROR** ";


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
