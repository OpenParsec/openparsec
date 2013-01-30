/*
 * PARSEC HEADER: con_ext.h
 */

#ifndef _CON_EXT_H_
#define _CON_EXT_H_


// types of external commands (relating to their directory)
enum {

	COMTYPE_PACKAGE,		// 0
	COMTYPE_ROOT,			// 1
	COMTYPE_REFERENCE,		// 2
	COMTYPE_STANDARD,		// 3
	COMTYPE_RECORDING,		// 4
	COMTYPE_MODIFICATION	// 5
};

// maximum valid length of command (script) name (filesystem dependent)
#define COMMAND_NAME_VALID_LEN		8

// allocated memory for command (script) name (excluding zero term.)
#define COMMAND_NAME_ALLOC_LEN		63

// maximum number of external scripts that will be recognized
#define MAX_EXTERNAL_COMMANDS		1024

// maximum number of registered mods (more than one is useful for data only)
#define MAX_REGISTERED_MODS			64


// external variables

extern int		processing_batch;
extern int		idle_reclevel;
extern int		rec_depth;

extern int		preempt_action_commands;

extern FILE *	log_fp;
extern int		log_commands;
extern char		external_commands[][ COMMAND_NAME_ALLOC_LEN + 1 ];
extern int		external_command_types[];
extern int		num_external_commands;

extern char		packet_record_name[];
extern char		packet_replay_name[];

extern char*    mod_names[];
extern int   	mod_numnames;
extern int   	mod_override;


// external functions

int		ProcessExternalLine( char *scan );
void	ExecConsoleFile( char *fname, int echo );
int		AddExternalCommands( char *path, int comtype );
void	BuildExternalCommandList();
int		CheckExternalCommand( char *com );
int		InsertCommandLog( char *command );
int		ExecExternalFile( char *command );
int		CompileDemoCut( FILE *fp, char *script );
int		ScriptReplayActive();
int		RestartExternalCommand( int echo );
int		OpenOutputBatch( char *cstr, int type );
void	CloseOutputBatch();
int		StartRecording( char *cstr, int savestate );
void	CloseRecording();
void	CallExternalCommand( int extcom, int echo );
void	ScriptStopReplay();

int		Cmd_CatExternalCommands( char *cstr );
int		Cmd_SetGameLoopBatchName( char *cstr );
void	Cmd_CloseOutputBatch();
void	Cmd_StopBatchReplay();
void	Cmd_QueryReplayInfo();
void	Cmd_RescanExternalCommands();


// macro to retrieve external command strings
#define ECMSTR(x) (external_commands[x])

// macro to retrieve string length of external commmands
#define ECMLEN(x) strlen(external_commands[x])


#endif // _CON_EXT_H_


