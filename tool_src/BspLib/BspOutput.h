//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BspOutput.h
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPOUTPUT_H_
#define _BSPOUTPUT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspFormat.h"
#include "BspInput.h"
#include "SingleOutput.h"


BSPLIB_NAMESPACE_BEGIN


// file output class for bsp format files -------------------------------------
//
class BspOutput : public BspFormat, public SingleOutput {

public:
	BspOutput( BspObjectList objectlist, const char *filename );
	BspOutput( const InputData3D& inputdata );
	~BspOutput();

	BspOutput&	operator =( const BspOutput& copyobj );

public:
	int			WriteOutputFile();
};


BSPLIB_NAMESPACE_END


#endif // _BSPOUTPUT_H_

