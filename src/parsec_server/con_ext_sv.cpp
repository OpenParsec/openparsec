/*
 * PARSEC - Server External Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:44 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2001
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

// global externals
#include "globals.h"

// local module header
#include "con_ext_sv.h"

// proprietary module headers
//#include "con_act.h"
#include "con_aux_sv.h"
#include "con_list_sv.h"
#include "con_main_sv.h"
#include "con_std_sv.h"
#include "con_vald.h"		// shared !
//#include "e_demo.h"
//#include "e_record.h"
//#include "e_replay.h"
#include "sys_file.h"
#include "sys_path.h"



// maximum depth of recursion in command batch files
#define MAX_BATCH_REC_DEPTH		16

// special eof needed in package console scripts
#define PACKAGE_CON_EOF			";eof"
#define PACKAGE_CON_EOF_LEN		4

// generic string paste area
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];

// string constants
static char con_extension[]		= CON_FILE_EXTENSION;
static char invalid_batch_name[]= "invalid name specifier.";
static char recursion_canceled[]= "command canceled due to recursion.";
static char recursion_maxdepth[]= "maximum recursion depth reached.";
static char replaying_script[]	= "replay in progress (script).";
static char replaying_binary[]	= "replay in progress (binary).";
static char no_replay[]			= "no replay in progress.";
static char stopped_replay[]	= "replay has been stopped.";
static char record_disallowed[]	= "recording not allowed (object camera is active).";

static char con_file_wldcard1[]	= "*" CON_FILE_EXTENSION;
static char con_file_wldcard2[] = REFCON_COMMANDS_DIR "*" CON_FILE_EXTENSION;
static char con_file_wldcard3[] = STDCON_COMMANDS_DIR "*" CON_FILE_EXTENSION;
static char con_file_wldcard4[] = RECORD_COMMANDS_DIR "*" CON_FILE_EXTENSION;

// name of batch command file to execute on every gameloop start
char gameloop_start_script[ PATH_MAX + 1 ] = "";

// list of external commands
char external_commands[ MAX_EXTERNAL_COMMANDS + 1 ][ COMMAND_NAME_ALLOC_LEN + 1 ];
int  external_command_types[ MAX_EXTERNAL_COMMANDS + 1 ];
int	 num_external_commands = 0;

// external batch execution vars
int processing_batch = 0;	// flag that commands are called from batch
int idle_reclevel    = 0;	// level of recursion idle wait stalled execution on
int	rec_depth        = 0;	// current level of recursion

// action commands not allowed?
int preempt_action_commands = FALSE;

// batch execution recursive call stack
static int	extcom_on_level[ MAX_BATCH_REC_DEPTH + 1 ];
static FILE *fp_for_level[ MAX_BATCH_REC_DEPTH + 1 ];

// log file (created command batch)
FILE *log_fp;
char log_name[ COMMAND_NAME_ALLOC_LEN + 1 ];
int  log_commands = 0;

// recording file
static char record_name[ COMMAND_NAME_ALLOC_LEN + 1 ];

// packet recording/replay file names
char packet_record_name[ COMMAND_NAME_ALLOC_LEN + 1 ];
char packet_replay_name[ COMMAND_NAME_ALLOC_LEN + 1 ];

// single command line
static char com_line[ MAX_CONSOLE_LINE_LENGTH + 1 ];

// cat feature variables
static char cat_name[ MAX_CONSOLE_LINE_LENGTH + 1 ];
static int  cat_pos;

// current active mod name
PUBLIC char* mod_names[ MAX_REGISTERED_MODS ];
PUBLIC int   mod_numnames = 0;
PUBLIC int   mod_override = FALSE;


// acquire scripts located in mod directories ---------------------------------
//
PRIVATE
void AcquireModScripts()
{
	for ( int mod = 0; mod < mod_numnames; mod++ ) {

		char *path = (char *) ALLOCMEM( strlen( mod_names[ mod ] ) + strlen( con_file_wldcard1 ) + 2 );
		if ( path == NULL ) {
			OUTOFMEM( 0 );
		}

		strcpy( path, mod_names[ mod ] );
		strcat( path, "/" );
		strcat( path, con_file_wldcard1 );

		char *npath = SYSs_ProcessPathString( path );
		SYSs_AcquireScriptPath( npath, COMTYPE_MODIFICATION, mod_names[ mod ] );

		FREEMEM( path );
	}
}


// build list of externally available commands --------------------------------
//
void BuildExternalCommandList()
{
	//NOTE:
	// the sequence of paths scanned in this function mostly determines the
	// display order of console scripts (ls command, tab completion, ...).
	// the actual precedence of scripts is determined by the search order
	// in OpenScript(). for consistency, the two orders should be the same.

	num_external_commands = 0;

	SYS_AcquirePackageScripts( COMTYPE_PACKAGE );

	if ( mod_override ) {
		AcquireModScripts();
	}

	SYSs_AcquireScriptPath( con_file_wldcard1, COMTYPE_ROOT, NULL );
	SYSs_AcquireScriptPath( con_file_wldcard2, COMTYPE_REFERENCE, NULL );
	SYSs_AcquireScriptPath( con_file_wldcard3, COMTYPE_STANDARD, NULL );
	SYSs_AcquireScriptPath( con_file_wldcard4, COMTYPE_RECORDING, NULL );

	if ( !mod_override ) {
		AcquireModScripts();
	}
}


// check if string is valid external command ----------------------------------
//
int CheckExternalCommand( char *com )
{
	ASSERT( com != NULL );

	//NOTE:
	// command script directories are only scanned once. therefore,
	// new commands copied into a directory while the game is still
	// running will not be noticed until next startup or explicit
	// rescanning via the "rescan" command.

	// to ensure scripts with explicitly specified paths work
	com = SYSs_ProcessPathString( com );

	// scan list in memory (no directory rescanning!!)
	int cid = -1;
	for ( int eid = 0; eid < num_external_commands; eid++ ) {
		ASSERT( external_commands[ eid ] != NULL );
		if ( stricmp( external_commands[ eid ], com ) == 0 ) {
			cid = eid;
			break;
		}
	}

	// return command number + 1 (thus 0 means command is invalid)
	return ( cid + 1 );
}


// insert command into log file (command batch) -------------------------------
//
int InsertCommandLog( char *command )
{
	ASSERT( command != NULL );

	if ( log_commands && ( strcmp( command, CMSTR( CM_CLOSE ) ) != 0 ) ) {
		// append command to currently open logfile
		fprintf( log_fp, "%s\n", command );
		return TRUE;
	}

	return FALSE;
}


// search all console script paths and open batch if it exists ----------------
//
PRIVATE
FILE *OpenScript( char *script )
{
	ASSERT( script != NULL );

	//NOTE:
	// the sequence of paths scanned in this function determines the
	// precedence of console scripts of the same name but located in
	// different directories.

	//NOTE:
	// the path search sequence is:
	// 1. data package
	// 2. root (local) directory
	// 3. reference directory (refs)
	// 4. standard directory (cons)
	// 5. recording directory (recs)

	//NOTE:
	// to avoid inconsistencies the search order in E_RECORD::LoadPacket()
	// and E_DEMO::DEMO_BinaryOpenDemo() should be consistent
	// with the search order used here.

	FILE *fp = NULL;

	strcpy( paste_str, script );
	strcat( paste_str, con_extension );

	// to ensure scripts with explicitly specified paths work
	char *path = SYSs_ProcessPathString( paste_str );

	if ( SV_PACKAGE_SCRIPTS ) {

		// try to read from package first
		if ( ( fp = SYS_fopen( path, "r" ) ) != NULL ) {
			return fp;
		}
	}

	// search root (local) directory
	if ( ( fp = fopen( path, "r" ) ) != NULL ) {
		return fp;
	}

	strcpy( paste_str, REFCON_COMMANDS_DIR );
	strcat( paste_str, script );
	strcat( paste_str, con_extension );

	// search reference directory (refs)
	if ( ( fp = fopen( paste_str, "r" ) ) != NULL ) {
		return fp;
	}

	strcpy( paste_str, STDCON_COMMANDS_DIR );
	strcat( paste_str, script );
	strcat( paste_str, con_extension );

	// search standard directory (cons)
	if ( ( fp = fopen( paste_str, "r" ) ) != NULL ) {
		return fp;
	}

	strcpy( paste_str, RECORD_COMMANDS_DIR );
	strcat( paste_str, script );
	strcat( paste_str, con_extension );

	// search recording directory (recs)
	return fopen( paste_str, "r" );
}


// display content of command batch file --------------------------------------
//
PRIVATE
void CatExternalCommands_ctd()
{
	int  i = 0;

	FILE *fp = OpenScript( cat_name );

	if ( fp != NULL ) {

		SYS_fseek( fp, cat_pos, SEEK_SET );

		int packeof = 0;
		while ( SYS_fgets( com_line, MAX_CONSOLE_LINE_LENGTH, fp ) != NULL ) {

			// check special package eof
			if ( strncmp( com_line, PACKAGE_CON_EOF, PACKAGE_CON_EOF_LEN ) == 0 ) {
				packeof = 1;
				break;
			}

			ProcessExternalLine( com_line );
			CON_AddLine( com_line );
			if ( ++i >= cur_vis_conlines - 1 ) {
				break;
			}
		}

		if ( packeof || SYS_feof( fp ) ) {
			CON_ListEndPrompt();
		} else {
			CON_ListCtdPrompt();
			cat_pos = SYS_ftell( fp );
		}

		SYS_fclose( fp );
	}
}


// display content of command batch file (command "cat") ----------------------
//
int Cmd_CatExternalCommands( char *cstr )
{
	ASSERT( cstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( cstr );

	char *name = strtok( cstr, " " );

	if ( ( name == NULL ) || ( strtok( NULL, " " ) != NULL ) ) {
		CON_AddLine( "syntax: cat <script>." );
		return TRUE;
	}

	if ( !CheckExternalCommand( name ) ) {
		CON_AddLine( "script not found." );
		return TRUE;
	}

	strcpy( cat_name, name );
	FILE *fp = OpenScript( cat_name );

	if ( fp != NULL ) {

		int i = 0;
		int packeof = 0;
		while ( SYS_fgets( com_line, MAX_CONSOLE_LINE_LENGTH, fp ) != NULL ) {

			// check special package eof
			if ( strncmp( com_line, PACKAGE_CON_EOF, PACKAGE_CON_EOF_LEN ) == 0 ) {
				packeof = 1;
				break;
			}

			ProcessExternalLine( com_line );
			CON_AddLine( com_line );
			if ( ++i >= cur_vis_conlines - 1 ) {
				break;
			}
		}

		if ( !packeof && !SYS_feof( fp ) ) {
			cat_pos		   = SYS_ftell( fp );
			await_keypress = TRUE;
			com_cont_func  = CatExternalCommands_ctd;
		}

		SYS_fclose( fp );
	}

	return TRUE;
}


// forward declaration --------------------------------------------------------
//
PRIVATE
int ExecExternalLine( char *line, int echo );


// execute console command file automatically on startup ----------------------
//
void ExecConsoleFile( char *fname, int echo )
{
	//NOTE:
	// the scripts recognized by this function NEED NOT be
	// contained in external_commands[] since files are opened
	// directly and no error will result if they are not found.

	//NOTE:
	// this function sets the preempt_action_commands flag
	// to prevent execution of all (also direct!) action
	// commands. the processing_batch flag also will not be
	// set by this function.

	ASSERT( fname != NULL );
	ASSERT( ( echo == FALSE ) || ( echo == TRUE ) );

	FILE *fp = SYS_fopen( fname, "r" );

	if ( fp != NULL ) {

		// used to determine if invoked from CON_MAIN::InitConsole()
		extern int console_init_done;

		// put together console message
		sprintf( paste_str, "executing %s", fname );

		if ( !console_init_done ) {
			// invocation from InitConsole() (exec init script)
				MSGPUT( "Executing console script (%s)...", fname );
			CON_AddLine( paste_str );
		} else {
			if ( !SV_CONSOLE_LEVEL_MESSAGES ) {
				// not invoked from InitConsole()
				CON_AddMessage( paste_str );
			}
		}

		// disallow action commands
		preempt_action_commands = TRUE;

		// execute script line for line
		strcpy( com_line, con_prompt );
		while ( SYS_fgets( com_line + 2, MAX_CONSOLE_LINE_LENGTH - 2, fp ) != NULL ) {

			// check special package eof
			if ( strncmp( com_line + 2, PACKAGE_CON_EOF, PACKAGE_CON_EOF_LEN ) == 0 ) {
				break;
			}

			ExecExternalLine( com_line, echo );
		}

		// allow action commands once again
		preempt_action_commands = FALSE;

		// insert new prompt
		if ( console_init_done && echo )
			CON_AddLine( con_prompt );

		MSGOUT( "\nScript execution done." );

		SYS_fclose( fp );
	}
}


// scan external line, overwrite invalid characters with space ----------------
//
int ProcessExternalLine( char *scan )
{
	ASSERT( scan != NULL );

	int exec = 0;

	while ( *scan != 0 ) {
		if ( ( *scan == '\r' ) || ( *scan == '\n' ) ) {
			// strip '\r' and '\n' at end of line
			*scan-- = 0;
		} else if ( !ConsoleASCIIValid[ (byte) *scan ] ) {
			if ( ( *scan == '\\' ) && ConsoleASCIIValid[ (byte) '/' ] ) {
				// replace backslashes with slashes
				*scan = '/';
			} else {
				// replace invalid characters with spaces
				*scan = ' ';
			}
		} else {
			// convert valid characters to lower-case
			*scan = tolower( *scan );
			exec  = 1;
		}
		scan++;
	}

	return exec;
}


// execute externally obtained console line -----------------------------------
//
PRIVATE
int	ExecExternalLine( char *line, int echo )
{
	ASSERT( line != NULL );
	ASSERT( ( echo == FALSE ) || ( echo == TRUE ) );

	//NOTE:
	// a ';' in the first column starts a comment

	int exec = FALSE;
	if ( *( line + 2 ) != ';' ) {

		if ( ProcessExternalLine( line + 2 ) ) {
			if ( echo )
				CON_AddLine( line );
			ExecConsoleCommand( line + 2, echo );
			exec = TRUE;
		}
	}

/*
	// count executed commands
	if ( !DEMO_BinaryReplayActive() ) {
		demoinfo_curline++;
	}
*/

	return exec;
}


