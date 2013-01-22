//-----------------------------------------------------------------------------
//	BSPLIB HEADER: ObjectBinFormat.h
//
//  Copyright (c) 1998-1999 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _OBJECTBINFORMAT_H_
#define _OBJECTBINFORMAT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObject.h"
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


// BspObject capable of writing itself in binary format -----------------------
//
class ObjectBinFormat : public BspObject, public virtual SystemIO {

public:

	// output formats
	enum {
		BINFORMAT_ODT,		// ODT (old format: single object)
		BINFORMAT_OD2,		// OD2 (new format: scene/tree)
	};

public:
	ObjectBinFormat( BspObject& object ) : BspObject( object ) { }
	~ObjectBinFormat() { }
	int			WriteDataToFile( const char *filename, int format );

private:
	void		ODT_CalcAffineMapping( Face& face, dword *dmatrx );
	byte *		ODT_CreateEngineObject( int& memblocksize );
	byte *		ODT_CreateFileObject( int& memblocksize, byte *engineobj );

	void		OD2_CalcAffineMapping( Face& face, dword *dmatrx );
	byte *		OD2_CreateEngineObject( int& memblocksize );
	byte *		OD2_CreateFileObject( int& memblocksize, byte *engineobj );

private:
	static char	line[]; // scratchpad
};


BSPLIB_NAMESPACE_END


#endif // _OBJECTBINFORMAT_H_

