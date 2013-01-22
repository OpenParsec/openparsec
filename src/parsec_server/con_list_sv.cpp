/*
 * PARSEC - Server List Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:47 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"
#include "od_props.h"

// global externals
#include "globals.h"
#include "e_world_trans.h"

// subsystem headers
//#include "inp_defs.h"
//#include "vid_defs.h"

// particle types
#include "parttype.h"

// local module header
#include "con_list_sv.h"

// proprietary module headers
#include "con_aux_sv.h"
#include "con_ext_sv.h"
#include "con_info_sv.h"
#include "con_int_sv.h"
#include "con_main_sv.h"
//#include "con_say_sv.h"
#include "con_std_sv.h"
//#include "e_demo.h"
//#include "obj_ctrl.h"
#include "obj_cust.h"
#include "obj_type.h"
#include "sys_file.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 1023
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char filter_too_long[]		= "specified filter too long.";
static char shiplist_empty[]		= "shiplist is empty.";
static char table_name_invalid[]	= "invalid data table name.";
static char table_name_syntax[]		= "syntax: listdata <tablename>.";
static char valid_tables_list[]		= "valid data tables are:";


// list functions administration ----------------------------------------------
//
int		await_keypress		= FALSE;
void	(*com_cont_func)()	= NULL;
int		cur_vis_conlines	= 0;
int		comm_to_print		= 0;

static ShipObject *print_ship_list;


// print single ship info -----------------------------------------------------
//
PRIVATE
void PrintShipInfo( ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	sprintf( paste_str, "objnum:     %u", (unsigned int)shippo->ObjectNumber );
	CON_AddLine( paste_str );
	sprintf( paste_str, "hostobjnum: %u", (unsigned int)shippo->HostObjNumber );
	CON_AddLine( paste_str );
	sprintf( paste_str, "objtype:    %u", (unsigned int)shippo->ObjectType );
	CON_AddLine( paste_str );
	sprintf( paste_str, "objclass:   %u", (unsigned int)shippo->ObjectClass );
	CON_AddLine( paste_str );
	sprintf( paste_str, "x position: %13f", GEOMV_TO_FLOAT( shippo->ObjPosition[ 0 ][ 3 ] ) );
	CON_AddLine( paste_str );
	sprintf( paste_str, "y position: %13f", GEOMV_TO_FLOAT( shippo->ObjPosition[ 1 ][ 3 ] ) );
	CON_AddLine( paste_str );
	sprintf( paste_str, "z position: %13f", GEOMV_TO_FLOAT( shippo->ObjPosition[ 2 ][ 3 ] ) );
	CON_AddLine( paste_str );
	sprintf( paste_str, "damage:     %d", shippo->CurDamage );
	CON_AddLine( paste_str );
	sprintf( paste_str, "speed:      %u", (unsigned int)shippo->CurSpeed );
	CON_AddLine( paste_str );
}


// continue ship info list ----------------------------------------------------
//
PRIVATE
void ShipInfo_ctd()
{
	print_ship_list = (ShipObject *) print_ship_list->NextObj;

	if ( print_ship_list ) {
		CON_AddLine( line_separator );
		PrintShipInfo( print_ship_list );
		if ( print_ship_list->NextObj ) {
			CON_ListCtdPrompt();
		} else {
			CON_ListEndPrompt();
		}
	} else {
		CON_ListEndPrompt();
	}
}


// print ship info list (command "ships") -------------------------------------
//
void Cmd_ShipInfo()
{
	print_ship_list = FetchFirstShip();

	if ( print_ship_list ) {
		PrintShipInfo( print_ship_list );
		if ( print_ship_list->NextObj ) {
			await_keypress = TRUE;
			com_cont_func  = ShipInfo_ctd;
		}
	} else {
		CON_AddLine( shiplist_empty );
	}
}


// determine how much lines can still be output in console --------------------
//
INLINE
int CheckVerticalSpace( int numalreadyoutput )
{
	int vsp = cur_vis_conlines - 1 - numalreadyoutput;
	return ( vsp > 0 ) ? vsp : 0;
}


// print line that is part of a list (generic) --------------------------------
//
PRIVATE
int PrintListLine( int *list_lines_printed, const char *line )
{
	ASSERT( list_lines_printed != NULL );
	ASSERT( line != NULL );

	//NOTE:
	// this function first determines if there is enough
	// vertical space left in the console to print the next
	// (passed in) line. this is done by using the first
	// parameter which has to point to an int parameter that
	// is incremented by this function if it was able to
	// output the line. therefore, the int parameter counts
	// the number of lines already output in the current run.

	if ( *list_lines_printed < cur_vis_conlines - 1 ) {

		// output line and increment externally provided counter
		CON_AddLine( line );
		(*list_lines_printed)++;
		return 1;

	} else {

		// not enough vertical space to output line
		return 0;
	}
}


// check if list needs to be continued (i.e. there is more than one run) ------
//
PRIVATE
void CheckListContinuation( int numitems, void (*contfunc)() )
{
	ASSERT( numitems >= 0 );
	ASSERT( contfunc != NULL );

	if ( comm_to_print != numitems ) {
		await_keypress = TRUE;
		com_cont_func  = contfunc;
	}
}


// check if list has ended and stop list function -----------------------------
//
PRIVATE
void CheckListEnd( int numitems )
{
	ASSERT( numitems >= 0 );

	if ( comm_to_print == numitems )
		CON_ListEndPrompt();
	else
		CON_ListCtdPrompt();
}


// number of listable types and classes ---------------------------------------
//
static int num_listable_types;
static int num_listable_classes;


// continue object type list --------------------------------------------------
//
PRIVATE
void ListTypes_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < num_listable_types ) &&
			( PrintListLine( &cnt, BuildTypeInfo( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListEnd( num_listable_types );
}


// list available object types ("listtypes") ----------------------------------
//
void Cmd_ListTypes()
{
	// list entries already output
	comm_to_print = 0;

	// determine number of types
	num_listable_types = NUM_DISTINCT_OBJTYPES + num_custom_types;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < num_listable_types ) &&
			( PrintListLine( &cnt, BuildTypeInfo( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListContinuation( num_listable_types, ListTypes_ctd );
}


// continue object class list -------------------------------------------------
//
PRIVATE
void ListClasses_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < num_listable_classes ) &&
			( PrintListLine( &cnt, BuildClassInfo( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListEnd( num_listable_classes );
}


// list available object classes ("listclasses") ------------------------------
//
void Cmd_ListClasses()
{
	// list entries already output
	comm_to_print = 0;

	// determine number of classes
	num_listable_classes = NumObjClasses;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < num_listable_classes ) &&
			( PrintListLine( &cnt, BuildClassInfo( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListContinuation( num_listable_classes, ListClasses_ctd );
}

/*
// display info about say macro -----------------------------------------------
//
PRIVATE
int DispSayMacro( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( num >= 0 );

	sprintf( paste_str, "say%d: %s", num, text_macros[ num ] );
	return PrintListLine( cnt, paste_str );
}


// continue command list ------------------------------------------------------
//
PRIVATE
void ListSayMacros_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < NUM_TEXT_MACROS ) &&
			( DispSayMacro( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListEnd( NUM_TEXT_MACROS );
}


// list all available say macros ("listsaymacros") ----------------------------
//
void Cmd_ListSayMacros()
{
	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < NUM_TEXT_MACROS ) &&
			( DispSayMacro( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListContinuation( NUM_TEXT_MACROS, ListSayMacros_ctd );
}
*/

