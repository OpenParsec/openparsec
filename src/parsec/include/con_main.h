/*
 * PARSEC HEADER: con_main.h
 */

#ifndef _CON_MAIN_H_
#define _CON_MAIN_H_


#include "inp_defs.h"


// if this is defined command parsing will be a lot slower
//#define NO_UNNECESSARY_COMMAND_DOMAINS


// maximum length of single console line
#define MAX_CONSOLE_LINE_LENGTH 	255

// number of console lines (storage)
#define NUM_CONSOLE_LINES			256
#define NUM_CONSOLE_LINES_MASK		( NUM_CONSOLE_LINES - 1 )

// number of history list lines
#define NUM_HISTORY_LINES			128
#define NUM_HISTORY_LINES_MASK		( NUM_HISTORY_LINES - 1 )

// size of console prompt (in chars)
#define PROMPT_SIZE 				2


/*
// additional directories for console scripts
#if defined( SYSTEM_WIN32_UNUSED )
	#define REFCON_COMMANDS_DIR		"refs\\"
	#define STDCON_COMMANDS_DIR		"cons\\"
	#define RECORD_COMMANDS_DIR		"recs\\"
#elif defined( SYSTEM_LINUX_UNUSED ) || defined( SYSTEM_MACOSX_UNUSED )*/
	#define REFCON_COMMANDS_DIR		"refs/"
	#define STDCON_COMMANDS_DIR		"cons/"
	#define RECORD_COMMANDS_DIR		"recs/"
/*#else
	#error "system not defined!"
#endif
	*/
// file extension of console scripts
#define CON_FILE_EXTENSION			".con"


// file extension of compiled console scripts
#define CON_FILE_COMPILED_EXTENSION	".dem"


// command domain restriction handling

#ifdef NO_UNNECESSARY_COMMAND_DOMAINS

	#define HANDLE_COMMAND_DOMAIN(x) \
		if ( ( *(x) != ' ' ) && ( *(x) != 0 ) ) { \
			return FALSE; \
		} else { \
		}

	#define HANDLE_COMMAND_DOMAIN_SEP(x) HANDLE_COMMAND_DOMAIN(x)

#else // NO_UNNECESSARY_COMMAND_DOMAINS

	#define HANDLE_COMMAND_DOMAIN(x)

	#define HANDLE_COMMAND_DOMAIN_SEP(x) \
		if ( ( *(x) != ' ' ) && ( *(x) != 0 ) ) { \
			CON_AddLine( unknown_command ); \
			return TRUE; \
		} else { \
		}

#endif // NO_UNNECESSARY_COMMAND_DOMAINS


// external variables

extern int		compile_script;
extern FILE *	compile_fp;

extern int		con_back_view;
extern int		con_content;
extern int		con_bottom;
extern int 		cursor_x;
extern int		press_space_curs_x;

extern int		con_in_talk_mode;
extern int		con_talk_line;
extern char		talk_escape_char;

extern int		con_non_interactive;

extern int		con_no_login_check;

extern char 	con_prompt[];
extern char 	ok_prompt[];
extern char 	press_space[];
extern char 	too_many_args[];
extern char 	arg_missing[];
extern char 	invalid_arg[];
extern char 	range_error[];
extern char 	unknown_command[];
extern char 	line_separator[];

extern char		con_lines[][ MAX_CONSOLE_LINE_LENGTH + 1 ];


// external functions

void	CheckConExtents();

void	ClearConScreen();
void	EraseConLine( int line );
void	CreateSeparatorLine( int line );

char*	SubstCurWorkDir( char *path );

void	WritePersistentCommands();

void	ExecNonCompilableCommand( char *command );
void	ExecConsoleCommand( char *command, int echo );

void	FlushConsoleBuffer();

void	CON_AddMessage( const char *text );
void	CON_AddLine( const char *text );
void	CON_AddLineFeed();
void	CON_EnableLineFeed();
void	CON_DisableLineFeed();
void	CON_DelLine();
char*	CON_GetLine();
void	CON_ListCtdPrompt();
void	CON_ListEndPrompt();

void	CON_HandleInput(keypress_s &kinfo);

int		QuicksayConsole();
void	SetQuicksayConsole( int enable );

void	ConsoleControl();
void	ToggleConsole();

void	ExecStartupScript( int echo );

void	InitConsole();
void	KillConsole();

void	InitConsoleFileHandling();


#endif // _CON_MAIN_H_


