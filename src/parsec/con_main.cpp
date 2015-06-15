/*
 * PARSEC - Console Main Module
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
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
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_font.h"

// local module header
#include "con_main.h"

// proprietary module headers
#include "con_act.h"
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_conf.h"
#include "con_ext.h"
#include "con_info.h"
#include "con_int.h"
#include "con_kmap.h"
#include "con_list.h"
#include "con_rc.h"
#include "con_std.h"
#include "con_tab.h"
#include "con_vald.h"
#include "e_color.h"
#include "e_demo.h"
#include "e_draw.h"
#include "g_supp.h"
#include "h_cockpt.h"
#include "h_strmap.h"
#include "keycodes.h"
#include "m_main.h"
#include "sys_path.h"
#include "sys_swap.h"
#include "g_bot_cl.h"

// flags
#define PROCESS_STARTUP_SCRIPT				// execute startup (boot) script
#define FORCE_CONSOLE_LOGIN					// have to login to exec commands
#define LOSE_KEYBOARD_FOCUS_ON_SLIDEUP		// console loses focus on slide up


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
PUBLIC char boot_script_name[]		= "boot" CON_FILE_EXTENSION;
PUBLIC char bot_script_name[]		= "bot" CON_FILE_EXTENSION;
static char demo_script_name[]		= "demo" CON_FILE_EXTENSION;

// various string constants
PUBLIC char con_prompt[]			= "::";
PUBLIC char ok_prompt[] 			= "ok.";
PUBLIC char press_space[]			= "- press space -";
PUBLIC char too_many_args[] 		= "too many arguments.";
PUBLIC char arg_missing[]			= "argument missing.";
PUBLIC char invalid_arg[]			= "invalid argument.";
PUBLIC char range_error[]			= "range error.";
PUBLIC char unknown_command[]		= "unknown command.";
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

int  con_in_talk_mode	= true;
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
	// if we are in bot mode, run bot.con
	if(headless_bot){
		startupscript = bot_script_name;
		ExecConsoleFile(startupscript, echo);
	} else {
		// start normal or mod boot scripts
		if ( mod_numnames > 0 ) {

			ASSERT( mod_names[ 0 ] != NULL );

			// if mod doesn't override our own packages, execute our boot.con too
			if ( !mod_override ) {
				startupscript = PlayDemo ? demo_script_name : boot_script_name;
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
			startupscript = PlayDemo ? demo_script_name : boot_script_name;
			ExecConsoleFile( startupscript, echo );
		}
	}
}


// init memory needed for console ---------------------------------------------
//
void InitConsole()
{
	ASSERT( !console_init_done );

	if ( !console_init_done ) {

		// erase console contents
		int lid = 0;
		for ( lid = 0; lid < NUM_CONSOLE_LINES; lid++ ) {
			EraseConLine( lid );
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

#ifdef PROCESS_STARTUP_SCRIPT

		// build list of external commands
		BuildExternalCommandList();

		// exec boot.con (also for registered mods)
		ExecStartupScript( ECHO_STARTUP_SCRIPT );

#endif // PROCESS_STARTUP_SCRIPT

		// insert initial prompt
		CON_AddLine( con_prompt );

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
	// make sure we call demo stop instead of full clear
	int oldclearflag = AUX_DISABLE_CLEARDEMO_ON_REPSTOP;
	AUX_DISABLE_CLEARDEMO_ON_REPSTOP = 1;

	// make sure no demo is playing
	DEMO_StopReplay();

	// restore previous flag state
	AUX_DISABLE_CLEARDEMO_ON_REPSTOP = oldclearflag;
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
void KillConsole()
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
	// Not needed
}


// add new message to console (preserve current input) ------------------------
//
void CON_AddMessage( const char *text )
{
	ASSERT( text != NULL );
	ASSERT( strlen( text ) <= MAX_CONSOLE_LINE_LENGTH );

	//NOTE:
	// this function saves the current console
	// line, then overwrites it with the supplied
	// text, starts a new line and re-outputs the
	// old text there. this is useful to display
	// messages in the console without destroying
	// the current input.

	// intermediate console line storage
	static char con_save[ MAX_CONSOLE_LINE_LENGTH + 1 ];

	// save input line and cursor pos
	int curspos = cursor_x;
	strcpy( con_save, con_lines[ con_bottom ] );

	// output supplied message
	EraseConLine( con_bottom );
	strcpy( con_lines[ con_bottom ], text );

	// save old state
	last_con_bottom  = con_bottom;
	last_con_content = con_content;

	// increase bottom pointer
	con_bottom = ( con_bottom + 1 ) & NUM_CONSOLE_LINES_MASK;
	if ( con_content < NUM_CONSOLE_LINES ) {
		con_content++;
	}

	// restore input line
	EraseConLine( con_bottom );
	strcpy( con_lines[ con_bottom ], con_save );

	// restore cursor pos
	cursor_x = curspos;
}


// flag whether next CON_AddLine() should overwrite the current line ----------
//
static int skip_next_nl = FALSE;


// add new line to console ----------------------------------------------------
//
void CON_AddLine( const char *text )
{
	ASSERT( text != NULL );
	ASSERT( strlen( text ) <= MAX_CONSOLE_LINE_LENGTH );

	//NOTE:
	// if console commands are executed in non-interactive mode
	// (i.e., after at least one ac.wait command has been executed),
	// CON_AddMessage() must be used instead of CON_AddLine() to
	// ensure correct line-feeds in the console.

	// allow mapping to CON_AddMessage()
	if ( con_non_interactive ) {
		CON_AddMessage( text );
		return;
	}

	//NOTE:
	// this function preserves the current console
	// line as it is, starts a new line and outputs
	// the supplied text in that line. the console
	// cursor is reset to the first column.

	// increase bottom pointer if not disabled
	if ( !skip_next_nl ) {

		// save old state
		last_con_bottom  = con_bottom;
		last_con_content = con_content;

		con_bottom = ( con_bottom + 1 ) & NUM_CONSOLE_LINES_MASK;
		if ( con_content < NUM_CONSOLE_LINES ) {
			con_content++;
		}
	}

	// clear flag
	skip_next_nl = FALSE;

	// output supplied line
	EraseConLine( con_bottom );
	strcpy( con_lines[ con_bottom ], text );

	// set cursor to first column
	cursor_x = 0;
}


// add line feed in console ---------------------------------------------------
//
void CON_AddLineFeed()
{
	// force line feed
	skip_next_nl = FALSE;
	CON_AddLine( con_prompt );
}


// set flag to disable next line feed in CON_AddLine() ------------------------
//
void CON_DisableLineFeed()
{
	// set flag
	skip_next_nl = TRUE;
}


// set flag to not eat next line feed in CON_AddLine() ------------------------
//
void CON_EnableLineFeed()
{
	// clear flag
	skip_next_nl = FALSE;
}


// delete current console line ------------------------------------------------
//
void CON_DelLine()
{
	EraseConLine( con_bottom );
}


// return pointer to current console line -------------------------------------
//
char *CON_GetLine()
{
	return con_lines[ con_bottom ];
}


// print prompt for list continuation -----------------------------------------
//
void CON_ListCtdPrompt()
{
	CON_AddLine( press_space );
	cursor_x = press_space_curs_x;

	// ensure proper view alignment
	edit_ofs_x = ( cursor_x > ConsoleEnterLength ) ?
		( cursor_x - ConsoleEnterLength ) : 0;
}


// print prompt after list has ended ------------------------------------------
//
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


// maximum console extents structure ------------------------------------------
//
struct maxconext_s {

	int	c_x,  c_y;
	int	c_w,  c_h;
	int	c_pw, c_ph;
};


// calculate current maximum console extents according to resolution ----------
//
PRIVATE
void GetMaxConExtents( maxconext_s *exs )
{
	int bordx;
	int bordy;
	int linedist;

	int chrwidth = CharsetInfo[ HUD_CHARSETNO ].width;

	console_frameofs_x = 4;
	console_frameofs_y = 0;
	console_frame_left = 8;
	console_frame_top  = 5;
	bordx = 3;
	bordy = 4;
	linedist = CharsetInfo[ HUD_CHARSETNO ].height + 2;

	exs->c_x  = ConsoleTextX - 1;
	exs->c_y  = ConsoleTextY - 1;
	exs->c_w  = Screen_Width / chrwidth - bordx;
	exs->c_h  = Screen_Height / linedist - bordy;
	exs->c_pw = ConsoleEnterLength + 3;
	exs->c_ph = ConsoleHeight;
}


// check geometric extents of console window ----------------------------------
//
void CheckConExtents()
{
	// only if video mode geometry known
	if ( TextModeActive )
		return;

	// get extent bounds
	maxconext_s exs;
	GetMaxConExtents( &exs );

	// check width
	if ( exs.c_x < exs.c_w / 2 ) {

		//NOTE:
		// if console window starts in left half of screen,
		// resize it to a valid width by adjusting the
		// window's width and leave x-position as selected
		// by the user.

		if ( exs.c_x + exs.c_pw > exs.c_w ) {
			exs.c_pw = exs.c_w - exs.c_x;
			if ( exs.c_pw - 3 < int_commands[ CONINFOINDX_CONWIDTH ].bmin ) {
				exs.c_x  = int_commands[ CONINFOINDX_CONWINX ].bmin - 1;
				exs.c_pw = exs.c_w - exs.c_x;
			}
		}
	} else {

		//NOTE:
		// if console window starts in right half of screen,
		// resize it to a valid width by adjusting the
		// window's x-position and leave its width as
		// selected by the user.

		if ( exs.c_x + exs.c_pw > exs.c_w ) {
			exs.c_x = exs.c_w - exs.c_pw;
			if ( exs.c_x + 1 < int_commands[ CONINFOINDX_CONWINX ].bmin ) {
				exs.c_x  = int_commands[ CONINFOINDX_CONWINX ].bmin - 1;
				exs.c_pw = exs.c_w - exs.c_x;
			}
		}
	}

	// check height
	if ( exs.c_y < exs.c_h / 2 ) {

		//NOTE:
		// if console window starts in upper half of screen,
		// resize it to a valid height by adjusting the
		// window's height and leave y-position as selected
		// by the user.

		if ( exs.c_y + exs.c_ph > exs.c_h ) {
			exs.c_ph = exs.c_h - exs.c_y;
			if ( exs.c_ph < int_commands[ CONINFOINDX_CONHEIGHT ].bmin ) {
				exs.c_y  = int_commands[ CONINFOINDX_CONWINY ].bmin - 1;
				exs.c_ph = exs.c_h - exs.c_y;
			}
		}

	} else {

		//NOTE:
		// if console window starts in lower half of screen,
		// resize it to a valid height by adjusting the
		// window's y-position and leave its height as
		// selected by the user.

		if ( exs.c_y + exs.c_ph > exs.c_h ) {
			exs.c_y = exs.c_h - exs.c_ph;
			if ( exs.c_y + 1 < int_commands[ CONINFOINDX_CONWINY ].bmin ) {
				exs.c_y  = int_commands[ CONINFOINDX_CONWINY ].bmin - 1;
				exs.c_ph = exs.c_h - exs.c_y;
			}
		}
	}

	// set new position and size
	ConsoleTextX		= exs.c_x + 1;
	ConsoleTextY		= exs.c_y + 1;
	ConsoleEnterLength	= exs.c_pw - 3;
	ConsoleHeight		= exs.c_ph;

	int_commands[ CONINFOINDX_CONWIDTH ].bmax	= min( exs.c_w - exs.c_x - 3, MAX_CONSOLE_LINE_LENGTH - 3 );
	int_commands[ CONINFOINDX_CONHEIGHT ].bmax	= min( exs.c_h - exs.c_y, NUM_CONSOLE_LINES );
	int_commands[ CONINFOINDX_CONWINX ].bmax	= exs.c_w - exs.c_pw + 1;
	int_commands[ CONINFOINDX_CONWINY ].bmax	= exs.c_h - exs.c_ph + 1;
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
		if ( int_commands[ icom ].fetch != NULL )
			*int_commands[ icom ].intref = (*int_commands[ icom ].fetch)();

		if ( CheckSetIntArgBounded( int_print_base, scan,
									(char * ) int_commands[ icom ].command,
									int_commands[ icom ].intref,
									int_commands[ icom ].bmin,
									int_commands[ icom ].bmax,
									int_commands[ icom ].realize ) ) {
			ConsoleHeight += 2;
			return TRUE;
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
	const char *scan = strtok( command, " " );

	// make correct name querying possible
	strcpy( temp_playername, LocalPlayerName );

	// check different commands
	if ( scan != NULL ) {

		if ( ExecGetSetInteger( scan ) )
			{}

		else if ( CheckSetObjTypeProperty( int_print_base, scan ) )
			{}

		else if ( CheckSetObjClassProperty( int_print_base, scan ) )
			{}

		else if ( CheckSetObjInstanceProperty( int_print_base, scan ) )
			{}

		else if ( CheckTypeInfo( scan ) )
			{}

		else if ( CheckClassInfo( scan ) )
			{}

		else if ( CheckInstanceInfo( scan ) )
			{}

		else if ( CheckKeyMappingEcho( (char *) scan ) )
			{}

		else if ( CheckKeyMappingSilent( (char *) scan ) )
			{}

		else if ( CheckSetIntArray( int_print_base, (char *) scan, auxflag_array_name, AuxEnabling, MAX_AUX_ENABLING ) )
			{}

		else if ( CheckSetIntArray( int_print_base, (char *) scan, auxdata_array_name, AuxData, MAX_AUX_DATA ) )
			{}

		else if ( CheckSetStrArgument( TRUE, (char *) scan, (char *) CMSTR( CM_NAME ), temp_playername, MAX_PLAYER_NAME ) )
			CheckPlayerName( temp_playername );

		else if ( CheckColorConfig( (char *) scan ) )
			{}

		else if ( CheckConfigCommand( (char *) scan ) )
			{}

		else
			CON_AddLine( unknown_command );

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
	KEY_COMPILE_AUTHOR
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
		CON_AddLine( unknown_command );
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

	// insert command into script if currently open
	if ( !InsertCommandLog( con_lines[ con_bottom ] + PROMPT_SIZE ) ) {

		// execute current linecontent
		ExecConsoleCommand( con_lines[ con_bottom ] + PROMPT_SIZE, TRUE );

		if ( await_keypress ) {
			// listcommand: print listmode prompt
			CON_ListCtdPrompt();
		} else {
			// normal command: print standard prompt
			CON_AddLine( con_prompt );
		}

	} else {

		// command was not executed, but only
		// written to an output script

		if ( HistoryForBatchEntry )
			AddLineToHistory( con_lines[ con_bottom ] + PROMPT_SIZE );
		CON_AddLine( con_prompt );
	}
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


// handle text input in console window (ASCII only for now) -------------------
//
void CON_HandleTextInput(const char character)
{
	if (await_keypress && com_cont_func)
		return;

	// don't write console open/close key
	if (character == '`' || character == '~')
		return;

	int curlinelen = strlen( con_lines[ con_bottom ] + PROMPT_SIZE );

	// typeable characters
	if ( ( character & CON_ASCII_MASK ) && (character <= CON_ASCII_MASK) && (cursor_x < edit_max_x ) ) {
		if ( insert_mode && ( cursor_x < curlinelen ) ) {
			if ( curlinelen < edit_max_x ) {
				strcpy( paste_str, con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x );
				strcpy( con_lines[ con_bottom ] + PROMPT_SIZE + cursor_x + 1, paste_str );
				con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x++ ] = (byte) character;
			}
		} else {
			con_lines[ con_bottom ][ PROMPT_SIZE + cursor_x++ ] = (byte) character;
		}
	}
}


// handle keypresses in console window ----------------------------------------
//
void CON_HandleKeyPress(dword key)
{
	int curlinelen = strlen( con_lines[ con_bottom ] + PROMPT_SIZE );

	if ( await_keypress && com_cont_func ) {

		// <space> or <enter> continues listing
		if ( ( key == CKC_SPACE ) || ( key == CKC_ENTER ) ) {

			EraseConLine( con_bottom );
			strcpy( con_lines[ con_bottom ], con_prompt );
			// ensure wait prompt gets overwritten
			con_bottom  = last_con_bottom;
			con_content = last_con_content;
			cursor_x    = 0;
			com_cont_func();

		// <escape> returns to command prompt immediately
		} else if ( key == CKC_ESCAPE ) {

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

	} else if ( key == CKC_ESCAPE ||
				key == CKC_CURSORUP ||
				key == CKC_CURSORDOWN ||
				key == CKC_CURSORLEFT ||
				key == CKC_CURSORRIGHT ||
				key == CKC_HOME ||
				key == CKC_END ||
				key == CKC_PAGEUP ||
				key == CKC_PAGEDOWN ||
				key == CKC_INSERT ||
				key == CKC_DELETE ||
				key == CKC_TAB ||
				key == CKC_ENTER ||
				key == CKC_BACKSPACE ) {

		// control keys (CKC_xx)
		switch ( key ) {

		// <escape>: erase entire command line
		case CKC_ESCAPE:
			EraseConLine( con_bottom );
			strcpy( con_lines[ con_bottom ], con_prompt );
			history_scan  = history_add;
			cursor_x      = 0;
			con_back_view = 0;
			return;
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
/*
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
*/
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
			if ( !con_logged_in )
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
			if ( !con_logged_in )
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
	}

	if (key == CKC_ENTER || key == CKC_ESCAPE) {
		// hide QuickSay console if the main console isn't open
		if (KeybFlags->ConEnabled && KeybFlags->ConActive && cur_vis_conlines <= 0)
			SetQuicksayConsole(FALSE);
	}
}


