/*
 * PARSEC - Common Commands
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
#include "od_props.h"

// global externals
#include "globals.h"
#include "e_world_trans.h"

// subsystem headers
//#include "aud_defs.h"
//#include "inp_defs.h"
#include "net_defs.h"
//#include "vid_defs.h"

// local module header
#include "con_com_sv.h"

// proprietary module headers
//#include "con_act.h"
#include "con_arg.h"
#include "con_aux_sv.h"
//#include "con_cani.h"
//#include "con_conf.h"
#include "con_ext_sv.h"
#include "con_info_sv.h"
#include "con_int_sv.h"
#include "con_list_sv.h"
#include "con_load_sv.h"
#include "con_main_sv.h"
//#include "con_say.h"
//#include "con_shad.h"
#include "con_std_sv.h"
//#include "e_demo.h"
//#include "e_supp.h"
//#include "g_supp.h"
//#include "h_supp.h"
#include "inp_main_sv.h"
//#include "m_main.h"
//#include "net_conn.h"
//#include "obj_ctrl.h"
#include "e_gameserver.h"
#include "sys_date.h"
#include "sys_path.h"
//#include "vid_init.h"
//#include "vid_plug.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char base_str[]				= "base is set to dec.";
#define BASE_INSERT_POS				15
static char base_dec[]				= "dec.";
static char base_hex[]				= "hex.";
static char base_oct[]				= "oct.";
static char base_set_to_dec[]		= "base set to dec.";
static char base_set_to_hex[]		= "base set to hex.";
static char base_set_to_oct[]		= "base set to oct.";
static char info_sep_line[]			= "-----------------------------------------------";
PUBLIC char dont_use_params[]		= "this command takes no parameters.";
static char server_name_missing[]	= "server must be supplied.";
static char mode_number_invalid[]	= "invalid mode number.";
static char mode_spec_invalid[]		= "invalid mode specifier. use something like \"640x480x32\".";
static char mode_spec_unavail[]		= "this mode is not available in the current video subsystem.";
static char mode_modifier_invalid[]	= "invalid mode modifier. use \"fullscreen\" or \"windowed\".";
static char only_alpha_chars[]		= "name must not contain non-alphabetic characters.";
static char name_change_preempted[]	= "name change preempted. please try again.";


// remove all objects (command "objectclear") ---------------------------------
//
PRIVATE
void Cmd_OBJECTCLEAR()
{
	// free objects and particles
	KillAllObjects();
}


// print object counts for all object lists (command "objectcount") -----------
//
PRIVATE
void Cmd_OBJECTCOUNT()
{
	sprintf( paste_str, "ships   :%3d", TheWorld->GetNumShipObjects() );
	CON_AddLine( paste_str );
	sprintf( paste_str, "lasers  :%3d", TheWorld->GetNumLaserObjects() );
	CON_AddLine( paste_str );
	sprintf( paste_str, "missiles:%3d", TheWorld->GetNumMisslObjects() );
	CON_AddLine( paste_str );
	sprintf( paste_str, "extras  :%3d (obj, counted %d) and %d particle", TheWorld->GetNumExtraObjects(), TheWorld->m_nCurrentNumExtras, TheWorld->m_nCurrentNumPrtExtras );
	CON_AddLine( paste_str );
	sprintf( paste_str, "customs :%3d", TheWorld->GetNumCustmObjects() );
	CON_AddLine( paste_str );
}


// display heap info (command "heap") -----------------------------------------
//
#ifdef PARSEC_DEBUG
PRIVATE
int Cmd_HEAP( char *params )
{
	ASSERT( params != NULL );
	USERCOMMAND_NOPARAM( params );

	extern unsigned int Dyn_Mem_Size;
	extern unsigned int Num_Allocs;
	extern unsigned int Num_Frees;

	sprintf( paste_str, "heapsize: %.2fmb :%d", (float) Dyn_Mem_Size / ( 1024 * 1024 ), Dyn_Mem_Size );
	CON_AddLine( paste_str );
	sprintf( paste_str, "memobjs : %d", Num_Allocs - Num_Frees );
	CON_AddLine( paste_str );

	return TRUE;
}
#endif // PARSEC_DEBUG


// show position of local ship (command "showposition") -----------------------
//
PRIVATE
void Cmd_SHOWPOSITION()
{
	//FIXME: we need a shipindex parameter here

	/*sprintf( paste_str, "x position: %13f", GEOMV_TO_FLOAT( MyShip->ObjPosition[ 0 ][ 3 ] ) );
	CON_AddLine( paste_str );

	sprintf( paste_str, "y position: %13f", GEOMV_TO_FLOAT( MyShip->ObjPosition[ 1 ][ 3 ] ) );
	CON_AddLine( paste_str );

	sprintf( paste_str, "z position: %13f", GEOMV_TO_FLOAT( MyShip->ObjPosition[ 2 ][ 3 ] ) );
	CON_AddLine( paste_str );*/
}

