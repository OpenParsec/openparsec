/*
 * PARSEC - Particle Rendering Encapsulation
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2001
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// compilation flags/debug support
#include "config.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "sys_defs.h"
#include "vid_defs.h"

// rendering subsystem
#include "r_part.h"

// particle types
#include "parttype.h"


// local module header
#include "ro_part.h"


// proprietary module headers
#include "con_aux.h"
#include "e_callbk.h"
#include "part_sys.h"
#include "ro_api.h"
#include "ro_supp.h"

#include "debug.h"

// mathematics header
#include "utl_math.h"



// flags
//#define USE_INDEXED_TRIANGLES
//#define REVERSED_DEPTH_RANGE		    // must be consistent with ogl setup


// cluster drawing info -------------------------------------------------------
//
struct pcldrawinf_s {

	int			width;
	int			height;
	int			scalewidth;
	int			scaleheight;
	depth_t		depthvalue;

	TextureMap*	texmap;
	imgtrafo_s*	imgtrafo;

	dword		itertype;
	dword		raststate;
	dword		rastmask;
};


// dynamic array for vertices of entire particle cluster ----------------------
//
static GLVertex4*	gl_cluster_vtxs;
static int			gl_cluster_vcount;
static GLTexInfo	gl_cluster_texinfo;
static pcldrawinf_s	gl_cluster_drawinf;

#ifdef USE_INDEXED_TRIANGLES
static dword*		gl_cluster_trindxs;
static int			gl_cluster_icount;
#endif


// enter vertices of one particle into cumulative vertex array ----------------
//
INLINE
void RO_ScheduleParticleVertices( SPoint *spt, pcldrawinf_s *pdi )
{
	ASSERT( spt != NULL );

	ASSERT( pdi->scalewidth > 0 );
	ASSERT( pdi->scaleheight > 0 );

	ASSERT( gl_cluster_vtxs != NULL );

//TODO:
//	pdi->imgtrafo

	depth_t depthvalue = pdi->depthvalue;
//	LogZValues( DEPTH_TO_INTEGER( depthvalue ) );
	GLfloat depth = DEPTH_TO_FLOAT( depthvalue ) * OPENGL_DEPTH_RANGE;

	int sw2 = pdi->scalewidth / 2;
	int sh2 = pdi->scaleheight / 2;

	// unclipped coordinates
	int x1 = spt->X - sw2;
	int y1 = spt->Y - sh2;
//	int u1 = 0;
//	int v1 = 0;

	int x2 = spt->X + sw2 - 1;
	int y2 = spt->Y - sh2;
	int u2 = pdi->width;
//	int v2 = 0;

	int x3 = spt->X + sw2 - 1;
	int y3 = spt->Y + sh2 - 1;
	int u3 = pdi->width;
	int v3 = pdi->height;

	int x4 = spt->X - sw2;
	int y4 = spt->Y + sh2 - 1;
//	int u4 = 0;
	int v4 = pdi->height;

	//NOTE:
	// clipping is done by OpenGL.

	float scale_u = gl_cluster_texinfo.coscale;
	float scale_v = gl_cluster_texinfo.aratio * scale_u;

	// append to vertex array
	int& bindx = gl_cluster_vcount;

	gl_cluster_vtxs[ bindx + 0 ].x = (GLfloat) x1;
	gl_cluster_vtxs[ bindx + 0 ].y = (GLfloat) y1;
	gl_cluster_vtxs[ bindx + 0 ].z = (GLfloat) depth;
	gl_cluster_vtxs[ bindx + 0 ].w = (GLfloat) 1.0f;
	gl_cluster_vtxs[ bindx + 0 ].s = (GLfloat) 0.0f; //u1 * scale_u;
	gl_cluster_vtxs[ bindx + 0 ].t = (GLfloat) 0.0f; //v1 * scale_v;

	gl_cluster_vtxs[ bindx + 1 ].x = (GLfloat) x2;
	gl_cluster_vtxs[ bindx + 1 ].y = (GLfloat) y2;
	gl_cluster_vtxs[ bindx + 1 ].z = gl_cluster_vtxs[ bindx ].z;
	gl_cluster_vtxs[ bindx + 1 ].w = (GLfloat) 1.0f;
	gl_cluster_vtxs[ bindx + 1 ].s = (GLfloat) u2 * scale_u;
	gl_cluster_vtxs[ bindx + 1 ].t = (GLfloat) 0.0f; //v2 * scale_v;

	gl_cluster_vtxs[ bindx + 2 ].x = (GLfloat) x3;
	gl_cluster_vtxs[ bindx + 2 ].y = (GLfloat) y3;
	gl_cluster_vtxs[ bindx + 2 ].z = gl_cluster_vtxs[ bindx ].z;
	gl_cluster_vtxs[ bindx + 2 ].w = (GLfloat) 1.0f;
	gl_cluster_vtxs[ bindx + 2 ].s = (GLfloat) u3 * scale_u;
	gl_cluster_vtxs[ bindx + 2 ].t = (GLfloat) v3 * scale_v;

	gl_cluster_vtxs[ bindx + 3 ].x = (GLfloat) x4;
	gl_cluster_vtxs[ bindx + 3 ].y = (GLfloat) y4;
	gl_cluster_vtxs[ bindx + 3 ].z = gl_cluster_vtxs[ bindx ].z;
	gl_cluster_vtxs[ bindx + 3 ].w = (GLfloat) 1.0f;
	gl_cluster_vtxs[ bindx + 3 ].s = (GLfloat) 0.0f; //u4 * scale_u;
	gl_cluster_vtxs[ bindx + 3 ].t = (GLfloat) v4 * scale_v;

#ifdef USE_INDEXED_TRIANGLES

	gl_cluster_trindxs[ gl_cluster_icount + 0 ] = bindx + 0;
	gl_cluster_trindxs[ gl_cluster_icount + 1 ] = bindx + 1;
	gl_cluster_trindxs[ gl_cluster_icount + 2 ] = bindx + 3;

	gl_cluster_trindxs[ gl_cluster_icount + 3 ] = bindx + 1;
	gl_cluster_trindxs[ gl_cluster_icount + 4 ] = bindx + 3;
	gl_cluster_trindxs[ gl_cluster_icount + 5 ] = bindx + 2;

	gl_cluster_icount += 6;

#endif

	bindx += 4;
}


// ----------------------------------------------------------------------------
//
INLINE
int DepthCull( SPoint *spt, geomv_t zdist )
{
	#define DEPTH_BIAS 2

	if ( AUX_DISABLE_POINT_VISIBILITY_DETECTION ) {
		return FALSE;
	}

	gl_cluster_drawinf.depthvalue = DEPTHBUFF_OOZ( zdist );

	// use depth-buffer to determine visibility of light-source
	depth_t midpoint = 0;

#ifdef FRACTIONAL_DEPTH_VALUES

	// compare with bias
#ifdef REVERSED_DEPTH_RANGE
	if ( midpoint < gl_cluster_drawinf.depthvalue + FIXED_TO_GEOMV( DEPTH_BIAS ) ) {
#else // REVERSED_DEPTH_RANGE
	if ( midpoint >= gl_cluster_drawinf.depthvalue + FIXED_TO_GEOMV( DEPTH_BIAS ) ) {
#endif // REVERSED_DEPTH_RANGE
		return TRUE;
	}

#else // FRACTIONAL_DEPTH_VALUES

#ifdef REVERSED_DEPTH_RANGE
	if ( (word)midpoint < gl_cluster_drawinf.depthvalue + DEPTH_BIAS ) {
#else // REVERSED_DEPTH_RANGE
	if ( (word)midpoint > gl_cluster_drawinf.depthvalue + DEPTH_BIAS ) {
#endif // REVERSED_DEPTH_RANGE
		return TRUE;
	}

#endif // FRACTIONAL_DEPTH_VALUES

	return FALSE;
}


// schedule single particle for drawing later on ------------------------------
//
INLINE
void RO_ScheduleParticle( particle_s *particle, SPoint *spt, geomv_t zdist )
{
	ASSERT( particle != NULL );
	ASSERT( spt != NULL );

	// field bitmap is actually rendering flags
	dword rendflags = particle->bitmap;

	if ( rendflags & PART_REND_POINTVIS ) {

		// midpoint not on screen means particle invisible
		if ( ( spt->X < 0 ) || ( spt->X >= Screen_Width  ) ||
			 ( spt->Y < 0 ) || ( spt->Y >= Screen_Height ) ) {
			return;
		}

		// midpoint occluded means entire particle invisible
		if ( DepthCull( spt, zdist ) ) {
			return;
		}
	}

	// calc texture size
	if ( rendflags & PART_REND_NODEPTHSCALE ) {

		// skip particle entirely if too far away
		if ( zdist > Far_View_Plane/2 ) {
			return;
		}

		//NOTE:
		// for no-depthscale particles the ref_z directly
		// determines the size of the particle with respect
		// to its texture width and height.

		// fixed texture size
		gl_cluster_drawinf.scalewidth  =
			( (int) ( gl_cluster_drawinf.width  * particle->ref_z ) ) & ~1; //TODO: cache?
		gl_cluster_drawinf.scaleheight =
			( (int) ( gl_cluster_drawinf.height * particle->ref_z ) ) & ~1; //TODO: remove even?

	} else {

		// calc scale factor according to reference z
		float scalefac = particle->ref_z / GEOMV_TO_FLOAT( zdist );

		// scale texture size
		gl_cluster_drawinf.scalewidth  =
			( (int) ( gl_cluster_drawinf.width  * scalefac ) ) & ~1; //TODO: remove even?
		gl_cluster_drawinf.scaleheight =
			( (int) ( gl_cluster_drawinf.height * scalefac ) ) & ~1;
	}
/*
	// check lower size boundary and draw either as texture or square
	if ( ( gl_cluster_drawinf.scalewidth  <= particle->sizebound ) ||
		 ( gl_cluster_drawinf.scaleheight <= particle->sizebound ) ) {

		visual_t drawcol = COLINDX_TO_VISUAL( particle->color );
		if ( ( rendflags & PART_REND_POINTVIS ) == 0 )
			gl_cluster_drawinf.depthvalue = DEPTHBUFF_OOZ( zdist );
		DrawParticleSquare( spt, &gl_cluster_drawinf, drawcol );

	} else
*/
	if ( ( gl_cluster_drawinf.scalewidth > 0 ) && ( gl_cluster_drawinf.scaleheight > 0 ) ) {

		int swidth2  = gl_cluster_drawinf.scalewidth  / 2;
		int sheight2 = gl_cluster_drawinf.scaleheight / 2;

		if ( ( spt->X > -swidth2  ) && ( spt->X < Screen_Width  + swidth2  ) &&
			 ( spt->Y > -sheight2 ) && ( spt->Y < Screen_Height + sheight2 ) ) {

			if ( ( rendflags & PART_REND_POINTVIS ) == 0 )
				gl_cluster_drawinf.depthvalue = DEPTHBUFF_OOZ( zdist );
			RO_ScheduleParticleVertices( spt, &gl_cluster_drawinf );
		}
	}
}


