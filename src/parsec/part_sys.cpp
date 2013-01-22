/*
 * PARSEC - System Core
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:24 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1999
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_iter.h"
#include "d_misc.h"

// rendering subsystem
#include "r_part.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// particle types
#include "parttype.h"

// local module header
#include "part_sys.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_record.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "obj_game.h"
#include "part_ani.h"
#include "part_api.h"
#include "g_sfx.h"
#include "g_wfx.h"


// flags
#define USE_Z_BUFFER				// use z buffer to draw particles
//#define STORE_MAX_Z_VALS			// log max/min z values
//#define USER_SPHERE_CREATION		// enable user sphere creation (console)
#define DO_CLUSTER_CULLING			// enable culling of whole clusters
#define USE_NEW_CULLING_CODE		// use new culling function
//#define USE_BOUNDING_BOX_CULLING	// cull bounding box instead of sphere
#define ENABLE_PARTICLE_TRIANGLES	// enable triangle drawing for particles


// bitmap size below which it is replaced by a square of a single color
#define BITMAP_SIZE_BOUNDARY		3

// typical lower boundary of particle bitmap size
int partbitmap_size_bound		= BITMAP_SIZE_BOUNDARY;


// flag if initialization of particle system already done ---------------------
//
static int particle_init_done	= FALSE;


// list of particle clusters --------------------------------------------------
//
pcluster_s *Particles			= NULL;
pcluster_s *CurLinearCluster	= NULL;
pcluster_s *CustomDrawCluster	= NULL;


// init particle system -------------------------------------------------------
//
void InitParticleSystem()
{
	ASSERT( !particle_init_done );

	if ( !particle_init_done ) {

		// create single element for dummy head and tail
		if ( ( Particles = (pcluster_s *) ALLOCMEM( sizeof( pcluster_s ) ) ) == NULL )
			OUTOFMEM( 0 );

		// set pointers thusly
		Particles->next  = (pcluster_s *) ( (char*)Particles + sizeof( pcluster_s* ) );
		Particles->prec  = NULL;
		Particles->rep   = (particle_s *) Particles;

		// initially there are no elements and no allocated storage
		Particles->numel    = 0;
		Particles->maxnumel = 0;

		// init sizes of particles
		InitParticleSizes();

		particle_init_done = TRUE;
	}
}


// make resoscale available for hacking ---------------------------------------
//
float cur_particle_resoscale = 1.0f;


// init reference z values for particles according to resolution --------------
//
void InitParticleSizes()
{
	float resoscale = 1.8f * D_Value / 1024.0f; // FIXME: not accurate with different FOVs

	cur_particle_resoscale = resoscale;

	PRT_InitParticleSizes( resoscale );
	PAN_InitParticleSizes( resoscale );
	WFX_InitParticleSizes( resoscale );
	SFX_InitParticleSizes( resoscale );
}


// kill particle system -------------------------------------------------------
//
void KillParticleSystem()
{
	if ( particle_init_done ) {
		particle_init_done = FALSE;

		FreeParticles();

		FREEMEM( Particles );
		Particles = NULL;
	}
}


// remove all particles and free storage --------------------------------------
//
void FreeParticles()
{
	// free clusters attached to local ship
	PRT_FreeAttachedClusterList( MyShip );

	//NOTE:
	// all other attached clusters have already been
	// freed, since FreeParticles() is always invoked
	// after FreeObjects() which in turn invokes
	// PRT_FreeAttachedClusterList() for each object
	// it frees. apart from MyShip, that is.

	ASSERT( Particles->next != NULL );

	// free all clusters contained in list (except head/tail)
	pcluster_s *scan = NULL;
	for ( scan = Particles->next; scan->next; ) {

		pcluster_s *temp = scan->next;
		FREEMEM( scan->rep );
		FREEMEM( scan );
		scan = temp;
	}

	// reinit next field of head and prec field of tail
	Particles->next = scan;
	scan->prec		= Particles;

	// no particles contained in list anymore
	Particles->numel    = 0;
	Particles->maxnumel = 0;

	// invalidate global cluster pointers
	CurLinearCluster  = NULL;
	CustomDrawCluster = NULL;

	// reset number of particle extras
	CurrentNumPrtExtras = 0;
}


// let user create spheres via console aux commands ---------------------------
//
PRIVATE
void DoUserControlledSphereCreation()
{

#ifdef USER_SPHERE_CREATION

	ShipObject *ship1 = (ShipObject *) FetchFirstShip();

	if ( AUXDATA_PSPHERE_CREATION_CODE == 7 ) {

		if ( PRT_ObjectHasAttachedClusters( MyShip ) ) {
			PRT_FreeAttachedClusterList( MyShip );
		} else {
			objectbase_pcluster_s *cluster = PRT_CreateObjectCenteredSphere( MyShip, MyShip->BoundingSphere, SAT_STOCHASTIC_MOTION, SPHERE_PARTICLES, INFINITE_LIFETIME, 0, PlayerId );
		}

	} else if ( AUXDATA_PSPHERE_CREATION_CODE == 1 ) {

		if ( SFX_EnableInvulnerabilityShield( MyShip ) )
			MyShip->MegaShieldAbsorption = MegaShieldStrength;

	} else if ( AUXDATA_PSPHERE_CREATION_CODE == 2 ) {

		if ( ship1 ) {
			ShipObject *shippo = (ShipObject*) ship1;

			if ( PRT_ObjectHasAttachedClusters( shippo ) ) {
				PRT_FreeAttachedClusterList( shippo );
			} else {
				objectbase_pcluster_s *cluster = PRT_CreateObjectCenteredSphere( shippo, shippo->BoundingSphere, SAT_ROTATING, SPHERE_PARTICLES, INFINITE_LIFETIME, 0, PlayerId );
			}
		}

	} else if ( AUXDATA_PSPHERE_CREATION_CODE == 3 ) {

		if ( ship1 && ship1->NextObj ) {
			ShipObject *shippo = (ShipObject*) ship1->NextObj;

			if ( PRT_ObjectHasAttachedClusters( shippo ) ) {
				PRT_FreeAttachedClusterList( shippo );
			} else {
				objectbase_pcluster_s *cluster = PRT_CreateObjectCenteredSphere( shippo, shippo->BoundingSphere, SAT_ROTATING, SPHERE_PARTICLES, INFINITE_LIFETIME, 0, PlayerId );
			}
		}

	} else if ( AUXDATA_PSPHERE_CREATION_CODE == 4 ) {

		if ( ship1 && ship1->NextObj && ship1->NextObj->NextObj ) {
			ShipObject *shippo = (ShipObject*) ship1->NextObj->NextObj;

			if ( PRT_ObjectHasAttachedClusters( shippo ) ) {
				PRT_FreeAttachedClusterList( shippo );
			} else {
				objectbase_pcluster_s *cluster = PRT_CreateObjectCenteredSphere( shippo, shippo->BoundingSphere, SAT_ROTATING, SPHERE_PARTICLES, INFINITE_LIFETIME, 0, PlayerId );
			}
		}

	} else if ( AUXDATA_PSPHERE_CREATION_CODE == 5 ) {

		if ( ship1 && ship1->NextObj && ship1->NextObj->NextObj && ship1->NextObj->NextObj->NextObj ) {
			ShipObject *shippo = (ShipObject*) ship1->NextObj->NextObj->NextObj;

			if ( PRT_ObjectHasAttachedClusters( shippo ) ) {
				PRT_FreeAttachedClusterList( shippo );
			} else {
				objectbase_pcluster_s *cluster = PRT_CreateObjectCenteredSphere( shippo, shippo->BoundingSphere, SAT_ROTATING, SPHERE_PARTICLES, INFINITE_LIFETIME, 0, PlayerId );
			}
		}

	} else if ( AUXDATA_PSPHERE_CREATION_CODE == 6 ) {

		Vertex3 position;

//		FetchTVector( MyShip->ObjPosition, &position );

		position.X = position.Y = position.Z = 0;

		int type;
		switch ( AUXDATA_PSPHERE_TYPE ) {
			case 1:
				type = SAT_ROTATING;
				break;

			case 2:
				type = SAT_PULSATING;
				break;

			case 3:
				type = SAT_CONTRACTING;
				break;

			case 4:
				type = SAT_EXPLODING;
				break;

			default:
				type = SAT_NO_ANIMATION;
				break;
		}

		pcluster_s *cluster;
		if ( AUXDATA_PSPHERE_RADIUS > 0 ) {
			if ( type == SAT_EXPLODING ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FIXED_TO_GEOMV( AUXDATA_PSPHERE_RADIUS ), SAT_EXPLODING,
					SPHERE_EXPL_PARTICLES, SPHERE_EXPLOSION_DURATION, NULL, PlayerId );
			} else if ( type == SAT_PULSATING ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FIXED_TO_GEOMV( AUXDATA_PSPHERE_RADIUS ), SAT_PULSATING,
					SPHERE_PARTICLES, INFINITE_LIFETIME, NULL, PlayerId );
			} else if ( type == SAT_CONTRACTING ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FIXED_TO_GEOMV( AUXDATA_PSPHERE_RADIUS ), SAT_CONTRACTING,
					SPHERE_PARTICLES, CONTRACTING_SPHERE_LIFETIME, NULL, PlayerId );
			} else if ( type == SAT_NO_ANIMATION ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FIXED_TO_GEOMV( AUXDATA_PSPHERE_RADIUS ), SAT_NO_ANIMATION,
					SPHERE_PARTICLES, INFINITE_LIFETIME, NULL, PlayerId );
			} else {
				cluster = PRT_CreateParticleSphereObject(
					position, FIXED_TO_GEOMV( AUXDATA_PSPHERE_RADIUS ), SAT_ROTATING,
					SPHERE_PARTICLES, INFINITE_LIFETIME, NULL, PlayerId );
			}
		} else {
			if ( type == SAT_EXPLODING ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FIXED_TO_GEOMV( 0x500 ), SAT_EXPLODING,
					SPHERE_EXPL_PARTICLES, SPHERE_EXPLOSION_DURATION, NULL, PlayerId );
			} else if ( type == SAT_PULSATING ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FLOAT_TO_GEOMV( 16.0 ), SAT_PULSATING,
					SPHERE_PARTICLES, INFINITE_LIFETIME, NULL, PlayerId );
			} else if ( type == SAT_CONTRACTING ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FIXED_TO_GEOMV( 0x1000 ), SAT_CONTRACTING,
					SPHERE_PARTICLES, CONTRACTING_SPHERE_LIFETIME, NULL, PlayerId );
			} else if ( type == SAT_NO_ANIMATION ) {
				cluster = PRT_CreateParticleSphereObject(
					position, FLOAT_TO_GEOMV( 28.0 ), SAT_NO_ANIMATION,
					SPHERE_PARTICLES, INFINITE_LIFETIME, NULL, PlayerId );
			} else {
				cluster = PRT_CreateParticleSphereObject(
					position, FLOAT_TO_GEOMV( 28.0 ), SAT_ROTATING,
					SPHERE_PARTICLES, INFINITE_LIFETIME, NULL, PlayerId );
			}
		}


	}

	// reset creation code
	AUXDATA_PSPHERE_CREATION_CODE = 0;

#endif

}


// log maximum and minimum z values encountered -------------------------------
//
INLINE
void LogZValues( int intdepth )
{

#ifdef STORE_MAX_Z_VALS

	if ( AUXDATA_LEAST_VERTEX_1_OVER_Z == 0 )
		AUXDATA_LEAST_VERTEX_1_OVER_Z = intdepth;
	else if ( intdepth < AUXDATA_LEAST_VERTEX_1_OVER_Z ) {
		AUXDATA_LEAST_VERTEX_1_OVER_Z = intdepth;
	}

	if ( AUXDATA_GREATEST_VERTEX_1_OVER_Z == 0 )
		AUXDATA_GREATEST_VERTEX_1_OVER_Z = intdepth;
	else if ( intdepth > AUXDATA_GREATEST_VERTEX_1_OVER_Z ) {
		AUXDATA_GREATEST_VERTEX_1_OVER_Z = intdepth;
	}

#endif

}


// particle drawing info ------------------------------------------------------
//
struct pdrawinf_s {

	int			width;
	int			height;
	int			scalewidth;
	int			scaleheight;
	depth_t		depthvalue;

	TextureMap*	texmap;
	char*		bitmap;
	imgtrafo_s*	imgtrafo;
};


// draw particle as square of single color ------------------------------------
//
INLINE
void DrawParticleSquare( SPoint *spt, pdrawinf_s *pdi, visual_t drawcol )
{
	ASSERT( spt != NULL );
	ASSERT( pdi != NULL );

	int drawsiz = ( pdi->scaleheight > 0 ) ? pdi->scaleheight : 1;

#ifdef USE_Z_BUFFER

	if ( ZBufferEnabled ) {
		int intdepth = DEPTH_TO_INTEGER( pdi->depthvalue );
		D_DrawSquareZ( spt->X - drawsiz/2, spt->Y - drawsiz/2, drawcol, drawsiz, intdepth );
		LogZValues( intdepth );
	} else {
		D_DrawSquare( spt->X - drawsiz/2, spt->Y - drawsiz/2, drawcol, drawsiz );
	}

#else

	D_DrawSquare( spt->X - drawsiz/2, spt->Y - drawsiz/2, drawcol, drawsiz );

#endif

}


// draw particle as bitmap ----------------------------------------------------
//
INLINE
void DrawParticleBitmap( SPoint *spt, pdrawinf_s *pdi )
{
	ASSERT( spt != NULL );
	ASSERT( pdi != NULL );
	ASSERT( pdi->scalewidth > 0 );
	ASSERT( pdi->scaleheight > 0 );

#ifdef USE_Z_BUFFER

	if ( ZBufferEnabled ) {
		int intdepth = DEPTH_TO_INTEGER( pdi->depthvalue );
		D_PutSTCBitmapZ( pdi->bitmap, pdi->width, pdi->height,
						 pdi->scalewidth, pdi->scaleheight,
					     spt->X - pdi->scalewidth/2, spt->Y - pdi->scaleheight/2,
						 intdepth );
		LogZValues( intdepth );
	} else {
		D_PutSTCBitmap( pdi->bitmap, pdi->width, pdi->height,
						pdi->scalewidth, pdi->scaleheight,
						spt->X - pdi->scalewidth/2, spt->Y - pdi->scaleheight/2 );
	}

#else

	D_PutSTCBitmap( pdi->bitmap, pdi->width, pdi->height,
					pdi->scalewidth, pdi->scaleheight,
					spt->X - pdi->scalewidth/2, spt->Y - pdi->scaleheight/2 );
#endif

}


// rasterizer configuration for particle drawing ------------------------------
//
static word particle_raststate = rast_nozwrite | rast_texclamp | rast_chromakeyoff;
static word particle_rastmask  = rast_mask_zcompare;


// draw particle as texture ---------------------------------------------------
//
INLINE
void DrawParticleTexture( SPoint *spt, pdrawinf_s *pdi, dword itertype )
{
	ASSERT( spt != NULL );
	ASSERT( pdi != NULL );
	ASSERT( pdi->scalewidth > 0 );
	ASSERT( pdi->scaleheight > 0 );

//TODO:
//	pdi->imgtrafo

#ifdef USE_Z_BUFFER
	depth_t depthvalue = pdi->depthvalue;
	LogZValues( DEPTH_TO_INTEGER( depthvalue ) );
#else
	depth_t depthvalue = 0;
#endif

	// unclipped coordinates
	int x1 = spt->X - pdi->scalewidth/2;
	int y1 = spt->Y - pdi->scaleheight/2;
	int u1 = 0;
	int v1 = 0;

	int x2 = spt->X + pdi->scalewidth/2 - 1;
	int y2 = spt->Y - pdi->scaleheight/2;
	int u2 = pdi->width;
	int v2 = 0;

	int x3 = spt->X + pdi->scalewidth/2 - 1;
	int y3 = spt->Y + pdi->scaleheight/2 - 1;
	int u3 = pdi->width;
	int v3 = pdi->height;

	int x4 = spt->X - pdi->scalewidth/2;
	int y4 = spt->Y + pdi->scaleheight/2 - 1;
	int u4 = 0;
	int v4 = pdi->height;

	// calc inverse scale factors
	float sfacx = (float) pdi->width / pdi->scalewidth;
	float sfacy = (float) pdi->height / pdi->scaleheight;

#ifdef ENABLE_PARTICLE_TRIANGLES
	int drawtriangle = AUX_SMALL_PARTICLE_TRIANGLES;
	#define PARTICLE_CLIPPED() drawtriangle = 0
#else
	#define PARTICLE_CLIPPED()
#endif

	// clip coordinates
	if ( x1 < 0 ) {
		PARTICLE_CLIPPED();
		u1 -= (int)( x1 * sfacx );
		x1  = 0;
	}
	if ( y1 < 0 ) {
		PARTICLE_CLIPPED();
		v1 -= (int)( y1 * sfacy );
		y1  = 0;
	}

	if ( x2 >= Screen_Width ) {
		PARTICLE_CLIPPED();
		u2 -= (int)( ( x2 - Screen_Width ) * sfacx );
		x2  = Screen_Width - 1;
	}
	if ( y2 < 0 ) {
		PARTICLE_CLIPPED();
		v2 = (int)( -y2 * sfacy );
		y2 = 0;
	}

	if ( x3 >= Screen_Width ) {
		PARTICLE_CLIPPED();
		u3 -= (int)( ( x3 - Screen_Width ) * sfacx );
		x3  = Screen_Width - 1;
	}
	if ( y3 >= Screen_Height ) {
		PARTICLE_CLIPPED();
		v3 -= (int)( ( y3 - Screen_Height ) * sfacy );
		y3  = Screen_Height - 1;
	}

	if ( x4 < 0 ) {
		PARTICLE_CLIPPED();
		u4 -= (int)( x4 * sfacx );
		x4  = 0;
	}
	if ( y4 >= Screen_Height ) {
		PARTICLE_CLIPPED();
		v4 -= (int)( ( y4 - Screen_Height ) * sfacy );
		y4  = Screen_Height - 1;
	}

#ifdef ENABLE_PARTICLE_TRIANGLES

	if ( drawtriangle ) {

		// determine sizebound
		int sizebound = ( Screen_Width * drawtriangle ) / 256;

		// only if small enough (fill-rate!)
		if ( pdi->scalewidth < sizebound ) {

			y1 -= pdi->scaleheight;
			v1 -= pdi->height;
			x2 += pdi->scalewidth;
			y2 += pdi->scaleheight;
			u2 += pdi->width;
			v2 += pdi->height;
			x3  = x4;
			y3  = y4;
			u3  = u4;
			v3  = v4;

		} else {
			drawtriangle = 0;
		}
	}

#endif

	// fill iter header
	IterRectangle2 itrect;
	itrect.flags	 = ITERFLAG_NONE;
	itrect.itertype	 = itertype;
	itrect.raststate = particle_raststate;
	itrect.rastmask  = particle_rastmask;
	itrect.texmap	 = pdi->texmap;

	// fill vertex structs
	itrect.Vtxs[ 0 ].X = INT_TO_RASTV( x1 );
	itrect.Vtxs[ 0 ].Y = INT_TO_RASTV( y1 );
	itrect.Vtxs[ 0 ].Z = DEPTH_TO_RASTV( depthvalue );
	itrect.Vtxs[ 0 ].W = GEOMV_1;
	itrect.Vtxs[ 0 ].U = INT_TO_GEOMV( u1 );
	itrect.Vtxs[ 0 ].V = INT_TO_GEOMV( v1 );
	itrect.Vtxs[ 0 ].R = 220;
	itrect.Vtxs[ 0 ].G = 220;
	itrect.Vtxs[ 0 ].B = 220;
	itrect.Vtxs[ 0 ].A = 180;

	itrect.Vtxs[ 1 ].X = INT_TO_RASTV( x2 );
	itrect.Vtxs[ 1 ].Y = INT_TO_RASTV( y2 );
	itrect.Vtxs[ 1 ].Z = itrect.Vtxs[ 0 ].Z;
	itrect.Vtxs[ 1 ].W = GEOMV_1;
	itrect.Vtxs[ 1 ].U = INT_TO_GEOMV( u2 );
	itrect.Vtxs[ 1 ].V = INT_TO_GEOMV( v2 );
	itrect.Vtxs[ 1 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 1 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 1 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 1 ].A = itrect.Vtxs[ 0 ].A;

	itrect.Vtxs[ 2 ].X = INT_TO_RASTV( x3 );
	itrect.Vtxs[ 2 ].Y = INT_TO_RASTV( y3 );
	itrect.Vtxs[ 2 ].Z = itrect.Vtxs[ 0 ].Z;
	itrect.Vtxs[ 2 ].W = GEOMV_1;
	itrect.Vtxs[ 2 ].U = INT_TO_GEOMV( u3 );
	itrect.Vtxs[ 2 ].V = INT_TO_GEOMV( v3 );
	itrect.Vtxs[ 2 ].R = itrect.Vtxs[ 0 ].R;
	itrect.Vtxs[ 2 ].G = itrect.Vtxs[ 0 ].G;
	itrect.Vtxs[ 2 ].B = itrect.Vtxs[ 0 ].B;
	itrect.Vtxs[ 2 ].A = itrect.Vtxs[ 0 ].A;

#ifdef ENABLE_PARTICLE_TRIANGLES

	if ( drawtriangle ) {

		D_DrawIterTriangle2( (IterTriangle2 *) &itrect );

	} else

#endif

	{
		itrect.Vtxs[ 3 ].X = INT_TO_RASTV( x4 );
		itrect.Vtxs[ 3 ].Y = INT_TO_RASTV( y4 );
		itrect.Vtxs[ 3 ].Z = itrect.Vtxs[ 0 ].Z;
		itrect.Vtxs[ 3 ].W = GEOMV_1;
		itrect.Vtxs[ 3 ].U = INT_TO_GEOMV( u4 );
		itrect.Vtxs[ 3 ].V = INT_TO_GEOMV( v4 );
		itrect.Vtxs[ 3 ].R = itrect.Vtxs[ 0 ].R;
		itrect.Vtxs[ 3 ].G = itrect.Vtxs[ 0 ].G;
		itrect.Vtxs[ 3 ].B = itrect.Vtxs[ 0 ].B;
		itrect.Vtxs[ 3 ].A = itrect.Vtxs[ 0 ].A;

		D_DrawIterRectangle2( &itrect );
	}
}


// stores pointer to extinfo that should be used for entire cluster -----------
//
static pextinfo_s *cluster_global_extinfo = NULL;


// draw new standard particle (with extinfo) ----------------------------------
//
INLINE
void DrawParticleExtInfo( particle_s *particle, SPoint *spt, geomv_t zdist )
{
	ASSERT( particle != NULL );
	ASSERT( spt != NULL );

	pdrawinf_s pdi;

	// field bitmap is actually rendering flags
	dword rendflags = particle->bitmap;

	if ( rendflags & PART_REND_POINTVIS ) {

		// midpoint not on screen means particle invisible
		if ( ( spt->X < 0 ) || ( spt->X >= Screen_Width  ) ||
			 ( spt->Y < 0 ) || ( spt->Y >= Screen_Height ) ) {
			return;
		}

		pdi.depthvalue = DEPTHBUFF_OOZ( zdist );
	}

	// fetch extinfo (current texture and trafo frame)
	pextinfo_s *extinfo  = cluster_global_extinfo ?
						   cluster_global_extinfo : particle->extinfo;
	ASSERT( extinfo != NULL );

	pdef_s *pdef = extinfo->partdef; //TODO: partdef_dest
	ASSERT( pdef != NULL );

	ASSERT( pdef->tex_table != NULL );
	texfrm_s *curtexframe = &pdef->tex_table[ extinfo->tex_pos ];
	xfofrm_s *curxfoframe = pdef->xfo_table ?
		&pdef->xfo_table[ extinfo->xfo_pos ] : NULL;

	TextureMap *curtex = curtexframe->texmap;
	ASSERT( curtex != NULL );

	// fill drawing info
	pdi.width    = 1 << curtex->Width;
	pdi.height   = 1 << curtex->Height;
	pdi.texmap   = curtex;
	pdi.bitmap   = NULL;
	pdi.imgtrafo = curxfoframe ? curxfoframe->imgtrafo : NULL;

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
		pdi.scalewidth	= ( (int) ( pdi.width  * particle->ref_z ) ) & ~1; //TODO: cache?
		pdi.scaleheight	= ( (int) ( pdi.height * particle->ref_z ) ) & ~1; //TODO: remove even?

	} else {

		// calc scale factor according to reference z
		float scalefac = particle->ref_z / GEOMV_TO_FLOAT( zdist );

		// scale texture size
		pdi.scalewidth	= ( (int) ( pdi.width  * scalefac ) ) & ~1; //TODO: remove even?
		pdi.scaleheight	= ( (int) ( pdi.height * scalefac ) ) & ~1;
	}

	// check lower size boundary and draw either as texture or square
	if ( ( pdi.scalewidth  <= particle->sizebound ) ||
		 ( pdi.scaleheight <= particle->sizebound ) ) {

		visual_t drawcol = COLINDX_TO_VISUAL( particle->color );
		if ( ( rendflags & PART_REND_POINTVIS ) == 0 )
			pdi.depthvalue = DEPTHBUFF_OOZ( zdist );
		DrawParticleSquare( spt, &pdi, drawcol );

	} else if ( ( pdi.scalewidth > 0 ) && ( pdi.scaleheight > 0 ) ) {

		int swidth2  = pdi.scalewidth  / 2;
		int sheight2 = pdi.scaleheight / 2;

		if ( ( spt->X > -swidth2  ) && ( spt->X < Screen_Width  + swidth2  ) &&
			 ( spt->Y > -sheight2 ) && ( spt->Y < Screen_Height + sheight2 ) ) {

			// save rast
			word oldraststate = particle_raststate;
			word oldrastmask  = particle_rastmask;
			
			// use z-compare instead of slow depth buffer reads
			particle_raststate |= rast_zcompare;
			particle_rastmask = rast_nomask;

			// disable z-cmp if specified
			// disabled for now: using z compare instead of depth buffer reads
			/*
			if ( rendflags & PART_REND_NODEPTHCMP ) {
				particle_raststate &= ~rast_mask_zcompare;
				particle_rastmask  &= ~rast_mask_zcompare;
			}
			 */

			if ( ( rendflags & PART_REND_POINTVIS ) == 0 )
				pdi.depthvalue = DEPTHBUFF_OOZ( zdist );
			DrawParticleTexture( spt, &pdi, rendflags & PART_REND_MASK_ITER );

			// enable z-cmp
			particle_raststate = oldraststate;
			particle_rastmask  = oldrastmask;
		}
	}
}