/*
// display info about key binding ---------------------------------------------
//
PRIVATE
int DispKeyBinding( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) &&
			( num < KEY_ADDITIONS_MAX  ) && ( num < KeyAdditional->size ) );

	if ( key_com_mappings[ num ].command != NULL ) {
		const char *fmt = key_com_mappings[ num ].echo ?
			"%s: %s [echo on]" : "%s: %s";
		sprintf( paste_str, fmt,
			GetAKCDescription( num ), key_com_mappings[ num ].command );
		return PrintListLine( cnt, paste_str );
	}

	return 1;
}


// display info about key binding to game function ----------------------------
//
PRIVATE
int DispKeyFunction( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( num >= 0 );

	#define FUNC_DESC_PAD_SIZE 15

	ASSERT( num < NUM_GAMEFUNC_KEYS );

	char *fdesc = GetGameFuncDescription( num );
	ASSERT( fdesc != NULL );
	sprintf( paste_str, "%s: ", fdesc );
	int descpad	= FUNC_DESC_PAD_SIZE - strlen( fdesc );
	char *ppad	= paste_str + strlen( fdesc ) + 2;
	while ( descpad-- > 0 )
		*ppad++ = ' ';
	strcpy( ppad, GetMKCDescription( num ) );

	#undef FUNC_DESC_PAD_SIZE

	return PrintListLine( cnt, paste_str );
}
*/

