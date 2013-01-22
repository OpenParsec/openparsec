/*
 * PARSEC HEADER: s_swarm.h
 */

#ifndef _S_SWARM_H_
#define _S_SWARM_H_


struct callback_pcluster_s;

// external functions

GenObject *	SWARM_Init( int owner, Vertex3 *origin, ShipObject *targetpo, int randseed );
int 		SWARM_Animate( callback_pcluster_s* cluster );
void 		SWARM_TimedAnimate( callback_pcluster_s* cluster );


#endif // _S_SWARM_H_