// draw legacy particle (just bitmap, no extinfo) -----------------------------
//
INLINE
void DrawParticleLegacy( particle_s *particle, SPoint *spt, geomv_t zdist )
{
	ASSERT( particle != NULL );
	ASSERT( spt != NULL );

	// use simple bitmap to draw particle
	int bindx	 = particle->bitmap;

	pdrawinf_s pdi;
	pdi.width    = BitmapInfo[ bindx ].width;
	pdi.height   = BitmapInfo[ bindx ].height;
	pdi.texmap   = NULL;
	pdi.bitmap   = BitmapInfo[ bindx ].bitmappointer;
	pdi.imgtrafo = NULL;

	// calc scale factor according to reference z
	float scalefac = particle->ref_z / GEOMV_TO_FLOAT( zdist );

	// scale texture size
	pdi.scalewidth	= ( (int) ( pdi.width  * scalefac ) ) & ~1; //TODO: remove even?
	pdi.scaleheight	= ( (int) ( pdi.height * scalefac ) ) & ~1;

	// check lower size boundary and draw either as bitmap or square
	if ( ( pdi.scalewidth  <= particle->sizebound ) ||
		 ( pdi.scaleheight <= particle->sizebound ) ) {

		visual_t drawcol = COLINDX_TO_VISUAL( particle->color );
		pdi.depthvalue   = DEPTHBUFF_OOZ( zdist );
		DrawParticleSquare( spt, &pdi, drawcol );

	} else if ( ( pdi.scalewidth > 0 ) && ( pdi.scaleheight > 0 ) ) {

		int swidth2  = pdi.scalewidth  / 2;
		int sheight2 = pdi.scaleheight / 2;

		if ( ( spt->X > -swidth2  ) && ( spt->X < Screen_Width  + swidth2 ) &&
			 ( spt->Y > -sheight2 ) && ( spt->Y < Screen_Height + sheight2 ) ) {

			pdi.depthvalue = DEPTHBUFF_OOZ( zdist );
			DrawParticleBitmap( spt, &pdi );
		}
	}
}