// store alphabetic start sequence --------------------------------------------
//
#define MAX_ALPHA_START 32
static char alphabetic_start[ MAX_ALPHA_START + 1 ];


// print command only if starting with a certain alphabetic sequence ----------
//
PRIVATE
int PrintFilteredCommand( int *count, const char *line )
{
	ASSERT( count != NULL );
	ASSERT( line != NULL );

	if ( alphabetic_start[ 0 ] != 0 ) {
		if ( strnicmp( alphabetic_start, line, strlen( alphabetic_start ) ) != 0 ) {
			return 1;
		}
	}

	return PrintListLine( count, line );
}


// init alphabetic sequence ---------------------------------------------------
//
PRIVATE
int InitAlphabeticStart( char *scan )
{
	if ( scan != NULL ) {

		if ( *(scan - 1) != ' ' ) {
			CON_AddLine( unknown_command );
			return FALSE;
		}

		if ( !strtok( NULL, " " ) ) {
			if ( strlen( scan ) > MAX_ALPHA_START ) {
				CON_AddLine( filter_too_long );
				return FALSE;
			}
			strcpy( alphabetic_start, scan );
		} else {
			CON_AddLine( too_many_args );
			return FALSE;
		}

	} else {
		alphabetic_start[ 0 ] = 0;
	}

	return TRUE;
}


// print line from directory listing annotated with entry type ----------------
//
PRIVATE
int PrintDirLine( int *iref, int comno )
{
	ASSERT( iref != NULL );
	ASSERT( ( comno >= 0 ) && ( comno < num_external_commands ) );

	#define FIELD_SIZE 9

	strcpy( paste_str, ECMSTR( comno ) );

	int len     = ECMLEN( comno );
	int padsize = ( len < FIELD_SIZE ) ? ( FIELD_SIZE - len ) : 0;

	while ( padsize-- > 0 ) {
		paste_str[ len++ ] = ' ';
	}
	paste_str[ len ] = 0;

	#undef FIELD_SIZE

	switch ( external_command_types[ comno ] ) {

		case COMTYPE_PACKAGE:
			if ( SV_PACKAGE_SCRIPT_LISTING ) {
				// allow exclusion of package scripts from list
				return 1;
			}
			strcat( paste_str, " (pack)" );
			break;

		case COMTYPE_ROOT:
			strcat( paste_str, " (root)" );
			break;

		case COMTYPE_REFERENCE:
			strcat( paste_str, " (ref)" );
			break;

		case COMTYPE_STANDARD:
			strcat( paste_str, " (std)" );
			break;

		case COMTYPE_RECORDING:
			strcat( paste_str, " (rec)" );
			break;

		case COMTYPE_MODIFICATION:
			strcat( paste_str, " (mod)" );
			break;
	}

	return PrintFilteredCommand( iref, paste_str );
}


