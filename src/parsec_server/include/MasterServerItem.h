/*
 * MasterServerList.h
 *
 *  Created on: Jan 2, 2013
 *      Author: jasonw
 */

#ifndef MASTERSERVERLIST_H_
#define MASTERSERVERLIST_H_
#include <time.h>	

/*
 *
 */
class MasterServerItem {
public:
	MasterServerItem();
	MasterServerItem(int SrvID, int CurrPlayers, int MaxPlayers, int PMajor, int PMinor, char ServerName[ MAX_SERVER_NAME + 1 ], char OS[MAX_OSNAME_LEN + 1 ], node_t *node);
	MasterServerItem(const MasterServerItem& msi_copy);
	virtual ~MasterServerItem();

	bool update(int SrvID, int CurrPlayers, int MaxPlayers, int PMajor, int PMinor, char ServerName[ MAX_SERVER_NAME + 1 ], char OS[MAX_OSNAME_LEN + 1 ], node_t *node);
	bool remove();

	bool operator < (const MasterServerItem &msl) const;
	bool operator == (const MasterServerItem &msl) const;
	bool operator != (const MasterServerItem &msl) const;

	bool isValid() const;

	int GetSrvID();
	int GetCurrPlayers();
	int GetMaxPlayers();
	int GetPMajor();
	int GetPMinor();
	int GetServerName(char *buffer, int buffer_sz);
	int GetOS(char *buffer, int buffer_sz);
	int GetNode(node_t *node);
	time_t GetMTime();

private:
	int _SrvID;
	int _CurrPlayers;
	int _MaxPlayers;
	int _PMajor;
	int _PMinor;
	time_t _MTime;
	char _ServerName[ MAX_SERVER_NAME + 1 ];
	char _OS[MAX_OSNAME_LEN + 1 ];
	node_t _Node;
};

#endif /* MASTERSERVERLIST_H_ */
