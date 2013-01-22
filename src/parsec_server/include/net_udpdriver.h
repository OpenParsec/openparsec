/*
* PARSEC HEADER: NET_UDPDriver.h
*/

#ifndef _NET_UDPDRIVER_H_
#define _NET_UDPDRIVER_H_

// NOTE: 
//
// platform dependent implementations are in e.g. NW_UDPDRIVER.CPP
//
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include <stdarg.h>
#include <ctype.h>
#include <vector>

#ifndef PARSEC_SERVER
	//FIXME: we should do a e_glob_svm.h just like e_glob_sv.h
	#define TheUDPDriver		(NET_UDPDriver::GetUDPDriver())
#endif // !PARSEC_SERVER


/* Following shortens all the type casts of pointer arguments */
#define	SA	struct sockaddr


// class defining a UDP driver ------------------------------------------------
//
class NET_UDPDriver
{
protected:
	int		m_socket;
	int		m_selected_port;
	char	m_szInterface[ MAX_IPADDR_LEN + 1 ];
	char	m_szIP       [ MAX_IPADDR_LEN + 1 ];
	node_t	m_Node;								// node containing the IP adress
	std::vector<node_t >		NetworkInterfaces;
	int 	m_PreferredInterface;
	bool 	m_DriverRunning;
protected:
	// init the OS API
	int	_InitOSAPI();
	
	// kill the OS API
	int	_KillOSAPI();
	
	// open and initialize the UDP socket 
	int	_OpenSocket();
	
	// close the UDP socket 
	int	_CloseSocket();
	
	// get the local IP 
	int	_RetrieveLocalIP();

	// setup the interface
	void _SetupInterface(node_t *p_Node);

	NET_UDPDriver();
	~NET_UDPDriver();

public:

	// SINGLETON pattern
	static NET_UDPDriver* GetUDPDriver()
	{
		static NET_UDPDriver _TheUDPDriver;
		return &_TheUDPDriver;
	}
	
	// init the UDP driver
	int	InitDriver( char* szInterface, int selected_port );
	
	// kill the UDP driver
	int KillDriver();
	
	// fetch a UDP packet from the socket
	int FetchPacket( char* buf, int maxlen, SA* saddr );
	
	// send a UDP packet on the socket
	int SendPacket( char* buf, int pktsize, SA* saddr );
	
	// sleep until we have a network input or a timeout occurs
	int SleepUntilNetInput( int timeout_msec )
	{
		fd_set			rset;
		struct timeval	select_timeout;
		
		// initialize socket set
		FD_ZERO( &rset );
		FD_SET( m_socket, &rset );
		
		select_timeout.tv_sec  = 0;
		select_timeout.tv_usec = timeout_msec * 1000;
		
		// select for readable socket
		int rc = select( m_socket + 1, &rset, NULL, NULL, &select_timeout );
		return rc;
	}

	// resolve a hostname using the DNS service
	int ResolveHostName( char *hostname, node_t* node );


	// ------------------------------------------------------------------------
	// accessor methods
	// ------------------------------------------------------------------------
	int			GetSocket()	{ return m_socket; }
	node_t*		GetNode()	{ return &m_Node; }
	int 		GetPreferredInterface() { return m_PreferredInterface; }
	void		SetPreferredInterface(int p_Interface) { m_PreferredInterface = p_Interface; }
	int 		IsRunning() { return m_DriverRunning; }
};



#endif // _NET_UDPDRIVER_H_