// continue command list ------------------------------------------------------
//
PRIVATE
void ListExternalCommands_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < num_external_commands ) &&
			( PrintDirLine( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListEnd( num_external_commands );
}


// list all available external console commands ("ls") ------------------------
//
int Cmd_ListExternalCommands( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );

	char *scan = strtok( command, " " );
	if ( !InitAlphabeticStart( scan ) ) {
		// recognize always (implicitly reserves domain)
		return TRUE;
	}

	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < num_external_commands ) &&
			( PrintDirLine( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListContinuation( num_external_commands, ListExternalCommands_ctd );

	return TRUE;
}

/*
// continue command list ------------------------------------------------------
//
PRIVATE
void ListKeyBindings_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < KeyAdditional->size ) &&
			( DispKeyBinding( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListEnd( KeyAdditional->size );
}


// list all additional key bindings ("listbindings") --------------------------
//
void Cmd_ListKeyBindings()
{
	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < KeyAdditional->size ) &&
			( DispKeyBinding( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListContinuation( KeyAdditional->size, ListKeyBindings_ctd );
}


// list all keys bound to game functions --------------------------------------
//
PRIVATE
void ListKeyFunctions_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < NUM_GAMEFUNC_KEYS ) &&
			( DispKeyFunction( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListEnd( NUM_GAMEFUNC_KEYS );
}


// list all keys bound to game functions ("listgamefunckeys") -----------------
//
void Cmd_ListKeyFunctions()
{
	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < NUM_GAMEFUNC_KEYS ) &&
			( DispKeyFunction( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListContinuation( NUM_GAMEFUNC_KEYS, ListKeyFunctions_ctd );
}


// display single video mode information --------------------------------------
//
PRIVATE
int DispVidMode( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( num >= 0 );

	if ( VID_MODE_AVAILABLE( num ) ) {

		short bytesperscanline = HiresModes[ num ].bytesperscanline;
		short colbits		   = HiresModes[ num ].colbits;

		if ( HiresModes[ num ].flags & HIRESMODEFLAG_VISUALINFO ) {
			bytesperscanline = 0;
			colbits			 = HiresModes[ num ].visual_colbits;
		}

		if ( bytesperscanline > 0 ) {
			sprintf( paste_str, "  mode: %4d x %4d %2dbpp %4dbpsl",
				 	 HiresModes[ num ].xresolution,
				 	 HiresModes[ num ].yresolution,
				 	 colbits, bytesperscanline );
		} else {
			sprintf( paste_str, "  mode: %4d x %4d %2dbpp",
				 	 HiresModes[ num ].xresolution,
				 	 HiresModes[ num ].yresolution,
				 	 colbits );
		}

		if ( VID_MODE_WINDOWED( num ) ) {
			strcat( paste_str, " (windowed)" );
		} else {
			// display refresh rate if available (and not default)
		}
		if ( num == VidModeIndex ) {
			paste_str[ 0 ] = '*';
		}

		return PrintListLine( cnt, paste_str );
	}

	return 1;
}


// continue video mode list ----------------------------------------------------
//
PRIVATE
void ListVidModes_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < MAX_NUM_GRAPH_MODES ) &&
			( DispVidMode( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListEnd( MAX_NUM_GRAPH_MODES );
}


// list all available video modes ("vid.listmodes") ---------------------------
//
void Cmd_ListVidModes()
{
	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < MAX_NUM_GRAPH_MODES ) &&
			( DispVidMode( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListContinuation( MAX_NUM_GRAPH_MODES, ListVidModes_ctd );
}


// display info about registered demo -----------------------------------------
//
PRIVATE
int DispDemoInfo( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < num_registered_demos ) );

	if ( registered_demo_names[ num ] != NULL ) {
		if ( registered_demo_titles[ num ] != NULL ) {
			sprintf( paste_str, "%s (%s)",
					 registered_demo_names[ num ],
					 registered_demo_titles[ num ] );
			return PrintListLine( cnt, paste_str );
		} else {
			sprintf( paste_str, "%s", registered_demo_names[ num ] );
			return PrintListLine( cnt, paste_str );
		}
	}

	return 1;
}


// continue demo list ----------------------------------------------------------
//
PRIVATE
void ListBinaryDemos_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < num_registered_demos ) &&
			( DispDemoInfo( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListEnd( num_registered_demos );
}


// list all registered demos ("listdemos") ------------------------------------
//
void Cmd_ListBinaryDemos()
{
	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < num_registered_demos ) &&
			( DispDemoInfo( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListContinuation( num_registered_demos, ListBinaryDemos_ctd );
}
*/

// display single table line of objects table (object classes) ----------------
//
PRIVATE
int DispObjectEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < NumLoadedObjects ) );

	int numlines = 2;
	if ( ObjectInfo[ num ].lodinfo != NULL ) {
		ASSERT( ObjectInfo[ num ].lodinfo->numlods > 0 );
		numlines += ObjectInfo[ num ].lodinfo->numlods - 1;
	}

	if ( CheckVerticalSpace( *cnt ) < numlines ) {
		// no space to output n consecutive lines
		return 0;
	}

	#define TEMP_TYPE_STRING_MAX 31

	const char *typestring = NULL;

	// scan table of object type ids to find type description
	char temp_string[ TEMP_TYPE_STRING_MAX + 1 ];
	for ( int tid = 0; tid < NUM_DISTINCT_OBJTYPES; tid++ ) {
		if ( objtype_id[ tid ] == ObjectInfo[ num ].type ) {
			const char *tstr = objtype_name[ tid ];
			ASSERT( tstr != NULL );
			// strip leading asterisk
			if ( *tstr == '*' )
				tstr++;
			strncpy( temp_string, tstr, TEMP_TYPE_STRING_MAX );
			temp_string[ TEMP_TYPE_STRING_MAX ] = 0;
			int tlen = strlen( temp_string );
			ASSERT( tlen > 0 );
			// strip trailing asterisk
			if ( temp_string[ tlen - 1 ] == '*' )
				temp_string[ tlen - 1 ] = 0;
			typestring = temp_string;
			break;
		}
	}

	#undef TEMP_TYPE_STRING_MAX

	// try custom type if not already found
	if ( typestring == NULL ) {
		typestring = OBJ_FetchCustomTypeName( ObjectInfo[ num ].type );
	}

	if ( typestring != NULL ) {
		sprintf( paste_str, "id %d object (%s) type %s (%u)", num,
				 ObjectInfo[ num ].name, typestring,
				 ObjectInfo[ num ].type & TYPENUMBERMASK );
	} else {
		sprintf( paste_str, "id %d object (%s) type %08x_h (%u)", num,
				 ObjectInfo[ num ].name, (unsigned int)ObjectInfo[ num ].type,
				 ObjectInfo[ num ].type & TYPENUMBERMASK );
	}

	ProcessExternalLine( paste_str );
	int rc = PrintListLine( cnt, paste_str );
	ASSERT( rc );

	if ( ObjectInfo[ num ].lodinfo == NULL ) {

		sprintf( paste_str, "   %d file %s", num, ObjectInfo[ num ].file );
		ProcessExternalLine( paste_str );
		rc = PrintListLine( cnt, paste_str );
		ASSERT( rc );

	} else {

		lodinfo_s *lodinfo = ObjectInfo[ num ].lodinfo;
		for ( int lod = 0; lod < lodinfo->numlods; lod++ ) {

			if ( lod < lodinfo->numlods - 1 ) {
				sprintf( paste_str, "   %d file %s lodmag %f lodmin %f", num,
					lodinfo->filetab[ lod ],
					GEOMV_TO_FLOAT( lodinfo->lodmags[ lod ] ),
					GEOMV_TO_FLOAT( lodinfo->lodmins[ lod ] ) );
			} else {
				sprintf( paste_str, "   %d file %s", num,
					lodinfo->filetab[ lod ] );
			}
			ProcessExternalLine( paste_str );
			rc = PrintListLine( cnt, paste_str );
			ASSERT( rc );
		}
	}

	return rc;
}

