//-----------------------------------------------------------------------------
//	BSPLIB HEADER: InputData3D.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _INPUTDATA3D_H_
#define _INPUTDATA3D_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObjectList.h"
#include "IOData3D.h"


BSPLIB_NAMESPACE_BEGIN


// generic input class for 3-D data -------------------------------------------
//
class InputData3D : public IOData3D {

public:
	// flags for automatic postprocessing of input data
	enum {
		CONVERT_COLINDXS_TO_RGB	= 0x0001,
		FILTER_SCALE_FACTORS	= 0x0002,
		FILTER_AXES_DIR_SWITCH	= 0x0004,
		FILTER_AXES_EXCHANGE	= 0x0008,
		DO_AXES_EXCHANGE		= FILTER_AXES_DIR_SWITCH | FILTER_AXES_EXCHANGE,
		FORCE_MAXIMUM_EXTENT	= 0x0010,
		ALLOW_N_GONS			= 0x0020,
	};

public:
	InputData3D( BspObjectList objectlist, const char *filename, int format = UNKNOWN_FORMAT );
	virtual ~InputData3D();

	virtual int		ParseObjectData();									// parse data file

	int				getObjectFormat() const { return m_objectformat; }	// get format of "real" object
	InputData3D*	getRealObject() const { return m_data; }			// get pointer to "real" object

	int				InputDataValid() const { return m_inputok; }		// input data parsed ok?

	// set/reset flags for automatic postprocessing of input data
	static int		EnableRGBConversion( int enable );
	static int		EnableScaleFactors( int enable );
	static int		EnableAxesChange( int enable );
	static int		EnableMaximumExtent( int enable, double extent );
	static int		EnableAllowNGons( int enable );

	// get state of flags for automatic postprocessing of input data
	static int		getRGBConversionFlag() { return ( ( PostProcessingFlags & CONVERT_COLINDXS_TO_RGB ) == CONVERT_COLINDXS_TO_RGB ); }
	static int		getScaleFactorsFlag() { return ( ( PostProcessingFlags & FILTER_SCALE_FACTORS ) == FILTER_SCALE_FACTORS ); }
	static int		getAxesChangeFlag() { return ( ( PostProcessingFlags & DO_AXES_EXCHANGE ) == DO_AXES_EXCHANGE ); }
	static int		getEnforceExtentsFlag() { return ( ( PostProcessingFlags & FORCE_MAXIMUM_EXTENT ) == FORCE_MAXIMUM_EXTENT ); }
	static double	getMaxExtents() { return MaximumExtentToForce; }
	static int		getAllowNGonFlag() { return ( ( PostProcessingFlags & ALLOW_N_GONS ) == ALLOW_N_GONS ); }

private:
	int				ReadFileSignature();

protected:
	static dword	PostProcessingFlags;
	static double	MaximumExtentToForce;
	static const char parser_err_str[];

protected:
	int				m_objectformat;
	int				m_inputok;

private:
	InputData3D*	m_data;
};


BSPLIB_NAMESPACE_END


#endif // _INPUTDATA3D_H_

