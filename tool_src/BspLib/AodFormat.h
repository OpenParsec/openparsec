//-----------------------------------------------------------------------------
//	BSPLIB HEADER: AodFormat.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _AODFORMAT_H_
#define _AODFORMAT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SingleFormat.h"


BSPLIB_NAMESPACE_BEGIN


// class containing aod format specifics --------------------------------------
//
class AodFormat : public virtual SingleFormat {

protected:

	// section enumeration
	enum {
		_num_aod_sections = _num_single_sections
	};

//protected:
public:
	AodFormat() { }
	~AodFormat() { }

	AodFormat( const AodFormat& copyobj ) : SingleFormat( copyobj ) { }
	AodFormat& operator =( const AodFormat& copyobj );

protected:
	static int			GetSectionId( char *section_name );
	static const char*	GetSectionName( int section_id );

protected:
	static const char	_aodsig_str[];
	static const char	AOD_FILE_EXTENSION[];
};

// assignment operator --------------------------------------------------------
inline AodFormat& AodFormat::operator =( const AodFormat& copyobj )
{
	*(SingleFormat *)this = copyobj;
	return *this;
}


BSPLIB_NAMESPACE_END


#endif // _AODFORMAT_H_