// execute all lines of external command batch --------------------------------
//
PRIVATE
void ExecLines( int echo )
{
	//NOTE:
	// this function is used by ExecExternalCommand(),
	// RestartExternalCommand(), and ExecExternalFile()
	// to execute all lines of a single console file.
	// it will be called recursively if a script invokes
	// another script.

	ASSERT( ( echo == FALSE ) || ( echo == TRUE ) );

	// go one recursion level down
	rec_depth++;

	strcpy( com_line, con_prompt );
	while ( SYS_fgets( com_line + 2, MAX_CONSOLE_LINE_LENGTH - 2, fp_for_level[ rec_depth - 1 ] ) != NULL ) {

		// check special package eof
		if ( strncmp( com_line + 2, PACKAGE_CON_EOF, PACKAGE_CON_EOF_LEN ) == 0 ) {
			break;
		}

		// execute current line
		ExecExternalLine( com_line, echo );

		// stall recursion if idlewait specified
		// FIXME: do we need the AC.WAIT command in the server ??
		/*if ( CurActionWait > 0 ) {
			break;
		}*/
	}

	// go one recursion level up
	rec_depth--;
}


// execute external command (command batch file) ------------------------------
//
PRIVATE
int ExecExternalCommand( int extcom, int echo )
{
	//NOTE:
	// this function simply assumes that the current
	// recursion level in rec_depth is valid and can
	// be used. it is called by CallExternalCommand()
	// which performs recursion level management.

	ASSERT( extcom >= 0 );
	ASSERT( ( echo == FALSE ) || ( echo == TRUE ) );

	// store number of command on this level
	extcom_on_level[ rec_depth ] = extcom;

	// try to open command file
	fp_for_level[ rec_depth ] = OpenScript( external_commands[ extcom ] );

	// exec command file
	if ( fp_for_level[ rec_depth ] != NULL ) {

		// store name to use as packet replay file
		ASSERT( strlen( external_commands[ extcom ] ) <= COMMAND_NAME_ALLOC_LEN );
		strcpy( packet_replay_name, external_commands[ extcom ] );

		// exec all lines of this batch
		ExecLines( echo );

		// keep file open if recursion stalled
		// FIXME: do we need the AC.WAIT command in the server ??
		//if ( CurActionWait == 0 ) {
			SYS_fclose( fp_for_level[ rec_depth ] );
			fp_for_level[ rec_depth ] = NULL;
		//}

		return TRUE;
	}

	return FALSE;
}


