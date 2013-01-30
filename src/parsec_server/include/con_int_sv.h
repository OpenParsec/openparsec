/*
 * PARSEC HEADER: con_int_sv.h
 */

#ifndef _CON_INT_SV_H_
#define _CON_INT_SV_H_


// integer commands table entry

struct int_command_s {

	int		persistence;	// persistent int commands are saved on exit
	const char*	command;	// name of command
	int 	bmin;			// lower bound
	int 	bmax;			// upper bound
	int*	intref;			// pointer to actual int var
	void	(*realize)();	// will be called after modification (optional)
	int		(*fetch)();		// will be called before reading (optional)
	int		default_val;	// default value for int var
};


// integer commands list
extern int_command_s *	int_commands;

// number of integer variable commands
extern int				num_int_commands;

// integer input/output configuration
extern const char *		int_print_base;
extern int				int_calc_base;


// macro to retrieve intvar command strings
#define ICMSTR(x)		(int_commands[x].command)

// macro to retrieve string length of intvar commmands
#define ICMLEN(x)		strlen(int_commands[x].command)


// position of console geometry variables in table
enum {

	CONINFOINDX_CONWINX 	= 0,
	CONINFOINDX_CONWINY 	= 1,
	CONINFOINDX_CONWIDTH	= 2,
	CONINFOINDX_CONHEIGHT	= 3
};


// external functions

void	CON_RegisterIntCommand( int_command_s *regcom );


#endif // _CON_INT_SV_H_