// open output script for writing (don't overwrite existing file) -------------
//
PRIVATE
int Cmd_Create( char *command )
{
	return OpenOutputBatch( command, CM_CREATE );
}


// open output script for writing (overwrite existing file) -------------------
//
PRIVATE
int Cmd_Open( char *command )
{
	return OpenOutputBatch( command, CM_OPEN );
}


// append to output script ----------------------------------------------------
//
PRIVATE
int Cmd_Append( char *command )
{
	return OpenOutputBatch( command, CM_APPEND );
}


// record demo script after saving state --------------------------------------
//
PRIVATE
int Cmd_Recdem( char *command )
{
	return StartRecording( command, TRUE );
}


// record demo script without saving state first ------------------------------
//
PRIVATE
int Cmd_Record( char *command )
{
	return StartRecording( command, FALSE );
}


// print parsec build info (command "build") ----------------------------------
//
PRIVATE
int Cmd_BuildInfo( char *cstr )
{
	ASSERT( cstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( cstr );

	// split off first token (detail)
	char *dstr = strtok( cstr, " " );
	if ( dstr == NULL ) {
		// display basic build info
		CON_AddLine( build_text );
		CON_AddLine( build_date );
		CON_AddLine( build_time );
		return TRUE;
	}

	// check if more than one parameter
	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( too_many_args );
		return TRUE;
	}

	// check for detail spec
	if ( strcmp( dstr, "x" ) == 0 ) {

		CON_AddLine( info_sep_line );
		CON_AddLine( build_text );
		CON_AddLine( info_sep_line );
		CON_AddLine( build_date );
		CON_AddLine( build_time );
		CON_AddLine( info_sep_line );
		//DisplayCurrentNetSubsys();
		CON_AddLine( info_sep_line );
		CON_AddLine( build_comp );
		CON_AddLine( build_bind );
		CON_AddLine( build_endn );
		CON_AddLine( info_sep_line );

	} else {

		CON_AddLine( invalid_arg );
	}

	return TRUE;
}


// exec display command (display arbitrary text in message area) --------------
//
PRIVATE
int Cmd_DisplayText( char *cstr )
{
	ASSERT( cstr != NULL );
	HANDLE_COMMAND_DOMAIN( cstr );

	const char *scan = GetStringBehindCommand( cstr, FALSE );

	if ( ( scan != NULL ) && ( strlen( scan ) <= MAX_MESSAGELEN ) ) {
		//FIXME: evt. precede text with "MESSAGE: "
		MSGOUT( scan );
	}

	// recognize always (implicitly reserves domain)
	return TRUE;
}


// exec write command (display arbitrary text in console) ---------------------
//
PRIVATE
int Cmd_WriteText( char *cstr )
{
	//NOTE:
	// this command normally only makes sense if
	// used from within a console script.

	ASSERT( cstr != NULL );
	HANDLE_COMMAND_DOMAIN( cstr );

	const char *scan = GetStringBehindCommand( cstr, TRUE );

	// output string in console (new-line possible)
	if ( scan != NULL ) {
		MSGPUT( "\n::%s", scan );
	}

	// recognize always (implicitly reserves domain)
	return TRUE;
}