// execute external command (command batch file) via filename -----------------
//
int ExecExternalFile( char *command )
{
	//NOTE:
	// this is the main internal interface to execute
	// a specified console script. (used to automatically
	// execute scripts on game loop entry and video
	// subsystem switches, for instance.) the scripts
	// recognized by this function need to be contained
	// in external_commands[].

	//NOTE:
	// the processing_batch flag will not be set by this
	// function. this means that most action commands
	// will not be allowed to execute.

	//CAVEAT:
	// no checks are performed whether any replay is
	// currently in progress and therefore execution
	// should be disallowed.

	if ( ( command == NULL ) || ( *command == 0 ) )
		return FALSE;

	int extcom = CheckExternalCommand( command );

	if ( extcom-- == 0 )
	   return FALSE;

	// store number of command on this level
	extcom_on_level[ rec_depth ] = extcom;

	// try to open command file
	fp_for_level[ rec_depth ] = OpenScript( external_commands[ extcom ] );

	// exec command file
	if ( fp_for_level[ rec_depth ] != NULL ) {

		// exec all lines of this batch
		ExecLines( FALSE );

		// keep file open if recursion stalled
		// FIXME: do we need the AC.WAIT command in the server ??
		//if ( CurActionWait == 0 ) {
			SYS_fclose( fp_for_level[ rec_depth ] );
			fp_for_level[ rec_depth ] = NULL;
		//}

		return TRUE;
	}

	return FALSE;
}


