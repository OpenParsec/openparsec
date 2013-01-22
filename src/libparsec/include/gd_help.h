/*
 * PARSEC HEADER: gd_help.h
 */

#ifndef _GD_HELP_H_
#define _GD_HELP_H_


// ----------------------------------------------------------------------------
// MISCELLANEOUS HELPER MACROS                                                -
// ----------------------------------------------------------------------------


// module registration macros (automatic module-init/deinit on startup) -------
//
//#define REGISTER_MODULE(f)			struct reg_##f {reg_##f();} inst_##f; reg_##f::reg_##f()
//#define REGISTER_MODULE_INIT(f)		struct reg_##f {reg_##f(); ~reg_##f();} inst_##f; reg_##f::reg_##f()
//#define REGISTER_MODULE_KILL(f)		reg_##f::~reg_##f()

// for C++ min/max functions --------------------------------------------------
//
#include <algorithm>

// new module handling --------------------------------------------------------
//
#include "e_modulemanager.h"

// automatically calculate the number of entries of an arbitrary array --------
//
#define CALC_NUM_ARRAY_ENTRIES(a)	(sizeof(a)/sizeof((a)[0]))


// conversion macros for 5, 4, 3, and 2 digit values --------------------------
//
#define DIG5_TO_STR( s, a ) 		(s)[0]=(((a)/10000)%10)+'0';\
									(s)[1]=(((a)/1000)%10)+'0';\
									(s)[2]=(((a)/100)%10)+'0';\
									(s)[3]=(((a)/10)%10)+'0';\
									(s)[4]=((a)%10)+'0';\
									(s)[5]='\0';

#define DIG4_TO_STR( s, a ) 		(s)[0]=(((a)/1000)%10)+'0';\
									(s)[1]=(((a)/100)%10)+'0';\
									(s)[2]=(((a)/10)%10)+'0';\
									(s)[3]=((a)%10)+'0';\
									(s)[4]='\0';

#define DIG3_TO_STR( s, a ) 		(s)[0]=(((a)/100)%10)+'0';\
									(s)[1]=(((a)/10)%10)+'0';\
									(s)[2]=((a)%10)+'0';\
									(s)[3]='\0';

#define DIG2_TO_STR( s, a ) 		(s)[0]=(((a)/10)%10)+'0';\
									(s)[1]=((a)%10)+'0';\
									(s)[2]='\0';


// define wrapper around pseudo random number generator -----------------------
//
#define RAND()						rand()


// min/max functions ----------------------------------------------------------
//
using std::min;
using std::max;


// safe string duplication ----------------------------------------------------
//
#define SAFE_STR_DUPLICATE( dst, src, len )		char dst[ len + 1 ]; \
												strncpy( dst, src, len ); \
												dst[ len ] = 0;

#endif // _GD_HELP_H_


