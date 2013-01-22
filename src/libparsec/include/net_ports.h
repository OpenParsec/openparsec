#ifndef _NET_PORTS_H_
#define _NET_PORTS_H_

// port numbers ---------------------------------------------------------------
//
#define DEFAULT_MASTERSERVER_TCP_PORT	6580
#define DEFAULT_MASTERSERVER_UDP_PORT	6580
#define DEFAULT_PEERTOPEER_UDP_PORT		6581
#define DEFAULT_GAMESERVER_UDP_PORT		6582

// /etc/services template -----------------------------------------------------
//
// parsec-master   6580/tcp                        # Parsec Masterserver
// parsec-master   6580/udp                        # Parsec Masterserver
// parsec-peer     6581/udp                        # Parsec Peer-to-Peer
// parsec-game     6582/udp                        # Parsec Gameserver

#endif // _NET_PORTS_H_
