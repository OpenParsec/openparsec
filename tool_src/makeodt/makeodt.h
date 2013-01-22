/*
 * makeodt.h
 */

#ifndef _MAKEODT_H_
#define _MAKEODT_H_


// windows specific includes
#ifdef _MSC_VER

	#include <windows.h>
	#include <commctrl.h>
	#include <commdlg.h>
	#include <afxres.h>
	#include <richedit.h>
	#include "resource.h"

	#define PATH_MAX _MAX_PATH

#endif

// opengl specific includes
//#include <gl\gl.h>
//#include <gl\glu.h>


// epsilon area for width of "infinitely thin" plane
#define SCALAR_EPS 0.00000005

// epsilon area for point on line determination
//#define POINT_ON_LINESEG_EPS 0.00000001
#define POINT_ON_LINESEG_EPS 0.02

// epsilon area for merging of vertices
#define VERTEX_MERGE_EPS 0.00000005



#endif // _MAKEODT_H_