// compile the state header script necessary for demo cutting -----------------
//
int CompileDemoCut( FILE *fp, char *script )
{
	ASSERT( fp != NULL );
	ASSERT( script != NULL );

	ASSERT( rec_depth == 0 );

	if ( num_external_commands == MAX_EXTERNAL_COMMANDS ) {
		return FALSE;
	}

	// make sure the script is known
	int extcom = CheckExternalCommand( script );
	if ( extcom == 0 ) {
		// if enough space append to list and set type to root
		ASSERT( strlen( script ) <= COMMAND_NAME_ALLOC_LEN );
		strcpy( external_commands[ num_external_commands ], script );
		external_command_types[ num_external_commands ] = COMTYPE_ROOT;
		extcom = ++num_external_commands;
	}

	// compile the script to already open file
	compile_fp       = fp;
	compile_script   = TRUE;
	processing_batch = TRUE;

	ExecExternalCommand( extcom - 1, FALSE );

	compile_fp       = NULL;
	processing_batch = FALSE;
	compile_script   = FALSE;

	return TRUE;
}


// determine if script replay currently in progress ---------------------------
//
int ScriptReplayActive()
{
	// if no recursion is stalled then no
	// replay is currently in progress
	return ( idle_reclevel > 0 );
}


// restart ExecExternalCommand() ----------------------------------------------
//
int RestartExternalCommand( int echo )
{
	//NOTE:
	// this function is called by E_REPLAY::FetchNewActions()
	// to recommence script execution. it is quite similar
	// to ExecExternalCommand() and that function will
	// actually be called by ExecLines() if further command
	// invocations are encountered.

	ASSERT( ( echo == FALSE ) || ( echo == TRUE ) );

	// check if command batch execution has been stalled
	if ( !ScriptReplayActive() )
		return FALSE;

	// set batch execution flag
	processing_batch = TRUE;

	// non-interactive console mode
	con_non_interactive = TRUE;

	// start in bottom level of recursion
	rec_depth = idle_reclevel;

	// restart stalled execution
	idle_reclevel = 0;

	do {

		// set correct recursion level
		rec_depth--;

		// exec all lines of this batch
		ExecLines( echo );

		// keep file open if recursion stalled
		// FIXME: do we need the AC.WAIT command in the server ??
		//if ( CurActionWait == 0 ) {
			SYS_fclose( fp_for_level[ rec_depth ] );
			fp_for_level[ rec_depth ] = NULL;
		/*} else {
			// simulate unraveling of
			// recursion to highest level
			rec_depth = 0;
			break;
		}*/

	} while ( rec_depth > 0 );

	// clear batch execution flag
	processing_batch = FALSE;

	// back to interactive console mode
	con_non_interactive = FALSE;

	// ensure no dangling automatic actions
	// if execution finished (not stalled)
	if ( idle_reclevel == 0 ) {
		//REPLAY_StopAutomaticActions();
	}

	return TRUE;
}


