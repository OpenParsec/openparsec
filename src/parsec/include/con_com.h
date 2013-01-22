/*
 * PARSEC HEADER: con_com.h
 */

#ifndef _CON_COM_H_
#define _CON_COM_H_


// user commands table entry

struct user_command_s {

	const char*	command;		// command string
	short	commlen;			// length of string
	short	numparams;			// num of parameters (used by tab-completion)
	int		(*execute)(char*);	// command function
	char*	(*statedump)();		// returns statedump string; (NULL==volatile)
};


// handle user commands not taking any parameters

#define USERCOMMAND_NOPARAM(x) \
{ \
	char *whsp = (x); \
	for ( ; *whsp; whsp++ ) \
		if ( *whsp != ' ' ) \
			break; \
	if ( *whsp != 0 ) { \
		if ( *(x) == ' ' ) { \
			CON_AddLine( dont_use_params ); \
			return TRUE; \
		} else { \
			return FALSE; \
		} \
	} \
} \


// macro to retrieve user command strings
#define USRSTR(x)		(user_commands[x].command)

// macro to retrieve string length of user commmands
#define USRLEN(x)		(user_commands[x].commlen)

// macro to determine number of arguments of user commmands
#define USRARG(x)		(user_commands[x].numparams)


// external variables

extern user_command_s*	user_commands;
extern int				num_user_commands;
extern char				dont_use_params[];


// external functions

void	CheckPlayerName( const char *name );

int		CheckCmdsNoParam( const char *command );
int		CheckCmdsCustomString( const char *command );
int		CheckCmdsUserDefined( const char *command );

int		CON_RegisterUserCommand( user_command_s *regcom );
int		CON_ReplaceUserCommand( user_command_s* repcom );


#endif // _CON_COM_H_