// draw single particle at specified view-space coordinates -------------------
//
INLINE
void DrawParticle( particle_s *particle, Vertex3 *position )
{
	ASSERT( particle != NULL );
	ASSERT( position != NULL );

	// project particle to screen
	SPoint screenpos;
	PROJECT_TO_SCREEN( *position, screenpos );

	// draw particle depending on drawing type
	if ( particle->extinfo == NULL ) {

		// legacy particle without extinfo
		DrawParticleLegacy( particle, &screenpos, position->Z );

	} else {

		// standard particle with extinfo
		DrawParticleExtInfo( particle, &screenpos, position->Z );
	}
}


// check if ship entered energy field -----------------------------------------
//
INLINE
void CheckEnergyField( pcluster_s *cluster )
{
	if ( ( cluster->type & CT_TYPEMASK ) == CT_PARTICLE_SPHERE ) {
 		sphereobj_pcluster_s *objcluster = (sphereobj_pcluster_s *) cluster;
		if ( objcluster->animtype == SAT_ENERGYFIELD_SPHERE ) {
			if ( PRT_ParticleInBoundingSphere( MyShip, objcluster->origin ) ) {
				OBJ_BoostEnergy( MyShip, CurScreenRefFrames * 2 );
			}
		}
	}
}


// determine if sphere is entirely outside the viewing frustum ----------------
//
INLINE
int CullSphereAgainstFrustum( sphereobj_pcluster_s *cluster )
{
	geomv_t radius = cluster->bdsphere;

#ifdef USE_NEW_CULLING_CODE

	Vertex3 origin = cluster->origin;

	#ifdef USE_BOUNDING_BOX_CULLING

		CullBox3 cullbox;

		cullbox.minmax[ 0 ] = origin.X - radius;
		cullbox.minmax[ 1 ] = origin.Y - radius;
		cullbox.minmax[ 2 ] = origin.Z - radius;

		cullbox.minmax[ 3 ] = origin.X + radius;
		cullbox.minmax[ 4 ] = origin.Y + radius;
		cullbox.minmax[ 5 ] = origin.Z + radius;

		dword cullmask = 0x3f;
		return CULL_BoxAgainstCullVolume( &cullbox, World_CullVolume, &cullmask );

	#else

		Sphere3 sphere;
		sphere.X = origin.X;
		sphere.Y = origin.Y;
		sphere.Z = origin.Z;
		sphere.R = radius;

		dword cullmask = 0x3f;
		return CULL_SphereAgainstVolume( &sphere, World_ViewVolume, &cullmask );

	#endif

#else

	// transform origin into view-space
	Vertex3 origin;
	MtxVctMUL( ViewCamera, &cluster->origin, &origin );

	// calc potentially nearest and farthest z coordinates
	geomv_t nearestZ  = origin.Z - radius;
	geomv_t farthestZ = origin.Z + radius;

	// too near?
	if ( farthestZ < Near_View_Plane )
		return TRUE;
	// too far away?
	if ( nearestZ > Far_View_Plane )
		return TRUE;

	geomv_t criterionX = GEOMV_MUL( farthestZ, Criterion_X );
	// too far to the left or right?
	if ( ( origin.X - radius ) > criterionX )
		return TRUE;
	if ( ( origin.X + radius ) < -criterionX )
		return TRUE;

	geomv_t criterionY = GEOMV_MUL( farthestZ, Criterion_Y );
	// too far up or down?
	if ( ( origin.Y - radius ) > criterionY )
		return TRUE;
	if ( ( origin.Y + radius ) < -criterionY )
		return TRUE;

	return FALSE;

#endif

}