// determine texture entire particle cluster should be rendered with ----------
//
INLINE
void RO_DetermineClusterTexture( pcluster_s *cluster )
{
	ASSERT( cluster != NULL );
	ASSERT( cluster->numel > 0 );

	particle_s *particle = &cluster->rep[ 0 ];

	// no legacy stuff here
	ASSERT( particle->extinfo != NULL );
	ASSERT( ( particle->bitmap & iter_base_mask ) >= iter_texonly );

	// fetch extinfo (current texture and trafo frame)
	pextinfo_s *extinfo = particle->extinfo;

	pdef_s *pdef = extinfo->partdef; //TODO: partdef_dest
	ASSERT( pdef != NULL );

	ASSERT( pdef->tex_table != NULL );
	texfrm_s *curtexframe = &pdef->tex_table[ extinfo->tex_pos ];
	xfofrm_s *curxfoframe = pdef->xfo_table ?
		&pdef->xfo_table[ extinfo->xfo_pos ] : NULL;

	TextureMap *curtex = curtexframe->texmap;
	ASSERT( curtex != NULL );

	// rasterizer configuration for entire cluster
	dword raststate = rast_nozwrite | rast_texclamp | rast_chromakeyoff | rast_zcompare;
	dword rastmask  = rast_nomask;
	
	/*
	if ( particle->bitmap & PART_REND_NODEPTHCMP ) {
		raststate &= ~rast_mask_zcompare;
		rastmask  &= ~rast_mask_zcompare;
	}
	*/

	// fill drawing info
	gl_cluster_drawinf.width     = 1 << curtex->Width;
	gl_cluster_drawinf.height    = 1 << curtex->Height;
	gl_cluster_drawinf.texmap    = curtex;
	gl_cluster_drawinf.imgtrafo  = curxfoframe ? curxfoframe->imgtrafo : NULL;
	gl_cluster_drawinf.itertype  = particle->bitmap & PART_REND_MASK_ITER;
	gl_cluster_drawinf.raststate = raststate;
	gl_cluster_drawinf.rastmask  = rastmask;

	// fill texinfo structure and determine scale factor for coordinates
	RO_TextureMap2GLTexInfo( &gl_cluster_texinfo, curtex );
}


