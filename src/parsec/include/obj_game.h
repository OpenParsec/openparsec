/*
 * PARSEC HEADER: obj_game.h
 */

#ifndef _OBJ_GAME_H_
#define _OBJ_GAME_H_


// afterburner constants

#define AFTERBURNER_SPEED	150000
#define AFTERBURNER_ENERGY	2400


// external functions

int		OBJ_DeviceAvailable( ShipObject *shippo, int mask );

void	OBJ_ShootLaser( ShipObject *shippo );
void	OBJ_LaunchMissile( ShipObject *shippo, dword launcher );
void	OBJ_LaunchHomingMissile( ShipObject *shippo, dword launcher, dword targetid );
void	OBJ_LaunchMine( ShipObject *shippo );
void 	OBJ_LaunchSwarmMissiles( ShipObject *shippo, dword targetid );

void	OBJ_EnableInvisibility( ShipObject *shippo );
void	OBJ_EnableInvulnerability( ShipObject *shippo );
void	OBJ_EnableDecoy( ShipObject *shippo );

void 	OBJ_EnableLaserUpgrade1( ShipObject *shippo );
void 	OBJ_EnableLaserUpgrade2( ShipObject *shippo );

void 	OBJ_EnableEmpUpgrade1( ShipObject *shippo );
void 	OBJ_EnableEmpUpgrade2( ShipObject *shippo );

char*	OBJ_BoostEnergy( ShipObject *shippo, int boost );

void 	OBJ_EnableAfterBurner( ShipObject *shippo );
void 	OBJ_DisableAfterBurner( ShipObject *shippo );
void 	OBJ_ActivateAfterBurner( ShipObject *shippo );
void 	OBJ_DeactivateAfterBurner( ShipObject *shippo );

void	OBJ_EventShipImpact( ShipObject *shippo, int shieldhit );


// external variables

extern int afterburner_active;
extern int afterburner_energy;


#endif // _OBJ_GAME_H_