//NOTE:
// this declaration is in global scope because of the extern "C".
// only gcc needs the otherwise redundant curly braces, however.


// set quicksay console state -------------------------------------------------
//
void SetQuicksayConsole( int enable )
{
	if ( enable ) {

		KeybFlags->ConEnabled = TRUE;
		KeybFlags->ConActive  = TRUE;

	} else {
		// FIXME: unintentionally disables main console input in some situations
		KeybFlags->ConEnabled  = FALSE;
	}
}


// draw quicksay console ------------------------------------------------------
//
int QuicksayConsole()
{
	// check whether main console has turned quicksay off
	if ( !KeybFlags->ConEnabled || !KeybFlags->ConActive ) {
		return FALSE;
	}

	// ensure proper edit line viewing window
	if ( cursor_x < edit_ofs_x ) {
		edit_ofs_x = cursor_x;
	} else if ( cursor_x > edit_ofs_x + ConsoleEnterLength ) {
		edit_ofs_x = cursor_x - ConsoleEnterLength;
	}

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );

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
	D_WriteTrString( draw_line, xpos, ypos, TRTAB_PANELTEXT );

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
		D_WriteTrString( cursorstr, xpos, ypos, TRTAB_PANELTEXT );
	}

	// allow caller to check for disabling
	return KeybFlags->ConActive;
}


