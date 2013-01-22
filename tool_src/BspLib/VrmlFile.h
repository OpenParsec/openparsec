//-----------------------------------------------------------------------------
//	BSPLIB HEADER: VrmlFile.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _VRMLFILE_H_
#define _VRMLFILE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BoundingBox.h"
#include "InputData3D.h"


BSPLIB_NAMESPACE_BEGIN


class BspObjectList;

// file manipulation class for vrml format files ------------------------------
//
class VrmlFile : public InputData3D {

	friend class BRep;

public:
	VrmlFile( BspObjectList objectlist, const char *filename );
	~VrmlFile();

	int				ParseObjectData();
	int				WriteOutputFile();

private:
	void			ApplySceneTransformations();
	void			EnforceSceneExtents( BoundingBox& unionbox );

private:
	BoundingBox*	m_bboxlist;
};


BSPLIB_NAMESPACE_END


#endif // _VRMLFILE_H_

