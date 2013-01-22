/*
 * PARSEC HEADER: con_conf.h
 */

#ifndef _CON_CONF_H_
#define _CON_CONF_H_



// color command spec

struct colcomm_s {

	const char	*cmd;
	colrgba_s	*col;
	void		(*func)();
};


// external functions

void	DisplayCurrentNetSubsys();

int		CheckColorConfig( char *scan );
int		CheckConfigCommand( char *scan );


// external variables

extern colcomm_s	col_comms[];
extern int			num_col_commands;


#endif // _CON_CONF_H_