// draw console window --------------------------------------------------------
//
PRIVATE
void DrawConsole( int numlines )
{
	ASSERT( numlines >= 0 );

	if ( numlines > NUM_CONSOLE_LINES )
		numlines = NUM_CONSOLE_LINES;
	if ( ConsoleEnterLength < CONSOLE_MIN_ENTER_LENGTH )
		ConsoleEnterLength = CONSOLE_MIN_ENTER_LENGTH;

	// disable back-viewing if console not fully open
	if ( ConsoleSliding != ( ConsoleHeight << CONSOLE_SLIDE_SPEED ) ) {
		con_back_view = 0;
	}

	// set font spacing according to resolution
	int chrwidth = CharsetInfo[ HUD_CHARSETNO ].width;
	int linedist = CharsetInfo[ HUD_CHARSETNO ].height + 2;

	// determine whether translucency should be used
	int translucent = VID_TRANSLUCENCY_SUPPORTED;

	// draw console background
	int px = ConsoleTextX * chrwidth - console_frame_left;
	int py = ConsoleTextY * linedist - console_frame_top;
	int pw = ( ConsoleEnterLength + PROMPT_SIZE + 1 ) * chrwidth + ( console_frame_left * 2 );
	int ph = numlines * linedist + ( console_frame_top * 2 );

	if ( translucent ) {
		D_DrawTrRect( px + console_frameofs_x, py + console_frameofs_y,
					  pw, ph, TRTAB_PANELBACK );

		DRAW_PanelDecorations( px + console_frameofs_x, py + console_frameofs_y, pw, ph );
	}

	// return if caption invisible
	if ( numlines < 1 ) {
		return;
	}

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );


	// display caption if console wide enough
	if ( (dword)ConsoleEnterLength >= strlen( console_caption ) - PROMPT_SIZE - 1 ) {
		D_WriteTrString( console_caption,
			  ConsoleTextX * chrwidth + console_frameofs_x,
			  ConsoleTextY * linedist + console_frameofs_y,
			  TRTAB_PANELTEXT );
	}

	// maintain blinking of cursor
	cursor_state += CurScreenRefFrames;

	int lineno    = 0;
	int numtodraw = numlines - CONSOLE_CAPTION_HEIGHT;

	// return if content invisible
	if ( numtodraw < 1 ) {
		return;
	}

	// calc content line to start with and number of lines to display
	if ( con_content < numtodraw ) {
		numtodraw     = con_content;
		con_back_view = 0;
	} else {
		lineno = ( con_bottom - numtodraw + 1 ) & NUM_CONSOLE_LINES_MASK;
		// allow viewing of past pages/lines
		if ( con_back_view > 0 ) {
			int backnum = con_content - numtodraw;
			if ( backnum < con_back_view )
				con_back_view = backnum;
			lineno = ( lineno - con_back_view ) & NUM_CONSOLE_LINES_MASK;
		}
	}

	// ensure proper edit line viewing window
	if ( cursor_x < edit_ofs_x ) {
		edit_ofs_x = cursor_x;
	} else if ( cursor_x > edit_ofs_x + ConsoleEnterLength ) {
		edit_ofs_x = cursor_x - ConsoleEnterLength;
	}

	int drawx = console_frameofs_x + ( ConsoleTextX ) * chrwidth;
	int drawy = console_frameofs_y + ( ConsoleTextY + CONSOLE_CAPTION_HEIGHT ) * linedist;

	// draw console content lines (except bottom)
	for ( int lct = numtodraw - 1; lct > 0; lct-- ) {

		memcpy( draw_line, con_lines[ lineno ], ConsoleEnterLength + PROMPT_SIZE );
		draw_line[ ConsoleEnterLength + PROMPT_SIZE ] = 0;

		D_WriteTrString( draw_line, drawx, drawy, TRTAB_PANELTEXT );

		drawy += linedist;
		lineno = ( lineno + 1 ) & NUM_CONSOLE_LINES_MASK;
	}

	// draw bottom console line
	if ( ( edit_ofs_x == 0 ) || ( con_back_view > 0 ) ) {
		memcpy( draw_line, con_lines[ lineno ], ConsoleEnterLength + PROMPT_SIZE );
		draw_line[ ConsoleEnterLength + PROMPT_SIZE ] = 0;
	} else {
		memcpy( draw_line, con_lines[ lineno ], PROMPT_SIZE );
		draw_line[ PROMPT_SIZE - 1 ] = '<';
		memcpy( draw_line + PROMPT_SIZE, con_lines[ lineno ] + PROMPT_SIZE + edit_ofs_x, ConsoleEnterLength );
		draw_line[ ConsoleEnterLength + PROMPT_SIZE ] = 0;
	}

