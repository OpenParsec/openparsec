/*
 * PARSEC HEADER: con_list.h
 */

#ifndef _CON_LIST_H_
#define _CON_LIST_H_


// external functions

void	Cmd_ShipInfo();
void	Cmd_ListTypes();
void	Cmd_ListClasses();
void	Cmd_ListSayMacros();
void	Cmd_ListKeyBindings();
void	Cmd_ListKeyFunctions();
void	Cmd_ListVidModes();
void	Cmd_ListBinaryDemos();
int		Cmd_ListExternalCommands( char *command );
int		Cmd_ListDataTable( char *command );
int		Cmd_ListIntCommands( char *command );
int		Cmd_ListStdCommands( char *command );


// external variables

extern int		await_keypress;
extern void		(*com_cont_func)();
extern int		cur_vis_conlines;


#endif // _CON_LIST_H_


