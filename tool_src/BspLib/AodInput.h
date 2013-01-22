//-----------------------------------------------------------------------------
//	BSPLIB HEADER: AodInput.h
//
//  Copyright (c) 1996-1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _AODINPUT_H_
#define _AODINPUT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "AodFormat.h"
#include "SingleInput.h"


BSPLIB_NAMESPACE_BEGIN


// file input class for aod format files --------------------------------------
//
class AodInput : public AodFormat, public SingleInput {

public:
	AodInput( BspObjectList objectlist, const char *filename );
	~AodInput();

	AodInput& operator =( const AodInput& copyobj );

public:
	int		ParseObjectData();

protected:
	void	ParseError( int section );
};


BSPLIB_NAMESPACE_END


#endif // _AODINPUT_H_

