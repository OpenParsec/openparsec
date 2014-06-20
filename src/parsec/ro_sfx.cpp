/*
 * PARSEC - Graphical Special Effects
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:40 $
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
#include "vid_defs.h"

// drawing subsystem
#include "d_iter.h"

// rendering subsystem
#include "r_obj.h"
#include "r_sfx.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "ro_sfx.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_level.h"
#include "g_camera.h"
#include "obj_clas.h"
#include "ro_api.h"
#include "ro_supp.h"



// star number of sun
//#define SUN_STAR_NO				52 //63

// star clipping z's
static geomv_t near_clip_z		= FLOAT_TO_GEOMV( 10.0 );
static geomv_t far_clip_z		= FLOAT_TO_GEOMV( 800.0 );

// depth query 'name' and result for sun visibility test
static GLuint flareDepthQuery = 0;
static GLuint flareDepthQueryResult = 0;


// draw single lens flare circle ----------------------------------------------
//
PRIVATE
void RO_FlareCircle( sgrid_t putx, sgrid_t puty, dword width, dword height, colrgba_s flarecol, float alpha )
{
	char *flaremap = BitmapInfo[ BM_LENSFLARE1 ].bitmappointer;
	int bm_width   = BitmapInfo[ BM_LENSFLARE1 ].width;
	int bm_height  = BitmapInfo[ BM_LENSFLARE1 ].height;

	// hard-coded for specific texture
	ASSERT( bm_width  == 64 );
	ASSERT( bm_height == 64 );
	
//	float scale = Screen_Height / 640.0f;

	// this is necessary if we want to avoid alpha-only textures
	dword format = ( ColorSetupFlags & COLORSETUP_ALPHA_8_TO_RGBA_8888 ) ?
					TEXFMT_RGBA_8888 : TEXFMT_ALPHA_8;

	GLTexInfo texinfo;
	texinfo.texmap	 = NULL;
	texinfo.data	 = flaremap;
	texinfo.width	 = bm_width;
	texinfo.height	 = bm_height;
	texinfo.format	 = format;
//	texinfo.coscale  = 1/64.0f;
//	texinfo.aratio	 = 1.0f;
	texinfo.lodsmall = TEXLOD_64;
	texinfo.lodlarge = TEXLOD_64;

	RO_SelectTexelSource( &texinfo );

	flarecol.A = 20 * alpha;
	RO_Render2DRectangle( putx, puty,
						  /*bm_width * texinfo.coscale*/1.0f, /*bm_height * texinfo.coscale*/1.0f,
						  width, height, 0, &flarecol );
}


// init glide state for flare circle drawing ----------------------------------
//
PRIVATE
void RO_FlareStateInit()
{
	// configure rasterizer
	dword itertype	= iter_rgbatexa | iter_additiveblend;
	dword raststate	= rast_texclamp | rast_chromakeyoff;
	dword rastmask	= rast_mask_zbuffer;

	RO_InitRasterizerState( itertype, raststate, rastmask );
	RO_TextureCombineState( texcomb_decal );
}


// deinit glide state changes after flare circle drawing ----------------------
//
PRIVATE
void RO_FlareStateRestore()
{
	// set rasterizer state to default
	RO_DefaultRasterizerState();
}


