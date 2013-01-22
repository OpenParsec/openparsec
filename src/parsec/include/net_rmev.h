/*
 * PARSEC HEADER: net_rmev.h
 */

#ifndef _NET_RMEV_H_
#define _NET_RMEV_H_


// net_rmev.c implements the following functions
// ---------------------------------------------
//	int				NET_RmEvAllowed( int re_type );
//	size_t			NET_RmEvGetSize( RE_Header *relist );
//	int				NET_RmEvPlayerName( char *name );
//	int				NET_RmEvSendText( const char *text );
//	int				NET_RmEvObject( const GenObject *objpo );
//	int				NET_RmEvLaser( const LaserObject *laserpo );
//	int				NET_RmEvMissile( const MissileObject *missilepo, dword targetobjid );
//	int				NET_RmEvExtra( const ExtraObject *extrapo );
//	int				NET_RmEvParticleObject( int type, const Vertex3& origin );
//	int				NET_RmEvKillObject( dword objid, byte listno );
//	int				NET_RmEvWeaponState( dword weapon, byte state, int energy, dword specials );
//	int				NET_RmEvStateSync( dword statekey, dword stateval );
//	int				NET_RmEvCreateSwarm( dword owner, dword targetobjid, dword seed );
//	int				NET_RmEvCreateEmp( int upgradelevel );
//  RE_PlayerAndShipStatus*  NET_RmEvPlayerAndShipStatus();
// 
//	void	NET_ExecRmEvCreateObject	( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvCreateLaser		( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvCreateMissile	( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvParticleObject	( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvCreateExtra		( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvKillObject		( RE_Header *rmev );
//	void	NET_ExecRmEvSendText		( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvPlayerName		( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvWeaponState		( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvStateSync		( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvCreateSwarm		( RE_Header *rmev, int ownerid );
//	void	NET_ExecRmEvCreateEmp		( RE_Header *rmev, int ownerid );

//	void		NET_ExecRmEvGameState			( RE_GameState* gamestate );
//	void		NET_ExecRmEvKillStats			( RE_KillStats* killstats );
//	void		NET_ExecRmEvCommandInfo			( RE_CommandInfo* commandinfo );


// syncable remote state ids

enum {

	RMEVSTATE_NEBULAID,
	RMEVSTATE_PROBEXTRA,
	RMEVSTATE_PROBHELIX,
	RMEVSTATE_PROBLIGHTNING,
	RMEVSTATE_PROBPHOTON,
	RMEVSTATE_PROBMINE,
	RMEVSTATE_PROBREPAIR,
	RMEVSTATE_PROBAFTERBURNER,
	RMEVSTATE_PROBHOLODECOY,
	RMEVSTATE_PROBINVISIBILITY,
	RMEVSTATE_PROBINVULNERABILITY,
	RMEVSTATE_PROBENERGYFIELD,
	RMEVSTATE_PROBLASERUPGRADE,
	RMEVSTATE_PROBLASERUPGRADE1,
	RMEVSTATE_PROBLASERUPGRADE2,
	RMEVSTATE_PROBMISSPACK,
	RMEVSTATE_PROBDUMBPACK,
	RMEVSTATE_PROBGUIDEPACK,
	RMEVSTATE_PROBSWARMPACK,
	RMEVSTATE_PROBEMPUPGRADE1,
	RMEVSTATE_PROBEMPUPGRADE2,
	RMEVSTATE_AMAZING,
	RMEVSTATE_BRILLIANT,
	RMEVSTATE_KILLLIMIT,
    RMEVSTATE_ENERGYBOOST,
    RMEVSTATE_REPAIRBOOST,
    RMEVSTATE_DUMBPACK,
    RMEVSTATE_HOMPACK,
    RMEVSTATE_SWARMPACK,
    RMEVSTATE_PROXPACK,

	RMEVSTATE_NUMSTATES
};


// external functions ---------------------------------------------------------
//
size_t	NET_RmEvList_GetSize( RE_Header* relist );
int		NET_RmEvList_IsWellFormed( RE_Header* relist );

#endif // _NET_RMEV_H_






