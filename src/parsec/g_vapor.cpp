/*
 * PARSEC - Vapor Trails
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Michael Woegerbauer <maiki@parsec.org> 1999
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// drawing subsystem
#include "d_iter.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_vapor.h"

// proprietary module headers
#include "con_aux.h"
#include "con_info.h"
#include "e_callbk.h"
#include "e_supp.h"
#include "obj_clas.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_cust.h"


// flags
#define USE_TRIANGLE_STRIPS



// preset vaportrail type property values -------------------------------------
//
#define VAPORTRAIL_LIFETIME		1200
#define VAPORTRAIL_WIDTH		3.0f
#define VAPORTRAIL_DELTA_X		0.0f
#define VAPORTRAIL_DELTA_Y		0.0f
#define VAPORTRAIL_DELTA_Z		-9.0f
#define VAPORTRAIL_ROT			BAMS_DEG0
#define VAPORTRAIL_RED			49
#define VAPORTRAIL_GREEN		105
#define VAPORTRAIL_BLUE			245
#define VAPORTRAIL_ALPHA		255

#define ROCKET_TRAIL_TEXNAME	"rockettr.3df"
#define MAX_TEX_NAME			128

#define MAX_SEGMENTS			160


// vapor trail custom type structure ------------------------------------------
//
struct VaporTrail : CustomObject {

	GenObject*	OwnerObj;		// needed for animation calculation
	int 		lifetime;
	int			max_segments;
	char		texname[ MAX_TEX_NAME + 1 ];
	TextureMap*	texmap;
	geomv_t		width;			// width of trail
	geomv_t		half_width;		// half of width of trail
	geomv_t		delta_x;		// relative position to owner object
	geomv_t		delta_y;
	geomv_t		delta_z;
	bams_t		rot;
	int			red;
	int			green;
	int			blue;
	int			alpha;
	Vertex3*	Trail_R;		// world coordinates of trail
	Vertex3*	Trail_L;
	refframe_t*	alive;			// refframes alive for each segment of trail
	int			startidx;		// index of first vertex pair to be shown
	int			length;
};

#define OFS_LIFETIME		offsetof( VaporTrail, lifetime )
#define OFS_MAX_SEGMENTS	offsetof( VaporTrail, max_segments )
#define OFS_WIDTH			offsetof( VaporTrail, width )
#define OFS_TEXNAME			offsetof( VaporTrail, texname )
#define OFS_DELTA_X			offsetof( VaporTrail, delta_x )
#define OFS_DELTA_Y			offsetof( VaporTrail, delta_y )
#define OFS_DELTA_Z			offsetof( VaporTrail, delta_z )
#define OFS_ROT				offsetof( VaporTrail, rot )
#define OFS_RED				offsetof( VaporTrail, red )
#define OFS_GREEN			offsetof( VaporTrail, green )
#define OFS_BLUE			offsetof( VaporTrail, blue )
#define OFS_ALPHA			offsetof( VaporTrail, alpha )


// list of console-accessible properties --------------------------------------
//
PRIVATE
proplist_s VaporTrail_PropList[] = {

	{ "lifetime",	OFS_LIFETIME,		60,		0xffff,		PROPTYPE_INT	},
	{ "maxsegments",OFS_MAX_SEGMENTS,	10,		0xff,		PROPTYPE_INT	},
	{ "texname",	OFS_TEXNAME,		0,	MAX_TEX_NAME,   PROPTYPE_STRING	},
	{ "width",		OFS_WIDTH,	 -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "delta.x",	OFS_DELTA_X, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "delta.y",	OFS_DELTA_Y, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "delta.z",	OFS_DELTA_Z, -0x7fffffff,	0x7fffffff,	PROPTYPE_GEOMV	},
	{ "rot",		OFS_ROT,			0,		0xffff,		PROPTYPE_INT	},
	{ "red",		OFS_RED,			0,		0xff,		PROPTYPE_INT	},
	{ "green",		OFS_GREEN,			0,		0xff,		PROPTYPE_INT	},
	{ "blue",		OFS_BLUE,			0,		0xff,		PROPTYPE_INT	},
	{ "alpha",		OFS_ALPHA,			0,		0xff,		PROPTYPE_INT	},

	{ NULL,			0,				0,		0,			0				}
};


// assigned type id for vapor type --------------------------------------------
//
static dword vapor_type_id;

extern int headless_bot;

// macro to set the properties of an itervertex -------------------------------
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


// draw vapor trail -----------------------------------------------------------
//
PRIVATE
int VaporTrail_Draw( void* param )
{
	if(headless_bot)
		return TRUE;
	
	ASSERT( param != NULL );
	VaporTrail *vapor = (VaporTrail *) param;

	// avoid overhead for empty trails
	if ( vapor->length < 1 )
		return TRUE;

	ASSERT( vapor->texmap != NULL );

	// setup transformation matrix (trails are defined in
	// world-space, so transform is world->view)
	D_LoadIterMatrix( ViewCamera );

	// fetch vertex color
	byte red   = vapor->red;
	byte green = vapor->green;
	byte blue  = vapor->blue;
	byte alpha = vapor->alpha;

#ifdef USE_TRIANGLE_STRIPS

	int numverts = vapor->length * 2 + 2;

	IterTriStrip3 *itstrip = (IterTriStrip3 *)
		ALLOCMEM( (size_t)&((IterTriStrip3*)0)->Vtxs[ numverts ] );

	itstrip->flags		= ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW |
						  ITERFLAG_Z_TO_DEPTH;
	itstrip->NumVerts	= numverts;
	itstrip->itertype	= iter_rgbatexa | iter_alphablend;
	itstrip->raststate	= rast_zcompare | rast_texwrap | rast_chromakeyoff;
	itstrip->rastmask	= rast_nomask;
	itstrip->texmap		= vapor->texmap;

	geomv_t vtx_u;
	geomv_t vtx_v;
	geomv_t tex_w = INT_TO_GEOMV( 1 << vapor->texmap->Width );
	geomv_t tex_h = INT_TO_GEOMV( 1 << vapor->texmap->Height );

	float decaydelta = (float)alpha / (float)vapor->lifetime;

	int curseg = vapor->startidx;
	for ( int curvtx = 0; curvtx < numverts; curvtx+=2 ) {

		byte decay = (int)(decaydelta * (float)vapor->alive[ curseg ]);

		// texture is used back to back, non-mirrored for
		// even quads, mirrored for odd quads
		vtx_u = GEOMV_0;
		vtx_v = ( curvtx & 0x02 ) ? tex_h : GEOMV_0;
		SET_ITER_VTX( &itstrip->Vtxs[ curvtx + 0 ], &vapor->Trail_L[ curseg ],
					  vtx_u, vtx_v, red, green, blue, alpha - decay );

		vtx_u = tex_w;
		SET_ITER_VTX( &itstrip->Vtxs[ curvtx + 1 ], &vapor->Trail_R[ curseg ],
					  vtx_u, vtx_v, red, green, blue, alpha - decay );

		// wrap around index of segments
		if ( ++curseg >= vapor->max_segments ) {
			curseg -= vapor->max_segments;
		}
	}

	// draw entire strip
	D_DrawIterTriStrip3( itstrip, 0x3f );

	FREEMEM( itstrip );

#else

	IterTriangle3 ittriangle;
	ittriangle.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW |
						   ITERFLAG_Z_TO_DEPTH | ITERFLAG_NONDESTRUCTIVE;
	ittriangle.itertype  = iter_rgbatexa | iter_alphablend;
	ittriangle.raststate = rast_zcompare | rast_texwrap | rast_chromakeyoff;
	ittriangle.rastmask  = rast_nomask;
	ittriangle.texmap	 = vapor->texmap;

	float decaydelta = (float)alpha / (float)vapor->lifetime;
	byte decay = decaydelta * (float)vapor->alive[ vapor->startidx ];

	geomv_t tex_w = INT_TO_GEOMV( 1L << vapor->texmap->Width );
	geomv_t tex_h = INT_TO_GEOMV( 1L << vapor->texmap->Height );

	geomv_t vtx_u = GEOMV_0;
	geomv_t vtx_v = GEOMV_0;
	SET_ITER_VTX( &ittriangle.Vtxs[ 0 ], &vapor->Trail_L[ vapor->startidx ],
				  vtx_u, vtx_v, red, green, blue, alpha - decay );

	vtx_u = tex_w;
	SET_ITER_VTX( &ittriangle.Vtxs[ 1 ], &vapor->Trail_R[ vapor->startidx ],
				  vtx_u, vtx_v, red, green, blue, alpha - decay );

	int curvtx = 2;
	int curseg = vapor->startidx;

	for ( int curquad = 0; curquad < vapor->length; curquad++ ) {

		// wrap around index of segments
		if ( ++curseg >= vapor->max_segments )
			curseg -= vapor->max_segments;

		decay = decaydelta * (float)vapor->alive[ curseg ];

		vtx_u = GEOMV_0;
		vtx_v = ( curseg & 0x01 ) ? tex_h : GEOMV_0;
		SET_ITER_VTX( &ittriangle.Vtxs[ curvtx ], &vapor->Trail_L[ curseg ],
					  vtx_u, vtx_v, red, green, blue, alpha - decay );
		if ( ++curvtx >= 3 )
			curvtx -= 3;

		// draw even triangle
		D_DrawIterTriangle3( &ittriangle, 0x3f );

		vtx_u = tex_w;
		SET_ITER_VTX( &ittriangle.Vtxs[ curvtx ], &vapor->Trail_R[ curseg ],
					  vtx_u, vtx_v, red, green, blue, alpha - decay );
		if ( ++curvtx >= 3 )
			curvtx -= 3;

		// draw odd triangle
		D_DrawIterTriangle3( &ittriangle, 0x3f );
	}

#endif // USE_TRIANGLE_STRIPS

	// restore identity transformation
	D_LoadIterMatrix( NULL );

	return TRUE;
}


// callback type and flags ----------------------------------------------------
//
static int callback_type = CBTYPE_DRAW_CUSTOM_ITER | CBFLAG_REMOVE;


// vapor animation callback ---------------------------------------------------
//
PRIVATE
int VaporTrailAnimate( CustomObject *base )
{
	ASSERT( base != NULL );
	VaporTrail *vapor = (VaporTrail *) base;

	// remove vapor trail if no texture found
	if ( vapor->texmap == NULL ) {

		// returning FALSE deletes the object
		return FALSE;
	}

	// fetch owner object
	GenObject *ownerpo = vapor->OwnerObj;

	// kill vapor trail when owner doesn't exist anymore and trail vanished
	if ( ( ownerpo == NULL ) && ( vapor->length < 1 ) ) {

		// returning FALSE deletes the object
		return FALSE;
	}

	// register the drawing callback for drawing the vapor trail
	CALLBACK_RegisterCallback( callback_type, VaporTrail_Draw, (void*) base );

	// update alive counters and shorten trail accordingly
	int curseg   = vapor->startidx;
	int numpairs = vapor->length + 1;
	for ( int paircnt = numpairs; paircnt > 0; paircnt-- ) {

		vapor->alive[ curseg ] += CurScreenRefFrames;
		if ( vapor->alive[ curseg ] > vapor->lifetime ) {

			vapor->startidx = curseg + 1;
			if ( vapor->startidx >= vapor->max_segments )
				vapor->startidx -= vapor->max_segments;

			// delete seg (length may become -1 here,
			// because length==numpairs-1)
			--vapor->length;
		}

		// wrap around index of segments
		if ( ++curseg >= vapor->max_segments )
			curseg -= vapor->max_segments;
	}

	// create new segments as long as owner exists
	if ( ownerpo != NULL ) {

		// check if maximum length reached
		if ( curseg == vapor->startidx ) {

			// delete oldest seg
			++vapor->startidx;
			if ( vapor->startidx >= vapor->max_segments )
				vapor->startidx -= vapor->max_segments;
			--vapor->length;
		}

		// attach next segment
		vapor->alive[ curseg ] = 0;
		++vapor->length;

		Xmatrx trailmatrx;
		MakeIdMatrx( trailmatrx );

		// rotate
		ObjRotZ( trailmatrx, vapor->rot );
		MtxMtxMUL( ownerpo->ObjPosition, trailmatrx, DestXmatrx );

		// get next pair of vertices
		Vertex3	deltavtxs;

		// right vertex
		deltavtxs.X = vapor->delta_x + vapor->half_width;
		deltavtxs.Y = vapor->delta_y;
		deltavtxs.Z = vapor->delta_z;
		MtxVctMUL( DestXmatrx, &deltavtxs, &vapor->Trail_R[ curseg ] );

		// left vertex
		deltavtxs.X = vapor->delta_x - vapor->half_width;
		MtxVctMUL( DestXmatrx, &deltavtxs, &vapor->Trail_L[ curseg ] );
	}

	return TRUE;
}


// create and attach missile vapor trail to object ----------------------------
//
PUBLIC
void CreateVaporTrail( GenObject *ownerpo )
{
	ASSERT( ownerpo != NULL );

	// object type determines whether a trail should be created
	switch ( ownerpo->ObjectType & TYPECONTROLMASK ) {

		case TYPEMISSILEISSTANDARD:
			if ( ( AUX_ENABLE_MISSILE_POLYGON_TRAILS & 0x01 ) == 0 )
				return;
			break;

		case TYPEMISSILEISHOMING:
			if ( ( AUX_ENABLE_MISSILE_POLYGON_TRAILS & 0x02 ) == 0 )
				return;
			break;

		default:
			MSGOUT( "CreateVaporTrail(): called for invalid type: %0x.", ownerpo->ObjectType );
			return;
	}

	// create vapor trail virtual custom object
	VaporTrail *vapor = (VaporTrail *) CreateVirtualObject( vapor_type_id );
	ASSERT( vapor != NULL );

	// remember owner object
	vapor->OwnerObj = ownerpo;

	// register custom object with owner to get notified of owner deletion
	OBJ_RegisterNotifyCustomObject( ownerpo, vapor );

	//NOTE:
	// the vapor trail will be deleted in the animation
	// callback if the texture cannot be found. for this,
	// the object could also not be created at all, but
	// we would have to know the texture name before
	// creating the object. no problem, it's either
	// vapor_type_template->texname or ROCKET_TRAIL_TEXNAME.
	// but then again, this knowledge would be in the code twice.

	// get pointer to texture map
	vapor->texmap = FetchTextureMap( vapor->texname );
	if ( vapor->texmap == NULL ) {
		MSGOUT( "vaportrail texture '%s' invalid.", vapor->texname );
		// vapor trail will be deleted in animation callback
		return;
	}

    Xmatrx trailmatrx;
	MakeIdMatrx( trailmatrx );

	// rotate trail
	ObjRotZ( trailmatrx, vapor->rot );
	MtxMtxMUL( ownerpo->ObjPosition, trailmatrx, DestXmatrx );

	vapor->startidx		= 0;
	vapor->alive[ 0 ]	= 0;
	vapor->length		= 0;
	vapor->half_width	= GEOMV_MUL( vapor->width, FLOAT_TO_GEOMV( 0.5 ) );

	// first pair of vertices
	Vertex3	deltavtxs;

	// right vertex
	deltavtxs.X = vapor->delta_x + vapor->half_width;
	deltavtxs.Y = vapor->delta_y;
	deltavtxs.Z = vapor->delta_z;
	MtxVctMUL( DestXmatrx, &deltavtxs, &vapor->Trail_R[ 0 ] );

	// left vertex
	deltavtxs.X = vapor->delta_x - vapor->half_width;
	MtxVctMUL( DestXmatrx, &deltavtxs, &vapor->Trail_L[ 0 ] );
}


// template for default values of fields; may be altered from the console -----
//
static VaporTrail *vapor_type_template = NULL;


// init type fields with default values ---------------------------------------
//
PRIVATE
void VaporTrailInitDefaults( VaporTrail *vapor )
{
	ASSERT( vapor != NULL );

	vapor->lifetime		= VAPORTRAIL_LIFETIME;
	vapor->max_segments	= MAX_SEGMENTS;
	vapor->width		= FLOAT_TO_GEOMV( VAPORTRAIL_WIDTH );

	strncpy( vapor->texname, ROCKET_TRAIL_TEXNAME, MAX_TEX_NAME );
	vapor->texname[ MAX_TEX_NAME ] = 0;

	vapor->delta_x	= FLOAT_TO_GEOMV( VAPORTRAIL_DELTA_X );
	vapor->delta_y	= FLOAT_TO_GEOMV( VAPORTRAIL_DELTA_Y );
	vapor->delta_z	= FLOAT_TO_GEOMV( VAPORTRAIL_DELTA_Z );
	vapor->rot		= VAPORTRAIL_ROT;
	vapor->red		= VAPORTRAIL_RED;
	vapor->green	= VAPORTRAIL_GREEN;
	vapor->blue		= VAPORTRAIL_BLUE;
	vapor->alpha	= VAPORTRAIL_ALPHA;
}


// type fields init function for vapor trail ----------------------------------
//
PRIVATE
void VaporTrailInitType( CustomObject *base )
{
	ASSERT( base != NULL );
	VaporTrail *vapor = (VaporTrail *) base;

	// init either from template or default values
	if ( !OBJ_InitFromCustomTypeTemplate( vapor, vapor_type_template ) ) {
		VaporTrailInitDefaults( vapor );
	}
}


// vapor trail constructor (class instantiation) ------------------------------
//
PRIVATE
void VaporTrailInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	VaporTrail *vapor = (VaporTrail *) base;

	// allocate memory for segments in one block
	char *segmem = (char *) ALLOCMEM( ( sizeof( Vertex3 ) * 2 + sizeof( refframe_t ) ) * vapor->max_segments );
	if ( segmem == NULL )
		OUTOFMEM( "no mem for vapor trail." );
	vapor->Trail_R	= (Vertex3 *) ( segmem );
	vapor->Trail_L	= (Vertex3 *) ( segmem + sizeof( Vertex3 ) * vapor->max_segments );
	vapor->alive	= (refframe_t *) ( segmem + sizeof( Vertex3 ) * vapor->max_segments * 2 );
}


// vapor trail destructor (instance destruction) ------------------------------
//
PRIVATE
void VaporTrailDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	VaporTrail *vapor = (VaporTrail *) base;

	// free segment memory
	FREEMEM( vapor->Trail_R );
	vapor->Trail_R	= NULL;
	vapor->Trail_L	= NULL;
	vapor->alive	= NULL;

	// unregister from owner object if it still exists
	if ( vapor->OwnerObj != NULL ) {
		int unreg = OBJ_UnregisterNotifyCustomObject( vapor->OwnerObj, vapor );
		ASSERT( unreg == TRUE );
	}

	// ensure pending callbacks are destroyed to avoid
	// calling them with invalid pointers
	int numremoved = CALLBACK_DestroyCallback( callback_type, (void *) base );
	if(!headless_bot)
	   ASSERT( numremoved <= 1 );
}


// ----------------------------------------------------------------------------
//
PRIVATE
void VaporTrailNotify( CustomObject *base, GenObject *genobj, int event )
{
	ASSERT( base != NULL );
	ASSERT( genobj != NULL );
	ASSERT( event == CUSTOM_NOTIFY_GENOBJECT_DELETE );

	VaporTrail *vapor = (VaporTrail *) base;

	// delete reference to owner
	vapor->OwnerObj = NULL;
}


// register object type for vapor trail ---------------------------------------
//
PRIVATE
void VaporTrailRegisterCustomType()
{
	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	// always try to allocate template
	vapor_type_template = (VaporTrail *) ALLOCMEM( sizeof( VaporTrail ) );
	if ( vapor_type_template != NULL ) {
		memset( vapor_type_template, 0, sizeof( VaporTrail ) );
		VaporTrailInitDefaults( vapor_type_template );
	}

	info.type_name			= "vaportrail";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( VaporTrail );
	info.type_template		= vapor_type_template;
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= VaporTrailInitType;
	info.callback_instant	= VaporTrailInstantiate;
	info.callback_destroy	= VaporTrailDestroy;
	info.callback_animate	= VaporTrailAnimate;
	info.callback_collide	= NULL;
	info.callback_notify	= VaporTrailNotify;
	info.callback_persist   = NULL;

	vapor_type_id = OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, VaporTrail_PropList );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_VAPOR )
{
	// register object type
	VaporTrailRegisterCustomType();
}



