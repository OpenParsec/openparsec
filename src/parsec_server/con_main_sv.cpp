/*
 * PARSEC - Server Console Main Module
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:47 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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
#include "inp_defs_sv.h"
//#include "vid_defs.h"

// drawing subsystem
//#include "d_bmap.h"
//#include "d_font.h"

// local module header
#include "con_main_sv.h"

// proprietary module headers
#include "con_act_sv.h"
#include "con_arg.h"
#include "con_aux_sv.h"
#include "con_com_sv.h"
//#include "con_conf.h"
#include "con_ext_sv.h"
#include "con_info_sv.h"
#include "con_int_sv.h"
//#include "con_kmap.h"
#include "con_list_sv.h"
//#include "con_rc.h"
#include "con_std_sv.h"
#include "con_tab.h"
#include "con_vald.h"
//#include "e_color.h"
//#include "e_demo.h"
//#include "e_draw.h"
#include "net_limits.h"
#include "net_defs.h"
#include "e_gameserver.h"
//#include "g_supp.h"
//#include "h_cockpt.h"
//#include "h_strmap.h"
#include "inp_main_sv.h"
#include "keycodes.h"
//#include "m_main.h"
#include "sys_path.h"
#include "sys_swap.h"

// CURSES library -------------------------------------------------------------
#ifdef USE_CURSES
	#ifdef SYSTEM_WIN32_UNUSED
		#pragma comment( lib, "curses.lib" )
	#endif // SYSTEM_WIN32_UNUSED
#endif // USE_CURSES

// flags
#define PROCESS_STARTUP_SCRIPT				// execute startup (boot) script
#define FORCE_CONSOLE_LOGIN					// have to login to exec commands
#define LOSE_KEYBOARD_FOCUS_ON_SLIDEUP		// console loses focus on slide up
#define CONSOLE_OUTPUT_WORKAROUND


// echo commands in startup script
#define ECHO_STARTUP_SCRIPT			TRUE	// echo+history for startup script

// speed of console sliding
#define CONSOLE_SLIDE_SPEED 		3		// less is faster

// miscellaneous defines
#define CONSOLE_MIN_ENTER_LENGTH	4		// lower bound for entry area
#define CURSOR_BLINK_SPEED			0x100	// in reference frames
#define CONSOLE_CAPTION_HEIGHT		2		// caption plus one empty line
#define CONSOLE_MAX_BACK_LINES		128		// number of lines to scroll back


// macros can be disabled to prevent overriding of internal commands
static int disable_macros			= FALSE;

// flag whether script compilation in progress
PUBLIC int compile_script			= FALSE;

// filepointer for compilation output
PUBLIC FILE *compile_fp				= NULL;

// echo commands executed from file (script)
PUBLIC int EchoExternalCommands		= FALSE;

// enter script commands into history list
PUBLIC int HistoryForBatchEntry		= TRUE;

// startup script file names (normal boot/demo boot)
PUBLIC char boot_script_name[]		= "boot_sv" CON_FILE_EXTENSION;
PUBLIC char boot_script_master_name[]		= "boot_ms" CON_FILE_EXTENSION;
static char demo_script_name[]		= "demo_sv" CON_FILE_EXTENSION;

// various string constants
PUBLIC char con_prompt[]			= "::";
PUBLIC char ok_prompt[] 			= "ok.";
PUBLIC char press_space[]			= "- press space -";
PUBLIC char too_many_args[] 		= "too many arguments.";
PUBLIC char arg_missing[]			= "argument missing.";
PUBLIC char invalid_arg[]			= "invalid argument.";
PUBLIC char range_error[]			= "range error.";
PUBLIC char unknown_command[]		= "unknown command: \"%s\"";
PUBLIC char unknown_script[]		= "unknown command script.";
PUBLIC char cmd_not_compilable[]	= "command cannot be compiled.";
PUBLIC char line_separator[]		= "----------------------------------";
static char console_login_passwd[]	= "argument.";
static char recursive_compilation[]	= "recursive compilation... rather not.";
static char compile_open_failed[]	= "cannot open output file.";

PUBLIC int	press_space_curs_x		= strlen( press_space ) - PROMPT_SIZE;

static char console_caption[]		= "parsec command console";
static char console_ready[] 		= "console ready...";

static char auxflag_array_name[]	= ".aux";
static char auxdata_array_name[]	= ".auxd";

static char insert_cursor[]			= "_";	// will be a underline cursor
static char overwrite_cursor[]		= "^";	// will be a block cursor

static char console_lock_done[]		= "console is now locked.";
static char console_unlock_done[]	= "console is now unlocked.";
static char console_is_locked[]		= "console locked.";

// generic string paste area
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];

#if ( PASTE_STR_LEN < MAX_CONSOLE_LINE_LENGTH )
	#error "CON_MAIN::paste_str too short!"
#endif

// console geometry
PRIVATE int console_frameofs_x = 4;
PRIVATE int console_frameofs_y = 0;
PRIVATE int console_frame_left = 8;
PRIVATE int console_frame_top  = 5;

// current console contents
PUBLIC char con_lines[ NUM_CONSOLE_LINES ][ MAX_CONSOLE_LINE_LENGTH + 1 ];

// temporary line from which actual drawing will be done
static char draw_line[ MAX_CONSOLE_LINE_LENGTH + 1 ];

// temporary line for current command (from console input line or file)
static char command_line[ MAX_CONSOLE_LINE_LENGTH + 1 ];

// history buffer (most recent commands history)
static char history_list[ NUM_HISTORY_LINES ][ MAX_CONSOLE_LINE_LENGTH + 1 ];

static int history_add	= 0;	// new entry will be placed here
static int history_scan = 0;	// current scroll position in list

static int insert_mode	= TRUE;	// default mode: insert

PUBLIC int con_back_view= 0;	// number of lines to view back in console

PUBLIC int con_content	= 1;	// valid lines in console buffer
PUBLIC int con_bottom	= 0;	// current line in console buffer (0..)

static int last_con_content;	// old state
static int last_con_bottom;		// (used in lists to overwrite space prompt)

static int cursor_state = CURSOR_BLINK_SPEED;
PUBLIC int cursor_x		= 0;	// current cursor position (0==behind prompt)

static int edit_ofs_x	= 0;	// x offset for viewing the edit line
static int edit_max_x	= MAX_CONSOLE_LINE_LENGTH - PROMPT_SIZE;

int  con_in_talk_mode	= FALSE;
int  con_talk_line		= 0;	// line just below separator in talk mode
char talk_escape_char	= '/';	// escape char for command input in talk mode

int con_non_interactive	= 0;	// CON_AddMessage() instead of CON_AddLine()

int con_logged_in		= 1;	// user logged in
int con_login_mode		= 0;	// login mode (must not display console input)
int con_no_login_check	= 0;	// used by keymapped commands

int ConsoleEnterLength	= 23;	// length of console line (excl. prompt+cursor)
int ConsoleTextX		= 3;	// x-pos of console in char-multiples
int ConsoleTextY		= 7;	// y-pos of console in char-multiples

// execute init only once
int console_init_done	= FALSE;

// scratchpad for path with current working directory prepended
static char curworkdir_path[ PATH_MAX + 1 ];


// CURSES library -------------------------------------------------------------
//
#ifdef USE_CURSES
	WINDOW* g_curses_in_win  = NULL;
	WINDOW* g_curses_out_win = NULL;
#endif // USE_CURSES


// completely erase (fill with zero-bytes) all console lines ------------------
//
void ClearConScreen()
{
	// clear all lines and make them valid
	memset( con_lines, 0, sizeof( con_lines ) );
	con_content = NUM_CONSOLE_LINES;

	// reset back-viewing
	con_back_view = 0;
}


// completely erase (fill with zero-bytes) an entire console line -------------
//
void EraseConLine( int line )
{
	ASSERT( ( line >= 0 ) && ( line < NUM_CONSOLE_LINES ) );
	memset( con_lines[ line ], 0, MAX_CONSOLE_LINE_LENGTH + 1 );
	wclear( g_curses_in_win );
	cursor_x = 0;
}


// fill current line with separator chars -------------------------------------
//
void CreateSeparatorLine( int line )
{
	ASSERT( ( line >= 0 ) && ( line < NUM_CONSOLE_LINES ) );
	memset( con_lines[ line ], '-', MAX_CONSOLE_LINE_LENGTH );
	con_lines[ line ][ MAX_CONSOLE_LINE_LENGTH ] = 0;
}


// return path with current workdir prepended if it starts with "./" ----------
//
char *SubstCurWorkDir( char *path )
{
	ASSERT( path != NULL );

	//NOTE:
	// a copy of the path is returned even if it
	// has not been altered to achieve the exact
	// same semantics as in case of modification.

	//NOTE:
	// this function recognizes ".:" as alternative specifier for
	// the workdir in order to allow MacOS path specifications in
	// console script paths.

	if ( ( strncmp( path, "./", 2 ) == 0 ) ||
		 ( strncmp( path, ".:", 2 ) == 0 ) ) {

		if ( AUX_FORCE_WORKDIR_TO_CURRENT ) {

			// just skip first two chars
			strncpy( curworkdir_path, path + 2, PATH_MAX );
			curworkdir_path[ PATH_MAX ] = 0;

		} else {

			// substitute current working directory for "."
			strcpy( curworkdir_path, CurWorkDir );
			int cwdlen = strlen( CurWorkDir );
			strncpy( curworkdir_path + cwdlen, path + 1, PATH_MAX - cwdlen );
			curworkdir_path[ PATH_MAX ] = 0;
		}

	} else {

		// just copy over unmodified path
		strncpy( curworkdir_path, path, PATH_MAX );
		curworkdir_path[ PATH_MAX ] = 0;
	}

	// convert path to system format
	path = SYSs_ProcessPathString( curworkdir_path );
	if ( path != curworkdir_path ) {
		strncpy( curworkdir_path, path, PATH_MAX );
		curworkdir_path[ PATH_MAX ] = 0;
	}

	// strip path if flag set
	char *substpath = curworkdir_path;
	if ( AUX_FORCE_FILEPATH_TO_CURRENT )
		substpath = SYSs_StripPath( curworkdir_path );

	return substpath;
}


// exec startup script (usually boot.con, but also boot scripts of mods) ------
//
void ExecStartupScript( int echo )
{
	// exec startup command script
	char *startupscript = NULL;

	// start normal or mod boot scripts
	if ( mod_numnames > 0 ) {

		ASSERT( mod_names[ 0 ] != NULL );

		// if mod doesn't override our own packages, execute our boot.con too
		if ( !mod_override ) {
			//startupscript = PlayDemo ? demo_script_name : boot_script_name;
			if(TheServer->GetServerIsMaster()){
				startupscript = boot_script_master_name;
			} else {
				startupscript = boot_script_name;
			}
			ExecConsoleFile( startupscript, echo );
		}

		// afterwards, exec <modname>/boot.con for each registered mod
		for ( int curmod = 0; curmod < mod_numnames; curmod++ ) {

			ASSERT( mod_names[ curmod ] != NULL );
			startupscript = (char *) ALLOCMEM(
				strlen( mod_names[ curmod ] ) + 1 + strlen( boot_script_name ) );
			if ( startupscript == NULL ) {
				OUTOFMEM( 0 );
			}
			strcpy( startupscript, mod_names[ curmod ] );
			strcat( startupscript, "/" );
			strcat( startupscript, boot_script_name );

			// must be done to ensure the file can be found independently of
			// whether it is read from a package or from a real directory
			char *path = SYSs_ProcessPathString( startupscript );

			ExecConsoleFile( path, echo );

			FREEMEM( startupscript );
			startupscript = NULL;
		}

	} else {

		// if no mod is active, we just exec boot.con
		//startupscript = PlayDemo ? demo_script_name : boot_script_name;
		if(TheServer->GetServerIsMaster()){
			startupscript = boot_script_master_name;
		} else {
			startupscript = boot_script_name;
		}
		ExecConsoleFile( startupscript, echo );
	}
}

// init CURSES windows --------------------------------------------------------
//
void CON_InitCurses()
{
#ifdef USE_CURSES

	initscr();
	cbreak();
	//echo();


	// create input window
	g_curses_in_win  = newwin( 1,  COLS, LINES - 1, 0 );
	//wattrset( g_curses_in_win, A_REVERSE );
	keypad( g_curses_in_win, TRUE );
	nodelay( g_curses_in_win, TRUE );
	
	// create output window ( -1 to ensure we do not overwrite the input window )
	g_curses_out_win = newwin( LINES - 1, COLS, 0, 0  );
	nodelay( g_curses_out_win, TRUE );

	// scrolling is ok
	scrollok( g_curses_in_win, TRUE );
	scrollok( g_curses_out_win, TRUE );

	//slk_init( 1 );
	/*slk_set(1, "Insert", 0);
	slk_set(2, "Quit", 0);
	slk_set(3, "Add", 0);
	slk_set(4, "Delete", 0);
	slk_set(5, "Undo", 0);
	slk_set(6, "Search", 0);
	slk_set(7, "Replace", 0);
	slk_set(8, "Save", 0);
	slk_refresh();
	*/


