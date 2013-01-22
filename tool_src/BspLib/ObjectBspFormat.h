//-----------------------------------------------------------------------------
//	BSPLIB HEADER: ObjectBspFormat.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _OBJECTBSPFORMAT_H_
#define _OBJECTBSPFORMAT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObject.h"
#include "BspFormat.h"


BSPLIB_NAMESPACE_BEGIN


// BspObject capable of BspFormat I/O operations ------------------------------
//
class ObjectBspFormat : public BspObject, public BspFormat {

public:
	ObjectBspFormat( BspObject& object, BspFormat& format );
	~ObjectBspFormat() { }

	virtual int		WriteVertexList( FileAccess& output );
	virtual int		WritePolygonList( FileAccess& output );
	virtual int		WriteFaceList( FileAccess& output );
	virtual int		WriteFaceProperties( FileAccess& output );
	virtual int		WriteTextureList( FileAccess& output );
	virtual int		WriteMappingList( FileAccess& output );
	virtual int		WriteNormals( FileAccess& output );
	virtual int		WriteBSPTree( FileAccess& output );

private:
	static char		line[]; // scratchpad
};

// construct by copying base class objects ------------------------------------
inline ObjectBspFormat::ObjectBspFormat( BspObject& object, BspFormat& format ) :
	BspObject( object ),
	BspFormat( format )
{
}


BSPLIB_NAMESPACE_END


#endif // _OBJECTBSPFORMAT_H_