// schedule active cluster particles for drawing later on ---------------------
//
INLINE
int RO_ScheduleClusterParticles( pcluster_s *cluster )
{
	ASSERT( cluster != NULL );

	// scan contained particles
	int numactive = 0;
	for ( int curp = 0; curp < cluster->numel; curp++ ) {

		particle_s *particle = &cluster->rep[ curp ];

		// skip inactive particles
		if ( ( particle->flags & PARTICLE_ACTIVE ) == 0 )
			continue;
		else
			numactive++;

		Vertex3 tempvert;
		Vertex3 posvec = particle->position;

		// if position is in (particle)object-space transform into world-space
		if ( cluster->type & CT_PARTICLE_OBJ_MASK ) {

			Vertex3& origin = ((particleobj_pcluster_s*)cluster)->origin;

			posvec.X += origin.X;
			posvec.Y += origin.Y;
			posvec.Z += origin.Z;

		// if position is in object-space transform into world-space
		} else if ( cluster->type & CT_GENOBJECTRELATIVE_OBJ_MASK ) {

			GenObject *baseobject = ((objectbase_pcluster_s*)cluster)->baseobject;
			ASSERT( baseobject != NULL );
			MtxVctMUL( baseobject->ObjPosition, &posvec, &tempvert );
			posvec = tempvert;
		}

		// transform position into view-space
		MtxVctMUL( ViewCamera, &posvec, &tempvert );

		// draw view-space particle
		if ( ( tempvert.Z > Near_View_Plane ) && ( tempvert.Z < Far_View_Plane ) ) {

			// project particle to screen
			SPoint screenpos;
			PROJECT_TO_SCREEN( tempvert, screenpos );

			// schedule for drawing
			RO_ScheduleParticle( particle, &screenpos, tempvert.Z );
		}
	}

	return numactive;
}