// set current working directory (general use) --------------------------------
//
PRIVATE
int Cmd_SetPathWorkingDirectory( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );

	if ( SetSingleStringCommand( command, CurWorkDir, PATH_MAX ) ) {

		// process file name
		char *path = SYSs_ProcessPathString( CurWorkDir );
		if ( path != CurWorkDir ) {
			strncpy( CurWorkDir, path, PATH_MAX );
			CurWorkDir[ PATH_MAX ] = 0;
		}
	}

	// recognize always (implicitly reserves domain)
	return TRUE;
}


// set file path for intro animation ------------------------------------------
//
PRIVATE
int Cmd_SetPathAnimIntro( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );
/*
	extern char demo_movie_name[];
	if ( SetSingleStringCommand( command, demo_movie_name, PATH_MAX ) ) {

		// process file name and use current workdir
		strcpy( demo_movie_name, SubstCurWorkDir( demo_movie_name ) );
	}
*/
	// recognize always (implicitly reserves domain)
	return TRUE;
}


// set file path for all audio streams ----------------------------------------
//
PRIVATE
int Cmd_SetPathAudioStreams( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );
/*
	if ( SetSingleStringCommand( command, console_audio_stream_path, PATH_MAX ) ) {

		// process file name and use current workdir
		strcpy( console_audio_stream_path, SubstCurWorkDir( console_audio_stream_path ) );
	}
*/
	// recognize always (implicitly reserves domain)
	return TRUE;
}


// set file path for intro audio stream ---------------------------------------
//
PRIVATE
int Cmd_SetPathAudioStreamIntro( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );
/*
	if ( SetSingleStringCommand( command, intro_stream_name, PATH_MAX ) ) {

		// process file name and use current workdir
		strcpy( intro_stream_name, SubstCurWorkDir( intro_stream_name ) );
	}
*/
	// recognize always (implicitly reserves domain)
	return TRUE;
}


// set file path for menu audio stream ----------------------------------------
//
PRIVATE
int Cmd_SetPathAudioStreamMenu( char *command )
{
	ASSERT( command != NULL );
	HANDLE_COMMAND_DOMAIN( command );

/*
	if ( SetSingleStringCommand( command, menu_stream_name, PATH_MAX ) ) {

		// process file name and use current workdir
		strcpy( menu_stream_name, SubstCurWorkDir( menu_stream_name ) );
	}
*/

	// recognize always (implicitly reserves domain)
	return TRUE;
}


// init console talk mode -----------------------------------------------------
//
PRIVATE
void InitTalkMode()
{
	ASSERT( con_in_talk_mode );

	// clear screen, set cursor to screen-bottom line
	ClearConScreen();

	// create separator line
	CreateSeparatorLine( con_bottom );

	// save number of line right below separator line
	CON_AddLineFeed();
	CON_DisableLineFeed();
	con_talk_line = con_bottom;
}


// command "cls" --------------------------------------------------------------
//
PRIVATE
void Cmd_CLS()
{
	if ( con_in_talk_mode ) {

		// reactivate talkmode
		InitTalkMode();

	} else {
		wclear( g_curses_out_win );
	}
}


// command "talk" (toggle talk mode on and off) -------------------------------
//
PRIVATE
void Cmd_TALK()
{
	// toggle talk mode
	con_in_talk_mode = !con_in_talk_mode;

	if ( con_in_talk_mode ) {
		// talkmode init clears screen
		InitTalkMode();
	} else {
		// clear screen
		Cmd_CLS();
	}
}


// command "quit" -------------------------------------------------------------
//
PRIVATE
void Cmd_QUIT()
{
	TheServer->SetQuitFlag();
}


// command "hex" --------------------------------------------------------------
//
PRIVATE
void Cmd_HEX()
{
	int_print_base = "%x";
	int_calc_base  = 16;
	CON_AddLine( base_set_to_hex );
	strcpy( base_str + BASE_INSERT_POS, base_hex );
}


