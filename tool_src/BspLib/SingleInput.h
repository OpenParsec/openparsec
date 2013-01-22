//-----------------------------------------------------------------------------
//	BSPLIB HEADER: SingleInput.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _SINGLEINPUT_H_
#define _SINGLEINPUT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObject.h"
#include "InputData3D.h"
#include "SingleFormat.h"


BSPLIB_NAMESPACE_BEGIN


// class providing single format type input capability ------------------------
//
class SingleInput : public InputData3D, public virtual SingleFormat {

	friend class AodOutput;
	friend class BspOutput;

protected:
	SingleInput( BspObjectList objectlist, const char *filename );
	~SingleInput();

protected:
	int			ConvertColIndxs();
	void		ApplyOriginTranslation();
	void		FilterAxesDirSwitch();
	void		FilterScaleFactors();
	void		FilterAxesExchange();
	void		EnforceMaximumExtents();

	virtual void ParseError( int section );
	void		ReadIntParameter( int& param, int lowbound );
	void		ReadFloatParameter( double& param );
	void		ReadTwoDoubles( double& x, double& y );
	void		ReadThreeDoubles( double& x, double& y, double& z );
	void		ReadFourDoubles( double& x, double& y, double& z, double& d );
	void		ReadSixDoubles( Vertex3& v1, Vertex3& v2 );

	void		ReadVertex();
	void		ReadFaceNormal( int& normalsread );
	void		ReadFace( int aod_read );
	void		ReadFaceProperties( int& facepropsread );
	void		ReadCorrespondences();
	void		ReadTextures();
	void		ReadWorldLocation();
	void		ReadCameraLocation();
	void		ReadPaletteFilename();
	void		ReadScaleFactors();
	void		ReadXChangeCommand();
	void		ReadOrigin();

private:
	void		ReadMaterialRGBA( ColorRGBA &col, int indx );

protected:
	static const int PARSER_DOT_SIZE;

protected:
	BspObject*	m_baseobject;
	FileAccess	m_input;

	char*		m_scanptr;
	int			m_section;
	int			m_parser_lineno;

	Vertex3		m_maximumextents;
};


BSPLIB_NAMESPACE_END


#endif // _SINGLEINPUT_H_

