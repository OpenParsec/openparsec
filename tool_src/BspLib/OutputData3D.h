//-----------------------------------------------------------------------------
//	BSPLIB HEADER: OutputData3D.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _OUTPUTDATA3D_H_
#define _OUTPUTDATA3D_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObjectList.h"
#include "IOData3D.h"
#include "InputData3D.h"


BSPLIB_NAMESPACE_BEGIN


// generic output class for 3-D data ------------------------------------------
//
class OutputData3D : public IOData3D {

public:
	OutputData3D( BspObjectList objectlist, const char *filename, int format = BSP_FORMAT_1_1 );
	OutputData3D( const InputData3D& inputdata, int format = BSP_FORMAT_1_1 );
	virtual ~OutputData3D();

	virtual int		WriteOutputFile();

protected:
	int				m_objectformat;

private:
	OutputData3D*	m_data;
};


BSPLIB_NAMESPACE_END


#endif // _OUTPUTDATA3D_H_

