/*
 * PARSEC - Cloud Drawing Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999
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
#include "sys_defs.h"
#include "vid_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_iter.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_cloud.h"

// proprietary module headers
#include "con_aux.h"
#include "e_callbk.h"
#include "e_supp.h"


#define NUM_LAYERS	5
#define SINTAB_SIZE	64

#define MODULATE_DISTANCE
#define MODULATE_COLOR
#define MODULATE_ALPHA

#define DISTANCE_BASE 		FLOAT_TO_GEOMV( 50.0 )
#define ALPHA_BASE			100
#define COLOR_BASE			120


// texture coordinate storage structure for one layer --------------------
//
struct layer_s {

	float		u[ 4 ];
	float		v[ 4 ];
	float		u_offs[ 4 ];
	float		v_offs[ 4 ];

};

static layer_s		layers[ NUM_LAYERS ];

static geomv_t	  	sintab[ SINTAB_SIZE ];

static refframe_t	prev_refframes 		= 0;
static refframe_t	mod_interval 		= 100;
static int			sintab_ctr 			= 0;
static int			sintab_ctr_col 		= 0;
static int			sintab_ctr_alpha 	= 0;

static geomv_t		cloud_dist 			= DISTANCE_BASE;
static int			red_amount 			= COLOR_BASE;
static int			alpha_amount 		= ALPHA_BASE;

static int			firstframe = TRUE;


// draw cloud ------------------------------------------------------------
//
int DrawSmokeCloud( void *param )
{
	if ( !AUX_ENABLE_VOLUMETRIC_CLOUDS )
		return FALSE;

	refframe_t cur_refframes = SYSs_GetRefFrameCount();

	if ( ( cur_refframes - prev_refframes ) > mod_interval ) {

#ifdef MODULATE_DISTANCE
		ModulateDistance();
#endif

#ifdef MODULATE_COLOR
		ModulateColor();
#endif

#ifdef MODULATE_ALPHA
		ModulateAlpha();
#endif

		int num_intervals = ( cur_refframes - prev_refframes ) / mod_interval;
		prev_refframes += ( mod_interval * num_intervals );
	}

	return DrawCloudLayers();
}


// draw all cloud layers for a given axis --------------------------------
//
int DrawCloudLayers()
{

	// get pointer to texture map
	TextureMap *texmap = FetchTextureMap( "cloud" );
	if ( texmap == NULL ) {
		MSGOUT( "texture 'cloud' was not found" );
		return FALSE;
	}

	int width 	= 1 << texmap->Width;
	int height 	= 1 << texmap->Height;
	int layerid = 0;
	int vid = 0;

	if ( firstframe ) {

		firstframe = FALSE;
		
		for ( layerid = 0; layerid < NUM_LAYERS; layerid++ ) {

			for ( vid = 0; vid < 4; vid++) {

				layers[ layerid ].u[ vid ] = RAND() % width;
				layers[ layerid ].v[ vid ] = RAND() % height;
				layers[ layerid ].u_offs[ vid ] = ( RAND() % width ) / 1500.0f;
				layers[ layerid ].v_offs[ vid ] = ( RAND() % height ) / 1500.0f;
			}
		}
	}


	for ( layerid = 0; layerid < NUM_LAYERS; layerid++ ) {
		for ( vid = 0; vid < 4; vid++) {

			layers[ layerid ].u[ vid ] += layers[ layerid ].u_offs[ vid ];
			layers[ layerid ].v[ vid ] += layers[ layerid ].v_offs[ vid ];

			if ( layers[ layerid ].u[ vid ] < 0 ) {
				layers[ layerid ].u[ vid ] = 0;
				layers[ layerid ].u_offs[ vid ] = - layers[ layerid ].u_offs[ vid ];
			}

			if ( layers[ layerid ].u[ vid ] > width ) {
				layers[ layerid ].u[ vid ] = width - 1;
				layers[ layerid ].u_offs[ vid ] = - layers[ layerid ].u_offs[ vid ];
			}

			if ( layers[ layerid ].v[ vid ] < 0 ) {
				layers[ layerid ].v[ vid ] = 0;
				layers[ layerid ].v_offs[ vid ] = - layers[ layerid ].v_offs[ vid ];
			}

			if ( layers[ layerid ].v[ vid ] > height ) {
				layers[ layerid ].v[ vid ] = height - 1;
				layers[ layerid ].v_offs[ vid ] = - layers[ layerid ].v_offs[ vid ];
			}

			layers[ layerid ].u[ vid ] += ( (float) CurYaw ) / 50.0f;
			layers[ layerid ].v[ vid ] += ( (float) CurPitch ) / 50.0f;
		
		}

		IterRectangle3 itrect;

		int half_x = Screen_Width / 2;
		int half_y = Screen_Height / 2;

		itrect.Vtxs[ 0 ].X = half_x;
		itrect.Vtxs[ 0 ].Y = - half_y;
		itrect.Vtxs[ 1 ].X = half_x;
		itrect.Vtxs[ 1 ].Y = half_y;
		itrect.Vtxs[ 2 ].X = - half_x;
		itrect.Vtxs[ 2 ].Y = half_y;
		itrect.Vtxs[ 3 ].X = - half_x;
		itrect.Vtxs[ 3 ].Y = - half_y;

		for ( vid = 0; vid < 4; vid++ ) {

			itrect.Vtxs[ vid ].Z = layerid * cloud_dist;
			itrect.Vtxs[ vid ].W = GEOMV_1;
			itrect.Vtxs[ vid ].U = layers[ layerid ].u[ vid ];
			itrect.Vtxs[ vid ].V = layers[ layerid ].v[ vid ];
			itrect.Vtxs[ vid ].R = red_amount; //140;
			itrect.Vtxs[ vid ].G = 0; //50;
			itrect.Vtxs[ vid ].B = 200;
			itrect.Vtxs[ vid ].A = alpha_amount;
		}

		itrect.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW | ITERFLAG_Z_TO_DEPTH;
//		itrect.itertype  = iter_rgb;
//		itrect.itertype  = iter_rgbatexa | iter_alphablend;
		itrect.itertype  = iter_texrgba | iter_alphablend;
		itrect.raststate = /*rast_zbuffer |*/ rast_texwrap | rast_chromakeyoff;
		itrect.rastmask  = rast_mask_zcompare;
		itrect.texmap	 = texmap;

		// clip and draw polygon
		D_DrawIterRectangle3( &itrect, 0x3f );
	}

	return TRUE;

}