// command "dec" --------------------------------------------------------------
//
PRIVATE
void Cmd_DEC()
{
	int_print_base = "%d";
	int_calc_base  = 10;
	CON_AddLine( base_set_to_dec );
	strcpy( base_str + BASE_INSERT_POS, base_dec );
}


// command "oct" --------------------------------------------------------------
//
PRIVATE
void Cmd_OCT()
{
	int_print_base = "%o";
	int_calc_base  = 8;
	CON_AddLine( base_set_to_oct );
	strcpy( base_str + BASE_INSERT_POS, base_oct );
}


// command "base" -------------------------------------------------------------
//
PRIVATE
void Cmd_BASE()
{
	CON_AddLine( base_str );
}

// close network connection ("disconnect") ------------------------------------
//
PRIVATE
void Cmd_DISCONNECT()
{
}


// commands corresponding to functions with no parameters ---------------------
//
struct cmd_noparam_s {

	int		commandid;
	void	(*commandfunc)(void);
};

cmd_noparam_s cmd_noparam[] = {

	//{ CM_REPINFO,				Cmd_QueryReplayInfo	   		},
	//{ CM_REPSTOP,				Cmd_StopBatchReplay			},
	{ CM_CLS,					Cmd_CLS						},
	{ CM_CLEAR,					Cmd_CLS						},
	{ CM_QUIT,					Cmd_QUIT            		},
	{ CM_EXIT,					Cmd_QUIT            		},
	{ CM_HEX,					Cmd_HEX             		},
	{ CM_DEC,					Cmd_DEC             		},
	{ CM_OCT,					Cmd_OCT						},
	{ CM_BASE,					Cmd_BASE		   			},
	{ CM_OBJECTCLEAR,			Cmd_OBJECTCLEAR				},
	{ CM_OBJECTCOUNT,			Cmd_OBJECTCOUNT				},
	{ CM_SHIPS,					Cmd_ShipInfo				},
	{ CM_LISTCLASSES,			Cmd_ListClasses				},
	{ CM_LISTTYPES,				Cmd_ListTypes				},
	//{ CM_LISTTEXTMACS,			Cmd_ListSayMacros			},
	//{ CM_LISTBINDINGS,			Cmd_ListKeyBindings			},
	//{ CM_SETORIGIN,				Cmd_SetShipOrigin	   		},
	{ CM_SHOWPOSITION,			Cmd_SHOWPOSITION			},
	//{ CM_CLOSE,					Cmd_CloseOutputBatch		},
	//{ CM_LISTGAMEFUNCKEYS,		Cmd_ListKeyFunctions		},
	{ CM_DISCONNECT,			Cmd_DISCONNECT				},
	//{ CM_TALKMODE,				Cmd_TALK					},
	{ CM_RESCAN,				Cmd_RescanExternalCommands	},
	//{ CM_LISTDEMOS,				Cmd_ListBinaryDemos			},
};

#define NUM_CMDS_NOPARAM		CALC_NUM_ARRAY_ENTRIES( cmd_noparam )


// commands corresponding to functions with no parameters ---------------------
//
int CheckCmdsNoParam( char *command )
{
	//NOTE:
	// this function returns TRUE if a valid command
	// has been detected and executed. it returns
	// FALSE if no valid command has been detected
	// and the command string therefore needs
	// further processing by the main dispatcher.

	ASSERT( command != NULL );

	// check for commands in "no param" table
	for ( unsigned int tid = 0; tid < NUM_CMDS_NOPARAM; tid++ ) {

		const char *cmdstr = CMSTR( cmd_noparam[ tid ].commandid );
		int  cmdlen  = CMLEN( cmd_noparam[ tid ].commandid );

		if ( strncmp( command, cmdstr, cmdlen ) == 0 ) {

			// eat trailing whitespace
			char *whsp = command + cmdlen;
			for ( ; *whsp; whsp++ )
				if ( *whsp != ' ' )
					break;
			if ( *whsp == 0 ) {
				// only whitespace -> command valid
				(*cmd_noparam[ tid ].commandfunc)();
				return TRUE;
			} else if ( command[ cmdlen ] == ' ' ) {
				// excess parameters
				CON_AddLine( dont_use_params );
				return TRUE;
			}
		}
	}

	return FALSE;
}