// blast out all scheduled particles using one vertex array -------------------
//
INLINE
void RO_DrawScheduledParticles()
{
	// enforce texel source
	RO_SelectTexelSource( &gl_cluster_texinfo );

	// save zcmp and zwrite state
	int zcmpstate   = RO_DepthCmpEnabled();
	int zwritestate = RO_DepthWriteEnabled();

	// configure rasterizer
	RO_InitRasterizerState( gl_cluster_drawinf.itertype,
							gl_cluster_drawinf.raststate,
							gl_cluster_drawinf.rastmask );
	RO_TextureCombineState( texcomb_decal );

	// set fixed color
	glColor4ub( (GLubyte) 220, (GLubyte) 220, (GLubyte) 220, (GLubyte) 180 );

	// specify vertex arrays
	RO_ClientState( VTXARRAY_VERTICES | VTXARRAY_TEXCOORDS );
	if ( RO_ArrayMakeCurrent( VTXPTRS_RO_PART_1, gl_cluster_vtxs ) ) {
		glVertexPointer( 4, GL_FLOAT, sizeof( GLVertex4 ), &gl_cluster_vtxs->x );
		glTexCoordPointer( 2, GL_FLOAT, sizeof( GLVertex4 ), &gl_cluster_vtxs->s );
	}

	// draw vertex array
	glDrawArrays( GL_QUADS, 0, gl_cluster_vcount );

	// disable vertex arrays
//	RO_ClientState( VTXARRAY_NONE );

	// set rasterizer state to default
	RO_DefaultRasterizerState();

	// restore zcmp and zwrite state
	RO_RestoreDepthState( zcmpstate, zwritestate );
}


