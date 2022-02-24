/*
 * PARSEC - Object and Data Loader
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "gd_heads.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "vid_defs.h"

// local module header
#include "e_loader.h"

// proprietary module headers
#include "con_ext.h"
#include "con_main.h"
#include "e_supp.h"
#include "obj_odt.h"
#include "obj_type.h"
#include "sys_file.h"
#include "sys_swap.h"


// flags
//#define SAMPLES_MUST_BE_PRELOADED
#define DISABLE_SONGS



// limitations of loader ------------------------------------------------------
//
#define LINE_LENGTH_MAX			(1024+2)	// max line length in control file
#define MAX_PALETTES			16			// max num of distinct palettes

// size of palette data
#define PALETTE_SIZE			VGA_PAL_SIZE

// texture geometry limitations in powers of two
#define TEX_WIDTH_POW2_MIN		5
#define TEX_WIDTH_POW2_MAX		8
#define TEX_HEIGHT_POW2_MIN		5
#define TEX_HEIGHT_POW2_MAX		7


// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static const char no_texture_mem[]		= "not enough mem for textures.";
static const char texture_not_found[]	= "texture \"%s\" not found";
static const char texture_readerror[]	= "texture \"%s\" readerror";
static const char tex_sig_invalid[]		= "tex signature invalid.";
static const char tex_ver_invalid[]		= "tex version not recognized.";

static const char no_bitmap_mem[]		= "not enough mem for bitmaps and fonts.";
static const char bitmap_not_found[]	= "bitmap \"%s\" not found";
static const char bitmap_readerror[]	= "bitmap \"%s\" readerror";
static const char bdt_sig_invalid[]		= "bdt signature invalid.";
static const char bdt_ver_invalid[]		= "bdt version not recognized.";

static const char font_not_found[]		= "font \"%s\" not found";
static const char font_readerror[]		= "font \"%s\" readerror";
static const char fnt_sig_invalid[]		= "fnt signature invalid.";
static const char fnt_ver_invalid[]		= "fnt version not recognized.";
static const char fnt_inconsistency[]	= "fnt geometry inconsistency.";

static const char fontinfo_not_found[]	= "fontinfo \"%s\" not found";
static const char fontinfo_readerror[]	= "fontinfo \"%s\" readerror";
static const char pfg_sig_invalid[]		= "pfg signature invalid.";
static const char pfg_ver_invalid[]		= "pfg version not recognized.";
static const char pfg_corrupted[]		= "font geometry invalid.";

static const char palette_not_found[]	= "palette \"%s\" not found";
static const char palette_readerror[]	= "palette \"%s\" readerror";
static const char no_palette_mem[]		= "not enough mem for palettes.";


// display flags ------------------------------

static int show_bitmaps_loaded 	= 0; // show filenames of bitmaps/fonts loaded
static int show_textures_loaded	= 0; // show filenames of textures loaded
static int show_objects_loaded 	= 0; // show filenames of objects loaded
static int show_samples_loaded 	= 0; // show filenames of samples loaded
static int show_palettes_loaded	= 0; // show filenames of palettes loaded


// section names of control file  ----------------------------

static const char	_palette_str[]		= "#palette";
static const char	_textures_str[] 	= "#textures";
static const char	_objects_str[]		= "#objects";
static const char	_bitmaps_str[]		= "#bitmaps";
static const char	_charsets_str[] 	= "#charsets";
static const char	_samples_str[]		= "#samples";
static const char	_songs_str[]		= "#songs";
static const char	_comment_str[]		= "#comment";

enum {

	_nil,

	_palette,
	_textures,
	_objects,
	_bitmaps,
	_charsets,
	_samples,
	_songs,
	_comment

} section = _nil;


// names of palette files ------------------------------------
PRIVATE	char*	palette_fnames[ MAX_PALETTES ];

// name of control file --------------------------------------
PRIVATE	char*	ctrl_file_name;

// flag if info texts should be displayed during loading -----
static	int		display_info;


// print error message if error in control file -------------------------------
//
PRIVATE
void ParseError( int section )
{
	const char *sectionname;

	switch( section ) {

		case _textures :
			sectionname = _textures_str;
			break;
		case _objects  :
			sectionname = _objects_str;
			break;
		case _bitmaps  :
			sectionname = _bitmaps_str;
			break;
		case _charsets :
			sectionname = _charsets_str;
			break;
		case _samples  :
			sectionname = _samples_str;
			break;
		case _songs    :
			sectionname = _songs_str;
			break;
		case _palette  :
			sectionname = _palette_str;
			break;
		default 	   :
			sectionname = "UNDEFINED#";
	}

	PERROR( "Control file-parser: [syntax error]: %s.", sectionname );
}


// parsing flags --------------------------------------------------------------
//
#define PARSE_TEXTURES		0x0001
#define PARSE_OBJECTS		0x0002
#define PARSE_BITMAPS		0x0004
#define PARSE_CHARSETS		0x0008
#define PARSE_SAMPLES		0x0010
#define PARSE_SONGS			0x0020
#define PARSE_PALETTES		0x0040
#define PARSE_EVERYTHING	0xffff


// table name insertion macros ------------------------------------------------
//
#define SETTABENTRY_NAME(t,i) { \
	if ( t[ i ].name != NULL ) FREEMEM( t[ i ].name ); \
	t[ i ].name = (char *) ALLOCMEM( strlen( scanptr ) + 1 ); \
	ASSERT( t[ i ].name != NULL ); \
	strcpy( t[ i ].name, scanptr ); \
}

#define SETTABENTRY_FILENAME(t,i) { \
	if ( t[ i ].file != NULL ) FREEMEM( t[ i ].file ); \
	t[ i ].file = (char *) ALLOCMEM( strlen( scanptr ) + 1 ); \
	ASSERT( t[ i ].file != NULL ); \
	strcpy( t[ i ].file, scanptr ); \
}


// parse control file ---------------------------------------------------------
//
PRIVATE
void ParseCtrlFile( dword flags )
{
	int   i;
	char* remptr;
	char  line[ LINE_LENGTH_MAX ];

	if ( flags & PARSE_TEXTURES ) NumLoadedTextures	= 0;
	if ( flags & PARSE_OBJECTS  ) NumLoadedObjects  = 0;
	if ( flags & PARSE_BITMAPS  ) NumLoadedBitmaps  = 0;
	if ( flags & PARSE_CHARSETS ) NumLoadedCharsets = 0;
	if ( flags & PARSE_SAMPLES  ) NumLoadedSamples  = 0;
	if ( flags & PARSE_SONGS    ) NumLoadedSongs	= 0;
	if ( flags & PARSE_PALETTES ) NumLoadedPalettes	= 0;

	FILE *fp = SYS_fopen( ctrl_file_name, "r" );
	if ( fp == NULL ) {
		SERROR( "Controlfile error" );
	}

	if ( display_info ) {
		MSGPUT( "Parsing control file..." );
	}

	// parse sections ---------------------------------------------
	int linecount = 0;
	while ( SYS_fgets( line, LINE_LENGTH_MAX, fp ) != NULL )
	{
		if ( ( ( linecount++ & 0x0f ) == 0 ) && display_info )
			MSGPUT( "." );

		char *scanptr = strtok( line, "/, \t\n\r" );

		if ( scanptr == NULL )
			continue;
		else if ( *scanptr == ';' )
			continue;
		else if ( strnicmp( scanptr, "<end", 4 ) == 0 )
				break;
		else if ( *scanptr == '#' ) {
			if ( stricmp( scanptr, _palette_str ) == 0 )
				section = _palette;
			else if ( stricmp( scanptr, _textures_str ) == 0 )
				section = _textures;
			else if ( stricmp( scanptr, _objects_str ) == 0 )
				section = _objects;
			else if ( stricmp( scanptr, _bitmaps_str ) == 0 )
				section = _bitmaps;
			else if ( stricmp( scanptr, _charsets_str ) == 0 )
				section = _charsets;
			else if ( stricmp( scanptr, _samples_str ) == 0 )
				section = _samples;
			else if ( stricmp( scanptr, _songs_str ) == 0 )
				section = _songs;
			else if ( stricmp( scanptr, _comment_str ) == 0 )
				section = _comment;
			else {
				SYS_fclose( fp );
				PERROR( "Control file-parser: "
						"[undefined section name]: %s.", scanptr );
			}
		}
		else
			switch ( section ) {

			// texturing data -----------------------------------
			case _textures :
				if ( flags & PARSE_TEXTURES ) {

				int txwidth, txheight;

				if ( *scanptr != '\"' ) {
					txwidth = strtol( scanptr, &remptr, 10 );
					if ( *remptr != '\0' ) ParseError( section );
					if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
						ParseError( section );
					txheight = strtol( scanptr, &remptr, 10 );
					if ( *remptr != '\0' ) ParseError( section );

					while ( *scanptr++ != '\0' )
						{}
					while ( ( *scanptr == ' ' ) || ( *scanptr == '\t' ) )
						scanptr++;
				} else {
					txwidth  = -1; // fill in later from
					txheight = -1; // texture file-header
					remptr = scanptr;
					while ( *remptr++ != '\0' )
						{}
					if ( *remptr == '\0' ) ParseError( section );
					*--remptr = ' ';
				}

				if ( ( scanptr = strtok( scanptr, "\"" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_NAME( TextureInfo, NumLoadedTextures );

				if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_FILENAME( TextureInfo, NumLoadedTextures );

				TextureInfo[ NumLoadedTextures ].width  = txwidth;
				TextureInfo[ NumLoadedTextures ].height = txheight;
				if ( ++NumLoadedTextures >= MAX_TEXTURES )
					PANIC( "too many textures." );

				}
				break;

			// object data files --------------------------------
			case _objects :
				if ( flags & PARSE_OBJECTS ) {

				int objnum = strtol( scanptr, &remptr, 10 );
				if ( *remptr != '\0' ) ParseError( section );

				while ( *scanptr++ != '\0' )
					{}
				while ( ( *scanptr == ' ' ) || ( *scanptr == '\t' ) )
					scanptr++;
				if ( ( scanptr = strtok( scanptr, "\"" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_NAME( ObjectInfo, NumLoadedObjects );

				if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
					ParseError( section );
				int objtype = -1;
				for ( i = 0; i < NUM_DISTINCT_OBJTYPES; i++ ) {
					if ( stricmp( scanptr, objtype_name[ i ] ) == 0 ) {
						objtype = i;
						break;
					}
				}
				if ( ( objtype < 0 ) || ( objtype >= NUM_DISTINCT_OBJTYPES ) )
					ParseError( section );
				ObjectInfo[ NumLoadedObjects ].type = objtype_id[ objtype ];
				if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_FILENAME( ObjectInfo, NumLoadedObjects );
				ObjectInfo[ NumLoadedObjects ].lodinfo = NULL;

				if ( ++NumLoadedObjects >= MAX_DISTINCT_OBJCLASSES )
					PANIC( "too many object classes." );

				}
				break;

			// bitmap files -------------------------------------
			case _bitmaps :
				if ( flags & PARSE_BITMAPS ) {

				int bitmapnum = strtol( scanptr, &remptr, 10 );
				if ( *remptr != '\0' ) ParseError( section );
				if ( ( scanptr = strtok( NULL, "x,/ \t\n\r" ) ) == NULL )
					ParseError( section );

				if ( isdigit( *scanptr ) ) {
					BitmapInfo[ NumLoadedBitmaps ].width = strtol( scanptr, &remptr, 10 );
					if ( *remptr != '\0' ) ParseError( section );
					if ( ( scanptr = strtok( NULL, "x,/ \t\n\r" ) ) == NULL )
						ParseError( section );
					BitmapInfo[ NumLoadedBitmaps ].height = strtol( scanptr, &remptr, 10 );
					if ( *remptr != '\0' ) ParseError( section );
					if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
						ParseError( section );
				} else {
					BitmapInfo[ NumLoadedBitmaps ].width  = -1; // fill in later from
					BitmapInfo[ NumLoadedBitmaps ].height = -1; // bitmapfile header
				}

				SETTABENTRY_FILENAME( BitmapInfo, NumLoadedBitmaps );

				BitmapInfo[ NumLoadedBitmaps ].bitmappointer = NULL;

				if ( ++NumLoadedBitmaps >= MAX_BITMAPS )
					PANIC( "too many bitmaps." );

				}
				break;

			// charset data -------------------------------------
			case _charsets :
				if ( flags & PARSE_CHARSETS ) {

				int charsetnum = strtol( scanptr, &remptr, 10 );
				if ( *remptr != '\0' ) ParseError( section );
				if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
					ParseError( section );

				if ( isdigit( *scanptr ) ) {
					CharsetInfo[ NumLoadedCharsets ].srcwidth = strtol( scanptr, &remptr, 10 );
					if ( *remptr != '\0' ) ParseError( section );
					if ( ( scanptr = strtok( NULL, "x,/ \t\n\r" ) ) == NULL )
						ParseError( section );
					CharsetInfo[ NumLoadedCharsets ].width = strtol( scanptr, &remptr, 10 );
					if ( *remptr != '\0' ) ParseError( section );
					if ( ( scanptr = strtok( NULL, "x,/ \t\n\r" ) ) == NULL )
						ParseError( section );
					CharsetInfo[ NumLoadedCharsets ].height = strtol( scanptr, &remptr, 10 );
					if ( *remptr != '\0' ) ParseError( section );
					if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
						ParseError( section );
				} else {
					CharsetInfo[ NumLoadedCharsets ].srcwidth = -1; // fill in later
					CharsetInfo[ NumLoadedCharsets ].width    = -1; // from font-info
					CharsetInfo[ NumLoadedCharsets ].height   = -1; // file
				}

				SETTABENTRY_FILENAME( CharsetInfo, NumLoadedCharsets );

				CharsetInfo[ NumLoadedCharsets ].charsetpointer	= NULL;
				CharsetInfo[ NumLoadedCharsets ].fonttexture	= NULL;

				if ( ++NumLoadedCharsets >= MAX_CHARSETS )
					PANIC( "too many charsets." );

				}
				break;

			// sample data --------------------------------------
			case _samples:
				if ( flags & PARSE_SAMPLES ) {

				int samplenum = strtol( scanptr, &remptr, 10 );
				if ( *remptr != '\0' ) ParseError( section );

				// defaults
				SampleInfo[ NumLoadedSamples ].samplepointer = NULL;
				SampleInfo[ NumLoadedSamples ].flags         = 0;
				SampleInfo[ NumLoadedSamples ].stereolevel   = 0;
				SampleInfo[ NumLoadedSamples ].volume        = AUD_MAX_VOLUME;
				SampleInfo[ NumLoadedSamples ].samplefreq    = 0;
				SampleInfo[ NumLoadedSamples ].stdfreq       = 44100.0f;

				while ( *scanptr++ != '\0' )
					{}
				if ( isdigit( *scanptr ) ) {
					if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
						ParseError( section );
					float stdfreq = strtod( scanptr, &remptr );
					if ( *remptr != '\0' ) ParseError( section );
					SampleInfo[ NumLoadedSamples ].stdfreq = stdfreq;
					while ( *scanptr++ != '\0' )
						{}
				}
				while ( ( *scanptr == ' ' ) || ( *scanptr == '\t' ) )
					scanptr++;

				if ( ( scanptr = strtok( scanptr, "\"" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_NAME( SampleInfo, NumLoadedSamples );

				if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_FILENAME( SampleInfo, NumLoadedSamples );

				if ( ++NumLoadedSamples >= MAX_SAMPLES )
					PANIC( "too many samples." );

				}
				break;

			// song data ----------------------------------------
			case _songs:
				if ( flags & PARSE_SONGS ) {

				int songnum = strtol( scanptr, &remptr, 10 );
				if ( *remptr != '\0' ) ParseError( section );

				while ( *scanptr++ != '\0' )
					{}
				while ( ( *scanptr == ' ' ) || ( *scanptr == '\t' ) )
					scanptr++;
				if ( ( scanptr = strtok( scanptr, "\"" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_NAME( SongInfo, NumLoadedSongs );

				if ( ( scanptr = strtok( NULL, ",/ \t\n\r" ) ) == NULL )
					ParseError( section );
				SETTABENTRY_FILENAME( SongInfo, NumLoadedSongs );

				SongInfo[ NumLoadedSongs ].songpointer = NULL;

				if ( ++NumLoadedSongs >= MAX_SONGS )
					PANIC( "too many songs." );

				}
				break;

			// filenames of palette files -----------------------
			case _palette :
				if ( flags & PARSE_PALETTES ) {

				ASSERT( palette_fnames[ NumLoadedPalettes ] == NULL );
				palette_fnames[ NumLoadedPalettes ] = (char *) ALLOCMEM( strlen( scanptr ) + 1 );
				ASSERT( palette_fnames[ NumLoadedPalettes ] != NULL );
				strcpy( palette_fnames[ NumLoadedPalettes ], scanptr );

				if ( ++NumLoadedPalettes >= MAX_PALETTES )
					PANIC( "too many palettes." );

				}
				break;
			default:
				break;
			}
	}

	SYS_fclose( fp );

	if ( display_info ) {
		MSGOUT( "done.\n" );
	}
}


// copy file name with altered extension --------------------------------------
//
PRIVATE
void CopyFileNameAlterExt( char *dest, char *src, const char *ext )
{
	while ( ( *src != '.' ) && ( *src != '\0' ) )
		*dest++ = *src++;

	if ( *src == '.' ) {
		*dest++ = '.';
		strcpy( dest, ext );
	} else {
		*dest = '\0';
	}
}


// names of default bitmaps ---------------------------------------------------
//
static const char *default_bitmap_names[] = {

	BM_NAME_CROSSHAIR,
	BM_NAME_RADAR,
	BM_NAME_MSTAR1,
	BM_NAME_MSTAR2,
	BM_NAME_MSTAR3,
	BM_NAME_MSTAR4,
	BM_NAME_BIGPLANET1,
	BM_NAME_SCLSTR1,
	BM_NAME_SCLSTR2,
	BM_NAME_MEDPLANET1,
	BM_NAME_MINPLANET1,
	BM_NAME_TARGET,
	BM_NAME_EXPLANIM01,
	BM_NAME_EXPLANIM02,
	BM_NAME_EXPLANIM03,
	BM_NAME_EXPLANIM04,
	BM_NAME_EXPLANIM05,
	BM_NAME_EXPLANIM06,
	BM_NAME_EXPLANIM07,
	BM_NAME_EXPLANIM08,
	BM_NAME_EXPLANIM09,
	BM_NAME_EXPLANIM10,
	BM_NAME_EXPLANIM11,
	BM_NAME_EXPLANIM12,
	BM_NAME_NEBULA,
	BM_NAME_LOGO1,
	BM_NAME_LOGO2,
	BM_NAME_SHIP1_BLUEPRINT,
	BM_NAME_SHIP2_BLUEPRINT,
	BM_NAME_ROCKET1,
	BM_NAME_ROCKET2,
	BM_NAME_CROSSHAIR2,
	BM_NAME_CROSSHAIR3,
	BM_NAME_MINE,
	BM_NAME_GUN1,
	BM_NAME_GUN2,
	BM_NAME_GUN3,
	BM_NAME_OBJSLOGO,
	BM_NAME_VIEWLOGO,
	BM_NAME_LEFTARROW,
	BM_NAME_RIGHTARRORW,
	BM_NAME_SELECT,
	BM_NAME_SUN,
	BM_NAME_LIGHTNING2,
	BM_NAME_LIGHTNING1,
	BM_NAME_FIREBALL1,
	BM_NAME_FIREBALL2,
	BM_NAME_SHIELD1,
	BM_NAME_SHIELD2,
	BM_NAME_PROPFUMES1,
	BM_NAME_PROPFUMES2,
	BM_NAME_PROPFUMES3,
	BM_NAME_FIREBALL3,
	BM_NAME_LENSFLARE1,
	BM_NAME_LENSFLARE2,
	BM_NAME_LENSFLARE3,
	BM_NAME_LENSFLARE4,
};

int num_default_bitmaps = CALC_NUM_ARRAY_ENTRIES( default_bitmap_names );


// read bitmaps and charsets into memory --------------------------------------
//
PRIVATE
void ReadBitmapsAndCharsets()
{
	// exit if nothing to read
	if ( ( NumLoadedBitmaps == 0 ) && ( NumLoadedCharsets == 0 ) )
		return;

	// print message
	if ( display_info ) {
		MSGPUT( "Loading bitmaps and charsets" );
		if ( show_bitmaps_loaded ) {
			MSGOUT( ":\n" );
		} else {
			MSGPUT( "..." );
		}
	}

	// get filesizes of bitmap files
	size_t bitmmemsize = 0;
	int bmid = 0;
	for ( bmid = 0; bmid < NumLoadedBitmaps; bmid++ ) {
		ssize_t siz = SYS_GetFileLength( BitmapInfo[ bmid ].file );
		if ( siz == -1 )
			FERROR( bitmap_not_found, BitmapInfo[ bmid ].file );
		bitmmemsize += siz;
	}

	// get filesizes of charset files
	int ftid = 0;
	for ( ftid = 0; ftid < NumLoadedCharsets; ftid++ ) {

		ssize_t siz;

		// if geometry not set interpret specified file as info-file
		if ( CharsetInfo[ ftid ].width == -1 ) {

			siz = SYS_GetFileLength( CharsetInfo[ ftid ].file );
			if ( siz  == -1 )
				FERROR( fontinfo_not_found, CharsetInfo[ ftid ].file );
			CopyFileNameAlterExt( paste_str, CharsetInfo[ ftid ].file, FONT_EXTENSION );
			ssize_t gsiz = SYS_GetFileLength( paste_str );
			if ( gsiz == -1 )
				FERROR( font_not_found, paste_str );
			siz += gsiz;

		} else {

			siz = SYS_GetFileLength( CharsetInfo[ ftid ].file );
			if ( siz == -1 )
				FERROR( font_not_found, CharsetInfo[ ftid ].file );
		}

		bitmmemsize += siz;
	}

	// allocate buffer for bitmaps and charsets
	if ( ( BitmapMem = (char *) ALLOCMEM( bitmmemsize ) ) == NULL )
		OUTOFMEM( no_bitmap_mem );

	char   *bitmreadpo = BitmapMem;
	size_t stillfree   = bitmmemsize;


	// read in raw bitmap files -----------------------------------------------
	for ( bmid = 0; bmid < NumLoadedBitmaps; bmid++ ) {

		if ( ( ( bmid & 0x03 ) == 0 ) && display_info && !show_bitmaps_loaded )
			MSGPUT( "." );
		FILE *fp = SYS_fopen( BitmapInfo[ bmid ].file, "rb" );
		if ( fp == NULL )
			FERROR( bitmap_not_found, BitmapInfo[ bmid ].file );
		if ( display_info && show_bitmaps_loaded )
			MSGOUT( "loading \"%s\" (bitmap)\n", BitmapInfo[ bmid ].file );
		ssize_t bytesread = SYS_fread( bitmreadpo, 1, stillfree, fp );
		if ( bytesread == 0 )
			FERROR( bitmap_readerror, BitmapInfo[ bmid ].file );

		// if width and height not already set header must be present
		if ( BitmapInfo[ bmid ].width == -1 ) {

			BdtHeader *header = (BdtHeader *) bitmreadpo;

			// swap endianness of BdtHeader
			SYS_SwapBdtHeader( header );

			if ( stricmp( header->signature, BDT_SIGNATURE ) != 0 )
				PERROR( bdt_sig_invalid );
			if ( header->version < REQUIRED_BDT_VERSION )
				PERROR( bdt_ver_invalid );

			// store width and height
			BitmapInfo[ bmid ].width  = header->width;
			BitmapInfo[ bmid ].height = header->height;

			// store pointer to bitmap into control structure (header excluded)
			BitmapInfo[ bmid ].bitmappointer = (char *) ( header + 1 );

		} else {

			// store pointer to bitmap into control structure (was raw format)
			BitmapInfo[ bmid ].bitmappointer = bitmreadpo;

		}

		// store pointer to originally loaded data (needed for different color depths)
		BitmapInfo[ bmid ].loadeddata = BitmapInfo[ bmid ].bitmappointer;

		// set bitmap name (static storage!)
		ASSERT( num_default_bitmaps == BM_CONTROLFILE_NUMBER );
		ASSERT( bmid < num_default_bitmaps );
		BitmapInfo[ bmid ].name = (char *) default_bitmap_names[ bmid ];

		stillfree  -= bytesread;
		bitmreadpo += bytesread;
		SYS_fclose( fp );
	}


	// read in raw charset data -----------------------------------------------
	int charset_datasize = 0;
	for ( ftid = 0; ftid < NumLoadedCharsets; ftid++ ) {

		if ( ( ( ftid & 0x03 ) == 0 ) && display_info && !show_bitmaps_loaded )
			MSGPUT( "." );

		if ( CharsetInfo[ ftid ].srcwidth == -1 ) {

			FILE *fp = SYS_fopen( CharsetInfo[ ftid ].file, "rb" );
			if ( fp == NULL )
				FERROR( fontinfo_not_found, CharsetInfo[ ftid ].file );
			if ( display_info && show_bitmaps_loaded )
				MSGOUT( "loading \"%s\" (fontinfo)\n", CharsetInfo[ ftid ].file );
			size_t gsiz = SYS_fread( bitmreadpo, 1, stillfree, fp );
			if ( gsiz < sizeof( PfgHeader ) + 8 )
				FERROR( fontinfo_readerror, CharsetInfo[ ftid ].file );

			PfgHeader *header = (PfgHeader *) bitmreadpo;

			// swap endianness of PfgHeader
			SYS_SwapPfgHeader( header );

			if ( stricmp( header->signature, PFG_SIGNATURE ) != 0 )
				PERROR( pfg_sig_invalid );
			if ( header->version < REQUIRED_PFG_VERSION )
				PERROR( pfg_ver_invalid );
			CharsetInfo[ ftid ].srcwidth = header->srcwidth;
			CharsetInfo[ ftid ].width	 = header->width;
			CharsetInfo[ ftid ].height	 = header->height;

			// ensure table is multiple of four in length
			size_t geomsiz = gsiz - sizeof( PfgHeader ) - 4;
			if ( ( geomsiz & 0x03 ) != 0x00 )
				FERROR( pfg_corrupted, CharsetInfo[ ftid ].file );

			// create pointer to table excluding startchar
			dword *geomtab = (dword *)( bitmreadpo + sizeof( PfgHeader ) + 4 );

			//TODO:
			//CAVEAT:
			//NOTE:
			// the font-type determination assumes that exactly
			// those fonts with width==height are fixed-size fonts.

			// determine whether fixed-size font
			int fonttype = ( header->width == header->height );

			// swap endianness of geometry table
			SYS_SwapPfgTable( fonttype, geomtab, geomsiz );

			// geometry pointer must point one dword past the geometry table!!
			CharsetInfo[ ftid ].geompointer = geomtab;
			bitmreadpo += gsiz;
			stillfree  -= gsiz;
			SYS_fclose( fp );

			CopyFileNameAlterExt( paste_str, CharsetInfo[ ftid ].file, FONT_EXTENSION );
			if ( ( fp = SYS_fopen( paste_str, "rb" ) ) == NULL )
				FERROR( font_not_found, paste_str );
			if ( display_info && show_bitmaps_loaded )
				MSGOUT( "loading \"%s\" (font)\n", paste_str );
			size_t bytesread = SYS_fread( bitmreadpo, 1, stillfree, fp );
			if ( bytesread == 0 )
				FERROR( font_readerror, paste_str );

			FntHeader *dheader = (FntHeader *) bitmreadpo;

			// swap endianness of FntHeader
			SYS_SwapFntHeader( dheader );

			if ( stricmp( dheader->signature, FNT_SIGNATURE ) != 0 )
				PERROR( fnt_sig_invalid );
			if ( dheader->version < REQUIRED_FNT_VERSION )
				PERROR( fnt_ver_invalid );
			if ( dheader->width != CharsetInfo[ ftid ].srcwidth )
				PERROR( fnt_inconsistency );

			CharsetInfo[ ftid ].charsetpointer = bitmreadpo + sizeof( FntHeader );

			bitmreadpo += bytesread;
			stillfree  -= bytesread;
			SYS_fclose( fp );

			charset_datasize = bytesread - sizeof( FntHeader );

		} else {

			FILE *fp = SYS_fopen( CharsetInfo[ ftid ].file, "rb" );
			if ( fp == NULL )
				FERROR( font_not_found, CharsetInfo[ ftid ].file );
			if ( display_info && show_bitmaps_loaded )
				MSGOUT( "loading \"%s\" (font)\n", CharsetInfo[ ftid ].file );
			size_t bytesread = SYS_fread( bitmreadpo, 1, stillfree, fp );
			if ( bytesread == 0 )
				FERROR( font_readerror, CharsetInfo[ ftid ].file );
			CharsetInfo[ ftid ].charsetpointer = bitmreadpo;

			// set pointer to geometry data
			//FIXME: [momentan noch pfusch!!]
			if ( ftid == 1 )
				CharsetInfo[ ftid ].geompointer = Char04x09Geom;
			else if ( ftid == 2 )
				CharsetInfo[ ftid ].geompointer = Char08x08Geom;
			else if ( ftid == 4 )
				CharsetInfo[ ftid ].geompointer = CharGoldGeom;
			else
				CharsetInfo[ ftid ].geompointer = Char16x16Geom;

			stillfree  -= bytesread;
			bitmreadpo += bytesread;
			SYS_fclose( fp );

			charset_datasize = bytesread;
		}

		// store pointer to originally loaded data (needed for different color depths)
		CharsetInfo[ ftid ].loadeddata = CharsetInfo[ ftid ].charsetpointer;
		ASSERT( CharsetInfo[ ftid ].fonttexture == NULL );

		// store size of data block
		CharsetInfo[ ftid ].datasize = charset_datasize;

		// set additional info fields
		CharsetInfo[ ftid ].flags = 0x00000000;
	}

	if ( display_info ) {
		MSGOUT( "done.\n" );
	}
}


// read needed textures into memory -------------------------------------------
//
PRIVATE
void ReadTextures()
{
	// exit if nothing to read
	if ( NumLoadedTextures == 0 )
		return;

	if ( display_info ) {
		MSGPUT( "Loading textures" );
		if ( show_textures_loaded ) {
			MSGOUT( ":\n" );
		} else {
			MSGPUT( "..." );
		}
	}

	// get filesizes of textures to allocate mem in one piece
	size_t texmemsize = 0;
	int tid =  0;
	for ( tid = 0; tid < NumLoadedTextures; tid++ ) {
		ssize_t siz = SYS_GetFileLength( TextureInfo[ tid ].file );
		if ( siz == -1 )
			FERROR( texture_not_found, TextureInfo[ tid ].file );
		texmemsize += siz;
	}

	// allocate texture buffer
	TextureMem = (char *) ALLOCMEM( texmemsize + sizeof( TextureMap ) * NumLoadedTextures );
	if ( TextureMem == NULL )
		OUTOFMEM( no_texture_mem );

	char	   *texreadpo = TextureMem + sizeof( TextureMap ) * NumLoadedTextures;
	TextureMap *controlpo = (TextureMap *) TextureMem;
	size_t	   stillfree  = texmemsize;

	// read in texturedata and fill control structures
	for ( tid = 0; tid < NumLoadedTextures; tid++ ) {

		if ( ( ( tid & 0x03 ) == 0 ) && display_info && !show_textures_loaded )
			MSGPUT( "." );

		FILE *fp = SYS_fopen( TextureInfo[ tid ].file, "rb" );
		if ( fp == NULL )
			FERROR( texture_not_found, TextureInfo[ tid ].file );

		if ( display_info && show_textures_loaded ) {
			MSGOUT( "loading \"%s\" (texture)\n", TextureInfo[ tid ].file );
		}

		size_t bytesread = SYS_fread( texreadpo, 1, stillfree, fp );
		if ( bytesread == 0 )
			FERROR( texture_readerror, TextureInfo[ tid ].file );

		// if geometry not explicitly specified read it from header
		if ( ( TextureInfo[ tid ].width == -1 ) || ( TextureInfo[ tid ].height == -1 ) ) {

			TexHeader *header = (TexHeader *) texreadpo;

			// swap endianness of TexHeader
			SYS_SwapTexHeader( header );

			if ( stricmp( header->signature, TEX_SIGNATURE ) != 0 )
				PERROR( tex_sig_invalid );
			if ( header->version < REQUIRED_TEX_VERSION )
				PERROR( tex_ver_invalid );
			TextureInfo[ tid ].width  = header->width;
			TextureInfo[ tid ].height = header->height;

			stillfree -= sizeof( TexHeader );
			bytesread -= sizeof( TexHeader );
			texreadpo += sizeof( TexHeader );
		}

		// set additional texture info fields
		TextureInfo[ tid ].flags = TEXINFOFLAG_NONE;

		// fill texture control structure -----------------------------------
		controlpo->Width  = CeilPow2Exp( TextureInfo[ tid ].width );
		controlpo->Height = CeilPow2Exp( TextureInfo[ tid ].height );

		// check validity of texture geometry
		if ( ( controlpo->Width  < TEX_WIDTH_POW2_MIN  ) ||
			 ( controlpo->Width  > TEX_WIDTH_POW2_MAX  ) ||
			 ( controlpo->Height < TEX_HEIGHT_POW2_MIN ) ||
			 ( controlpo->Height > TEX_HEIGHT_POW2_MAX ) ||
			 ( controlpo->Width  < controlpo->Height   ) ||
			 // check texture's aspect ratio
			 ( ( controlpo->Width - controlpo->Height ) > 1 ) ) {

			SYS_fclose( fp );
			PERROR( "texture dimension not allowed (texture %d).", tid + 1 );
		}

		// calculate geometry code used by texture mapper
		dword geom = ( controlpo->Width - TEX_WIDTH_POW2_MIN ) * 2;
		if ( controlpo->Width == controlpo->Height )
			geom++;
		controlpo->Geometry 	= geom - 1;

		// set pointers to texture bitmap and name
		controlpo->BitMap		= texreadpo;
		controlpo->TexMapName	= TextureInfo[ tid ].name;

		// set additional texture fields
		controlpo->Flags		= TEXFLG_NONE;
		controlpo->LOD_small	= 0;
		controlpo->LOD_large	= 0;
		controlpo->TexPalette	= NULL;
		controlpo->TexelFormat	= TEXFMT_STANDARD;

		// store pointer to texture in texture info table
		TextureInfo[ tid ].texpointer	  = controlpo;
		TextureInfo[ tid ].standardbitmap = controlpo->BitMap;

		stillfree -= bytesread;
		texreadpo += bytesread;
		controlpo++;

		SYS_fclose( fp );
	}

	if ( display_info ) {
		MSGOUT( "done.\n" );
	}
}


// read sample data into buffer -----------------------------------------------
//
PRIVATE
void ReadSamples()
{

#ifdef SAMPLES_MUST_BE_PRELOADED

	// exit if nothing to read
	if ( ( NumLoadedSamples == 0 ) && ( NumLoadedSongs == 0 ) )
		return;

	if ( display_info ) {
		MSGPUT( "Loading sound data" );
		if ( show_samples_loaded ) {
			MSGOUT( ":\n" );
		} else {
			MSGPUT( "..." );
		}
	}

	// get filesizes of samples to allocate mem in one piece
	size_t samplememsize = 0;
	for ( int i = 0; i < NumLoadedSamples; i++ ) {
		size_t = SYS_GetFileLength( SampleInfo[ i ].file );
		if ( siz == -1 )
			FERROR( sample_not_found, SampleInfo[ i ].file );
		samplememsize += siz;
	}

#ifndef DISABLE_SONGS

	// get filesizes of songs to allocate mem in one piece together with samples
	for ( i = 0; i < NumLoadedSongs; i++ ) {
		size_t = SYS_GetFileLength( SongInfo[ i ].file );
		if ( siz == -1 )
			FERROR( song_not_found, SongInfo[ i ].file );
		samplememsize += siz;
	}

#endif

	// allocate sample buffer (also used for songdata)
	if ( ( SampleMem = (char *) ALLOCMEM( samplememsize ) ) == NULL )
		OUTOFMEM( no_sample_mem );

	char   *samplereadpo = SampleMem;
	size_t stillfree	 = samplememsize;

	// read in sample data
	for ( i = 0; i < NumLoadedSamples; i++ ) {

		if ( ( ( i & 0x01 ) == 0 ) && display_info && !show_samples_loaded )
			MSGPUT( "." );
		FILE *fp = SYS_fopen( SampleInfo[ i ].file, "rb" );
		if ( fp == NULL )
			FERROR( sample_not_found, SampleInfo[ i ].file );
		if ( display_info && show_samples_loaded )
			MSGOUT( "loading \"%s\" (sample)\n", SampleInfo[ i ].file );
		size_t bytesread = SYS_fread( samplereadpo, 1, stillfree, fp );
		if ( bytesread == 0 )
			FERROR( sample_readerror, SampleInfo[ i ].file );

		// store pointer to sample into control structure ------------
		SampleInfo[ i ].samplepointer	= samplereadpo;
		SampleInfo[ i ].size			= bytesread;

		stillfree	 -= bytesread;
		samplereadpo += bytesread;
		SYS_fclose( fp );
	}

#ifndef DISABLE_SONGS

	// read in song data
	for ( i = 0; i < NumLoadedSongs; i++ ) {

		if ( ( ( i & 0x01 ) == 0 ) && display_info && !show_samples_loaded )
			MSGPUT( "." );
		FILE *fp = SYS_fopen( SongInfo[ i ].file, "rb" );
		if ( fp == NULL )
			FERROR( song_not_found, SongInfo[ i ].file );
		if ( display_info && show_samples_loaded )
			MSGOUT( "loading \"%s\" (song)\n", SongInfo[ i ].file );
		size_t bytesread = SYS_fread( samplereadpo, 1, stillfree, fp );
		if ( bytesread == 0 )
			FERROR( song_readerror, SongInfo[ i ].file );

		// store pointer to song into control structure --------------
		SongInfo[ i ].songpointer = samplereadpo;

		stillfree	 -= bytesread;
		samplereadpo += bytesread;
		SYS_fclose( fp );
	}

#endif

	if ( display_info ) {
		MSGOUT( "done.\n" );
	}

#endif

}


// read palette files into buffer ---------------------------------------------
//
PRIVATE
void ReadPalettes()
{
	// exit if nothing to read
	if ( NumLoadedPalettes == 0 )
		PANIC( "no palette defined." );

	if ( ( PaletteMem = (char *) ALLOCMEM( PALETTE_SIZE * NumLoadedPalettes ) ) == NULL )
		OUTOFMEM( no_palette_mem );

	if ( display_info ) {
		MSGPUT( "Loading palettes" );
		if ( show_palettes_loaded ) {
			MSGOUT( ":\n" );
		} else {
			MSGPUT( "..." );
		}
	}

	// load all palettes
	size_t readofs = 0;
	for ( int pid = 0; pid < NumLoadedPalettes; pid++ ) {

		if ( display_info && !show_palettes_loaded )
			MSGPUT( "." );

		FILE *fp = SYS_fopen( palette_fnames[ pid ], "rb" );
		if ( fp == NULL )
			FERROR( palette_not_found, palette_fnames[ pid ] );

		if ( display_info && show_palettes_loaded ) {
			MSGOUT( "loading \"%s\" (palette)\n", palette_fnames[ pid ] );
		}

		size_t bytesread = SYS_fread( PaletteMem + readofs, 1, PALETTE_SIZE, fp );
		if ( bytesread != PALETTE_SIZE )
			FERROR( palette_readerror, palette_fnames[ pid ] );
		readofs += PALETTE_SIZE;

		SYS_fclose( fp );
	}

	if ( display_info ) {
		MSGOUT( "done.\n" );
	}
}


// load objects from binary data files ----------------------------------------
//
PRIVATE
void LoadObjectData()
{
	// set global number of object classes
	NumObjClasses = NumLoadedObjects;

	// exit if nothing to read
	if ( NumLoadedObjects == 0 ) {
		MSGOUT( "*** WARNING: No objects at all. ***\n" );
		return;
	}

	if ( display_info ) {
		MSGPUT( "Loading objects" );
		if ( show_objects_loaded ) {
			MSGOUT( ":\n" );
		} else {
			MSGPUT( "..." );
		}
	}

	// load all object data
	for ( int oid = 0; oid < NumLoadedObjects; oid++ ) {

		if ( display_info ) {
			if ( show_objects_loaded ) {
				MSGOUT( "loading \"%s\" (object)\n", ObjectInfo[ oid ].file );
			} else if ( ( oid & 0x01 ) == 0 ) {
				MSGPUT( "." );
			}
		}

		// load object in odt format
		OBJ_LoadODT( oid, OBJLOAD_DEFAULT, NULL );
	}

	if ( display_info ) {
		MSGOUT( "done.\n" );
	}
}


// load all data specified in control file ------------------------------------
//
void LoadData( char *cfn, int dispinfo )
{
	ASSERT( cfn != NULL );
	extern int headless_bot;
	
	ctrl_file_name	= cfn;
	display_info	= FALSE; //dispinfo;

	//NOTE:
	// we currently disable loader message output
	// at all times, since most data is now loaded
	// via console scripts and these messages would
	// just be irritating.

	ParseCtrlFile( PARSE_EVERYTHING );
	if(!headless_bot){
		ReadPalettes();
		ReadTextures();
		ReadSamples();
		ReadBitmapsAndCharsets();
	}
	LoadObjectData();
	
}


// free memory blocks containing resolution dependent data --------------------
//
void FreeResolutionData()
{
	for ( int bid = 0; bid < NumLoadedBitmaps; bid++ ) {
		if ( BitmapInfo[ bid ].bitmappointer != BitmapInfo[ bid ].loadeddata ) {
			ASSERT( BitmapInfo[ bid ].bitmappointer != NULL );
			FREEMEM( BitmapInfo[ bid ].bitmappointer );
			BitmapInfo[ bid ].bitmappointer = NULL;
		}
	}

	for ( int cid = 0; cid < NumLoadedCharsets; cid++ ) {
		if ( CharsetInfo[ cid ].charsetpointer != CharsetInfo[ cid ].loadeddata ) {
			if ( CharsetInfo[ cid ].fonttexture != NULL ) {
				ASSERT( (void*)CharsetInfo[ cid ].charsetpointer ==
						(void*)CharsetInfo[ cid ].fonttexture );
				CharsetInfo[ cid ].fonttexture = NULL;
			}
			ASSERT(  CharsetInfo[ cid ].charsetpointer != NULL );
			FREEMEM( CharsetInfo[ cid ].charsetpointer );
			CharsetInfo[ cid ].charsetpointer = NULL;
		}
	}

	ASSERT( BitmapMem );
	FREEMEM( BitmapMem );
}


// reload resolution-dependent data files -------------------------------------
//
void ReloadResData( char *cfn )
{
	ASSERT( cfn != NULL );

	FreeResolutionData();

	ctrl_file_name	= cfn;
	display_info	= FALSE;

	ParseCtrlFile( PARSE_BITMAPS | PARSE_CHARSETS );

	ReadBitmapsAndCharsets();
}


// reload graphics detail-dependent data files --------------------------------
//
void ReloadDetailData( char *cfn )
{
	ASSERT( cfn != NULL );

	ASSERT(	NumObjClasses == NumLoadedObjects );
	for ( int oid = NumLoadedObjects - 1; oid >= 0; oid-- ) {
		ASSERT( ObjClasses[ oid ] );
		FREEMEM( ObjClasses[ oid ] );
		ObjClasses[ oid ] = NULL;
	}

	ctrl_file_name	= cfn;
	display_info	= FALSE;

	ParseCtrlFile( PARSE_OBJECTS );

	LoadObjectData();

	// re-exec startup script
	ExecStartupScript( FALSE );
}


// free memory buffers allocated for data loaded from disk --------------------
//
void FreeLoaderDataBuffers()
{
	if ( BitmapMem != NULL ) {
		FREEMEM( BitmapMem );
		BitmapMem = NULL;
	}

	if ( SampleMem != NULL ) {
		FREEMEM( SampleMem );
		SampleMem = NULL;
	}

	if ( TextureMem != NULL ) {
		FREEMEM( TextureMem );
		TextureMem = NULL;
	}

	if ( PaletteMem != NULL ) {
		FREEMEM( PaletteMem );
		PaletteMem = NULL;
	}
}


// set flags for display of loaded files' names -------------------------------
//
void ShowFilesLoaded( int flag )
{
	show_bitmaps_loaded 	= flag;
	show_textures_loaded	= flag;
	show_objects_loaded 	= flag;
	show_samples_loaded 	= flag;
	show_palettes_loaded	= flag;
}