#endif // USE_CURSES
}


// kill all CURSES windows and related stuff ----------------------------------
//
void CON_KillCurses()
{
#ifdef USE_CURSES
	nocbreak();
	echo();
	endwin();
#endif // USE_CURSES
}


// init memory needed for console ---------------------------------------------
//
void CON_InitConsole()
{
	ASSERT( !console_init_done );

	if ( !console_init_done ) {
        int lid;
		// erase console contents
		for ( lid = 0; lid < NUM_CONSOLE_LINES; lid++ ) {
			memset( con_lines[ lid ], 0, MAX_CONSOLE_LINE_LENGTH + 1 );
		}

		// erase history buffer
		for ( lid = 0; lid < NUM_HISTORY_LINES; lid++ ) {
			history_list[ lid ][ 0 ] = '\0';
		}

		// insert startup message into top line
		strcpy( con_lines[ 0 ], console_ready );

		// make startup message valid
		con_content = 1;
		con_bottom  = 0;

		EraseConLine( con_bottom );
		strcpy( con_lines[ con_bottom ], con_prompt );

#ifdef PROCESS_STARTUP_SCRIPT

		// there's no reason to run the external scripts when we are only
		// running in master server mode.
		if(!TheServer->GetServerIsMaster()) {
			// build list of external commands
			BuildExternalCommandList();

			// exec boot.con (also for registered mods)
			ExecStartupScript( ECHO_STARTUP_SCRIPT );
		}
#endif // PROCESS_STARTUP_SCRIPT

		//CAVEAT:
		// this has to stay last, because CON_EXT::ExecConsoleFile()
		// uses it to determine whether it has been called from here.
		console_init_done = TRUE;
	}
}


