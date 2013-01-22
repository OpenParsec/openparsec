/*
 * PARSEC HEADER: part_ani.h
 */

#ifndef _PART_ANI_H_
#define _PART_ANI_H_


// external functions

void	LinearParticleCollision( linear_pcluster_s *cluster, int pid );
void 	CalcSphereParticlePosition( Vertex3& position, geomv_t radius, int spheretype );
void    CalcSphereParticleRotation( Vertex3& position, bams_t pitch, bams_t yaw, bams_t roll );
int     CalcSphereContraction( Vertex3& position, geomv_t speed );

void	PAN_InitParticleSizes( float resoscale );
void	PAN_AnimateParticles();


#endif // _PART_ANI_H_


