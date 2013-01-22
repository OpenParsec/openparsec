//-----------------------------------------------------------------------------
//	BSPLIB HEADER: AodOutput.h
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _AODOUTPUT_H_
#define _AODOUTPUT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "AodFormat.h"
#include "SingleOutput.h"
#include "InputData3D.h"


BSPLIB_NAMESPACE_BEGIN


// file output class for aod format files -------------------------------------
//
class AodOutput : public AodFormat, public SingleOutput {

public:
	AodOutput( BspObjectList objectlist, const char *filename );
	AodOutput( const InputData3D& inputdata );
	~AodOutput();

	AodOutput&	operator =( const AodOutput& copyobj );

public:
	int			WriteOutputFile();
};


BSPLIB_NAMESPACE_END


#endif // _AODOUTPUT_H_