// make sure no demo is playing when console is inited/reinited/killed --------
//
PRIVATE
void KillDemoReplay()
{
//FIXME: needed on server ?

/*	// make sure we call demo stop instead of full clear
	int oldclearflag = AUX_DISABLE_CLEARDEMO_ON_REPSTOP;
	AUX_DISABLE_CLEARDEMO_ON_REPSTOP = 1;

	// make sure no demo is playing
	//DEMO_StopReplay();

	// restore previous flag state
	AUX_DISABLE_CLEARDEMO_ON_REPSTOP = oldclearflag;
*/
}


// (re)init file pointers used by console -------------------------------------
//
void InitConsoleFileHandling()
{
	// make sure no demo is playing
	KillDemoReplay();

	if ( log_commands ) {
		fclose( log_fp );
		log_commands = 0;
	}

	if ( RecordingActive ) {
		fclose( RecordingFp );
		RecordingActive = 0;
		RemoteRecSessionId++;
	}
}


// kill resources used by console ---------------------------------------------
//
void CON_KillConsole()
{
	if ( console_init_done ) {
		console_init_done = FALSE;

		InitConsoleFileHandling();
	}
}


// flush keyboard buffer for console ------------------------------------------
//
void FlushConsoleBuffer()
{
	dword *pos;

	while ( *( pos = &KeybBuffer->Data + KeybBuffer->ReadPos ) ) {
		KeybBuffer->ReadPos = ( KeybBuffer->ReadPos + 1 ) & KEYB_BUFF_SIZ_M;
		*pos = 0;
	}
}


// add new message to console (preserve current input) ------------------------
//
PUBLIC
void CON_AddMessage( const char *text )
{
	ASSERT( text != NULL );

	// normal message output
	MSGOUT( text );
}


// flag whether next CON_AddLine() should overwrite the current line ----------
//
static int skip_next_nl = FALSE;


// add new line to console ----------------------------------------------------
//
PUBLIC
void CON_AddLine( const char *text )
{
	ASSERT( text != NULL );

	// normal text output
	MSGOUT( text );
}


// add line feed in console ---------------------------------------------------
//
PUBLIC
void CON_AddLineFeed()
{
	// force line feed
	skip_next_nl = FALSE;
	CON_AddLine( con_prompt );
}


// set flag to disable next line feed in CON_AddLine() ------------------------
//
PUBLIC
void CON_DisableLineFeed()
{
	// set flag
	skip_next_nl = TRUE;
}


