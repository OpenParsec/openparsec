//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BspTool.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPTOOL_H_
#define _BSPTOOL_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SystemIO.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// BspTool contains miscellaneous helper functions ----------------------------
//
class BspTool {

public:
	static String		ChangeExtension( const char *filename, const char *extension );
	static const String	SkipPath( const char *name );
};


BSPLIB_NAMESPACE_END


#endif // _BSPTOOL_H_