/*
// display single table line of textures table --------------------------------
//
PRIVATE
int DispTextureEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < NumLoadedTextures ) );

	if ( CheckVerticalSpace( *cnt ) < 2 ) {
		// no space to output two consecutive lines
		return 0;
	}

	sprintf( paste_str, "id %d texture (%s) width %d height %d", num,
			 TextureInfo[ num ].name,
			 TextureInfo[ num ].width,
			 TextureInfo[ num ].height );
	ProcessExternalLine( paste_str );
	int rc = PrintListLine( cnt, paste_str );
	ASSERT( rc );

	sprintf( paste_str, "   %d file %s", num, TextureInfo[ num ].file );
	ProcessExternalLine( paste_str );

	return PrintListLine( cnt, paste_str );
}
*/

/*
// display single table line of bitmaps table ---------------------------------
//
PRIVATE
int DispBitmapEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < NumLoadedBitmaps ) );

	sprintf( paste_str, "id %d bitmap (%s) width %d height %d file %s", num,
			 BitmapInfo[ num ].name,
			 BitmapInfo[ num ].width,
			 BitmapInfo[ num ].height,
			 BitmapInfo[ num ].file );
	ProcessExternalLine( paste_str );
	return PrintListLine( cnt, paste_str );
}
*/

/*
// display single table line of samples table ---------------------------------
//
PRIVATE
int DispSampleEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < NumLoadedSamples ) );

	sprintf( paste_str, "id %d sample (%s) file %s volume %d", num,
			 SampleInfo[ num ].name,
			 SampleInfo[ num ].file,
			 SampleInfo[ num ].volume );
	ProcessExternalLine( paste_str );
	return PrintListLine( cnt, paste_str );
}
*/

// exported by PART_API.C -----------------------------------------------------
//
extern int			NumParticleDefinitions;
extern pdefref_s	ParticleDefinitions[];