// set flag to not eat next line feed in CON_AddLine() ------------------------
//
PUBLIC
void CON_EnableLineFeed()
{
	// clear flag
	skip_next_nl = FALSE;
}


// delete current console line ------------------------------------------------
//
PUBLIC
void CON_DelLine()
{
	EraseConLine( con_bottom );
}


// return pointer to current console line -------------------------------------
//
PUBLIC
char *CON_GetLine()
{
	return con_lines[ con_bottom ];
}


// print prompt for list continuation -----------------------------------------
//
PUBLIC
void CON_ListCtdPrompt()
{
	CON_AddLine( press_space );

	// ensure proper view alignment
	edit_ofs_x = ( cursor_x > ConsoleEnterLength ) ? ( cursor_x - ConsoleEnterLength ) : 0;
}


// print prompt after list has ended ------------------------------------------
//
PUBLIC
void CON_ListEndPrompt()
{
	await_keypress = FALSE;
	com_cont_func  = NULL;

	if ( con_in_talk_mode ) {

		// restore talkmode prompt
		CON_AddLineFeed();
		CreateSeparatorLine( con_bottom );
		CON_AddLineFeed();
		con_talk_line = con_bottom;

	} else {

		// restore standard prompt
		CON_AddLine( con_prompt );
	}
}


// check geometric extents of console window ----------------------------------
//
void CheckConExtents()
{
	ASSERT( FALSE );
}


// check commands with single integer argument/query --------------------------
//
PRIVATE
int ExecGetSetInteger( const char *scan )
{
	ASSERT( scan != NULL );

	ConsoleHeight -= 2;

	for ( int icom = 0; icom < num_int_commands; icom++ ) {

		ASSERT( int_commands[ icom ].command != NULL );
		ASSERT( int_commands[ icom ].intref != NULL );

		// need fetch function for int vars that need to be
		// made current before they contain the correct value
		if(strncmp(scan, int_commands[ icom ].command, strlen(int_commands[ icom ].command)) ==0) {
			if ( int_commands[ icom ].fetch != NULL )
				*int_commands[ icom ].intref = (*int_commands[ icom ].fetch)();

			if ( CheckSetIntArgBounded( int_print_base, scan,
									(char *) int_commands[ icom ].command,
									int_commands[ icom ].intref,
									int_commands[ icom ].bmin,
									int_commands[ icom ].bmax,
									int_commands[ icom ].realize ) ) {
				ConsoleHeight += 2;
				return TRUE;
			}
		}
	}

	ConsoleHeight += 2;

	return FALSE;
}


// temporary storage in which user can alter their name -----------------------
//
static char temp_playername[ MAX_PLAYER_NAME + 1 ];


// exec command with a single optional parameter ------------------------------
//
PRIVATE
void ExecGetSetCommand( char *command )
{
	ASSERT( command != NULL );

	// split off first token (command specifier)
	char *scan = strtok( command, " " );

	// check different commands
	if ( scan != NULL ) {

		if ( ExecGetSetInteger( scan ) ) {
		} else if ( CheckSetObjTypeProperty( int_print_base, scan ) ) {
		} else if ( CheckSetObjClassProperty( int_print_base, scan ) ) {
		} else if ( CheckSetObjInstanceProperty( int_print_base, scan ) ) {
		} else if ( CheckTypeInfo( scan ) ) {
		} else if ( CheckClassInfo( scan ) ) {
		} else if ( CheckInstanceInfo( scan ) ) {
/*		} else if ( CheckKeyMappingEcho( scan ) ) {
		} else if ( CheckKeyMappingSilent( scan ) ) {
		*/} else if ( CheckSetIntArray( int_print_base, scan, auxflag_array_name, AuxEnabling, MAX_SV_ARRAY_SIZE ) ) {
		} else {
			// printout the command that could not be found
			char szBuffer[ 512 ];
			sprintf( szBuffer, unknown_command, scan );
			CON_AddLine( szBuffer );
		}
	} else {

		CON_AddLine( ok_prompt );
	}
}