// commands corresponding to functions with an arbitrary string parameter -----
//
struct cmd_customstring_s {

	int		commandid;
	int		(*commandfunc)(char*);
};

cmd_customstring_s cmd_customstring[] = {

	{ CM_BUILD,					Cmd_BuildInfo				},
	//{ CM_PATH_ANIM_INTRO,		Cmd_SetPathAnimIntro		},
	//{ CM_PATH_ASTREAM_INTRO,	Cmd_SetPathAudioStreamIntro	},
	//{ CM_PATH_ASTREAM_MENU,		Cmd_SetPathAudioStreamMenu	},
	//{ CM_PATH_ASTREAM,			Cmd_SetPathAudioStreams		},
	{ CM_WORKINGDIRECTORY,		Cmd_SetPathWorkingDirectory	},
	{ CM_LISTINTCOMMANDS,		Cmd_ListIntCommands			},
	{ CM_LISTSTDCOMMANDS,		Cmd_ListStdCommands			},
	{ CM_LISTEXTCOMMS,			Cmd_ListExternalCommands	},
	{ CM_DISPLAY,				Cmd_DisplayText				},
	{ CM_CAT,					Cmd_CatExternalCommands		},
	//{ CM_GAMELOOPBATCH,			Cmd_SetGameLoopBatchName	},		????
	//{ CM_SAY,					Cmd_SayText					},
	{ CM_LISTDATA,				Cmd_ListDataTable			},
	{ CM_WRITESTRING,			Cmd_WriteText				},
	{ CM_CREATE,				Cmd_Create					},
	{ CM_OPEN,					Cmd_Open					},
	{ CM_RECDEM,				Cmd_Recdem					},
	{ CM_RECORD,				Cmd_Record					},
	{ CM_APPEND,				Cmd_Append					},
	//{ CM_PLAYDEMO,				Cmd_PlayBinaryDemoFile		},
	{ CM_LOAD,					Cmd_ExecLoadCommand			},
};

#define NUM_CMDS_CUSTOMSTRING	CALC_NUM_ARRAY_ENTRIES( cmd_customstring )


// commands corresponding to functions taking an arbitrary string parameter ---
//
int CheckCmdsCustomString( char *command )
{
	ASSERT( command != NULL );

	//NOTE:
	// this function returns TRUE if a valid command
	// has been detected and executed. it returns
	// FALSE if no valid command has been detected
	// and the command string therefore needs
	// further processing by the main dispatcher.

	//NOTE:
	// a space as delimiter behind a command is not
	// enforced by this function. that is, the supplied
	// parsing functions can grab an entire domain
	// (set of commands starting with the same characters).
	// but this also means they have to ensure the delimiter
	// is indeed there if there should be only one command.

	//NOTE:
	// due to the semantics of this function and most parsing
	// functions it calls, most "custom string" commands reserve
	// an entire domain. that is, after this function no
	// commands starting with the same characters as a
	// "custom string" command will be recognized.
	// this behavior also depends on HANDLE_COMMAND_DOMAIN().

	// check for commands in "custom string" table
	for ( unsigned int tid = 0; tid < NUM_CMDS_CUSTOMSTRING; tid++ ) {

		const char *cmdstr = CMSTR( cmd_customstring[ tid ].commandid );
		int  cmdlen  = CMLEN( cmd_customstring[ tid ].commandid );

		if ( strncmp( command, cmdstr, cmdlen ) == 0 ) {

			ASSERT( cmd_customstring[ tid ].commandfunc != NULL );
			return (*cmd_customstring[ tid ].commandfunc)( command + cmdlen );
		}
	}

	return FALSE;
}


