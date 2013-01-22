/*
 * PARSEC - Explosion Drawing Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:25 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1995-1999
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
#include <stddef.h>
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
#include "net_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_iter.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_explod.h"

// proprietary module headers
#include "con_aux.h"
#include "h_supp.h"
#include "obj_ctrl.h"
#include "part_api.h"
#include "part_def.h"
#include "part_sys.h"
#include "g_shkwav.h"



// constants ------------------------------------------------------------------
//
#define EXPLOSION_REFERENCE_Z	900.0
#define EXPLOSION_REFERENCE_Z_m 420.0
#define EXPLOSION_REFERENCE_Z_h 1380.0
#define EXPLOSION_REFERENCE_Z_v 1600.0


// draw explosion using custom draw particles ---------------------------------
//
PRIVATE
void DrawParticleBaseExplosion( const ShipObject *shippo )
{
	if ( shippo->ExplosionCount == MAX_EXPLOSION_COUNT ) {

		static int dfac_1 = RAND() % 7 + 3;
		static int dfac_2 = RAND() % 7 + 3;
		static int dfac_3 = RAND() % 7 + 3;
		static int dfac_4 = RAND() % 7 + 3;
		static int dfac_5 = RAND() % 7 + 3;

		// alter basic attributes
		float ref_z = 600;//400;//spreadfire_ref_z * 2;
		int bitmap    = iter_texrgba | iter_specularadd;
		int color	  = 0;//SPREADFIRE_PARTICLE_COLOR;
		int sizebound = PRT_NO_SIZEBOUND;
		int lifetime  = 1000; //shippo->SpreadLifeTime;

		// fetch pdef
		pdef_s *pdef = PDEF_explode1();
		if ( pdef == NULL )
			return;

		// create pextinfo
		pextinfo_s extinfo;
		PRT_InitParticleExtInfo( &extinfo, pdef, NULL, NULL );

		Vector3 dirvec;
		fixed_t speed = 0;
		DirVctMUL( shippo->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );

		Vertex3 object_space;
		object_space.X = GEOMV_0;
		object_space.Y = GEOMV_0;
		object_space.Z = GEOMV_0;

		Vertex3 world_space;
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );

		dword owner = GetObjectOwner( shippo );

		particle_s particle;
		PRT_InitParticle( particle, bitmap, color, sizebound,
							ref_z, &world_space, &dirvec,
							lifetime, owner, &extinfo );
		PRT_CreateCustomDrawParticle( particle, shippo );
//--
		ref_z = 100;
		object_space.X = INT_TO_GEOMV( 0 + dfac_1 );
		object_space.Y = INT_TO_GEOMV( 30 - dfac_1 );
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
		PRT_InitParticle( particle, bitmap, color, sizebound,
							ref_z, &world_space, &dirvec,
							lifetime, owner, &extinfo );
		PRT_CreateCustomDrawParticle( particle, shippo );
//--
		ref_z = 80;
		object_space.X = INT_TO_GEOMV( -20 + dfac_2 );
		object_space.Y = INT_TO_GEOMV( 10 - dfac_2 );
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
		PRT_InitParticle( particle, bitmap, color, sizebound,
							ref_z, &world_space, &dirvec,
							lifetime, owner, &extinfo );
		PRT_CreateCustomDrawParticle( particle, shippo );
//--
		ref_z = 120;
		object_space.X = INT_TO_GEOMV( 20 - dfac_3 );
		object_space.Y = INT_TO_GEOMV( 0 + dfac_3 );
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
		PRT_InitParticle( particle, bitmap, color, sizebound,
							ref_z, &world_space, &dirvec,
							lifetime, owner, &extinfo );
		PRT_CreateCustomDrawParticle( particle, shippo );
//--
		ref_z = 30;
		object_space.X = INT_TO_GEOMV( 30 - dfac_4 );
		object_space.Y = INT_TO_GEOMV( -20 + dfac_4 );
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
		PRT_InitParticle( particle, bitmap, color, sizebound,
							ref_z, &world_space, &dirvec,
							lifetime, owner, &extinfo );
		PRT_CreateCustomDrawParticle( particle, shippo );
//--
		ref_z = 140;
		object_space.X = INT_TO_GEOMV( 0 + dfac_5 );
		object_space.Y = INT_TO_GEOMV( -30 + dfac_5 );
		MtxVctMUL( shippo->ObjPosition, &object_space, &world_space );
		PRT_InitParticle( particle, bitmap, color, sizebound,
							ref_z, &world_space, &dirvec,
							lifetime, owner, &extinfo );
		PRT_CreateCustomDrawParticle( particle, shippo );
//--
	} else {

		// move particles along with the ship
		if ( !AUX_DISABLE_DYING_SHIP_MOVEMENT ) {

			Vector3 dirvec;
			fixed_t speed = shippo->CurSpeed * CurScreenRefFrames;
			DirVctMUL( shippo->ObjPosition, FIXED_TO_GEOMV( speed ), &dirvec );

			PRT_TranslateCustomParticles( dirvec, shippo );
		}
	}

	PRT_DrawCustomParticles( shippo );
}


// draw legacy explosion using bitmaps ----------------------------------------
//
PRIVATE
void DrawBitmapExplosion( const ShipObject *shippo )
{
	float refz = EXPLOSION_REFERENCE_Z_h * Screen_Height / 760.0f; // FIXME

	dword frameno = BM_EXPLANIMBASE + ( BM_NUMEXPLFRAMES-1 - shippo->ExplosionCount / EXPL_REF_SPEED );

	Vertex3 shipt;
	FetchTVector( shippo->CurrentXmatrx, &shipt );
	SPoint shiploc;
	PROJECT_TO_SCREEN( shipt, shiploc );

	float scalefac = refz / GEOMV_TO_FLOAT( shipt.Z );

	dword scalewidth  = (dword) ( BitmapInfo[ frameno ].width * scalefac );
	dword scaleheight = (dword) ( BitmapInfo[ frameno ].height * scalefac );

	scalewidth  &= ~1;
	scaleheight &= ~1;

	D_PutSTCBitmap( BitmapInfo[ frameno ].bitmappointer,
				  BitmapInfo[ frameno ].width,
				  BitmapInfo[ frameno ].height,
				  scalewidth, scaleheight,
				  shiploc.X - scalewidth/2,
				  shiploc.Y - scaleheight/2 );

	// draw secondary explosion bitmaps
	if ( !AUX_USE_SIMPLE_EXPLOSION ) {

		static int dfac_1 = 1;
		static int dfac_2 = 1;
		static int dfac_3 = 1;
		static int dfac_4 = 1;
		static int dfac_5 = 1;

		if ( shippo->ExplosionCount == MAX_EXPLOSION_COUNT ) {
			dfac_1 = RAND() % 7 + 3;
			dfac_2 = RAND() % 7 + 3;
			dfac_3 = RAND() % 7 + 3;
			dfac_4 = RAND() % 7 + 3;
			dfac_5 = RAND() % 7 + 3;
		}

		dword _scalewidth  = scalewidth  / 2;
		dword _scaleheight = scaleheight / 2;
		if ( scalewidth > 0 && scaleheight > 0 )
			D_PutSTCBitmap( BitmapInfo[ frameno ].bitmappointer,
						  BitmapInfo[ frameno ].width,
						  BitmapInfo[ frameno ].height,
						  _scalewidth, _scaleheight,
						  shiploc.X - _scalewidth/2  - _scalewidth/dfac_1,
						  shiploc.Y - _scaleheight/2 - _scaleheight/dfac_2 );
		if ( scalewidth > 0 && scaleheight > 0 )
			D_PutSTCBitmap( BitmapInfo[ frameno ].bitmappointer,
						  BitmapInfo[ frameno ].width,
						  BitmapInfo[ frameno ].height,
						  _scalewidth, _scaleheight,
						  shiploc.X - _scalewidth/2  + _scalewidth/dfac_3,
						  shiploc.Y - _scaleheight/2 + _scaleheight/dfac_4 );

		_scalewidth  = scalewidth  / 3;
		_scaleheight = scaleheight / 3;
		if ( scalewidth > 0 && scaleheight > 0 )
			D_PutSTCBitmap( BitmapInfo[ frameno ].bitmappointer,
						  BitmapInfo[ frameno ].width,
						  BitmapInfo[ frameno ].height,
						  _scalewidth, _scaleheight,
						  shiploc.X - _scalewidth/2  + _scalewidth/dfac_2,
						  shiploc.Y - _scaleheight/2 - _scaleheight/dfac_5 );

		_scalewidth  = scalewidth  / 4;
		_scaleheight = scaleheight / 4;
		if ( scalewidth > 0 && scaleheight > 0 )
			D_PutSTCBitmap( BitmapInfo[ frameno ].bitmappointer,
						  BitmapInfo[ frameno ].width,
						  BitmapInfo[ frameno ].height,
						  _scalewidth, _scaleheight,
						  shiploc.X - _scalewidth/2  + _scalewidth/dfac_3,
						  shiploc.Y - _scaleheight/2 + _scaleheight/dfac_1 );

		_scalewidth  = scalewidth  / 5;
		_scaleheight = scaleheight / 5;
		if ( scalewidth > 0 && scaleheight > 0 )
			D_PutSTCBitmap( BitmapInfo[ frameno ].bitmappointer,
						  BitmapInfo[ frameno ].width,
						  BitmapInfo[ frameno ].height,
						  _scalewidth, _scaleheight,
						  shiploc.X - _scalewidth/2  + _scalewidth/dfac_5,
						  shiploc.Y - _scaleheight/2 + _scaleheight/dfac_4 );
	}
}


// draw single frame of explosion animation -----------------------------------
//
void DrawExpAnim( const ShipObject *shippo )
{
	ASSERT( shippo != NULL );

	// create shock wave if enabled (on first frame)
	if ( AUX_EXPLOSION_DRAW_SHOCKWAVE ) {
		ExplosionShockWave( shippo );
	}

	// draw explosion
	if ( AUX_PARTICLE_BASE_EXPLOSION ) {

		// anim frames are particles
		DrawParticleBaseExplosion( shippo );

	} else {

		// anim frames are bitmaps
		DrawBitmapExplosion( shippo );
	}
}