/*
// display single particle definition -----------------------------------------
//
PRIVATE
int DispPDefEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < NumParticleDefinitions ) );

	ASSERT( ParticleDefinitions[ num ].defname != NULL );
	ASSERT( ParticleDefinitions[ num ].def != NULL );
	pdef_s *pdef = ParticleDefinitions[ num ].def;

	sprintf( paste_str, "id %d pdef (%s) texframes %d xfoframes %d", num,
			 ParticleDefinitions[ num ].defname,
			 pdef->tex_table ? pdef->tex_end + 1 : 0,
			 pdef->xfo_table ? pdef->xfo_end + 1 : 0 );
	ProcessExternalLine( paste_str );
	return PrintListLine( cnt, paste_str );
}
*/

/*
// display single shader ------------------------------------------------------
//
PRIVATE
int DispShaderEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < num_registered_shaders ) );

	shader_s *shader = &registered_shaders[ num ];
	ASSERT( shader->name != NULL );

	sprintf( paste_str, "id %d name (%s) shader (%s)", num,
			 shader->name, GetShaderDescription( num ) );

	// display face color after shader only if no colanim defined
	if ( ( shader->flags & FACE_SHADING_FACECOLOR ) && ( shader->colanim == NULL ) ) {
		int endpos = strlen( paste_str );
		sprintf( paste_str + endpos, " color (%d %d %d %d)",
				 shader->facecolor.R, shader->facecolor.G,
				 shader->facecolor.B, shader->facecolor.A );
	}

	if ( shader->texanim != NULL ) {
		strcat( paste_str, " texanim (" );

		for ( int pid = 0; pid < NumParticleDefinitions; pid++ ) {
			if ( shader->texanim == ParticleDefinitions[ pid ].def ) {
				strcat( paste_str, ParticleDefinitions[ pid ].defname );
				break;
			}
		}
		if ( pid == NumParticleDefinitions ) {
			strcat( paste_str, "unregistered" );
		}

		strcat( paste_str, ")" );
	}

	if ( shader->colanim != NULL ) {
		strcat( paste_str, " colanim (" );

		for ( int cid = 0; cid < num_registered_colanims; cid++ ) {
			if ( shader->colanim == &registered_colanims[ cid ].colanim ) {
				strcat( paste_str, registered_colanims[ cid ].name );
				break;
			}
		}
		if ( cid == num_registered_colanims ) {
			strcat( paste_str, "unregistered" );
		}

		strcat( paste_str, ")" );

		if ( shader->flags & FACE_SHADING_FACECOLOR ) {
			// display face color only if it will not be overwritten by colanim
			if ( ( shader->colflags & FACE_ANIM_BASE_MASK ) != FACE_ANIM_BASEIGNORE ) {
				int endpos = strlen( paste_str );
				sprintf( paste_str + endpos, " color (%d %d %d %d)",
						 shader->facecolor.R, shader->facecolor.G,
						 shader->facecolor.B, shader->facecolor.A );
			}
		} else {
			// if face color is disabled colanim will not be
			// visible if the face is textured
			strcat( paste_str, " [facecolor disabled]" );
		}
	}

	ProcessExternalLine( paste_str );
	return PrintListLine( cnt, paste_str );
}
*/

/*
// display single colanim -----------------------------------------------------
//
PRIVATE
int DispColAnimEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < num_registered_colanims ) );

	ASSERT( registered_colanims[ num ].name != NULL );
	colanim_s *colanim = &registered_colanims[ num ].colanim;

	sprintf( paste_str, "id %d colanim (%s) coltab0 %d coltab1 %d", num,
			 registered_colanims[ num ].name,
			 colanim->col_table0 ? colanim->col_end0 + 1 : 0,
			 colanim->col_table1 ? colanim->col_end1 + 1 : 0 );
	ProcessExternalLine( paste_str );
	return PrintListLine( cnt, paste_str );
}
*/

// display single data package ------------------------------------------------
//
PRIVATE
int DispPackageEntry( int *cnt, int num )
{
	ASSERT( cnt != NULL );
	ASSERT( ( num >= 0 ) && ( num < num_data_packages ) );

	ASSERT( package_filename[ num ] != NULL );

	sprintf( paste_str, "id %d package (%s)", num, package_filename[ num ] );
	ProcessExternalLine( paste_str );
	return PrintListLine( cnt, paste_str );
}


// data table descriptors -----------------------------------------------------
//
typedef int (*dtpf_t)(int*,int);

struct datatable_desc_s {

	const char* tablename;
	int*	numentries;
	dtpf_t	printfunc;
	int		_padto_16;
};

