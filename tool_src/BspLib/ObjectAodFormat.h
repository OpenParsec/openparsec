//-----------------------------------------------------------------------------
//	BSPLIB HEADER: ObjectAodFormat.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _OBJECTAODFORMAT_H_
#define _OBJECTAODFORMAT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObject.h"
#include "AodFormat.h"


BSPLIB_NAMESPACE_BEGIN


// BspObject capable of AodFormat I/O operations ------------------------------
//
class ObjectAodFormat : public BspObject, public AodFormat {

public:
	ObjectAodFormat( BspObject& object, AodFormat& format );
	~ObjectAodFormat() { }

	virtual int		WriteVertexList( FileAccess& output );
	virtual int		WriteFaceList( FileAccess& output );
	virtual void	WriteFaceInfo( FileAccess& output, Polygon *poly, int& num );
	virtual int		WriteFaceProperties( FileAccess& output );
	virtual int		WriteTextureList( FileAccess& output );
	virtual int		WriteMappingList( FileAccess& output );
	virtual int		WriteNormals( FileAccess& output );

private:
	static char		line[]; // scratchpad
};

// construct by copying base class objects ------------------------------------
inline ObjectAodFormat::ObjectAodFormat( BspObject& object, AodFormat& format ) :
	BspObject( object ),
	AodFormat( format )
{
}


BSPLIB_NAMESPACE_END


#endif // _OBJECTAODFORMAT_H_