// draw lens flare if related light-source visible ----------------------------
//
void R_DrawLensFlare()
{
	// lens flares rely on hardware depth occlusion querying now
	if ( !(DoLensFlare && (GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)) )
		return;

	int flarestarno = SUN_STAR_NO;

	Camera fixedstarcam;
	CAMERA_MakeFixedStarCam( fixedstarcam );

	Vertex3 tempvert;
	MtxVctMUL( fixedstarcam, &FixedStars[ flarestarno ].location, &tempvert );

	if ( tempvert.Z > near_clip_z ) {

		SPoint loc;
		PROJECT_TO_SCREEN( tempvert, loc );

		//int w = 24 * ((float) Screen_Width / 640.0f), h = 24 * ((float) Screen_Height / 480.0f);
		int h = 24 * ((float) Screen_Height / 480.0f);
		int w = h;
		
		bool firstQuery = false;
		
		// generate a new occlusion query object if we don't have one or the current one isn't valid
		if (flareDepthQuery == 0) {
			glGenQueriesARB(1, &flareDepthQuery);
			firstQuery = true;
		}

		// check whether GPU is done the previous occlusion query
		GLuint queryAvailable = GL_FALSE;
		if (!firstQuery)
			glGetQueryObjectuivARB(flareDepthQuery, GL_QUERY_RESULT_AVAILABLE, &queryAvailable);
		 
		// only do querying when it won't force a pipeline flush
		if ((queryAvailable == GL_TRUE) || firstQuery) {
			
			// get result of previous query, if it exists
			if (!firstQuery)
				glGetQueryObjectuivARB(flareDepthQuery, GL_QUERY_RESULT, &flareDepthQueryResult);
			
			GLboolean depthMask = RO_DepthWriteEnabled();
			int depthTestEnabled = RO_DepthCmpEnabled();
			
			// enable depth test and disable all other drawing operations
			RO_EnableDepthTest(TRUE);
			RO_DepthFunc(GL_GREATER);
			RO_DepthMask(GL_FALSE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			
			// start new depth query
			glBeginQueryARB(GL_SAMPLES_PASSED, flareDepthQuery);
			
			// "draw" the bounding box of the sun, with just depth testing & querying enabled
			
			GLfloat vertices[] = {
				loc.X - w/2, loc.Y - h/2, 1.0f / tempvert.Z,
				loc.X + w/2, loc.Y - h/2, 1.0f / tempvert.Z,
				loc.X + w/2, loc.Y + h/2, 1.0f / tempvert.Z,
				loc.X - w/2, loc.Y + h/2, 1.0f / tempvert.Z,
			};
			
			RO_ClientState(VTXARRAY_VERTICES);
			RO_ArrayMakeCurrent(VTXPTRS_NONE, NULL);
			
			glVertexPointer(3, GL_FLOAT, 0, vertices);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			
			// we're done telling the GPU what to query (we won't know the result until the next glGetQueryObject call)
			glEndQueryARB(GL_SAMPLES_PASSED);
			
			// restore drawing operations
			RO_DepthMask(depthMask);
			RO_EnableDepthTest(depthTestEnabled);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}

		if (flareDepthQueryResult > 0) {
			float oprojx = loc.X - Screen_XOfs;
			float oprojy = loc.Y - Screen_YOfs;
			
			// distance from the center of the screen to the sun
			float dist = sqrtf(oprojx * oprojx + oprojy * oprojy);
			
			// visible pixels / total pixels
			float flareAlpha = min((float) flareDepthQueryResult / (w*h), 1.0f);

			// draw lensflare circles
			if ( !AUX_DONT_DRAW_FLARE_CIRCLES ) {

				RO_FlareStateInit();

				colrgba_s flarecol;

				flarecol.R = 60;
				flarecol.G = 190;
				flarecol.B = 190;
				loc.X = GEOMV_TO_INT( oprojx * FLOAT_TO_GEOMV(-0.6000 ) ) + Screen_XOfs;
				loc.Y = GEOMV_TO_INT( oprojy * FLOAT_TO_GEOMV(-0.6000 ) ) + Screen_YOfs;
				RO_FlareCircle( loc.X-15, loc.Y-15, 31, 31, flarecol, flareAlpha );

				flarecol.R = 180;
				flarecol.G = 60;
				flarecol.B = 190;
				loc.X = GEOMV_TO_INT( oprojx * FLOAT_TO_GEOMV(-0.4000 ) ) + Screen_XOfs;
				loc.Y = GEOMV_TO_INT( oprojy * FLOAT_TO_GEOMV(-0.4000 ) ) + Screen_YOfs;
				RO_FlareCircle( loc.X-8, loc.Y-8, 17, 17, flarecol, flareAlpha );

				flarecol.R = 20;
				flarecol.G = 40;
				flarecol.B = 200;
				loc.X = GEOMV_TO_INT( oprojx * FLOAT_TO_GEOMV(-0.2000 ) ) + Screen_XOfs;
				loc.Y = GEOMV_TO_INT( oprojy * FLOAT_TO_GEOMV(-0.2000 ) ) + Screen_YOfs;
				RO_FlareCircle( loc.X-30, loc.Y-30, 61, 61, flarecol, flareAlpha );

				flarecol.R = 0;
				flarecol.G = 190;
				flarecol.B = 220;
				loc.X = GEOMV_TO_INT( oprojx * FLOAT_TO_GEOMV( 0.1200 ) ) + Screen_XOfs;
				loc.Y = GEOMV_TO_INT( oprojy * FLOAT_TO_GEOMV( 0.1200 ) ) + Screen_YOfs;
				RO_FlareCircle( loc.X-35, loc.Y-35, 71, 71, flarecol, flareAlpha );

				flarecol.R = 80;
				flarecol.G = 0;
				flarecol.B = 200;
				loc.X = GEOMV_TO_INT( oprojx * FLOAT_TO_GEOMV( 0.4500 ) ) + Screen_XOfs;
				loc.Y = GEOMV_TO_INT( oprojy * FLOAT_TO_GEOMV( 0.4500 ) ) + Screen_YOfs;
				RO_FlareCircle( loc.X-40, loc.Y-40, 81, 81, flarecol, flareAlpha );

				flarecol.R = 0;
				flarecol.G = 50;
				flarecol.B = 255;
				loc.X = GEOMV_TO_INT( oprojx * FLOAT_TO_GEOMV( 0.7000 ) ) + Screen_XOfs;
				loc.Y = GEOMV_TO_INT( oprojy * FLOAT_TO_GEOMV( 0.7000 ) ) + Screen_YOfs;
				RO_FlareCircle( loc.X-20, loc.Y-20, 41, 41, flarecol, flareAlpha );

				RO_FlareStateRestore();
			}

			// flash screen
			if ( !AUX_DONT_FLASH_SCREEN_ON_FLARE ) {

				// determine flare intensity
				int flash = (int) min(dist, 140.0f);
				
				// flash intensity also depends on percent of sun visible
				flash = (140 - flash) * flareAlpha;

				// use screen dazzling capability
				// to create dazzling flare
				if ( SetScreenWhite < flash )
					SetScreenWhite = flash;
			}
		}
	}
}


// created clipped and transformed panorama object ----------------------------
//
PRIVATE
GenObject *RO_ClipTransformPanorama( GenObject *clipobj )
{
	ASSERT( clipobj != NULL );

	Camera fixedstarcam;
	CAMERA_MakeFixedStarCam( fixedstarcam );

	// transform frustum into panorama object space and clip there
	BackTransformVolume( fixedstarcam, View_Volume, Object_ViewVolume, 0x3d );
	clipobj = CLIP_VolumeGenObject( clipobj, Object_ViewVolume, 0x3d );
	if ( clipobj == NULL ) {
		return NULL;
	}

	Poly*     polylist	= clipobj->PolyList;
	dword*    vpolylist	= clipobj->VisPolyList;
	Vertex3*  vtxlist	= clipobj->VertexList;

	// process entire polygon list
	for ( unsigned int pid = 0; pid < clipobj->NumPolys; pid++, polylist++ ) {

		// reject degenerated polygons
		if ( polylist->NumVerts < 3 ) {

			//NOTE:
			// needed by clipper for trivial
			// rejection of polygons.

			continue;
		}

		// append polygon to list of visible polygons
		*vpolylist++ = pid;

		// visit all vertices of this polygon and set visible=true in turn
		dword *vindxs = polylist->VertIndxs;
		for ( int vct = polylist->NumVerts; vct > 0; vct-- ) {
			Vertex3 *vtx = &vtxlist[ *vindxs++ ];
			vtx->VisibleFrame = CurVisibleFrame;
		}
		
		// calc texture gradients
		Face *face = &clipobj->FaceList[ polylist->FaceIndx ];
		MtxMtxMUL( fixedstarcam, face->TexXmatrx, DestXmatrx );
		AdjointMtx( DestXmatrx, face->CurTexXmatrx );
	}
	
	// store number of currently visible polygons
	clipobj->NumVisPolys = vpolylist - clipobj->VisPolyList;
	
	// calculate vertex coordinates that belong to visible polygons
	memcpy( clipobj->CurrentXmatrx, fixedstarcam, sizeof( Xmatrx ) );
	ProcessObject( clipobj );
	
	return clipobj;
}


// acquired panorama layer objects --------------------------------------------
//
static GenObject *panorama_layer1;
static GenObject *panorama_layer2;
static GenObject *panorama_layer3;

// acquire panorama layer objects ---------------------------------------------
//
PRIVATE
void RO_AcquirePanoramaObjects()
{
	// class ids of panorama objects
	static dword pobjid1 = CLASS_ID_INVALID;
	static dword pobjid2 = CLASS_ID_INVALID;
	static dword pobjid3 = CLASS_ID_INVALID;
	
	// nebula layer may be switched using aux-flag
	int nebulaid = AUXDATA_BACKGROUND_NEBULA_ID;
	
	static int lastnebulaid = -1;
	if ( lastnebulaid != nebulaid ) {
		lastnebulaid = nebulaid;
		pobjid1 = CLASS_ID_INVALID;
		pobjid3 = CLASS_ID_INVALID;
		
		// HACK: load the level for a specific nebula id
		if( !AUX_DISABLE_LEVEL_SYNC ) {
			LVL_LoadIntLevel( nebulaid );
		}
	}

	char pobjname1[] = "panorama1_00";
	if ( ( nebulaid >= 1 ) && ( nebulaid <= 6 ) ) {
		pobjname1[ 11 ] = '0' + nebulaid;
	} else {
		pobjname1[ 9 ] = 0;
	}
	
	// object for stars layer depends on current resolution
	static resinfo_s lastscreenres;
	if (lastscreenres != GameScreenRes) {
		lastscreenres = GameScreenRes;
		pobjid2 = CLASS_ID_INVALID;
	}

	const char *pobjname2 = ( Screen_Height >= 720 ) ?
		"panorama2_hi" : "panorama2";

	// detail objects layer
	char pobjname3[] = "panorama3_00";
	if ( ( nebulaid >= 1 ) && ( nebulaid <= 6 ) ) {
		pobjname3[ 11 ] = '0' + nebulaid;
	} else {
		pobjname3[ 9 ] = 0;
	}

	// acquire layer 1 if not disabled (nebula)
	if ( !AUX_DISABLE_PANORAMIC_LAYER_1 ) {
		panorama_layer1 = OBJ_ReacquireObjectClass( &pobjid1, pobjname1 );
	}

	// acquire layer 2 if not disabled (stars)
	if ( !AUX_DISABLE_PANORAMIC_LAYER_2 ) {
		panorama_layer2 = OBJ_ReacquireObjectClass( &pobjid2, pobjname2 );
	}

	// acquire layer 1 if not disabled (detail objects)
	if ( !AUX_DISABLE_PANORAMIC_LAYER_3 ) {
		panorama_layer3 = OBJ_ReacquireObjectClass( &pobjid3, pobjname3 );
	}
}


// draw panoramic background image --------------------------------------------
//
void R_DrawPanorama()
{
	// reset layer object pointers
	panorama_layer1 = NULL;
	panorama_layer2 = NULL;
	panorama_layer3 = NULL;

	// acquire layer objects
	RO_AcquirePanoramaObjects();

	// render panorama object for layer 1 (nebula)
	if ( panorama_layer1 != NULL ) {

		// clip and transform object
		GenObject *clipobj = RO_ClipTransformPanorama( panorama_layer1 );
		if ( clipobj != NULL ) {

			// render clipped object
			int oldwrapmode = AUX_DISABLE_TEXTURE_WRAPPING;
			AUX_DISABLE_TEXTURE_WRAPPING = 1;
			R_RenderObject( clipobj );
			AUX_DISABLE_TEXTURE_WRAPPING = oldwrapmode;
		}
	}

	// render panorama object for layer 2 (stars)
	if ( panorama_layer2 != NULL ) {

		// clip and transform object
		GenObject *clipobj = RO_ClipTransformPanorama( panorama_layer2 );
		if ( clipobj != NULL ) {

			// render clipped object
			R_RenderObject( clipobj );
		}
	}

	// render panorama object for layer 3 (detail objects)
	if ( panorama_layer3 != NULL ) {

		// clip and transform object
		GenObject *clipobj = RO_ClipTransformPanorama( panorama_layer3 );
		if ( clipobj != NULL ) {

			// render clipped object
			R_RenderObject( clipobj );
		}
	}
}


// icosahedron geometry (from OpenGL red book) --------------------------------
//
#define PLANET_X .525731112119133606f
#define PLANET_Z .850650808352039932f

static Point3h_f icosahedron_vertices[ 12 ] = {

	{ -PLANET_X, GEOMV_0,  PLANET_Z },
	{  PLANET_X, GEOMV_0,  PLANET_Z },
	{ -PLANET_X, GEOMV_0, -PLANET_Z },
	{  PLANET_X, GEOMV_0, -PLANET_Z },

	{ GEOMV_0,  PLANET_Z,  PLANET_X },
	{ GEOMV_0,  PLANET_Z, -PLANET_X },
	{ GEOMV_0, -PLANET_Z,  PLANET_X },
	{ GEOMV_0, -PLANET_Z, -PLANET_X },

	{  PLANET_Z,  PLANET_X, GEOMV_0 },
	{ -PLANET_Z,  PLANET_X, GEOMV_0 },
	{  PLANET_Z, -PLANET_X, GEOMV_0 },
	{ -PLANET_Z, -PLANET_X, GEOMV_0 },
};

static int icosahedron_indexes[ 20 ][ 3 ] = {

	{ 1, 4, 0 },  { 4, 9, 0 },  { 4, 5, 9 },  { 8, 5, 4 },  { 1, 8, 4 },	//  0 ..  4
	{ 1, 10, 8 }, {	10, 3, 8 }, { 8, 3, 5 },  { 3, 2, 5 },  { 3, 7, 2 },	//  5 ..  9
	{ 3, 10, 7 }, { 10, 6, 7 }, { 6, 11, 7 }, { 6, 0, 11 }, { 6, 1, 0 },	// 10 .. 14
	{ 10, 1, 6 }, { 11, 0, 9 }, { 2, 11, 9 }, { 5, 2, 9 },  { 11, 2, 7 },	// 15 .. 19
};

static int icosahedron_adjacencies[ 20 ][ 3 ] = {

	{  1,  4, 14 },		// 0
	{  0,  2, 16 },		// 1
	{  1,  3, 18 },		// 2
	{  2,  4,  7 },		// 3
	{  0,  3,  5 },		// 4
	{  4,  6, 15 },		// 5
	{  5,  7, 10 },		// 6
	{  3,  6,  8 },		// 7
	{  7,  9, 18 },		// 8
	{  8, 10, 19 },		// 9
	{  6,  9, 11 },		// 10
	{ 10, 12, 15 },		// 11
	{ 11, 13, 19 },		// 12
	{ 12, 14, 16 },		// 13
	{  0, 13, 15 },		// 14
	{  5, 11, 14 },		// 15
	{  1, 13, 17 },		// 16
	{ 16, 18, 19 },		// 17
	{  2,  8, 17 },		// 18
	{  9, 12, 17 },		// 19
};


// ----------------------------------------------------------------------------
//
#define MAX_SUBDIVISION_LEVEL	4 //0 //4


// ----------------------------------------------------------------------------
//
static int			sphere_numverts;
static int			sphere_numtris;
static int*			sphere_indexes		= NULL;
static Point3h_f*	sphere_vertices		= NULL;
static Plane3*		sphere_faceplanes	= NULL;
static Point3h_f*	sphere_vtxnormals	= NULL;
static int*			sphere_triinactive	= NULL;

static float		sphere_scale;

static Vector3		sphere_oscam;


#define MAX_NUM_ENTS	10000


// ----------------------------------------------------------------------------
//
PRIVATE
void RO_PlanetCalcFacePlane( int tri )
{
	ASSERT( tri < sphere_numtris );

	int baseindx = tri * 3;

	Vector3 va;
	va.X = FLOAT_TO_GEOMV(
		sphere_vertices[ sphere_indexes[ baseindx + 1 ] ].X -
		sphere_vertices[ sphere_indexes[ baseindx + 0 ] ].X );
	va.Y = FLOAT_TO_GEOMV(
		sphere_vertices[ sphere_indexes[ baseindx + 1 ] ].Y -
		sphere_vertices[ sphere_indexes[ baseindx + 0 ] ].Y );
	va.Z = FLOAT_TO_GEOMV(
		sphere_vertices[ sphere_indexes[ baseindx + 1 ] ].Z -
		sphere_vertices[ sphere_indexes[ baseindx + 0 ] ].Z );

	Vector3 vb;
	vb.X = FLOAT_TO_GEOMV(
		sphere_vertices[ sphere_indexes[ baseindx + 2 ] ].X -
		sphere_vertices[ sphere_indexes[ baseindx + 0 ] ].X );
	vb.Y = FLOAT_TO_GEOMV(
		sphere_vertices[ sphere_indexes[ baseindx + 2 ] ].Y -
		sphere_vertices[ sphere_indexes[ baseindx + 0 ] ].Y );
	vb.Z = FLOAT_TO_GEOMV(
		sphere_vertices[ sphere_indexes[ baseindx + 2 ] ].Z -
		sphere_vertices[ sphere_indexes[ baseindx + 0 ] ].Z );

	CrossProduct( &va, &vb, PLANE_NORMAL( &sphere_faceplanes[ tri ] ) );
	NormVctX( PLANE_NORMAL( &sphere_faceplanes[ tri ] ) );

	Vertex3 vtxonplane;
	vtxonplane.X = FLOAT_TO_GEOMV( sphere_vertices[ sphere_indexes[ baseindx ] ].X );
	vtxonplane.Y = FLOAT_TO_GEOMV( sphere_vertices[ sphere_indexes[ baseindx ] ].Y );
	vtxonplane.Z = FLOAT_TO_GEOMV( sphere_vertices[ sphere_indexes[ baseindx ] ].Z );

	PLANE_OFFSET( &sphere_faceplanes[ tri ] ) =
		DOT_PRODUCT( &vtxonplane, PLANE_NORMAL( &sphere_faceplanes[ tri ] ) );
}


// ----------------------------------------------------------------------------
//
PRIVATE
int RO_PlanetFaceBackfacing( int tri )
{
	ASSERT( tri < sphere_numtris );

	geomv_t dist = DOT_PRODUCT( &sphere_oscam, PLANE_NORMAL( &sphere_faceplanes[ tri ] ) );
	return ( dist < PLANE_OFFSET( &sphere_faceplanes[ tri ] ) );
}


// subdivide a single triangle into four --------------------------------------
//
PRIVATE
void RO_PlanetSubdivideTriangle( int level, int tri )
{
	ASSERT( tri < sphere_numtris );

	if ( level == MAX_SUBDIVISION_LEVEL ) {
		return;
	}

	int baseindx = tri * 3;

	// base vertex indexes
	int bvid0 = sphere_indexes[ baseindx + 0 ];
	int bvid1 = sphere_indexes[ baseindx + 1 ];
	int bvid2 = sphere_indexes[ baseindx + 2 ];

	// new vertex indexes
	int nvid0 = sphere_numverts;
	int nvid1 = nvid0 + 1;
	int nvid2 = nvid1 + 1;

	// three additional vertices
	sphere_numverts += 3;
	ASSERT( sphere_numverts <= MAX_NUM_ENTS );

	// generate at edge midpoints
	sphere_vertices[ nvid0 ].X =
		( sphere_vertices[ bvid0 ].X + sphere_vertices[ bvid1 ].X ) * 0.5f;
	sphere_vertices[ nvid0 ].Y =
		( sphere_vertices[ bvid0 ].Y + sphere_vertices[ bvid1 ].Y ) * 0.5f;
	sphere_vertices[ nvid0 ].Z =
		( sphere_vertices[ bvid0 ].Z + sphere_vertices[ bvid1 ].Z ) * 0.5f;

	sphere_vertices[ nvid1 ].X =
		( sphere_vertices[ bvid1 ].X + sphere_vertices[ bvid2 ].X ) * 0.5f;
	sphere_vertices[ nvid1 ].Y =
		( sphere_vertices[ bvid1 ].Y + sphere_vertices[ bvid2 ].Y ) * 0.5f;
	sphere_vertices[ nvid1 ].Z =
		( sphere_vertices[ bvid1 ].Z + sphere_vertices[ bvid2 ].Z ) * 0.5f;

	sphere_vertices[ nvid2 ].X =
		( sphere_vertices[ bvid2 ].X + sphere_vertices[ bvid0 ].X ) * 0.5f;
	sphere_vertices[ nvid2 ].Y =
		( sphere_vertices[ bvid2 ].Y + sphere_vertices[ bvid0 ].Y ) * 0.5f;
	sphere_vertices[ nvid2 ].Z =
		( sphere_vertices[ bvid2 ].Z + sphere_vertices[ bvid0 ].Z ) * 0.5f;

	// pull to sphere surface
	float oolen = sphere_scale / sqrt(
		sphere_vertices[ nvid0 ].X * sphere_vertices[ nvid0 ].X +
		sphere_vertices[ nvid0 ].Y * sphere_vertices[ nvid0 ].Y +
		sphere_vertices[ nvid0 ].Z * sphere_vertices[ nvid0 ].Z );
	sphere_vertices[ nvid0 ].X *= oolen;
	sphere_vertices[ nvid0 ].Y *= oolen;
	sphere_vertices[ nvid0 ].Z *= oolen;

	oolen = sphere_scale / sqrt(
		sphere_vertices[ nvid1 ].X * sphere_vertices[ nvid1 ].X +
		sphere_vertices[ nvid1 ].Y * sphere_vertices[ nvid1 ].Y +
		sphere_vertices[ nvid1 ].Z * sphere_vertices[ nvid1 ].Z );
	sphere_vertices[ nvid1 ].X *= oolen;
	sphere_vertices[ nvid1 ].Y *= oolen;
	sphere_vertices[ nvid1 ].Z *= oolen;

	oolen = sphere_scale / sqrt(
		sphere_vertices[ nvid2 ].X * sphere_vertices[ nvid2 ].X +
		sphere_vertices[ nvid2 ].Y * sphere_vertices[ nvid2 ].Y +
		sphere_vertices[ nvid2 ].Z * sphere_vertices[ nvid2 ].Z );
	sphere_vertices[ nvid2 ].X *= oolen;
	sphere_vertices[ nvid2 ].Y *= oolen;
	sphere_vertices[ nvid2 ].Z *= oolen;

	// for next subdivision
	level++;

	// change base triangle in place
	sphere_indexes[ baseindx + 0 ] = bvid0;
	sphere_indexes[ baseindx + 1 ] = nvid0;
	sphere_indexes[ baseindx + 2 ] = nvid2;

	RO_PlanetCalcFacePlane( tri );

	if ( level == MAX_SUBDIVISION_LEVEL ) {
		sphere_triinactive[ tri ] = RO_PlanetFaceBackfacing( tri );
	}

//	if ( !RO_PlanetFaceBackfacing( tri ) ) {
		RO_PlanetSubdivideTriangle( level, tri );
//	}

	// three additional triangles
	ASSERT( sphere_numtris <= MAX_NUM_ENTS - 3 );

	sphere_numtris++;
	sphere_indexes[ sphere_numtris * 3 - 3 ] = nvid0;
	sphere_indexes[ sphere_numtris * 3 - 2 ] = bvid1;
	sphere_indexes[ sphere_numtris * 3 - 1 ] = nvid1;

	RO_PlanetCalcFacePlane( sphere_numtris - 1 );

	if ( level == MAX_SUBDIVISION_LEVEL ) {
		sphere_triinactive[ sphere_numtris - 1 ] = RO_PlanetFaceBackfacing( sphere_numtris - 1 );
	}

//	if ( !RO_PlanetFaceBackfacing( sphere_numtris - 1 ) ) {
		RO_PlanetSubdivideTriangle( level, sphere_numtris - 1 );
//	}

	sphere_numtris++;
	sphere_indexes[ sphere_numtris * 3 - 3 ] = nvid2;
	sphere_indexes[ sphere_numtris * 3 - 2 ] = nvid1;
	sphere_indexes[ sphere_numtris * 3 - 1 ] = bvid2;

	RO_PlanetCalcFacePlane( sphere_numtris - 1 );

	if ( level == MAX_SUBDIVISION_LEVEL ) {
		sphere_triinactive[ sphere_numtris - 1 ] = RO_PlanetFaceBackfacing( sphere_numtris - 1 );
	}

//	if ( !RO_PlanetFaceBackfacing( sphere_numtris - 1 ) ) {
		RO_PlanetSubdivideTriangle( level, sphere_numtris - 1 );
//	}

	sphere_numtris++;
	sphere_indexes[ sphere_numtris * 3 - 3 ] = nvid0;
	sphere_indexes[ sphere_numtris * 3 - 2 ] = nvid1;
	sphere_indexes[ sphere_numtris * 3 - 1 ] = nvid2;

	RO_PlanetCalcFacePlane( sphere_numtris - 1 );

	if ( level == MAX_SUBDIVISION_LEVEL ) {
		sphere_triinactive[ sphere_numtris - 1 ] = RO_PlanetFaceBackfacing( sphere_numtris - 1 );
	}

//	if ( !RO_PlanetFaceBackfacing( sphere_numtris - 1 ) ) {
		RO_PlanetSubdivideTriangle( level, sphere_numtris - 1 );
//	}
}


// generate the sphere mesh ---------------------------------------------------
//
PRIVATE
void RO_PlanetGenerateSphere( PlanetObject *planet )
{
	ASSERT( planet != NULL );

	sphere_scale = GEOMV_TO_FLOAT( planet->BoundingSphere );

	if ( sphere_vertices == NULL ) {
		sphere_vertices   = (Point3h_f *) ALLOCMEM( 3 * MAX_NUM_ENTS * sizeof( Point3h_f ) );
		sphere_faceplanes = (Plane3 *) &sphere_vertices[ MAX_NUM_ENTS ];
		sphere_vtxnormals = &sphere_vertices[ MAX_NUM_ENTS * 2 ];
	}
	if ( sphere_indexes == NULL ) {
		sphere_indexes     = (int *) ALLOCMEM( ( MAX_NUM_ENTS + MAX_NUM_ENTS * 3 ) * sizeof( int ) );
		sphere_triinactive = &sphere_indexes[ MAX_NUM_ENTS * 3 ];
	}

	// start off with icosahedron
	sphere_numverts = 12;
	sphere_numtris  = 20;

	// store icosahedron vertices
	for ( int vid = 0; vid < sphere_numverts; vid++ ) {

		sphere_vertices[ vid ].X = icosahedron_vertices[ vid ].X * sphere_scale;
		sphere_vertices[ vid ].Y = icosahedron_vertices[ vid ].Y * sphere_scale;
		sphere_vertices[ vid ].Z = icosahedron_vertices[ vid ].Z * sphere_scale;
	}

	// calculate object space to view space transformation
	MtxMtxMUL( ViewCamera, planet->ObjPosition, planet->CurrentXmatrx );

	// calc viewpoint in object space
	CalcObjSpaceCamera( planet, &sphere_oscam );

	// store icosahedron triangles
	int dstindx = 0;
	int dsttri  = 0;
	int tri = 0;
	for ( tri = 0; tri < sphere_numtris; tri++ ) {

		sphere_indexes[ dstindx + 0 ] = icosahedron_indexes[ tri ][ 0 ];
		sphere_indexes[ dstindx + 1 ] = icosahedron_indexes[ tri ][ 1 ];
		sphere_indexes[ dstindx + 2 ] = icosahedron_indexes[ tri ][ 2 ];

		sphere_triinactive[ dsttri ] = FALSE;

		// calc and store face normal and distance
		RO_PlanetCalcFacePlane( dsttri );

//		// skip triangle if backfacing
//		if ( !RO_PlanetFaceBackfacing( dsttri ) ) {
			dstindx += 3;
			dsttri++;
//		}
	}
	sphere_numtris = dsttri;

	// subdivide starting with icosahedron
	int basenumtris = sphere_numtris;
	for ( tri = 0; tri < basenumtris; tri++ ) {
		RO_PlanetSubdivideTriangle( 0, tri );
	}
}


// ----------------------------------------------------------------------------
//
PRIVATE
void RO_PlanetDrawSphereTris( IterArray3 *itarray )
{
	ASSERT( itarray != NULL );

	int numtriindxs = sphere_numtris * 3;

	uint16 *vindxs = (uint16 *) ALLOCMEM( numtriindxs * sizeof( uint16 ) );
	if ( vindxs == NULL )
		OUTOFMEM( "no mem for indexes." );

	int dsttri = 0;
	for ( int tri = 0; tri < sphere_numtris; tri++ ) {

		if ( sphere_triinactive[ tri ] ) {
			continue;
		}

		vindxs[ dsttri + 0 ] = sphere_indexes[ tri * 3 + 0 ];
		vindxs[ dsttri + 1 ] = sphere_indexes[ tri * 3 + 1 ];
		vindxs[ dsttri + 2 ] = sphere_indexes[ tri * 3 + 2 ];

		dsttri += 3;
	}

	// draw indexed triangles in a single call
	D_DrawIterArrayIndexed(
		ITERARRAY_MODE_TRIANGLES, dsttri, vindxs, 0x3f );

	FREEMEM( vindxs );
}


// ----------------------------------------------------------------------------
//
PRIVATE
void RO_PlanetDrawSphereTrisWireFrame( IterArray3 *itarray )
{
	ASSERT( itarray != NULL );

	int numtriindxs = sphere_numtris * 6;

	uint16 *vindxs = (uint16 *) ALLOCMEM( numtriindxs * sizeof( uint16 ) );
	if ( vindxs == NULL )
		OUTOFMEM( "no mem for indexes." );

	int dsttri = 0;
	for ( int tri = 0; tri < sphere_numtris; tri++ ) {

		if ( sphere_triinactive[ tri ] ) {
			continue;
		}

		vindxs[ dsttri + 0 ] = (uint16) sphere_indexes[ tri * 3 + 0 ];
		vindxs[ dsttri + 1 ] = (uint16) sphere_indexes[ tri * 3 + 1 ];
		vindxs[ dsttri + 2 ] = (uint16) sphere_indexes[ tri * 3 + 1 ];
		vindxs[ dsttri + 3 ] = (uint16) sphere_indexes[ tri * 3 + 2 ];
		vindxs[ dsttri + 4 ] = (uint16) sphere_indexes[ tri * 3 + 2 ];
		vindxs[ dsttri + 5 ] = (uint16) sphere_indexes[ tri * 3 + 0 ];

		dsttri += 6;
	}

	glDrawElements( GL_LINES, dsttri, GL_UNSIGNED_SHORT, vindxs );

	FREEMEM( vindxs );
}


// ----------------------------------------------------------------------------
//
PRIVATE
void RO_PlanetDrawSphere( PlanetObject *planet )
{
	ASSERT( planet != NULL );

	// create vertex array
	IterArray3 *itarray = (IterArray3 *) ALLOCMEM(
		(size_t)&((IterArray3*)0)->Vtxs[ sphere_numverts ] );
	if ( itarray == NULL )
		OUTOFMEM( "no mem for vertex array." );

	itarray->NumVerts	= sphere_numverts;
	itarray->arrayinfo	= ITERARRAY_USE_COLOR |
						  ITERARRAY_USE_TEXTURE | ITERARRAY_GLOBAL_TEXTURE;
	itarray->flags		= ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW |
						  ITERFLAG_Z_TO_DEPTH;
	itarray->itertype	= iter_texrgb | iter_overwrite;
	itarray->raststate	= rast_zbuffer | rast_texclamp | rast_chromakeyoff;
	itarray->rastmask	= rast_nomask;
	itarray->texmap		= planet->FaceList[ 0 ].TexMap;

	int texwidth  = 1 << itarray->texmap->Width;
	int texheight = 1 << itarray->texmap->Height;

	// fill vertex array
	for ( int vid = 0; vid < sphere_numverts; vid++ ) {

		itarray->Vtxs[ vid ].X = FLOAT_TO_GEOMV( sphere_vertices[ vid ].X );
		itarray->Vtxs[ vid ].Y = FLOAT_TO_GEOMV( sphere_vertices[ vid ].Y );
		itarray->Vtxs[ vid ].Z = FLOAT_TO_GEOMV( sphere_vertices[ vid ].Z );
		itarray->Vtxs[ vid ].W = GEOMV_1;

		// wrap u texture coordinate by rotating about y
		float len = sqrt(
			sphere_vertices[ vid ].X * sphere_vertices[ vid ].X +
			sphere_vertices[ vid ].Z * sphere_vertices[ vid ].Z );
		float nx = sphere_vertices[ vid ].X / len;

		// acos(): [-1,1]->[pi,0]
		float tu = acos( nx ) / HPREC_TWO_PI;
		if ( sphere_vertices[ vid ].Z <= 0.0f ) {
			tu = 1.0f - tu;
		}

		// map v texture coordinate from y only
		float ny = sphere_vertices[ vid ].Y / sphere_scale;

		// asin(): [-1,1]->[-pi/2,pi/2]
		float tv = 0.5f + asin( ny ) / HPREC_PI;

		itarray->Vtxs[ vid ].U = FLOAT_TO_GEOMV( tu * texwidth );
		itarray->Vtxs[ vid ].V = FLOAT_TO_GEOMV( tv * texheight );

		itarray->Vtxs[ vid ].R = 255;
		itarray->Vtxs[ vid ].G = 255;
		itarray->Vtxs[ vid ].B = 255;
		itarray->Vtxs[ vid ].A = 255;
	}

	// calculate transformation matrix
	MtxMtxMUL( ViewCamera, planet->ObjPosition, DestXmatrx );

	// setup transformation matrix
	D_LoadIterMatrix( DestXmatrx );
///*
	// draw array
	D_LockIterArray3( itarray, 0, itarray->NumVerts );
	RO_PlanetDrawSphereTris( itarray );
	D_UnlockIterArray();
//*/
/*
	// turn off z-buffer
	RO_DisableDepthBuffer( true, true );
*/
	// g400 driver bugs when rendering twice from locked array
/*	D_LockIterArray3( itarray, 0, itarray->NumVerts );
	RO_PlanetDrawSphereTrisWireFrame( itarray );
	D_UnlockIterArray(); */

	// restore identity transformation
	D_LoadIterMatrix( NULL );

	// free vertex array
	FREEMEM( itarray );
}


// draw dynamically tessellated planet ----------------------------------------
//
void R_DrawPlanet( PlanetObject *planet )
{
	ASSERT( planet != NULL );

	RO_PlanetGenerateSphere( planet );
	RO_PlanetDrawSphere( planet );

	// avoid standard geometry drawing
//	planet->VisibleFrame = VISFRAME_NEVER;
	planet->NumVerts = 0;
	planet->NumPolys = 0;
	planet->NumFaces = 0;
}



