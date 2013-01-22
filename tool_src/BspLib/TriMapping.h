//-----------------------------------------------------------------------------
//	BSPLIB HEADER: TriMapping.h
//
//  Copyright (c) 1997 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _TRIMAPPING_H_
#define _TRIMAPPING_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SystemIO.h"
#include "Vertex.h"


BSPLIB_NAMESPACE_BEGIN


// affine mapping specification for a triangle --------------------------------
//
class TriMapping : public virtual SystemIO {

	void		Error() const;

public:
	TriMapping() { }
	~TriMapping() { }

	Vertex2&	getMapXY( int indx ) { if ( indx > 2 ) Error(); return map_xy[ indx ]; }
	Vertex2&	getMapUV( int indx ) { if ( indx > 2 ) Error(); return map_uv[ indx ]; }

private:
	Vertex2		map_xy[ 3 ];	// mapping is specified via three
	Vertex2		map_uv[ 3 ];	// point correspondences
};


BSPLIB_NAMESPACE_END


#endif // _TRIMAPPING_H_