// render particle cluster ----------------------------------------------------
//
int R_DrawParticleCluster( pcluster_s *cluster, int *numactive )
{
	ASSERT( cluster != NULL );
	ASSERT( numactive != NULL );

	//NOTE:
	// this function exists purely for performance improvements, it
	// adds no functionality to what PART_SYS::DrawClusterParticles()
	// is able to do. it however allows to trade off code duplication
	// for performance improvements in the form of better (and direct)
	// accommodation of the actual underlying rendering API (OpenGL).

	// determine whether cluster can be rendered here
	if ( ( cluster->type & CT_HINT_PARTICLES_IDENTICAL ) == 0 )
		return FALSE;
	if ( ( cluster->type & CT_HINT_PARTICLES_HAVE_EXTINFO ) == 0 )
		return FALSE;
	if ( cluster->numel == 0 )
		return FALSE;
	ASSERT( cluster->rep[ 0 ].extinfo != NULL );
	if ( ( cluster->rep[ 0 ].bitmap & iter_base_mask ) < iter_texonly  )
		return FALSE;

	// determine texture for all particles in cluster
	RO_DetermineClusterTexture( cluster );

	// create empty vertex array
	gl_cluster_vcount = 0;
	gl_cluster_vtxs = (GLVertex4 *) ALLOCMEM( cluster->numel * sizeof( GLVertex4 ) * 4 );
	if ( gl_cluster_vtxs == NULL ) {
		OUTOFMEM( "no mem for cluster vertex array." );
	}

#ifdef USE_INDEXED_TRIANGLES

	// create index array
	gl_cluster_icount = 0;
	gl_cluster_trindxs = (dword *) ALLOCMEM( cluster->numel * sizeof( dword ) * 6 );
	if ( gl_cluster_trindxs == NULL ) {
		OUTOFMEM( "no mem for vertex indexes." );
	}

#endif // USE_INDEXED_TRIANGLES

	// schedule active particles
	*numactive = RO_ScheduleClusterParticles( cluster );

	// draw active and scheduled particles
	if ( ( *numactive > 0 ) && ( gl_cluster_vcount > 0 ) ) {
		RO_DrawScheduledParticles();
	}

	// delete vertex array
	FREEMEM( gl_cluster_vtxs );
	gl_cluster_vtxs = NULL;

#ifdef USE_INDEXED_TRIANGLES

	// delete index array
	FREEMEM( gl_cluster_trindxs );
	gl_cluster_trindxs = NULL;

#endif // USE_INDEXED_TRIANGLES

	return TRUE;
}


// render particles -----------------------------------------------------------
//
void R_DrawParticles()
{
	//FIXME:
	// fldcw is test only!

	RO_EnableDepthBuffer( true, true );

	//NOTE:
	// currently, only iterated particles are influenced
	// by the z-buffer setting done here. and then only
	// z-compare state. z-write is enabled for each particle.
	// for bitmap particles the z-buffer (both write and
	// compare) is enabled for each particle.

	// walk pre-callbacks
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_PRE_PARTICLES );

	// call particle system main
	PRTSYS_DrawParticles();

	// walk post-callbacks
	CALLBACK_WalkCallbacks( CBTYPE_DRAW_POST_PARTICLES );

	RO_DisableDepthBuffer( true, true );
}