// modulate distance between cloud layers --------------------------------
//
void ModulateDistance()
{

	cloud_dist = DISTANCE_BASE + sintab[ sintab_ctr++ ];
	if ( sintab_ctr == SINTAB_SIZE )
		sintab_ctr = 0;

}


// modulate alpha of cloud -----------------------------------------------
//
void ModulateAlpha()
{

	red_amount = (int)(ALPHA_BASE + 8 * sintab[ sintab_ctr_col++ ]);
	if ( sintab_ctr_col == SINTAB_SIZE )
		sintab_ctr_col = 0;

}


// modulate color of cloud ----------------------------------------------------
//
void ModulateColor()
{

	alpha_amount = (int)(COLOR_BASE + 4 * sintab[ sintab_ctr_alpha++ ]);
	if ( sintab_ctr_alpha == SINTAB_SIZE )
		sintab_ctr_alpha = 0;

}


// register cloud callbacks ---------------------------------------------------
//
PRIVATE
void CLOUD_RegisterCallbacks()
{
	// specify callback type and flags
	int callbacktype = CBTYPE_DRAW_POST_WORLD | CBFLAG_PERSISTENT;

	// register the drawing callback
	CALLBACK_RegisterCallback( callbacktype, DrawSmokeCloud, (void*) NULL );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_CLOUD )
{
	// precalculate sinus table for cloud modulation
	for ( int i = 0; i < SINTAB_SIZE; i++ ) {
		sintab[ i ] = FLOAT_TO_GEOMV( 9 * sin( ( i * 3.14159265359 ) / ( SINTAB_SIZE >> 1 ) ) );
	}

	CLOUD_RegisterCallbacks();
}



