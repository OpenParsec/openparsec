//-----------------------------------------------------------------------------
//	BSPLIB HEADER: SingleOutput.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _SINGLEOUTPUT_H_
#define _SINGLEOUTPUT_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObject.h"
#include "OutputData3D.h"
#include "SingleFormat.h"


BSPLIB_NAMESPACE_BEGIN


// class providing single format type output capability -----------------------
//
class SingleOutput : public OutputData3D, public virtual SingleFormat {

protected:
	SingleOutput( BspObjectList objectlist, const char *filename );
	~SingleOutput();

	SingleOutput&	operator =( const SingleOutput& copyobj );

	void			InitOutput();
	virtual int		WriteOutputFile();

protected:
	BspObject*		m_baseobject;
	FileAccess		m_output;
};


BSPLIB_NAMESPACE_END


#endif // _SINGLEOUTPUT_H_