// key table for compile command ----------------------------------------------
//
key_value_s compile_key_value[] = {

	{ "title",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "desc",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "author",		NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_COMPILE_TITLE,
	KEY_COMPILE_DESC,
	KEY_COMPILE_AUTHOR,
};


// write demo header if at least one info field specified ---------------------
//
PRIVATE
void WriteDemoHeader( char *title, char *desc, char *author )
{
	if ( ( title == NULL ) && ( desc == NULL ) && ( author == NULL ) ) {
		return;
	}

	ASSERT( strlen( DEM_SIGNATURE ) == 6 );

	DemHeader hdr;
	strcpy( hdr.signature, DEM_SIGNATURE );
	hdr.version = REQUIRED_DEM_VERSION;

	dword headersize = sizeof( DemHeader );

	headersize += ( title  != NULL ) ? strlen( title )  + 2 : 0;
	headersize += ( desc   != NULL ) ? strlen( desc )   + 2 : 0;
	headersize += ( author != NULL ) ? strlen( author ) + 2 : 0;

	hdr.headersize = headersize;

	SYS_SwapDemHeader( &hdr );

	// write fixed-size part of header
	fwrite( &hdr, sizeof( hdr ), 1, compile_fp );

	// write title
	byte demokey = 0;
	if ( title != NULL ) {
		demokey = DEMO_KEY_TITLE;
		fwrite( &demokey, sizeof( demokey ), 1, compile_fp );
		fwrite( title, strlen( title ) + 1, 1, compile_fp );
	}

	// write description
	if ( desc != NULL ) {
		demokey = DEMO_KEY_DESCRIPTION;
		fwrite( &demokey, sizeof( demokey ), 1, compile_fp );
		fwrite( desc, strlen( desc ) + 1, 1, compile_fp );
	}

	// write author
	if ( author != NULL ) {
		demokey = DEMO_KEY_AUTHOR;
		fwrite( &demokey, sizeof( demokey ), 1, compile_fp );
		fwrite( author, strlen( author ) + 1, 1, compile_fp );
	}
}


// check compile command and compile script if valid --------------------------
//
PRIVATE
void CompileCommand( char *command )
{
	ASSERT( command != NULL );

	// check for compile command in script to compile
	if ( compile_script ) {
		CON_AddLine( recursive_compilation );
		return;
	}

	// skip command string
	command += CMLEN( CM_COMPILE );

	// check if there is a space between command and argument
	if ( ( *command != ' ' ) && ( *command != 0 ) ) {

		// printout the command that could not be found
		char szBuffer[ 512 ];
		sprintf( szBuffer, unknown_command, command );
		CON_AddLine( szBuffer );

		return;
	}

	// split off argument (name of command script)
	char *scan = strtok( command, " " );
	if ( scan == NULL ) {
		CON_AddLine( arg_missing );
		return;
	}

	// scan out all values to keys
	if ( !ScanKeyValuePairs( compile_key_value, NULL ) )
		return;

	// get demo information strings (not mandatory, might be NULL)
	char *title  = compile_key_value[ KEY_COMPILE_TITLE ].value;
	char *desc   = compile_key_value[ KEY_COMPILE_DESC ].value;
	char *author = compile_key_value[ KEY_COMPILE_AUTHOR ].value;

	// check if command name is valid
	int extcom = CheckExternalCommand( scan );

	if ( extcom > 0 ) {

		// open output file
		strcpy( paste_str, scan );
		strcat( paste_str, CON_FILE_COMPILED_EXTENSION );
		compile_fp = fopen( paste_str, "wb" );

		if ( compile_fp != NULL ) {

			// write header if at least one info field specified
			WriteDemoHeader( title, desc, author );

			// compile script
			compile_script = TRUE;
			CallExternalCommand( extcom - 1, FALSE );
			compile_script = FALSE;

			// close output file
			if ( compile_fp != NULL ) {
				// if no error occurred write eof marker and close file
				int endmarker = 0;
				fwrite( &endmarker, sizeof( endmarker ), 1, compile_fp );
				fclose( compile_fp );
			}

		} else {
			// filepointer for output invalid
			CON_AddLine( compile_open_failed );
		}

	} else {
		// name of script is not in internal table
		CON_AddLine( unknown_script );
	}
}


// execute command that cannot be compiled ------------------------------------
//
void ExecNonCompilableCommand( char *command )
{
	ASSERT( command != NULL );

	//NOTE:
	// this function is mainly used by ExecConsoleCommand().
	// apart from that it is only used by AcExecCommandString()
	// to execute a command supplied as string in a demo.

	// this is first in order to allow user overriding
	// of all non-compilable commands
	if ( CheckCmdsUserDefined( command ) ) {

	// check commands having no parameters
	} else if ( CheckCmdsNoParam( command ) ) {

	// check commands that do their own parameter parsing
	} else if ( CheckCmdsCustomString( command ) ) {

	} else {

		ExecGetSetCommand( command );
	}
}


// add console line to history list -------------------------------------------
//
INLINE
void AddLineToHistory( char *command )
{
	ASSERT( command != NULL );
	ASSERT( strlen( command ) <= MAX_CONSOLE_LINE_LENGTH );

	// store command in current history position, advance in
	// list, and clear history scrolling
	strcpy( history_list[ history_add ], command );
	history_add  = ( history_add + 1 ) & NUM_HISTORY_LINES_MASK;
	history_scan = history_add;

	// clear next two positions in history ring-buffer
	// (this has to be done in order to make scrolling
	// in both directions work properly)
	// this is where entries start to get reused if there
	// is not enough space in the history list
	*( history_list[ history_add ] ) = 0;
	*( history_list[ ( history_add + 1 ) & NUM_HISTORY_LINES_MASK ] ) = 0;
}


// check for console login state and login/logout commands --------------------
//
PRIVATE
int CheckConsoleLogin( char *command )
{
	ASSERT( command != NULL );

	// used by keymapped commands
	if ( con_no_login_check ) {
		return TRUE;
	}

	// avoid preempting startup script
	if ( console_init_done ) {

		if ( con_login_mode ) {
			con_login_mode = 0;

			// check for password
			if ( strcmp( command, console_login_passwd ) == 0 ) {
				con_logged_in = 1;
				CON_AddLine( console_unlock_done );
			}
			// erase password to prevent it from being displayed
			for ( int spos = 0; spos < ConsoleEnterLength; spos++ ) {
				if ( command[ spos ] == 0 )
					break;
				command[ spos ] = '*';
			}
			return FALSE;
		}

		if ( con_logged_in ) {

			if ( strcmp( command, CMSTR( CM_LOGOUT ) ) == 0 ) {
				con_logged_in = 0;
				CON_AddLine( console_lock_done );
				return FALSE;
			}

		} else {

			if ( strcmp( command, CMSTR( CM_LOGIN ) ) == 0 ) {
				con_login_mode = 1;
				return FALSE;
			}

			CON_AddLine( console_is_locked );
			return FALSE;
		}
	}

	return TRUE;
}


// exec console command (this is the main command dispatcher) -----------------
//
void ExecConsoleCommand( char *command, int echo )
{
	//NOTE:
	// this function may be called recursively from within
	// functions called by CON_EXT::CallExternalCommand()
	// to allow execution of command scripts.

	ASSERT( command != NULL );
	ASSERT( ( echo == FALSE ) || ( echo == TRUE ) );

#ifdef FORCE_CONSOLE_LOGIN

	// check login state and login/logout commands
	if ( !CheckConsoleLogin( command ) ) {
		return;
	}

#endif // FORCE_CONSOLE_LOGIN

	// fetch console line as command
	strncpy( command_line, command, MAX_CONSOLE_LINE_LENGTH );
	command_line[ MAX_CONSOLE_LINE_LENGTH ] = 0;
	command = command_line;

	// eat leading whitespace
	for ( ; *command; command++ ) {
		if ( *command != ' ' ) {
			break;
		}
	}

	// early out for empty lines
	if ( *command == 0 ) {
		if ( echo ) {
			CON_AddLine( ok_prompt );
		}
		return;
	}

	if ( echo ) {
		// add command to history list only if echo is on
		AddLineToHistory( command );
	}

	// temp for extcom id
	int extcom;

	if ( strcmp( command, CMSTR( CM_NOMACROS ) ) == 0 ) {

		// disable calls to CallExternalCommand()
		disable_macros = TRUE;

	} else if ( strcmp( command, CMSTR( CM_ENABLEMACROS ) ) == 0 ) {

		// enable calls to CallExternalCommand()
		disable_macros = FALSE;

	} else if ( strncmp( command, CMSTR( CM_COMPILE ), CMLEN( CM_COMPILE ) ) == 0 ) {

		CompileCommand( command );

	} else if ( strncmp( command, ACMSTR( ACM_SIGNATURE ), ACMLEN( ACM_SIGNATURE ) ) == 0 ) {

		// check/exec commands with "ac." prefix
		ActionCommand( command );

	} else if ( !disable_macros && ( extcom = CheckExternalCommand( command ) ) ) {

		// execute command script
		CallExternalCommand( extcom - 1, echo ? EchoExternalCommands : FALSE );

	} else if ( !compile_script ) {

		// check/exec all other commands
		ExecNonCompilableCommand( command );

	} else {

		// error if compilation in progress and non-compilable command found
		CON_AddLine( cmd_not_compilable );
	}
}


// handle console input in talk mode ------------------------------------------
//
PRIVATE
void HandleTalkMode()
{
	if ( con_in_talk_mode ) {

		if ( con_lines[ con_bottom ][ PROMPT_SIZE ] == talk_escape_char ) {

			// remove '/' escape char from input
			strcpy( paste_str, con_lines[ con_bottom ] + PROMPT_SIZE + 1 );

		} else {

			// prepend "say " to input
			strcpy( paste_str, CMSTR( CM_SAY ) );
			paste_str[ CMLEN( CM_SAY ) ] = ' ';
			strncpy( paste_str + CMLEN( CM_SAY ) + 1,
					 con_lines[ con_bottom ] + PROMPT_SIZE,
					 MAX_CONSOLE_LINE_LENGTH - PROMPT_SIZE - CMLEN( CM_SAY ) - 1 );
			paste_str[ MAX_CONSOLE_LINE_LENGTH - PROMPT_SIZE ] = 0;
		}

		// set bottom to line just below separator in order to
		// overwrite previous output. the current input is also
		// copied into this line
		con_bottom = con_talk_line;
		strcpy( con_lines[ con_bottom ] + PROMPT_SIZE, paste_str );

		// ensure the input line gets overwritten by the first console
		// output the command produces. if the command outputs nothing
		// the terminating prompt will overwrite the former input line
		CON_DisableLineFeed();
	}
}


// <enter> key pressed in console ---------------------------------------------
//
PRIVATE
void EnterConsoleCommand()
{
	// handle input in talkmode
	HandleTalkMode();

	// output command
	CON_AddLine( con_lines[ con_bottom ] );

	// insert command into script if currently open
	if ( !InsertCommandLog( con_lines[ con_bottom ] + PROMPT_SIZE ) ) {

		// execute current linecontent
		ExecConsoleCommand( con_lines[ con_bottom ] + PROMPT_SIZE, TRUE );

		if ( await_keypress ) {
			// listcommand: print listmode prompt
			CON_ListCtdPrompt();
		} else {
			// normal command: print standard prompt
			//CON_AddLine( con_prompt );
		}

	} else {

		// command was not executed, but only
		// written to an output script

		if ( HistoryForBatchEntry )
			AddLineToHistory( con_lines[ con_bottom ] + PROMPT_SIZE );
		//CON_AddLine( con_prompt );
	}

	EraseConLine( con_bottom );
	strcpy( con_lines[ con_bottom ], con_prompt );
}


// fetch history list entry into current console line -------------------------
//
PRIVATE
void FetchHistoryEntry( int entryid )
{
	EraseConLine( con_bottom );
	strcpy( con_lines[ con_bottom ], con_prompt );

	if ( con_in_talk_mode ) {

		// handle talkmode
		if ( ( strncmp( history_list[ entryid ], CMSTR( CM_SAY ), CMLEN( CM_SAY ) ) == 0 ) &&
			 ( history_list[ entryid ][ CMLEN( CM_SAY ) ] == ' ' ) ) {

			// strip "say " in order to produce correct talkmode input
			strcpy( con_lines[ con_bottom ] + PROMPT_SIZE, history_list[ entryid ] + CMLEN( CM_SAY ) + 1 );

		} else {

			// prepend '/' escape char in order to produce correct talkmode input
			con_lines[ con_bottom ][ PROMPT_SIZE ] = talk_escape_char;
			strncpy( con_lines[ con_bottom ] + PROMPT_SIZE + 1,
					 history_list[ entryid ],
					 MAX_CONSOLE_LINE_LENGTH - PROMPT_SIZE - 1 );
			con_lines[ con_bottom ][ MAX_CONSOLE_LINE_LENGTH ] = 0;
		}

	} else {

		// use history line as is
		strcpy( con_lines[ con_bottom ] + PROMPT_SIZE, history_list[ entryid ] );
	}

	// set cursor behind end of command
	cursor_x = strlen( con_lines[ con_bottom ] + PROMPT_SIZE );

	// ensure proper view alignment
	edit_ofs_x = ( cursor_x > ConsoleEnterLength ) ?
		( cursor_x - ConsoleEnterLength ) : 0;
}


// check input for console window ---------------------------------------------
//
PRIVATE
bool_t CheckConsoleInput()
{
	// scan keyboardbuffer and process key-presses sequentially
	dword *pos;
	bool_t console_input_done = false;
	while ( *( pos = &KeybBuffer->Data + KeybBuffer->ReadPos ) ) {

		int curlinelen = strlen( con_lines[ con_bottom ] + PROMPT_SIZE );

		if ( await_keypress && com_cont_func ) {

			// <space> or <enter> continues listing
			if ( ( *pos == CKC_SPACE ) || ( *pos == CKC_ENTER ) ) {

				EraseConLine( con_bottom );
				strcpy( con_lines[ con_bottom ], con_prompt );
				// ensure wait prompt gets overwritten
			    con_bottom  = last_con_bottom;
			    con_content = last_con_content;
				cursor_x    = 0;
				com_cont_func();

			// <escape> returns to command prompt immediately
			} else if ( *pos == CKC_ESCAPE ) {

				await_keypress = FALSE;
				com_cont_func  = NULL;
				if ( con_in_talk_mode ) {
					// restore talkmode prompt
					CreateSeparatorLine( con_bottom );
					CON_AddLineFeed();
					con_talk_line = con_bottom;
				} else {
					// restore standard prompt
					EraseConLine( con_bottom );
					strcpy( con_lines[ con_bottom ], con_prompt );
				}
				cursor_x = 0;
			}

		} else if ( *pos & CKC_CTRL_MASK ) {

			// control keys (CKC_xx)
			switch ( *pos ) {

			// <escape>: erase entire command line
			case CKC_ESCAPE:
				EraseConLine( con_bottom );
				strcpy( con_lines[ con_bottom ], con_prompt );
				history_scan  = history_add;
				cursor_x      = 0;
				con_back_view = 0;
				break;

			// <cursor-up>: scroll one line up in history buffer
			case CKC_CURSORUP:
				if ( !con_logged_in )
					break;
				if ( *( history_list[ ( history_scan - 1 ) & NUM_HISTORY_LINES_MASK ] ) != 0 ) {
					history_scan = ( history_scan - 1 ) & NUM_HISTORY_LINES_MASK;
					FetchHistoryEntry( history_scan );
				}
				break;

			// <cursor-down>: scroll one line down in history buffer
			case CKC_CURSORDOWN:
				if ( !con_logged_in )
					break;
				if ( *( history_list[ ( history_scan + 1 ) & NUM_HISTORY_LINES_MASK ] ) != 0 ) {
					history_scan = ( history_scan + 1 ) & NUM_HISTORY_LINES_MASK;
					FetchHistoryEntry( history_scan );
				}
				break;

			// <cursor-left>: set cursor one position left
			case CKC_CURSORLEFT:
				if ( cursor_x > 0 )
					cursor_x--;
				break;

			// <cursor-right>: set cursor one position right
			case CKC_CURSORRIGHT:
				if ( cursor_x < curlinelen )
					cursor_x++;
				break;

			// <shift + cursor-up>: view one line backward
			case CKC_CURSORUP_SHIFTED:
				if ( !con_logged_in || con_in_talk_mode )
					break;
				if ( con_back_view < CONSOLE_MAX_BACK_LINES )
					con_back_view++;
				break;

			// <shift + cursor-down>: view one line forward
			case CKC_CURSORDOWN_SHIFTED:
				if ( !con_logged_in || con_in_talk_mode )
					break;
				if ( con_back_view > 0 )
					con_back_view--;
				break;

			// <shift + cursor-left>: set cursor one word to the left
			case CKC_CURSORLEFT_SHIFTED:
				while ( ( cursor_x > 0 ) &&
						!isalnum( con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x - 1 ] ) )
					cursor_x--;
				while ( ( cursor_x > 0 ) &&
						isalnum( con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x - 1 ] ) )
					cursor_x--;
				break;

			// <shift + cursor-right>: set cursor one word to the right
			case CKC_CURSORRIGHT_SHIFTED:
				while ( ( cursor_x < curlinelen ) &&
						isalnum( con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x ] ) )
					cursor_x++;
				while ( ( cursor_x < curlinelen ) &&
						!isalnum( con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x ] ) )
					cursor_x++;
				break;

			// <home>: set cursor to first column
			case CKC_HOME:
				cursor_x = 0;
				break;

			// <end>: set cursor behind last column
			case CKC_END:
				cursor_x = curlinelen;
				break;

			// <page-up>: view one page backward
			case CKC_PAGEUP:
				if ( !con_logged_in || con_in_talk_mode )
					break;
				if ( cur_vis_conlines <= 0 )
					break;
				if ( con_back_view < CONSOLE_MAX_BACK_LINES )
					con_back_view += cur_vis_conlines;
				if ( con_back_view > CONSOLE_MAX_BACK_LINES )
					con_back_view = CONSOLE_MAX_BACK_LINES;
				break;

			// <page-down>: view one page forward
			case CKC_PAGEDOWN:
				if ( !con_logged_in || con_in_talk_mode )
					break;
				if ( cur_vis_conlines <= 0 )
					break;
				if ( con_back_view > 0 )
					con_back_view -= cur_vis_conlines;
				if ( con_back_view < 0 )
					con_back_view = 0;
				break;

			// <insert>: toggle insert mode
			case CKC_INSERT:
				insert_mode = !insert_mode;
				break;

			// <del>: delete and shift left
			case CKC_DELETE:
				if ( cursor_x < curlinelen ) {
					strcpy( paste_str, con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x + 1 );
					strcpy( con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x, paste_str );
				}
				break;

			// <tab>: find completion for current input
			case CKC_TAB:
				if ( !con_logged_in )
					break;
				CompleteCommand();
				break;

			// <enter>: execute current line
			case CKC_ENTER:
				EnterConsoleCommand();
				break;

			// <backspace>: step left and shift righthand side left
			case CKC_BACKSPACE:
				if ( cursor_x > 0 ) {
					if ( cursor_x == curlinelen ) {
						cursor_x--;
						con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x ] = 0;
					} else {
						cursor_x--;
						strcpy( paste_str, con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x + 1 );
						strcpy( con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x, paste_str );
					}
				}
				break;
			}

		} else {

			// typeable characters
			if ( ( ConsoleASCIIValid[ *pos & CON_ASCII_MASK ] ) &&
				 ( cursor_x < edit_max_x ) ) {
				if ( insert_mode && ( cursor_x < curlinelen ) ) {
					if ( curlinelen < edit_max_x ) {
						strcpy( paste_str, con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x );
						strcpy( con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x + 1, paste_str );
						con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x++ ] = (byte)*pos;
					}
				} else {
					con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x++ ] = (byte)*pos;
				}
			}
		}

		// advance one position in circular keybuffer
		KeybBuffer->ReadPos = ( KeybBuffer->ReadPos + 1 ) & KEYB_BUFF_SIZ_M;
		*pos = 0;

		console_input_done = true;
	}

	return console_input_done;
}


