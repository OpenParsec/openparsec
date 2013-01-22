/*
 * PARSEC HEADER: con_act_sv.h
 */

#ifndef _CON_ACT_SV_H_
#define _CON_ACT_SV_H_


// action commands

enum {

	ACM_SIGNATURE,

	ACM_IDLEWAIT,

	ACM_PITCH,
	ACM_YAW,
	ACM_ROLL,
	ACM_SLIDEX,
	ACM_SLIDEY,
	ACM_MOVE,

	ACM_CAMROT1,
	ACM_CAMROT2,
	ACM_CAMROT3,
	ACM_CAMORIGIN,

	ACM_FIXROT1,
	ACM_FIXROT2,
	ACM_FIXROT3,

	ACM_PACKET,

	ACM_FADEOUT,
	ACM_FADEIN,
	ACM_WFADEOUT,
	ACM_WFADEIN,
	ACM_BLACK,
	ACM_WHITE,
	ACM_STDPALETTE,

	ACM_SAVESCREEN,
	ACM_SAVELIST,
	ACM_SAVECAMERA,
	ACM_SAVEFSTARS,
	ACM_SAVEPSTARS,
	ACM_SAVEREMOTE,

	ACM_KILLPSTARS,
	ACM_ADDPSTAR,

	ACM_PREPRESTORE,

	ACM_CREATEOBJECT,
	ACM_CREATEEXTRA,
	ACM_DESTROYLIST,

	ACM_REMOTEPLAYER,
	ACM_REMOTEMATRIX,
	ACM_REMOTENAME,
	ACM_REMOTEJOIN,
	ACM_REMOTEUNJOIN,

	ACM_CLEARDEMO,

	ACM_OBJMATRXCOL1,
	ACM_OBJMATRXCOL2,
	ACM_OBJMATRXCOL3,
	ACM_OBJMATRXCOL4,

	ACM_LASER_SPEED,
	ACM_LASER_DIRVEC,
	ACM_LASER_OWNER,
	ACM_MISSL_SPEED,
	ACM_MISSL_DIRVEC,
	ACM_MISSL_OWNER,
	ACM_MISSL_TARGET,
	ACM_MINE_OWNER,

	ACM_ENERGYFIELD,
	ACM_SPREADFIRE,
	ACM_LIGHTNINGON,
	ACM_LIGHTNINGOFF,

	ACM_SETOBJECTID,
	ACM_SETOBJECTHOSTID,

	ACM_SIMULKEYPRESS,
	ACM_SIMULKEYRELEASE,

	ACM_MS_CURDAMAGE,
	ACM_MS_MAXDAMAGE,
	ACM_MS_CURENERGY,
	ACM_MS_MAXENERGY,
	ACM_MS_CURSPEED,
	ACM_MS_MAXSPEED,
	ACM_MS_NUMMISSLS,
	ACM_MS_MAXNUMMISSLS,
	ACM_MS_NUMHOMMISSLS,
	ACM_MS_MAXNUMHOMMISSLS,
	ACM_MS_NUMMINES,
	ACM_MS_MAXNUMMINES,
	ACM_MS_WEAPONS,
	ACM_MS_SPECIALS,

	ACM_GL_SELECTEDLASER,
	ACM_GL_SELECTEDMISSILE,
	ACM_GL_CURGUN,
	ACM_GL_CURLAUNCHER,
	ACM_GL_PANEL3CONTROL,
	ACM_GL_PANEL4CONTROL,
	ACM_GL_TARGETOBJNUMBER,

	ACM_WRITESMALL,
	ACM_WRITEBIG1,
	ACM_WRITEBIG2,
	ACM_WRITEBIG3,
	ACM_CLEARTEXT,

	ACM_PLAYSTREAM,
	ACM_STOPSTREAM,
	ACM_PLAYSOUND,

	ACM_AUX,
	ACM_ENTRYMODE,

	ACM_SERVERMSG,

	ACM_HELIXON,
	ACM_HELIXOFF,

	ACM_REMOTEMAXPLAYERS,

	ACM_INCLUDEDEMO,

	ACM_MS_WEAPONSACTIVE,
	ACM_REMOTEWEAPONSACTIVE,

	ACM_WRITECONSOLETEXT,
	ACM_DISPLAYMESSAGE,

	ACM_CD_OPEN,
	ACM_CD_CLOSE,
	ACM_CD_PLAY,
	ACM_CD_PAUSE,
	ACM_CD_RESUME,
	ACM_CD_STOP,
	ACM_CD_VOLUME,

	ACM_EXECCOMMAND,

	ACM_CLASSMAPID,
	ACM_CLASSMAPNAME,

	ACM_AUXDATA,
	ACM_SYSREFFRAMES,

	ACM_SCHEDULECOMMAND,

	ACM_LIGHTAMBIENT,
	ACM_LIGHTDIFFUSE,
	ACM_LIGHTSPECULAR,

	ACM_MS_NUMPARTMISSLS,
	ACM_MS_MAXNUMPARTMISSLS,

	ACM_PHOTONON,
	ACM_PHOTONOFF,

	ACM_EMPON,
	ACM_EMPOFF,
	ACM_EMP,

	ACM_NUM_COMMANDS		// must be last!!
};


// action commands flags

#define ACF_NODIRECT		0x0000		// command not directly callable
#define ACF_DIRECT			0x0001		// command directly callable
#define ACF_NOCOMPILE		0x0000		// command not compilable
#define ACF_COMPILE			0x0002		// command is compilable
#define ACF_NOPARAMS		0x0000		// command has no parameters
#define ACF_INTPARAMS		0x0010		// command has int parameters
#define ACF_FLOATPARAMS		0x0020		// command has float parameters
#define ACF_STRINGPARAM		0x0040		// command has a string parameter
#define ACF_PACKETPARAM		0x0080		// command has a packet attached
#define ACF_DEMOPARAM		0x0100		// command has an entire demo attached
#define ACF_SCHEDPARAM		0x0200		// command has a command to schedule

#define ACF_STRIP_SPACE		0x1000		// strip leading and trailing whitespace
#define ACF_STRINGSTRIP		( ACF_STRINGPARAM | ACF_STRIP_SPACE )

#define ACM_EAT_SPACE(c)	( ( action_commands[ c ].flags & ACF_STRIP_SPACE ) != 0 )
#define ACF_EAT_SPACE(f)	( ( (f) & ACF_STRIP_SPACE ) != 0 )


// action command description

struct act_command_s {

	const char *command;	// command string
	void	(*func)(void);	// function responsible for command execution
	dword 	flags; 			// flag word
	int		numparameters;	// number of parameters the command takes
};

extern act_command_s action_commands[];


// macro to retrieve action command strings
#define ACMSTR(x)		(action_commands[x].command)

// macro to retrieve string length of action commmands
#define ACMLEN(x)		strlen(action_commands[x].command)

// macro to determine if action command is directly callable
#define ACMCALLABLE(x)	((action_commands[x].flags&ACF_DIRECT)!=0)


// external functions

int			ExecBinCommands( char* &data, int interactive );
void		ResetClassMapTable();
void		ResetScheduledActions();
void		SaveGameState();
void		ActionCommand( char *cstr );


// external variables

extern int	con_audio_stream_started;


#endif // _CON_ACT_SV_H_


