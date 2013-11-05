/*
 * MasterServer.h
 *
 *  Created on: Jan 2, 2013
 *      Author: jasonw
 */

#ifndef MASTERSERVER_H_
#define MASTERSERVER_H_

#include "e_gameserver.h"
#include "MasterServerItem.h"

// C++ STL vector for the resolution list
#include <vector>

/*
 *
 */
class MasterServer: public E_GameServer {
public:
	MasterServer();
	MasterServer(E_GameServer *);
	virtual ~MasterServer();

	int RemoveStaleEntries();


	std::vector<MasterServerItem>		ServerList;
private:
	int last_check;
};

#endif /* MASTERSERVER_H_ */