#ifdef FORCE_CONSOLE_LOGIN

	// hide bottom line during login mode
	if ( con_login_mode ) {
		for ( int spos = 0; spos < ConsoleEnterLength; spos++ ) {
			if ( draw_line[ spos + PROMPT_SIZE ] == 0 )
				break;
			draw_line[ spos + PROMPT_SIZE ] = '*';
		}
	}

#endif // FORCE_CONSOLE_LOGIN

	D_WriteTrString( draw_line, drawx, drawy, TRTAB_PANELTEXT );

	// cursor only if no back-viewing
	if ( con_back_view == 0 ) {

		// draw cursor
		if ( !KeybFlags->ConActive || ( cursor_state & CURSOR_BLINK_SPEED ) ) {

			// determine type of cursor (insert/overwrite)
			char *cursorstr = insert_mode ? insert_cursor : overwrite_cursor;

			int curcursorline = numtodraw + CONSOLE_CAPTION_HEIGHT - 1;

			drawx = ConsoleTextX + cursor_x - edit_ofs_x + PROMPT_SIZE;
			drawy = ConsoleTextY + curcursorline;

			drawx = console_frameofs_x + drawx * chrwidth;
			drawy = console_frameofs_y + drawy * linedist;

			D_WriteTrString( cursorstr, drawx, drawy, TRTAB_PANELTEXT );
		}
	}
}