// cull cluster as a whole against viewing frustum ----------------------------
//
INLINE
int CullWholeCluster( pcluster_s *cluster )
{
	if ( AUX_DONT_CULL_PARTICLE_OBJECTS )
		return FALSE;

#ifdef DO_CLUSTER_CULLING

	//TODO:
	// cull all clusters where bdsphere is valid.

	if ( ( cluster->type & CT_TYPEMASK ) == CT_PARTICLE_SPHERE ) {

 		sphereobj_pcluster_s *objcluster = (sphereobj_pcluster_s *) cluster;
		if ( objcluster->animtype == SAT_ENERGYFIELD_SPHERE ) {

			// return culling status
			return CullSphereAgainstFrustum( objcluster );
		}
	}

#endif

	return FALSE;
}


// check if point is contained in ship's bounding sphere ----------------------
//
int PRT_ParticleInBoundingSphere( ShipObject *shippo, Vertex3& point )
{
	ASSERT( shippo != NULL );

	geomv_t objX = shippo->ObjPosition[ 0 ][ 3 ];
	geomv_t objY = shippo->ObjPosition[ 1 ][ 3 ];
	geomv_t objZ = shippo->ObjPosition[ 2 ][ 3 ];

	geomv_t bdsphere  = shippo->BoundingSphere;
	geomv_t bdsphere2 = shippo->BoundingSphere2;

	if ( point.X < ( objX - bdsphere ) ) return FALSE;
	if ( point.X > ( objX + bdsphere ) ) return FALSE;
	if ( point.Y < ( objY - bdsphere ) ) return FALSE;
	if ( point.Y > ( objY + bdsphere ) ) return FALSE;
	if ( point.Z < ( objZ - bdsphere ) ) return FALSE;
	if ( point.Z > ( objZ + bdsphere ) ) return FALSE;

	Vector3 dvec;
	dvec.X = point.X - objX;
	dvec.Y = point.Y - objY;
	dvec.Z = point.Z - objZ;

	return ( DOT_PRODUCT( &dvec, &dvec ) < bdsphere2 );
}


