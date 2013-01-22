//-----------------------------------------------------------------------------
//	BSPLIB MODULE: EpsAreas.cpp
//
//  Copyright (c) 1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib headers
#include "BspLibDefs.h"


BSPLIB_NAMESPACE_BEGIN


// epsilon area specifications ------------------------------------------------
//
double EpsAreas::eps_scalarproduct		= 1e-7;
double EpsAreas::eps_planethickness		= 1e-7;
double EpsAreas::eps_vanishdenominator	= 1e-7;
double EpsAreas::eps_vanishcomponent	= 1e-7;
double EpsAreas::eps_pointonline		= 1e-5;
double EpsAreas::eps_pointonlineseg		= 1e-4;
double EpsAreas::eps_vertexmergearea	= 1e-4;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
