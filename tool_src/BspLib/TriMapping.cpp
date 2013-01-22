//-----------------------------------------------------------------------------
//	BSPLIB MODULE: TriMapping.cpp
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "TriMapping.h"


BSPLIB_NAMESPACE_BEGIN


// invalid index for mapping vertex -------------------------------------------
//
void TriMapping::Error() const
{
	ErrorMessage( "\n***ERROR*** Invalid index in BspLib::TriMapping." );
	HandleCriticalError();
}


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