// draw customdraw particles belonging to specified object --------------------
//
int DrawCustomParticles( const GenObject *baseobject )
{
	int cfound = FALSE;

	// save rast
	word oldraststate = particle_raststate;
	word oldrastmask  = particle_rastmask;

	// disable z-cmp
	particle_raststate &= ~rast_mask_zcompare;
	particle_rastmask  &= ~rast_mask_zcompare;

	//TODO:
	// use clusters attached to baseobject to
	// avoid scanning all clusters.

	// walk list of clusters
	pcluster_s *scan = Particles->next;
	for ( ; scan->next; scan = scan->next ) {
		if ( ( scan->type & CT_TYPEMASK ) == CT_CUSTOMDRAW ) {

			customdraw_pcluster_s *cluster = (customdraw_pcluster_s *) scan;

			if ( cluster->baseobject != baseobject )
				continue;

			// to return if at least one found
			cfound = TRUE;

			// draw particles in cluster
			for ( int curp = 0; curp < cluster->numel; curp++ ) {

				particle_s *particle = &cluster->rep[ curp ];

				// skip inactive particles
				if ( ( particle->flags & PARTICLE_ACTIVE ) == 0 )
					continue;

				// transform position into view-space
				Vertex3 tempvert;
				MtxVctMUL( ViewCamera, &particle->position, &tempvert );

				// draw view-space particle
				if ( ( tempvert.Z > Near_View_Plane ) && ( tempvert.Z < Far_View_Plane ) ) {
					DrawParticle( particle, &tempvert );
				}
			}
		}
	}

	// enable z-cmp
	particle_raststate = oldraststate;
	particle_rastmask  = oldrastmask;

	return cfound;
}


