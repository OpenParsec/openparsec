/*
 * PARSEC HEADER: net_conn.h
 */

#ifndef _NET_CONN_H_
#define _NET_CONN_H_


// external functions

const char*	NET_GetCurrentProtocolName();
int		NET_SwitchNetSubsys( const char *protocol );

int		NET_AutomaticConnect();
int		NET_CommandConnect( const char *server );
int		NET_CommandDisconnect();


// structure definitions

struct protocol_s {
	const char*	name;
	int		id;
};


// external variables

extern protocol_s*	protocol_table;


#endif // _NET_CONN_H_


