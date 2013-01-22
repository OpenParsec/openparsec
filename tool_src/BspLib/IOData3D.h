//-----------------------------------------------------------------------------
//	BSPLIB HEADER: IOData3D.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _IODATA3D_H_
#define _IODATA3D_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObjectList.h"
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


// base class for all input/output classes dealing with 3-D data --------------
//
class IOData3D : public virtual SystemIO {

public:

	// format identifiers
	enum {
		DONT_CREATE_OBJECT	= 0x0000,	// indicates to not create an object
		UNKNOWN_FORMAT		= 0x0001,	// no valid signature detected
		AOD_FORMAT_1_1		= 0x0002,	// aod  v1.1
		VRML_FORMAT_1_0		= 0x0003,	// vrml v1.0
		BSP_FORMAT_1_1		= 0x0004,	// bsp  v1.1
		_3DX_FORMAT_1_0		= 0x0005,	// 3dx  v1.0
		//ADD_FORMAT:
	};

protected:

	// construction flags
	enum {
		NO_CHECKS		= 0x0000,		// no validity checks whatsoever
		CHECK_FILENAME	= 0x0001,		// check if supplied filename is valid pointer
		ALL_CHECKS		= 0x0001
	};

public:
	IOData3D( BspObjectList objectlist, const String& filename, int checkflags = ALL_CHECKS );
	~IOData3D() { }

	BspObjectList	getObjectList() const { return m_objectlist; }
	String			getFileName() const { return m_filename; }
	void			setFileName( const String& filename ) { m_filename = filename; }

protected:
	BspObjectList	m_objectlist;	// object database
	String			m_filename;		// name of data file

protected:
	// file signatures
	static const char  AOD_SIGNATURE_1_1[];
	static const char VRML_SIGNATURE_1_0[];
	static const char  BSP_SIGNATURE_1_1[];
	static const char _3DX_SIGNATURE_1_0[];
	//ADD_FORMAT:

	// storage for a single line of text
	static const int TEXTLINE_MAX;
	static char	line[];
};


BSPLIB_NAMESPACE_END


#endif // _IODATA3D_H_

