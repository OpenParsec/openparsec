/*
 * PARSEC HEADER: part_wfx.h
 */

#ifndef _PART_WFX_H_
#define _PART_WFX_H_


// constants for particle weapons

enum {

	PARTICLEGUN_SPREADFIRE
};


// external functions

void	WFX_InitParticleSizes( float resoscale );

int		WFX_ShootParticleWeapon( ShipObject *shippo, int type );
void	WFX_RemoteShootSpreadfire( int playerid );

int		WFX_MaintainHelix( ShipObject *shippo, int playerid );
int		WFX_ActivateHelix( ShipObject *shippo );
void	WFX_DeactivateHelix( ShipObject *shippo );
void	WFX_RemoteActivateHelix( int playerid );
void	WFX_RemoteDeactivateHelix( int playerid );
void	WFX_EnsureHelixInactive( ShipObject *shippo );

void	WFX_MaintainLightning( ShipObject *shippo );
int		WFX_ActivateLightning( ShipObject *shippo );
void	WFX_DeactivateLightning( ShipObject *shippo );
void	WFX_RemoteActivateLightning( int playerid );
void	WFX_RemoteDeactivateLightning( int playerid );
void	WFX_EnsureLightningInactive( ShipObject *shippo );

struct photon_sphere_pcluster_s;
void    WFX_CalcPhotonSphereAnimation( photon_sphere_pcluster_s *cluster );
int     WFX_ActivatePhoton( ShipObject *shippo );
void    WFX_DeactivatePhoton( ShipObject *shippo );
void    WFX_RemoteActivatePhoton( int playerid );
void    WFX_RemoteDeactivatePhoton( int playerid );
void    WFX_EnsurePhotonInactive( ShipObject *shippo );

void	WFX_EnsureParticleWeaponsInactive( ShipObject *shippo );


#endif // _PART_WFX_H_