// check validity of batchname ------------------------------------------------
//
PRIVATE
int CheckBatchName( char *name )
{
	ASSERT( name != NULL );

	int fnlen = strlen( name );

	if ( ( fnlen < 1 ) || ( fnlen > COMMAND_NAME_VALID_LEN ) ||
		 ( strtok( NULL, " " ) != NULL ) ) {
		CON_AddLine( invalid_batch_name );
		return FALSE;
	}

	for ( int pos = 0; pos < fnlen; pos++ ) {
		// only alphanumeric characters and underscores are allowed
		if ( !isalnum( name[ pos ] ) && ( name[ pos ] != '_' ) ) {
			CON_AddLine( invalid_batch_name );
			return FALSE;
		}
	}

	return TRUE;
}


// open output script for writing ---------------------------------------------
//
int OpenOutputBatch( char *cstr, int type )
{
	ASSERT( cstr != NULL );
	ASSERT( ( type == CM_CREATE ) || ( type == CM_OPEN ) || ( type == CM_APPEND ) );
	HANDLE_COMMAND_DOMAIN_SEP( cstr );

	if ( log_commands ) {
		CON_AddLine( "there is another script open." );
		return TRUE;
	}

	char *name = strtok( cstr, " " );
	if ( name == NULL ) {
		CON_AddLine( arg_missing );
		return TRUE;
	}

	if ( !CheckBatchName( name ) )
		return TRUE;
	strcpy( log_name, name );

	if ( CheckExternalCommand( name ) && ( type == CM_CREATE ) ) {
		CON_AddLine( "there is already a script with this name." );
		return TRUE;
	}
	strcpy( paste_str, name );
	strcat( paste_str, con_extension );

	const char *mode = ( type == CM_APPEND ) ? "a" : "w";
	if ( ( log_fp = fopen( paste_str, mode ) ) != NULL ) {
		log_commands = 1;
	} else {
		CON_AddLine( "cannot open script with this name." );
	}

	return TRUE;
}


