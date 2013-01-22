/*
 * PARSEC HEADER: obj_part.h
 */

#ifndef _OBJ_PART_H_
#define _OBJ_PART_H_


// render modes for attached particles

enum {

	PARTICLE_RENDERMODE_POSLIGHT,
	PARTICLE_RENDERMODE_THRUST,
	PARTICLE_RENDERMODE_MISSILE,
};


// function pointer type for particle attachment
typedef void (*attach_particles_fpt)( GenObject *obj );


// external functions

void	OBJ_AttachClassParticles( GenObject *obj );
void	OBJ_ResetRegisteredClassParticles( dword objclass );
int		OBJ_RegisterClassParticle( dword objclass, const char *pdefname, float refz, int rendmode, Vector3 *pos );


#endif // _OBJ_PART_H_


