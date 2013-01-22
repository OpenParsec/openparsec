/*
 * PARSEC HEADER: obj_expl.h
 */

#ifndef _OBJ_EXPL_H_
#define _OBJ_EXPL_H_


// external functions

void	LetShipExplode( ShipObject *shippo );

void	OBJ_ShipHelixDamage( ShipObject *shippo, int owner );
void	OBJ_ShipLightningDamage( ShipObject *shippo, int owner );
void	OBJ_ShipPhotonDamage( ShipObject *shippo, int owner );
void	OBJ_ShipSwarmDamage( ShipObject *shippo, int owner );
void	OBJ_ShipEmpDamage( ShipObject *shippo, int owner, int hitpoints );

void	OBJ_CheckExplosions();


#endif // _OBJ_EXPL_H_


