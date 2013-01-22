//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BspInput.h
//
//  Copyright (c) 1996-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BSPINPUT_H_
#define _BSPINPUT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspFormat.h"
#include "SingleInput.h"


BSPLIB_NAMESPACE_BEGIN


// file input class for bsp format files --------------------------------------
//
class BspInput : public BspFormat, public SingleInput {

public:
	BspInput( BspObjectList objectlist, const char *filename );
	~BspInput();

	BspInput& operator =( const BspInput& copyobj );

public:
	int		ParseObjectData();

private:
	void	CorrectMappingCoordinates();
	void	ReadPolyIndxs( int& facesread );
	void	ReadDirectCorrespondences( int& facesread );
	void	ReadBspTree();

protected:
	void	ParseError( int section );
};


BSPLIB_NAMESPACE_END


#endif // _BSPINPUT_H_