// draw all particles in a cluster --------------------------------------------
//
INLINE
int DrawClusterParticles( pcluster_s *cluster )
{
	ASSERT( cluster != NULL );

	// early-out
	if ( cluster->numel == 0 )
		return 0;

	// count particles found active
	int numactive = 0;

	// try to use faster rendering function if enabled
	if ( AUX_DIRECT_CLUSTER_RENDERING ) {

		// the function determines whether it can render this cluster
		if ( R_DrawParticleCluster( cluster, &numactive ) ) {
			return numactive;
		}
	}

	// use only one extinfo if specified
	cluster_global_extinfo = ( cluster->type & CT_CLUSTER_GLOBAL_EXTINFO ) ?
		cluster->rep[ 0 ].extinfo : NULL;

	// render each particle separately
	for ( int curp = 0; curp < cluster->numel; curp++ ) {

		particle_s *particle = &cluster->rep[ curp ];

		// skip inactive particles
		if ( ( particle->flags & PARTICLE_ACTIVE ) == 0 ) {
			continue;
		} else {
			numactive++;
		}

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
			DrawParticle( particle, &tempvert );
		}
	}

	// avoid dangling extinfo
	cluster_global_extinfo = NULL;

	return numactive;
}


// draw all clusters in global list of particle clusters ----------------------
//
PRIVATE
void DrawClusterList()
{
	// count number of clusters in list and actually visible clusters
	int walked_clusters  = 0;
	int visible_clusters = 0;

	// walk list of clusters
	pcluster_s *cluster = Particles->next;
	for ( ; cluster->next; ++walked_clusters ) {

		// skip cluster if it should not be drawn here
		if ( cluster->type & CT_DONT_DRAW_AUTOMATICALLY ) {
			cluster = cluster->next;
			continue;
		}

		// don't draw particles if cockpit view active?
		if ( cluster->type & CT_DONT_DRAW_IN_COCKPIT_MASK ) {
			ASSERT( cluster->type & CT_GENOBJECTRELATIVE_OBJ_MASK );
			objectbase_pcluster_s *bcluster = (objectbase_pcluster_s*) cluster;
			if ( ( bcluster->baseobject == MyShip ) && !ObjCameraActive ) {
				cluster = cluster->next;
				continue;
			}
		}

		// don't draw genobject clusters if baseobject not visible
		if ( ( cluster->type & CT_TYPEMASK ) == CT_GENOBJECT_PARTICLES ) {

			if ( AUX_DISABLE_GENOBJECT_PARTICLES ) {
				cluster = cluster->next;
				continue;
			}

			genobject_pcluster_s *gencluster =  (genobject_pcluster_s *) cluster;

			GenObject *baseobject = gencluster->baseobject;
			ASSERT( baseobject != NULL );

			if ( ( cluster->type & CT_DONT_CULL_WITH_GENOBJECT ) == 0 ) {
				// cull cluster if object is invisible
				if ( baseobject->VisibleFrame != CurVisibleFrame ) {
					cluster = cluster->next;
					continue;
				}
			}
		}

		// don't draw cluster if base ship invisible during explosion
		if ( cluster->type & CT_DONT_DRAW_IF_BASE_VISNEVER ) {
			objectbase_pcluster_s *bcluster = (objectbase_pcluster_s*) cluster;
			ASSERT( bcluster->baseobject != NULL );
			if ( bcluster->baseobject->VisibleFrame == VISFRAME_NEVER ) {
				cluster = cluster->next;
				continue;
			}
		}

		// determine if cluster visible
		if ( CullWholeCluster( cluster ) ) {
			cluster = cluster->next;
			continue;
		}
		visible_clusters++;

		// check if ship entered energy field
		CheckEnergyField( cluster );

		// draw particles in cluster
		int numactive = DrawClusterParticles( cluster );

		// remember next cluster in list
		pcluster_s *temp = cluster->next;

		// remove cluster if no active particles contained anymore
		if ( numactive == 0 ) {
//			ASSERT(0);//FIXME: we currently assume this will never be hit!

   			PRT_DeleteCluster( cluster );
		}

		cluster = temp;
	}

	AUXDATA_NUM_PARTICLE_CLUSTERS = walked_clusters;
	AUXDATA_NUM_VISIBLE_PCLUSTERS = visible_clusters;
}


// main entry point of the particle system ------------------------------------
//
void PRTSYS_DrawParticles()
{
	//NOTE:
	// this function is called each frame by
	// R_DrawParticles() (Rx_PART.C), which is
	// called by the gameloop (G_MAIN.C).

	// particle system enabled?
	if ( !ParticleSysEnabled || AUX_DISABLE_PARTICLE_SYSTEM )
		return;

	// let user manually create/destroy spheres (via console)
	DoUserControlledSphereCreation();

	// perform particle actions
	PAN_AnimateParticles();

	// walk list of clusters
	DrawClusterList();
}