// main console function: process keyboard input and draw console window ------
//
PRIVATE
void ConsoleMain( int numlines )
{
	ASSERT( numlines >= 0 );

	// set number of currently visible console lines
	// (negative values allowed!)
	cur_vis_conlines = numlines - CONSOLE_CAPTION_HEIGHT;

	// draw console
	DrawConsole( numlines );
}


// control console sliding ----------------------------------------------------
//
void ConsoleControl()
{
	//NOTE:
	// this function is called from within the gameloop.
	// (G_MAIN::GameLoop().)

	//NOTE:
	// ( ConsoleSliding == 0 ) means the console
	// is currently not visible.

	if ( ConsoleSliding != 0 ) {

		int concurheight = ConsoleHeight;
		int conslidemax  = ConsoleHeight << CONSOLE_SLIDE_SPEED;

		if ( ConsoleSliding < 0 ) {

			if ( ConsoleSliding < -CurScreenRefFrames ) {

				// slide console up
				ConsoleSliding += CurScreenRefFrames;

			} else {

				// console is fully up: disable it
				ConsoleSliding		  = 0;
				KeybFlags->ConEnabled = FALSE;
			}

			// derive current height from ConsoleSliding
			concurheight = ( -ConsoleSliding ) >> CONSOLE_SLIDE_SPEED;

		} else if ( ConsoleSliding != conslidemax ) {

			//NOTE:
			// if the console is open (and not in the process
			// of being closed) its size should be conslidemax.
			// if this is not the case try to reach this goal by
			// sliding up or down, respectively.

			if ( ConsoleSliding < conslidemax ) {

				// try to reach conslidemax by sliding down
				ConsoleSliding += CurScreenRefFrames;

				// guard against overshoot
				if ( ConsoleSliding > conslidemax )
					ConsoleSliding = conslidemax;

			} else {

				// try to reach conslidemax by sliding up
				ConsoleSliding -= CurScreenRefFrames;

				// guard against overshoot
				if ( ConsoleSliding < conslidemax )
					ConsoleSliding = conslidemax;

				// get extent bounds
				maxconext_s exs;
				GetMaxConExtents( &exs );

				// ensure console height is not invalid while
				// sliding up and trying to reach its valid end-height
				int curheight = ConsoleSliding >> CONSOLE_SLIDE_SPEED;
				if ( exs.c_y + curheight > exs.c_h ) {
					curheight      = exs.c_h - exs.c_y;
					ConsoleSliding = curheight << CONSOLE_SLIDE_SPEED;
				}
			}

			// derive current height from ConsoleSliding
			concurheight = ConsoleSliding >> CONSOLE_SLIDE_SPEED;
		}

		// draw console
		ConsoleMain( concurheight );
	}
}