static datatable_desc_s datatable_desc[] = {

	{ "objects",	&NumLoadedObjects,			DispObjectEntry,	0	},
	//{ "textures",	&NumLoadedTextures,			DispTextureEntry,	0	},
	//{ "bitmaps",	&NumLoadedBitmaps,			DispBitmapEntry,	0	},
	//{ "samples",	&NumLoadedSamples,			DispSampleEntry,	0	},
	//{ "pdefs",	&NumParticleDefinitions,	DispPDefEntry,		0	},
	//{ "shaders",	&num_registered_shaders,	DispShaderEntry,	0	},
	//{ "colanims",	&num_registered_colanims,	DispColAnimEntry,	0	},
	{ "packages",	&num_data_packages,			DispPackageEntry,	0	},
};

#define NUM_DATATABLES		CALC_NUM_ARRAY_ENTRIES( datatable_desc )

static int		datatable_entries;
static dtpf_t	datatable_entry_print;


// continue data table list ---------------------------------------------------
//
PRIVATE
void ListDataTable_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < datatable_entries ) &&
			( datatable_entry_print( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListEnd( datatable_entries );
}


// display list of valid data table names -------------------------------------
//
PRIVATE
void ListValidDataTables()
{
	CON_AddLine( valid_tables_list );

	for ( unsigned int tid = 0; tid < NUM_DATATABLES; tid++ ) {
		paste_str[ 0 ] = ' ';
		strcpy( paste_str + 1, datatable_desc[ tid ].tablename );
		CON_AddLine( paste_str );
	}
}


// list contents of data table ("listdata <tab>") -----------------------------
//
int Cmd_ListDataTable( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( command );

	// isolate table name
	char *tabname = strtok( command, " " );
	if ( tabname == NULL ) {
		CON_AddLine( table_name_syntax );
		ListValidDataTables();
		return TRUE;
	}
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( table_name_syntax );
		return TRUE;
	}
    unsigned int tid;
	// scan valid table names
	for ( tid = 0; tid < NUM_DATATABLES; tid++ ) {
		if ( strcmp( datatable_desc[ tid ].tablename, tabname ) == 0 ) {
			datatable_entries     = *datatable_desc[ tid ].numentries;
			datatable_entry_print = datatable_desc[ tid ].printfunc;
			break;
		}
	}

	// check if table name found
	if ( tid == NUM_DATATABLES ) {
		CON_AddLine( table_name_invalid );
		ListValidDataTables();
		return TRUE;
	}

	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < datatable_entries ) &&
			( datatable_entry_print( &cnt, comm_to_print ) ) )
		comm_to_print++;

	CheckListContinuation( datatable_entries, ListDataTable_ctd );

	return TRUE;
}


// continue command list (integer argument commands) --------------------------
//
PRIVATE
void ListIntCommands_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < num_int_commands ) &&
			( PrintFilteredCommand( &cnt, ICMSTR( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListEnd( num_int_commands );
}


// list all available integer argument commands ("listintcommands") -----------
//
int Cmd_ListIntCommands( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );

	char *scan = strtok( command, " " );
	if ( !InitAlphabeticStart( scan ) ) {
		// recognize always (implicitly reserves domain)
		return TRUE;
	}

	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < num_int_commands ) &&
			( PrintFilteredCommand( &cnt, ICMSTR( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListContinuation( num_int_commands, ListIntCommands_ctd );

	return TRUE;
}


// continue command list (standard commands) ----------------------------------
//
PRIVATE
void ListStdCommands_ctd()
{
	// continued run
	int cnt = 0;
	while ( ( comm_to_print < num_std_commands ) &&
			( PrintFilteredCommand( &cnt, CMSTR( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListEnd( num_std_commands );
}


// list all available standard console commands ("listcommands") --------------
//
int Cmd_ListStdCommands( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );

	char *scan = strtok( command, " " );
	if ( !InitAlphabeticStart( scan ) ) {
		// recognize always (implicitly reserves domain)
		return TRUE;
	}

	// list entries already output
	comm_to_print = 0;

	// first run
	int cnt = 0;
	while ( ( comm_to_print < num_std_commands ) &&
			( PrintFilteredCommand( &cnt, CMSTR( comm_to_print ) ) ) )
		comm_to_print++;

	CheckListContinuation( num_std_commands, ListStdCommands_ctd );

	return TRUE;
}