// close output batch ---------------------------------------------------------
//
void CloseOutputBatch()
{
	if ( log_commands ) {

		fclose( log_fp );
		log_commands = 0;
		sprintf( paste_str, "script %s closed.", log_name );
		CON_AddLine( paste_str );

		// if command not already in list: append
		if ( !CheckExternalCommand( log_name ) &&
			 ( num_external_commands < MAX_EXTERNAL_COMMANDS ) ) {

			// if enough space append to list and set type to root
			ASSERT( strlen( log_name ) <= COMMAND_NAME_ALLOC_LEN );
			strcpy( external_commands[ num_external_commands ], log_name );
			external_command_types[ num_external_commands ] = COMTYPE_ROOT;
			num_external_commands++;
		}

	} else {
		CON_AddLine( "there is no script open." );
	}
}


// set file name of script automatically executed on start of game loop -------
//
int Cmd_SetGameLoopBatchName( char *cstr )
{
	ASSERT( cstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( cstr );

	char *name = strtok( cstr, " " );

	if ( name == NULL ) {
		if ( *gameloop_start_script != 0 ) {
			// display name of currently selected
			// gameloop start-script
			CON_AddLine( gameloop_start_script );
		} else {
			CON_AddLine( "no gameloop start-script set." );
		}
		return TRUE;
	}

	if ( strtok( NULL, " " ) != NULL ) {
		CON_AddLine( "syntax: glbatch <script>." );
		return TRUE;
	}

	if ( CheckBatchName( name ) ) {
		// set new name for gameloop start-script
		strcpy( gameloop_start_script, name );
		sprintf( paste_str, "gameloop start-script set to: %s", gameloop_start_script );
		CON_AddLine( paste_str );
	} else {
		CON_AddLine( "invalid script name." );
	}

	return TRUE;
}


// start recording into command batch -----------------------------------------
//
int StartRecording( char *cstr, int savestate )
{
	ASSERT( cstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( cstr );

	if ( !RecordingActive ) {

		// fetch name for recording
		char *name = strtok( cstr, " " );
		if ( name == NULL ) {
			CON_AddLine( arg_missing );
			return TRUE;
		}
		if ( !CheckBatchName( name ) )
			return TRUE;

		// store name (needed by CloseRecording())
		strcpy( record_name, name );

		// store name to use as packet record file
		strcpy( packet_record_name, name );

		// create actual file name
		strcpy( paste_str, name );
		strcat( paste_str, con_extension );

		if ( ( RecordingFp = fopen( paste_str, "w" ) ) != NULL ) {

			RemoteRecPacketId = 0;
			RecordingActive   = TRUE;

			// warn user if packet recording is off
			if ( !RecordRemotePackets ) {
				CON_AddLine( "[warning]: remote packet recording is off." );
			}

			// save game state (data screenshot) if desired
			if ( savestate ) {
/*
				REC_InitMatrices();
				SaveGameState();
*/
				CON_AddLine( "game state saved." );
			}

		} else {

			CON_AddLine( "cannot start a recording with this name." );
		}

	} else {

		CON_AddLine( "recording already in progress." );
	}

	return TRUE;
}


// close recording batch ------------------------------------------------------
//
void CloseRecording()
{
	fclose( RecordingFp );

	RecordingActive = 0;
	RemoteRecSessionId++;

	sprintf( paste_str, "recording %s closed.", record_name );
	CON_AddLine( paste_str );

	// if command not already in list: append
	if ( !CheckExternalCommand( record_name ) &&
		 ( num_external_commands < MAX_EXTERNAL_COMMANDS ) ) {

		// if enough space append to list and set type to
		// root (not recording since it will be placed
		// into the root directory!)
		ASSERT( strlen( record_name ) <= COMMAND_NAME_ALLOC_LEN );
		strcpy( external_commands[ num_external_commands ], record_name );
		external_command_types[ num_external_commands ] = COMTYPE_ROOT;
		num_external_commands++;
	}
}


// call external command and check recursion; called by ExecConsoleCommand() --
//
void CallExternalCommand( int extcom, int echo )
{
	//NOTE:
	// no new command can be started if replay in progress.
	// unfortunately, this means that no script at all
	// can be executed while any other script is stalled.
	// alas, this includes utility scripts.

	ASSERT( extcom >= 0 );
	ASSERT( ( echo == FALSE ) || ( echo == TRUE ) );

/*
	// init demo info
	if ( !DEMO_BinaryReplayActive() ) {
		DEMO_InitInfo();
	}
*/

	// assume action commands will be allowed
	int actionflag = TRUE;

	// remember action wait
	// FIXME: do we need the AC.WAIT command in the server ??
	//int curactionwait = CurActionWait;

	// handle non-recursive call
	if ( rec_depth == 0 ) {

		if ( ScriptReplayActive() ) {
			CON_AddLine( "script replay in progress. command not executed." );
			return;
		}

		/*if ( DEMO_BinaryReplayActive() ) {

			// disallow action commands to allow
			// simultaneous playback of binary demo
			actionflag = FALSE;

			//NOTE:
			// this ensures that no other ac.wait will be
			// executed, conflicting with the current one.

			// prevent batch execution from stalling
			// FIXME: do we need the AC.WAIT command in the server ??
			CurActionWait = 0;
		}*/
	}

	// check if maximum recursion depth reached
	if ( rec_depth <= MAX_BATCH_REC_DEPTH ) {

		// set batch execution flag
		processing_batch = actionflag;

		// check all recursion levels above this one
		int lvl;
        for ( lvl = 0; lvl < rec_depth; lvl++ )
			if ( extcom_on_level[ lvl ] == extcom )
				break;

		// open contained batch if no endless recursion detected
		if ( lvl == rec_depth ) {
			ExecExternalCommand( extcom, echo );
		} else {
			CON_AddLine( recursion_canceled );
		}

	} else {
		// output error message if no more recursion levels available
		CON_AddLine( recursion_maxdepth );
	}

	// handle non-recursive call
	if ( rec_depth == 0 ) {

		// clear batch execution flag
		processing_batch = FALSE;

		// distinguish script only/simultaneous binary replay
		if ( actionflag == FALSE ) {

			// restore wait interval if binary demo
			// replay simultaneously in progress
			// FIXME: do we need the AC.WAIT command in the server ??
			//CurActionWait = curactionwait;

		} else {

			// ensure no dangling automatic actions
			// (only if no simultaneous binary replay)
			//REPLAY_StopAutomaticActions();
		}
	}
}


// kill batch execution that is currently in progress -------------------------
//
void ScriptStopReplay()
{
	// immediately stop idle waiting
	// FIXME: do we need the AC.WAIT command in the server ??
	//CurActionWait = 0;

	// shut down recursive call stack
	while ( idle_reclevel > 0 ) {
		// step one level up
		idle_reclevel--;
		// close files kept open to maintain
		// state of recursion for script files
		if ( fp_for_level[ idle_reclevel ] != NULL ) {
			SYS_fclose( fp_for_level[ idle_reclevel ] );
			fp_for_level[ idle_reclevel ] = NULL;
		}
	}
	ASSERT( idle_reclevel == 0 );

	// kill dangling automatic actions
	//REPLAY_StopAutomaticActions();
}


// close either recording or output batch -------------------------------------
//
void Cmd_CloseOutputBatch()
{
	if ( RecordingActive )
		CloseRecording();
	else
		CloseOutputBatch();
}


// stop replay of command batch (command "repstop") ---------------------------
//
void Cmd_StopBatchReplay()
{
//	if ( !DEMO_ReplayActive() ) {
		CON_AddLine( no_replay );
/*	} else {
		// stop both binary replay and command batch replay
		DEMO_StopReplay();
		CON_AddLine( stopped_replay );
	}
*/
}


// query status of command batch replay (command "repinfo") -------------------
//
void Cmd_QueryReplayInfo()
{
	/*if ( ScriptReplayActive() )
		CON_AddLine( replaying_script );
	else if ( DEMO_BinaryReplayActive() )
		CON_AddLine( replaying_binary );
	else*/
		CON_AddLine( no_replay );
}


// rescan list of external commands (command "rescan") ------------------------
//
void Cmd_RescanExternalCommands()
{
	// rebuild demo info
	//DEMO_RegisterInitialDemos();

	// rebuild script cache
	BuildExternalCommandList();
	CON_AddLine( "script cache rebuilt." );
}