// toggle console on/off (slide down and up, respectively) --------------------
//
void ToggleConsole()
{
	//NOTE:
	// this function is called from within the gameloop
	// if the console toggle key has been pressed.
	// (G_MAIN::SpecialKeyFunctions().)

	//NOTE:
	// ( ConsoleSliding == 0 ) means the console
	// is currently not visible. otherwise it will
	// either be negative (sliding up) or positive
	// (sliding down or currently at full size).

	// slide console down if currently off
	if ( ConsoleSliding == 0 ) {

		KeybFlags->ConEnabled = TRUE;
		HelpActive	   		  = FALSE;
		ConsoleSliding		  = 1;

		//NOTE:
		// after ConsoleSliding has been set to 1
		// it will be increased by ConsoleControl()
		// until it reaches full height. there
		// it will stay until the console is closed
		// or ConsoleHeight is altered to change
		// the console's height. (whereafter
		// ConsoleControl() will once again try to
		// achieve ( ConsoleSliding == ConsoleHeight*fac )
		// by sliding up or down, as needed.

		// hide certain screen elements
		SlideOutFloatingMenu();
		FadeOutCockpit();
		MAP_FadeOutStarmap();

		// ensure no dangling keys in
		// console's keyboard buffer
		FlushConsoleBuffer();

	} else {

		// invert direction of sliding
		ConsoleSliding = -ConsoleSliding;

#ifdef LOSE_KEYBOARD_FOCUS_ON_SLIDEUP

		// console loses focus if sliding up
		KeybFlags->ConEnabled = ( ConsoleSliding > 0 );

#endif // LOSE_KEYBOARD_FOCUS_ON_SLIDEUP

		// display/hide certain screen elements
		if ( ConsoleSliding < 0 ) {

			SlideInFloatingMenu();
			FadeInCockpit();
			MAP_FadeInStarmap();

		} else {

			SlideOutFloatingMenu();
			FadeOutCockpit();
			MAP_FadeOutStarmap();
		}
	}
}