// type for writestring function pointer --------------------------------------
//
typedef void (*WSFP)( ... );

//NOTE:
// this declaration is in global scope because of the extern "C".
// only gcc needs the otherwise redundant curly braces, however.



// process quicksay console input and draw it ---------------------------------
//
int QuicksayConsole()
{
/*	// check whether main console has turned quicksay off
	if ( !KeybFlags->ConEnabled || !KeybFlags->ConActive ) {
		return FALSE;
	}

	dword *pos;
	dword readpos = KeybBuffer->ReadPos;

	// scan console input buffer for ESCAPE or ENTER which
	// should turn off quicksay mode again
	while ( *( pos = &KeybBuffer->Data + readpos ) ) {

		if ( *pos == CKC_ESCAPE || *pos == CKC_ENTER ) {
			SetQuicksayConsole( FALSE );
		}

		readpos = ( readpos + 1 ) & KEYB_BUFF_SIZ_M;
	}

	// use standard input handling
	CheckConsoleInput();

	// ensure proper edit line viewing window
	if ( cursor_x < edit_ofs_x ) {
		edit_ofs_x = cursor_x;
	} else if ( cursor_x > edit_ofs_x + ConsoleEnterLength ) {
		edit_ofs_x = cursor_x - ConsoleEnterLength;
	}

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );

	// write text translucent only for color depths below 32 bit per pixel
	WSFP wsfp = translucent ? (WSFP) &D_WriteTrString : (WSFP) &D_WriteString;

	// determine chars to draw
	if ( edit_ofs_x == 0 ) {
		memcpy( draw_line, con_lines[ con_bottom ], ConsoleEnterLength + PROMPT_SIZE );
		draw_line[ ConsoleEnterLength + PROMPT_SIZE ] = 0;
	} else {
		memcpy( draw_line, con_lines[ con_bottom ], PROMPT_SIZE );
		draw_line[ PROMPT_SIZE - 1 ] = '<';
		memcpy( draw_line + PROMPT_SIZE, con_lines[ con_bottom ] + PROMPT_SIZE + edit_ofs_x, ConsoleEnterLength );
		draw_line[ ConsoleEnterLength + PROMPT_SIZE ] = 0;
	}

	int xpos = ConsoleTextX * CharsetInfo[ HUD_CHARSETNO ].width + console_frameofs_x;
	int ypos = 130;

	// draw background bar
	int barwidth  = ( ConsoleEnterLength + PROMPT_SIZE + 1 ) * CharsetInfo[ HUD_CHARSETNO ].width + 6;
	int barheight = CharsetInfo[ HUD_CHARSETNO ].height + 4;
	D_DrawTrRect( xpos - 3, ypos - 2, barwidth, barheight, TRTAB_PANELBACK );

	// draw line contents
	wsfp( draw_line, xpos, ypos, TRTAB_PANELTEXT );

	// maintain blinking of cursor if not done by main console
	if ( ConsoleSliding == 0 ) {
		cursor_state += CurScreenRefFrames;
	}

	// draw cursor
	if ( cursor_state & CURSOR_BLINK_SPEED ) {

		xpos  = ConsoleTextX + cursor_x - edit_ofs_x + PROMPT_SIZE;
		xpos *= CharsetInfo[ HUD_CHARSETNO ].width;
		xpos += console_frameofs_x;

		char *cursorstr = insert_mode ? insert_cursor : overwrite_cursor;
		wsfp( cursorstr, xpos, ypos, TRTAB_PANELTEXT );
	}

	// allow caller to check for disabling
	return KeybFlags->ConActive;
	*/

	return FALSE;
}


// draw console window --------------------------------------------------------
//
PRIVATE
void DrawConsole( bool_t bClear )
{
	//NOTE: in order to minimize screen flicker, we only clear the inputline, 
	//      if the previous contents was longers
	if ( bClear )
		wclear( g_curses_in_win );

	mvwprintw( g_curses_in_win, 0, 0, con_lines[ con_bottom ] );
	wmove( g_curses_in_win, 0, cursor_x + PROMPT_SIZE );
	wrefresh( g_curses_in_win );
}


// main console function: process keyboard input and draw console window ------
//
PUBLIC
void CON_ConsoleMain()
{
	static bool_t init_prompt_drawn = false;

	// set number of currently visible console lines
	// (negative values allowed!)
	cur_vis_conlines = ConsoleHeight;

	unsigned int len_before = strlen( con_lines[ con_bottom ] );

	// process console keyboard input
	if ( CheckConsoleInput() || !init_prompt_drawn ) {

		init_prompt_drawn = true;
		// draw console
		DrawConsole( len_before > strlen( con_lines[ con_bottom ] ) );
	}
}
