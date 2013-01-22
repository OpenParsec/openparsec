#ifndef  _QV_BASIC_
#define  _QV_BASIC_

#ifdef WIN32
typedef unsigned long u_long;
#define M_PI 3.1415926536
#define M_PI_4 (M_PI/4.0)
#endif

#include <sys/types.h>
#ifndef WIN32
//#include <libc.h>
#include <stdlib.h>

#else
#include <stdlib.h>
#endif /* WIN32 */
#include <stdio.h>

#ifndef FALSE
#   define FALSE	0
#   define TRUE		1
#endif

typedef int QvBool;

// This uses the preprocessor to quote a string
#if defined(__STDC__) || defined(__ANSI_CPP__)		/* ANSI C */
#  define QV__QUOTE(str)	#str
#else							/* Non-ANSI C */
#ifdef WIN32
#  define QV__QUOTE(str)	#str
#else
#  define QV__QUOTE(str)	"str"
#endif
#endif

// This uses the preprocessor to concatenate two strings
#if defined(__STDC__) || defined(__ANSI_CPP__)		/* ANSI C */
#   define QV__CONCAT(str1, str2)	str1##str2
#else							/* Non-ANSI C */
#   define QV__CONCAT(str1, str2)	str1/**/str2
#endif

#endif /* _QV_BASIC_ */

