/*
 * PARSEC HEADER: part_sys.h
 */

#ifndef _PART_SYS_H_
#define _PART_SYS_H_


// external functions

void	InitParticleSystem();
void	InitParticleSizes();
void	KillParticleSystem();
void	FreeParticles();

int		PRT_ParticleInBoundingSphere( ShipObject *shippo, Vertex3& point );

int		DrawCustomParticles( const GenObject *baseobject );

void	PRTSYS_DrawParticles();


#endif // _PART_SYS_H_


