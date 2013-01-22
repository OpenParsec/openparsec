/*
 * PARSEC HEADER: g_sfx.h
 */

#ifndef _G_SFX_H_
#define _G_SFX_H_


// external functions

void	SFX_InitParticleSizes( float resoscale );

void	SFX_CreateEnergyField( Vertex3& origin );
void	SFX_ParticleExplosion( ShipObject *shippo );
void	SFX_FlashProtectiveShield( ShipObject *shippo );
int		SFX_EnableInvulnerabilityShield( ShipObject *shippo );
void	SFX_HullImpact( ShipObject *shippo, Vertex3 *impact, Plane3 *plane );

struct basesphere_pcluster_s;
basesphere_pcluster_s *
		SFX_FetchInvulnerabilityShield( ShipObject *shippo, float *strength );

void	SFX_RemoteEnableInvulnerabilityShield( int owner );

void	SFX_CreatePropulsionParticles( ShipObject *shippo );
void	SFX_MissilePropulsion( MissileObject *missile );

void	SFX_CreateStargate( ShipObject *shippo );
void	SFX_CreateExtra( ExtraObject *extra );
void	SFX_VanishExtra( ExtraObject *extra );


#endif //_G_SFX_H_


