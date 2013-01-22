/*
 * PARSEC - Explosion ShockWave Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:36 $
 *
 * Orginally written by:
 *   Copyright (c) Michael Woegerbauer <maiki@parsec.org> 1998-2000
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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
#include "g_shkwav.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_info.h"
#include "con_main.h"
#include "e_callbk.h"
#include "e_supp.h"
#include "obj_ctrl.h"
#include "obj_cust.h"
#include "part_api.h"
#include "part_def.h"
#include "part_sys.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char shockwave_inval_lifetime_spec[]	= "lifetime invalid";
static char shockwave_inval_maxwidth_spec[]	= "maxwidth invalid";
static char shockwave_inval_lambda_spec[]	= "lambda invalid";
static char shockwave_inval_fadeout_spec[]  = "fadeout invalid";


// preset shock wave type property values -------------------------------------
//
#define SHOCKWAVE_LIFETIME		1200
#define SHOCKWAVE_MAX_LIFETIME	2400
#define SHOCKWAVE_MIN_LIFETIME	60
#define SHOCKWAVE_DELAY			400
#define SHOCKWAVE_MAX_WIDTH		600.0f
#define SHOCKWAVE_FADEOUT       400
#define SHOCKWAVE_RED           220
#define SHOCKWAVE_GREEN			220
#define SHOCKWAVE_BLUE			220
#define SHOCKWAVE_ALPHA			180
#define SHOCKWAVE_LAMBDA		3.0
#define SHOCKWAVE_YAW_LOW		( BAMS_DEG45 / 2 )	//0x0800
#define SHOCKWAVE_YAW_HIGH		BAMS_DEG45			//0x1500		//0x1800


// shock wave custom type structure -------------------------------------------
//
struct ShockWave : CustomObject {

	Xmatrx		WorldXmatrx;	// object- to worldspace matrix
	Vertex3		ViewVtxs[ 4 ];	// vertices in worldspace
	TextureMap*	texmap;
	GenObject*	owner;
	int			vtxsvalid;
	long		alive;
	int			delay;
	int			red;
	int			green;
	int			blue;
	int			alpha;
};

#define OFS_DELAY			offsetof( ShockWave, delay )
#define OFS_RED				offsetof( ShockWave, red )
#define OFS_GREEN			offsetof( ShockWave, green )
#define OFS_BLUE			offsetof( ShockWave, blue )
#define OFS_ALPHA			offsetof( ShockWave, alpha )


// list of console-accessible properties --------------------------------------
//
PRIVATE
proplist_s ShockWave_PropList[] = {

	{ "delay",		OFS_DELAY,		0,			0xffff,		PROPTYPE_INT	},
	{ "red",		OFS_RED,		0,			0xff,		PROPTYPE_INT	},
	{ "green",		OFS_GREEN,		0,			0xff,		PROPTYPE_INT	},
	{ "blue",		OFS_BLUE,		0,			0xff,		PROPTYPE_INT	},
	{ "alpha",		OFS_ALPHA,		0,			0xff,		PROPTYPE_INT	},

	{ NULL,			0,			0,			0,			0				}
};


// assigned type id for shock wave type ---------------------------------------
//
static dword shockwave_type_id;


// ----------------------------------------------------------------------------
//
static Vertex3   ObjVtxs[ 4 ] = {

	{ INT_TO_GEOMV( 0 ), INT_TO_GEOMV(  1 ), INT_TO_GEOMV(  1  ) },
	{ INT_TO_GEOMV( 0 ), INT_TO_GEOMV(  1 ), INT_TO_GEOMV( -1  ) },
	{ INT_TO_GEOMV( 0 ), INT_TO_GEOMV( -1 ), INT_TO_GEOMV( -1  ) },
	{ INT_TO_GEOMV( 0 ), INT_TO_GEOMV( -1 ), INT_TO_GEOMV(  1  ) },
};


// texture maps for shock wave ------------------------------------------------
//
static TextureMap **shockwave_texmaps	= NULL;
static int			shockwave_texframes	= 0;


// full table for shock wave expansion ----------------------------------------
//
static float *expansion_tab = NULL;


// ----------------------------------------------------------------------------
//
static int			shockwave_lifetime	= SHOCKWAVE_LIFETIME;
static geomv_t		shockwave_max_width	= SHOCKWAVE_MAX_WIDTH;
static float		shockwave_lambda	= SHOCKWAVE_LAMBDA;
static int          shockwave_fadeout   = SHOCKWAVE_FADEOUT;


// macro to set the properties of a itervertex --------------------------------
//
#define SET_ITER_VTX( itvtx, vtx, u, v, r, g, b, a ) \
	(itvtx)->X = (vtx)->X;	\
	(itvtx)->Y = (vtx)->Y;	\
	(itvtx)->Z = (vtx)->Z;	\
	(itvtx)->W = GEOMV_1;	\
	(itvtx)->U = (u);		\
	(itvtx)->V = (v);		\
	(itvtx)->R = (r);		\
	(itvtx)->G = (g);		\
	(itvtx)->B = (b);		\
	(itvtx)->A = (a);


// color combination macro ----------------------------------------------------
//
#define COLOR_MUL(t,a,b)	{ \
	int tmp; \
	tmp = ( (int)(a) * (int)(b) ) / 255; \
	if ( tmp > 255 ) \
		tmp = 255; \
	(t) = tmp; \
}


// draw shock wave ------------------------------------------------------------
//
PRIVATE
int ShockWaveDraw( void* param )
{
	ASSERT( param != NULL );
	ShockWave *shockwave = (ShockWave *) param;

	ASSERT( shockwave->texmap != NULL );

	// setup transformation matrix (shockwaves are defined
	// in world-space, so transform is world->view)
	D_LoadIterMatrix( ViewCamera );

	// set vertex color
	byte red   = shockwave->red;
	byte green = shockwave->green;
	byte blue  = shockwave->blue;
	byte alpha = shockwave->alpha;

    int remaining = ( shockwave_lifetime + shockwave->delay - shockwave->alive );
    if ( remaining <= shockwave_fadeout ) {
        alpha = shockwave->alpha - ( ( shockwave->alpha * ( shockwave_fadeout - remaining ) ) / shockwave_fadeout );
    }

    IterRectangle3 itrect;
	itrect.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW | ITERFLAG_Z_TO_DEPTH;
	itrect.itertype  = iter_texrgba | iter_specularadd;
	itrect.raststate = rast_zcompare | rast_texwrap | rast_chromakeyoff;
	itrect.rastmask  = rast_nomask;
	itrect.texmap	 = shockwave->texmap;

	geomv_t tex_u[ 4 ];
	geomv_t tex_v[ 4 ];
	geomv_t tex_w = INT_TO_GEOMV( 1L << shockwave->texmap->Width );
	geomv_t tex_h = INT_TO_GEOMV( 1L << shockwave->texmap->Height );

	// set texture coordinates
	tex_u[ 0 ] = GEOMV_0;
	tex_u[ 1 ] = tex_w;
	tex_u[ 2 ] = tex_w;
	tex_u[ 3 ] = GEOMV_0;

	tex_v[ 0 ] = GEOMV_0;
	tex_v[ 1 ] = GEOMV_0;
	tex_v[ 2 ] = tex_h;
	tex_v[ 3 ] = tex_h;

	COLOR_MUL( red,   red,   alpha );
	COLOR_MUL( green, green, alpha );
	COLOR_MUL( blue,  blue,  alpha );

	for ( int curvtx = 0; curvtx < 4; curvtx++ ) {
		SET_ITER_VTX( &itrect.Vtxs[ curvtx ], &shockwave->ViewVtxs[ curvtx ],
					  tex_u[ curvtx ], tex_v[ curvtx ],
					  red, green, blue, 255 );
	}

	// draw quad
	D_DrawIterRectangle3( &itrect, 0x3f );

	// restore identity transformation
	D_LoadIterMatrix( NULL );

	return TRUE;
}


// callback type and flags ----------------------------------------------------
//
static int callback_type = CBTYPE_DRAW_CUSTOM_ITER | CBFLAG_REMOVE;


// shock wave animation callback ----------------------------------------------
//
PRIVATE
int ShockWaveAnimate( CustomObject *base )
{
	ASSERT( base != NULL );
	ShockWave *shockwave = (ShockWave *) base;

	shockwave->alive += CurScreenRefFrames;
	if ( shockwave->alive >= ( shockwave_lifetime + shockwave->delay ) ) {

		// returning FALSE deletes the object
		return FALSE;
	}

	// check shock wave delay
	if ( shockwave->alive < shockwave->delay ) {
		return TRUE;
	}

	// register the drawing callback for drawing the shockwave
	CALLBACK_RegisterCallback( callback_type, ShockWaveDraw, (void*) base );

	// calculate vertices
	if ( !shockwave->vtxsvalid ) {

    	Xmatrx    ViewXmatrx, invViewCamera;
		Vertex3   Pos, viewPos;

		// get object position vector
		FetchTVector( shockwave->owner->ObjPosition, &Pos );
		MtxVctMUL( ViewCamera, &Pos, &viewPos );

		MakeIdMatrx( ViewXmatrx );

		ViewXmatrx[ 0 ][ 3 ] = viewPos.X;
		ViewXmatrx[ 1 ][ 3 ] = viewPos.Y;
		ViewXmatrx[ 2 ][ 3 ] = viewPos.Z;

		bams_t pitch = RAND() % BAMS_DEG360;
		bams_t yaw   = SHOCKWAVE_YAW_LOW + ( RAND() % ( SHOCKWAVE_YAW_HIGH - SHOCKWAVE_YAW_LOW ) );
		bams_t roll  = RAND() % BAMS_DEG360;

		// rotate shockwave
		ObjRotZ( ViewXmatrx, roll );
		ObjRotY( ViewXmatrx, yaw );
		ObjRotX( ViewXmatrx, pitch );

		CalcOrthoInverse( ViewCamera, DestXmatrx );
		memcpy( invViewCamera, DestXmatrx, sizeof( Xmatrx ) );

		//	calculate worldspace matrix
		MtxMtxMUL( invViewCamera, ViewXmatrx, DestXmatrx );
		memcpy( shockwave->WorldXmatrx, DestXmatrx, sizeof( Xmatrx ) );

		// avoid recalculation
		shockwave->vtxsvalid = TRUE;
	}

	Vertex3   InstVtxs[ 4 ];

	ASSERT( ( shockwave->alive - shockwave->delay ) < shockwave_lifetime );

	int curframe = (int)(expansion_tab[ shockwave->alive - shockwave->delay ] * ( shockwave_texframes - 1 ));

	ASSERT( shockwave_texmaps != NULL );
	shockwave->texmap = shockwave_texmaps[ curframe ];

	// scale shock wave
	geomv_t sc = GEOMV_MUL( shockwave_max_width, FLOAT_TO_GEOMV( expansion_tab[ shockwave->alive - shockwave->delay ] ) );

	for ( int curvtx = 0; curvtx < 4; curvtx++ ) {
		InstVtxs[ curvtx ].X = GEOMV_MUL( sc, ObjVtxs[ curvtx ].X );
		InstVtxs[ curvtx ].Y = GEOMV_MUL( sc, ObjVtxs[ curvtx ].Y );
		InstVtxs[ curvtx ].Z = GEOMV_MUL( sc, ObjVtxs[ curvtx ].Z );

		// transform vtxs into worldspace
		MtxVctMUL( shockwave->WorldXmatrx, &InstVtxs[ curvtx ],
				   &shockwave->ViewVtxs[ curvtx ] );
	}

	return TRUE;
}


// create shockwave object ----------------------------------------------------
//
PRIVATE
void CreateShockWave( GenObject *owner, int delay )
{
	ASSERT( owner != NULL );

	// check if textures are loaded
	if ( shockwave_texmaps == NULL ) {

		extern const char *expwave1_texnames[];
		extern int expwave1_texframes;

		shockwave_texframes = expwave1_texframes;
		shockwave_texmaps   = (TextureMap **)
			ALLOCMEM( sizeof( TextureMap* ) * shockwave_texframes );

		//NOTE:
		// the shockwave texture table will never be
		// freed once entirely valid. thus, the
		// time-consuming string lookups are only
		// performed once. (if all textures are valid.)

		for ( int curtex = 0; curtex < shockwave_texframes; curtex++ ) {

			// try to fetch texture map by name
			TextureMap *texmap = FetchTextureMap( expwave1_texnames[ curtex ] );
			if ( texmap == NULL ) {

				FREEMEM( shockwave_texmaps );
				shockwave_texmaps = NULL;

				MSGOUT( "shockwave texture not found: %s.",
						expwave1_texnames[ curtex ] );
				return;
			}

			shockwave_texmaps[ curtex ] = texmap;
		}
	}

	// create shockwave object
	ShockWave *shockwave = (ShockWave *) CreateVirtualObject( shockwave_type_id );
	ASSERT( shockwave != NULL );

	shockwave->owner	 = owner;
	shockwave->vtxsvalid = FALSE;
	shockwave->alive	 = 0;
	shockwave->delay	 = delay;
}


// template for default values of fields; may be altered from the console -----
//
static ShockWave *shockwave_type_template = NULL;


// init type fields with default values ---------------------------------------
//
PRIVATE
void ShockWaveInitDefaults( ShockWave *shockwave )
{
	ASSERT( shockwave != NULL );

	shockwave->delay = SHOCKWAVE_DELAY;
	shockwave->red   = SHOCKWAVE_RED;
	shockwave->green = SHOCKWAVE_GREEN;
	shockwave->blue  = SHOCKWAVE_BLUE;
	shockwave->alpha = SHOCKWAVE_ALPHA;
}


// type fields init function for shockwave ------------------------------------
//
PRIVATE
void ShockWaveInitType( CustomObject *base )
{
	ASSERT( base != NULL );
	ShockWave *shockwave = (ShockWave *) base;

	// init either from template or default values
	if ( !OBJ_InitFromCustomTypeTemplate( shockwave, shockwave_type_template ) ) {
		ShockWaveInitDefaults( shockwave );
	}
}


// shock wave constructor (class instantiation) -------------------------------
//
PRIVATE
void ShockWaveInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	ShockWave *shockwave = (ShockWave *) base;

	// no dynamic mem
}


// shock wave destructor (instance destruction) -------------------------------
//
PRIVATE
void ShockWaveDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	ShockWave *shockwave = (ShockWave *) base;

	// ensure pending callbacks are destroyed to avoid
	// calling them with invalid pointers
	int numremoved = CALLBACK_DestroyCallback( callback_type, (void *) base );
	ASSERT( numremoved <= 1 );
}


// register object type for shock wave ----------------------------------------
//
PRIVATE
void ShockWaveRegisterCustomType()
{
	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	// always try to allocate template
	shockwave_type_template = (ShockWave *) ALLOCMEM( sizeof( ShockWave ) );
	if ( shockwave_type_template != NULL ) {
		memset( shockwave_type_template, 0, sizeof( ShockWave ) );
		ShockWaveInitDefaults( shockwave_type_template );
	}

	info.type_name			= "shockwave";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( ShockWave );
	info.type_template		= shockwave_type_template;
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= ShockWaveInitType;
	info.callback_instant	= ShockWaveInstantiate;
	info.callback_destroy	= ShockWaveDestroy;
	info.callback_animate	= ShockWaveAnimate;
	info.callback_collide	= NULL;
	info.callback_notify	= NULL;
	info.callback_persist   = NULL;

	shockwave_type_id = OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, ShockWave_PropList );
}


// precalculate expansion table for given parameters --------------------------
//
PRIVATE
void InitExpansionTable( int lifetime, geomv_t max_width, float lambda )
{
	ASSERT( ( lifetime >= SHOCKWAVE_MIN_LIFETIME ) && ( lifetime <= SHOCKWAVE_MAX_LIFETIME ) );
	ASSERT( max_width > GEOMV_0 );
	ASSERT( lambda > 0.0 );

	if ( expansion_tab != NULL ) {
		FREEMEM( expansion_tab );
		expansion_tab = NULL;
	}

	expansion_tab = (float *) ALLOCMEM( ( lifetime ) * sizeof( float ) );
	if ( expansion_tab == NULL )
		OUTOFMEM( 0 );

	shockwave_lifetime = lifetime;
	shockwave_max_width = max_width;
	shockwave_lambda = lambda;

	// precalc full expansion curve in time resolution
	// exactly as needed for shock wave expansion
	float ref_factor = 1.0 / ( 1.0 - exp( -lambda ) );

	for ( int i = 0; i < lifetime; i++ ) {
		expansion_tab[ i ] = ( 1.0 - exp( -lambda * ( (float) i / (float) lifetime) ) ) * ref_factor;
	}
}


// key table for shockwave command --------------------------------------------
//
key_value_s shockwave_key_value[] = {

	{ "lifetime",	NULL,	KEYVALFLAG_NONE				},
	{ "maxwidth",	NULL,	KEYVALFLAG_NONE				},
	{ "lambda",		NULL,	KEYVALFLAG_NONE				},
    { "fadeout",    NULL,   KEYVALFLAG_NONE             },

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_SHOCKWAVE_LIFETIME,
	KEY_SHOCKWAVE_MAXWIDTH,
	KEY_SHOCKWAVE_LAMBDA,
    KEY_SHOCKWAVE_FADEOUT,
};


// customize shock wave -------------------------------------------------------
//
PRIVATE
int Cmd_SHOCKWAVE( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// shockwave_command	::= 'shockwave' [<lifetime_spec>] [<maxwidth_spec>]
	//							[<lambda_spec>] [<fadeout_spec>]
	// lifetime_spec		::= 'lifetime' <int>
	// maxwidth_spec		::= 'maxwidth' <float>
	// lambda_spec			::= 'lambda' <float>
	// fadeout_spec			::= 'fadeout' <int>

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( shockwave_key_value, paramstr ) )
		return TRUE;

	int alldefaults  = TRUE;
	int		lifetime	= shockwave_lifetime;
	float max_width	= GEOMV_TO_FLOAT( shockwave_max_width );
	float lambda		= shockwave_lambda;
    int     fadeout     = shockwave_fadeout;

	if ( shockwave_key_value[ KEY_SHOCKWAVE_LIFETIME ].value != NULL ) {

		if ( ScanKeyValueInt( &shockwave_key_value[ KEY_SHOCKWAVE_LIFETIME ], &lifetime ) < 0 ) {
			CON_AddLine( shockwave_inval_lifetime_spec );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	if ( ( lifetime < SHOCKWAVE_MIN_LIFETIME ) || ( lifetime > SHOCKWAVE_MAX_LIFETIME ) ||
		( lifetime < fadeout ) ) {
		CON_AddLine( shockwave_inval_lifetime_spec );
		return TRUE;
	}

	if ( shockwave_key_value[ KEY_SHOCKWAVE_MAXWIDTH ].value != NULL ) {

		if ( ScanKeyValueFloat( &shockwave_key_value[ KEY_SHOCKWAVE_MAXWIDTH ], &max_width ) < 0 ) {
			CON_AddLine( shockwave_inval_maxwidth_spec );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	if ( max_width <= 0.0 ) {
		CON_AddLine( shockwave_inval_maxwidth_spec );
		return TRUE;
	}

	if ( shockwave_key_value[ KEY_SHOCKWAVE_LAMBDA ].value != NULL ) {

		if ( ScanKeyValueFloat( &shockwave_key_value[ KEY_SHOCKWAVE_LAMBDA ], &lambda ) < 0 ) {
			CON_AddLine( shockwave_inval_lambda_spec );
			return TRUE;
		}
		alldefaults = FALSE;
	}

	if ( lambda <= 0.0 ) {
		CON_AddLine( shockwave_inval_maxwidth_spec );
		return TRUE;
	}

    if ( shockwave_key_value[ KEY_SHOCKWAVE_FADEOUT ].value != NULL ) {

        if ( ScanKeyValueInt( &shockwave_key_value[ KEY_SHOCKWAVE_FADEOUT ], &fadeout ) < 0 ) {
            CON_AddLine( shockwave_inval_fadeout_spec );
        }
        if ( ( fadeout > lifetime ) || ( fadeout < 0 ) ) {
            CON_AddLine( shockwave_inval_fadeout_spec );
        }
        else {
            shockwave_fadeout = fadeout;
        }
        return TRUE;
    }

    if ( !alldefaults ) {
		InitExpansionTable( lifetime, FLOAT_TO_GEOMV( max_width ), lambda );
	}
	else {
		sprintf( paste_str, "lifetime: %d (%d)", lifetime, SHOCKWAVE_LIFETIME );
		CON_AddLine( paste_str );
		sprintf( paste_str, "maxwidth: %f (%f)", max_width, SHOCKWAVE_MAX_WIDTH );
		CON_AddLine( paste_str );
		sprintf( paste_str, "lambda: %f (%f)", lambda, SHOCKWAVE_LAMBDA );
		CON_AddLine( paste_str );
		sprintf( paste_str, "fadeout: %f (%f)", (double)fadeout, (double)SHOCKWAVE_FADEOUT );
		CON_AddLine( paste_str );
	}

	return TRUE;
}


// create shockwave objects on first frame of explosion -----------------------
//
void ExplosionShockWave( const ShipObject *shippo )
{
	ASSERT( shippo != NULL );
	ASSERT( AUX_EXPLOSION_DRAW_SHOCKWAVE > 0 );

	if ( shippo->ExplosionCount == MAX_EXPLOSION_COUNT ) {

		CreateShockWave( (GenObject *) shippo, SHOCKWAVE_DELAY );

		if ( AUX_EXPLOSION_DRAW_SHOCKWAVE > 1 ) {
			if ( RAND() % 100  < 50 ) {

				CreateShockWave( (GenObject *) shippo, SHOCKWAVE_DELAY + ( RAND() % 600 ) );

				if ( AUX_EXPLOSION_DRAW_SHOCKWAVE > 2 ) {
					if ( RAND() % 100  < 10 ) {

						CreateShockWave( (GenObject *) shippo, SHOCKWAVE_DELAY + 600 + ( RAND() % 600 ) );
					}
				}
			}
		}
	}
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_SHKWAV )
{
	// register type
	ShockWaveRegisterCustomType();

	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "shockwave" command
	regcom.command	 = "shockwave";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_SHOCKWAVE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// init default expansion table
	InitExpansionTable( SHOCKWAVE_LIFETIME, SHOCKWAVE_MAX_WIDTH, SHOCKWAVE_LAMBDA );
}