// user command handling ------------------------------------------------------
//
PRIVATE	user_command_s	user_commands_default[ 32 ];
PUBLIC	user_command_s*	user_commands = user_commands_default;
PUBLIC	int				num_user_commands = 0;
PRIVATE	int				max_user_commands = 32;


// check user commands --------------------------------------------------------
//
int CheckCmdsUserDefined( char *command )
{
	ASSERT( command != NULL );

	//NOTE:
	// the semantics of this function is basically
	// the same as for CheckCmdsCustomString().

	//NOTE:
	// this function will be called very early in
	// the sequence of command comparsions. thus,
	// it can override most other commands. the
	// callback functions have to be very careful
	// not to block entire domains without any reason.

	// check for commands in user commands table
	for ( int cid = 0; cid < num_user_commands; cid++ ) {

		const char *cmdstr = user_commands[ cid ].command;
		int  cmdlen  = user_commands[ cid ].commlen;

		if ( strncmp( command, cmdstr, cmdlen ) == 0 ) {

			ASSERT( user_commands[ cid ].execute != NULL );
			return (*user_commands[ cid ].execute)( command + cmdlen );
		}
	}

	return FALSE;
}


// register a new user-defined command ----------------------------------------
//
int CON_RegisterUserCommand( user_command_s *regcom )
{
	//NOTE:
	//CAVEAT:
	// the supplied command string is not copied
	// by this function. thus, the caller MUST ENSURE
	// that this string is available indefinitely.
	// (e.g., allocated statically.)

	//NOTE:
	// the commlen field need not be valid, it will be
	// set by this function. (in the original struct!)

	ASSERT( regcom != NULL );
	ASSERT( regcom->command != NULL );
	ASSERT( regcom->execute != NULL );
	ASSERT( regcom->numparams >= 0 );
	ASSERT( num_user_commands <= max_user_commands );

	// expand table memory if already used up
	if ( num_user_commands == max_user_commands ) {

		// expand exponentially
		int newtabsize = max_user_commands * 2;

		// alloc new table
		user_command_s *newlist = (user_command_s *) ALLOCMEM( sizeof( user_command_s ) * newtabsize );
		if ( newlist == NULL ) {
			ASSERT( 0 );
			return FALSE;
		}

		// set new size
		max_user_commands = newtabsize;

		// move old table
		memcpy( newlist, user_commands, sizeof( user_command_s ) * num_user_commands );
		if ( user_commands != user_commands_default )
			FREEMEM( user_commands );
		user_commands = newlist;
	}

	// append new command
	regcom->commlen = strlen( regcom->command );
	ASSERT( num_user_commands < max_user_commands );
	user_commands[ num_user_commands++ ] = *regcom;

	return TRUE;
}

// replace the entry for an already registered user-defined command -----------
//
int CON_ReplaceUserCommand( user_command_s* repcom )
{
	//NOTE:
	//CAVEAT:
	// the supplied command string is not copied
	// by this function. thus, the caller MUST ENSURE
	// that this string is available indefinitely.
	// (e.g., allocated statically.)
	
	//NOTE:
	// the commlen field need not be valid, it will be
	// set by this function. (in the original struct!)

	ASSERT( repcom != NULL );
	ASSERT( repcom->command != NULL );
	ASSERT( repcom->execute != NULL );
	ASSERT( repcom->numparams >= 0 );

	for( int user_command_index = 0; user_command_index < num_user_commands; user_command_index++ ) {

		// check for equal command string
		if( stricmp( user_commands[ user_command_index ].command , repcom->command ) == 0 ) {
			repcom->commlen = strlen( repcom->command );
			user_commands[ user_command_index ] = *repcom;

			return TRUE;
		}
	}

	return FALSE;
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( CON_COM_SV )
{

#ifdef PARSEC_DEBUG

	// register "heap" command
	user_command_s regcom;
	regcom.command	 = "heap";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_HEAP;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

#endif // PARSEC_DEBUG

}



