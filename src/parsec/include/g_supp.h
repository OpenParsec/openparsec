/*
 * PARSEC HEADER: g_supp.h
 */

#ifndef _G_SUPP_H_
#define _G_SUPP_H_


// external functions

void	PrepInfoScreenKeys();
int		CheckInfoScreenExit();
void	InitLocalShipStatus( dword myclassid );
void	MoveLocalShip();
void	ObjCamOn();
void	ObjCamOff();
void	ObjectCameraControl();
void	InitFloatingMyShip();
void	InitJoinPosition();
void	InitJoinReplay();
void	Cmd_SetShipOrigin();
void	SelectCrosshairTarget();
void	SelectNextTarget();
void	ShipIntelligence();
void	MaintainDurationWeapons( int playerid );
void	KillDurationWeapons( ShipObject *shippo );



// determine if standard game mode active -------------------------------------
//
#define GAME_MODE_ACTIVE()		( !EntryMode && !InFloatingMenu )


#endif // _G_SUPP_H_


