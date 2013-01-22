/*
 * PARSEC HEADER: gd_limit.h
 */

#ifndef _GD_LIMIT_H_
#define _GD_LIMIT_H_


// ----------------------------------------------------------------------------
// ARBITRARY LIMITATIONS                                                      -
// ----------------------------------------------------------------------------


// maximum numbers of object-types and object-classes -------------------------
//
#define MAX_DISTINCT_OBJTYPES		32
#define MAX_DISTINCT_OBJCLASSES 	128


// maximum numbers of data items ----------------------------------------------
//
#define MAX_TEXTURES				1024
#define MAX_TEXFONTS				16
#define MAX_BITMAPS 				128
#define MAX_CHARSETS				16
#define MAX_SAMPLES 				96
#define MAX_SONGS					16


// maximum numbers of pseudo-stars and fixed-stars ----------------------------
//
#define MAX_PSEUDO_STARS			512
#define MAX_FIXED_STARS 			120


// maximum number of vertices of an iterpoly n-gon ----------------------------
//
#define MAX_ITERPOLY_VERTICES		31


// maximum number of vertices of an iterline line-strip -----------------------
//
#define MAX_ITERLINE_VERTICES		128


// message area limits --------------------------------------------------------
//
#define MAX_MESSAGELEN				127
#define MAX_SCREENMESSAGES			16


// texfont message limits -----------------------------------------------------
//
#define MAX_TEXFONT_MESSAGES		16


// maximum name string lengths (excluding terminating zero) -------------------
//
#define MAX_TEXNAME 				31
#define MAX_OBJNAME 				31
#define MAX_SAMNAME 				31
#define MAX_SNGNAME 				31


// maximum length of player name (network game) -------------------------------
//
#define MAX_PLAYER_NAME				31


#endif // _GD_LIMIT_H_


