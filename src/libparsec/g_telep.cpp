/*
 * PARSEC - Teleporter Model
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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
#include <math.h>
#include <limits.h>

#ifndef isnan
#define isnan(x) _isnan(x)
#endif

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem linkage info
#include "linkinfo.h"

#ifndef PARSEC_SERVER
// subsystem headers
#include "aud_defs.h"

// drawing subsystem
#include "d_iter.h"
#include "d_misc.h"

#endif 


// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_telep.h"

// proprietary module headers
#include "con_arg.h"
#include "net_defs.h"
#ifndef PARSEC_SERVER
#include "aud_game.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_ext.h"
#include "con_info.h"
#include "con_main.h"
#include "e_callbk.h"
#include "e_level.h"
#include "e_supp.h"
#include "obj_game.h"
#include "net_game.h"
#include "net_limits.h"
#include "net_stream.h"
#else
#include "con_info_sv.h"
#include "con_main_sv.h"
#include "con_com_sv.h"
#include "net_game_sv.h"
#include "net_limits.h"
#include "e_gameserver.h"
#endif

#include "obj_clas.h"
#ifndef PARSEC_SERVER
#include "obj_ctrl.h"
#endif
#include "obj_cust.h"
#include "sys_date.h"
#include "sys_path.h"

// assigned type id for teleporter
dword teleporter_type;

// string constants -----------------------------------------------------------
//
static char telep_level_inval_level_spec[]	  = "invalid level specified";
static char telep_level_inval_filename_spec[] = "invalid filename specified";
static char telep_exit_class_not_found[] 	  = "object class for teleporter exit not found.";
static char telep_texture_not_found[]		  = "texture '%s' was not found.";

// flags ----------------------------------------------------------------------
//
//#define ANIMATE_TUNNEL_FRAME PARSEC_DEBUG
//#define DONT_MOVE_REMOTE_SHIPS

// teleporter limits and constants --------------------------------------------
//
#define TELEPORTER_DEFAULT_TEX_NAME_INTERIOR	"tp_int.3df"
#define TELEPORTER_DEFAULT_TEX_NAME_TUNNEL		"tp_tun.3df"
#define TELEPORTER_EXIT_MODEL_NAME				"telep_exit"
#define TELEP_BOUNDARY_HORIZ					38						// half of horizontal extent of the teleporter start interior rect
#define TELEP_BOUNDARY_VERT						30.5					// half of vertical   extent of the teleporter start interior rect

// helper macros --------------------------------------------------------------
//
//#define SETRGBA( x, r,g,b,a ) { (x)->R = r; (x)->G = g; (x)->B = b; (x)->A = a; }

// module local functions -----------------------------------------------------
//
PRIVATE	void	Teleporter_Animate_Tunnel			( Teleporter* teleporter );
PRIVATE void	Teleporter_Calc_Tunnel_Verts		( Teleporter* teleporter );
PRIVATE int		Teleporter_Check_Tunnel_Visibility	( Teleporter* teleporter );
PRIVATE	void	Teleporter_Draw_Tunnel				( Teleporter* teleporter );
PRIVATE void	Teleporter_Draw_Entry_Interior		( Teleporter* teleporter );
PRIVATE void	Teleporter_Debug_Show_Tunnel_Spline	( Teleporter* teleporter );

PRIVATE int		TeleporterModify_ExitPropsChanged	( GenObject* base );
PRIVATE int		TeleporterModify_StartPropsChanged	( GenObject* base );
PRIVATE int		TeleporterModify_TexPropsChanged	( GenObject* base );
PRIVATE int		TeleporterModify_SplinePropsChanged	( GenObject* base );

PRIVATE void	Teleporter_Rotation_Transform		( float phi, float theta, Xmatrx trafo );


// offset definitions into the Teleporter -------------------------------------
//
#define OFS_EXITDELTAX			offsetof( Teleporter, exit_delta_x )
#define OFS_EXITDELTAY			offsetof( Teleporter, exit_delta_y )
#define OFS_EXITDELTAZ			offsetof( Teleporter, exit_delta_z )
#define OFS_EXITROTPHI			offsetof( Teleporter, exit_rot_phi )
#define OFS_EXITROTTHETA		offsetof( Teleporter, exit_rot_theta )
#define OFS_ACTOFFSET			offsetof( Teleporter, actoffset )
#define OFS_ACTIVE				offsetof( Teleporter, active )
#define OFS_TEXNAME_INT			offsetof( Teleporter, tex_name_interior )
#define OFS_TEXNAME_TUNNEL		offsetof( Teleporter, tex_name_tunnel )
#define OFS_U_VAR1				offsetof( Teleporter, u_variation[ 0 ] )
#define OFS_V_VAR1				offsetof( Teleporter, v_variation[ 0 ] )
#define OFS_U_VAR2				offsetof( Teleporter, u_variation[ 1 ] )
#define OFS_V_VAR2				offsetof( Teleporter, v_variation[ 1 ] )
#define OFS_START_SEC_LY		offsetof( Teleporter, start_tex_sec_layer )
#define OFS_START_TEX_ALPHA		offsetof( Teleporter, start_tex_alpha )
#define OFS_ACT_CONE_ANGLE		offsetof( Teleporter, act_cone_angle )
#define OFS_TUNNEL_SLERP		offsetof( Teleporter, tunnel_slerp )
#define OFS_TUNNEL_LERP			offsetof( Teleporter, tunnel_lerp)
#define OFS_TUNNEL_TAN_MUL1		offsetof( Teleporter, tunnel_tangent_scale1 )
#define OFS_TUNNEL_TAN_MUL2		offsetof( Teleporter, tunnel_tangent_scale2 )
#define OFS_TUNNEL_ANIM_SPD		offsetof( Teleporter, tunnel_anim_speed )
#define OFS_TUNNEL_SPLINE		offsetof( Teleporter, tunnel_spline )
#define OFS_TUNNEL_SPL_CORDS	offsetof( Teleporter, tunnel_spline_cords )
#define OFS_TUNNEL_TEX_ALPHA	offsetof( Teleporter, tunnel_tex_alpha )
#define OFS_TUNNEL_LEN			offsetof( Teleporter, tunnel_len )
#define OFS_START_X				offsetof( Teleporter, start.X )
#define OFS_START_Y				offsetof( Teleporter, start.Y )
#define OFS_START_Z				offsetof( Teleporter, start.Z )
#define OFS_STARTROTPHI			offsetof( Teleporter, start_rot_phi )
#define OFS_STARTROTTHETA		offsetof( Teleporter, start_rot_theta )


// list of console-accessible properties --------------------------------------
//
PRIVATE
proplist_s Teleporter_PropList[] = {
	
	{ "exit_delta_x",			OFS_EXITDELTAX,		-0x40000000,0x40000000,					PROPTYPE_FLOAT,		TeleporterModify_ExitPropsChanged	},
	{ "exit_delta_y",			OFS_EXITDELTAY,		-0x40000000,0x40000000,					PROPTYPE_FLOAT,		TeleporterModify_ExitPropsChanged	},
	{ "exit_delta_z",			OFS_EXITDELTAZ,		-0x40000000,0x40000000,					PROPTYPE_FLOAT,		TeleporterModify_ExitPropsChanged	},
	{ "exit_rot_phi",			OFS_EXITROTPHI,		0x00000,	0x1680000,					PROPTYPE_FLOAT,		TeleporterModify_ExitPropsChanged	},
	{ "exit_rot_theta",			OFS_EXITROTTHETA,	0x00000,	0x1680000,					PROPTYPE_FLOAT,		TeleporterModify_ExitPropsChanged	},
	{ "actoffset",				OFS_ACTOFFSET,		0x10000,	0x4000000,					PROPTYPE_FLOAT,		NULL	},
	{ "active",					OFS_ACTIVE,			0x0,		0x1,						PROPTYPE_INT,		NULL    },
	{ "tex_name_interior",		OFS_TEXNAME_INT,	0,			TELEPORTER_MAX_TEX_NAME,	PROPTYPE_STRING,	TeleporterModify_TexPropsChanged	},
	{ "tex_name_tunnel",		OFS_TEXNAME_TUNNEL,	0,			TELEPORTER_MAX_TEX_NAME,	PROPTYPE_STRING,	TeleporterModify_TexPropsChanged	},
	{ "tex_u_var1",				OFS_U_VAR1,			0x00000,	0x0010000,					PROPTYPE_FLOAT,		TeleporterModify_TexPropsChanged	},
	{ "tex_v_var1",				OFS_V_VAR1,			0x00000,	0x0010000,					PROPTYPE_FLOAT,		TeleporterModify_TexPropsChanged	},
	{ "tex_u_var2",				OFS_U_VAR2,			0x00000,	0x0010000,					PROPTYPE_FLOAT,		TeleporterModify_TexPropsChanged	},
	{ "tex_v_var2",				OFS_V_VAR2,			0x00000,	0x0010000,					PROPTYPE_FLOAT,		TeleporterModify_TexPropsChanged	},
	{ "secly",					OFS_START_SEC_LY,	0x0,		0x1,						PROPTYPE_INT,		NULL    },
	{ "start_tex_alpha",		OFS_START_TEX_ALPHA,0x0,		0xff,						PROPTYPE_INT,		NULL    },
	{ "act_cone_angle",			OFS_ACT_CONE_ANGLE,	0x00000,	0x0590000,					PROPTYPE_FLOAT,		NULL	},
	{ "tslerp",					OFS_TUNNEL_SLERP,	0x0,		0x1,						PROPTYPE_INT,		NULL    },
	{ "tlerp",					OFS_TUNNEL_LERP,	0x0,		0x2,						PROPTYPE_INT,		NULL    },
	{ "tunnel_ts1",				OFS_TUNNEL_TAN_MUL1,0x1,		0x40000000,					PROPTYPE_FLOAT,		TeleporterModify_SplinePropsChanged	},	
	{ "tunnel_ts2",				OFS_TUNNEL_TAN_MUL2,0x1,		0x40000000,					PROPTYPE_FLOAT,		TeleporterModify_SplinePropsChanged	},	
	{ "tunnel_spline_cords",	OFS_TUNNEL_SPL_CORDS,0x1,		0x800000,					PROPTYPE_INT,		TeleporterModify_SplinePropsChanged },
	{ "tunnel_anim_speed",		OFS_TUNNEL_ANIM_SPD,0x0,		0x2580000,					PROPTYPE_FLOAT,		NULL	},	
	{ "tunnel_spline",			OFS_TUNNEL_SPLINE,	0x0,		0x3f,						PROPTYPE_INT,		NULL    },
	{ "tunnel_tex_alpha",		OFS_TUNNEL_TEX_ALPHA,0x0,		0xff,						PROPTYPE_INT,		NULL    },
	{ "tunnel_len",				OFS_TUNNEL_LEN,		0x0,		0x4000000,					PROPTYPE_INT,		TeleporterModify_SplinePropsChanged },
	{ "start.x",				OFS_START_X,		-0x7fffffff,0x7fffffff,					PROPTYPE_GEOMV,		TeleporterModify_StartPropsChanged	},
	{ "start.y",				OFS_START_Y,		-0x7fffffff,0x7fffffff,					PROPTYPE_GEOMV,		TeleporterModify_StartPropsChanged	},
	{ "start.z",				OFS_START_Z,		-0x7fffffff,0x7fffffff,					PROPTYPE_GEOMV,		TeleporterModify_StartPropsChanged	},
	{ "start_rot_phi",			OFS_STARTROTPHI,	-0x2d00000,	0x2d00000,					PROPTYPE_FLOAT,		TeleporterModify_StartPropsChanged	},
	{ "start_rot_theta",		OFS_STARTROTTHETA,	-0x2d00000,	0x2d00000,					PROPTYPE_FLOAT,		TeleporterModify_StartPropsChanged	},
	
	{ NULL,				0,				0,			0,							0,					NULL	},
};


// calculate the vertices making up the tunnel geometry -----------------------
//
PRIVATE
void Teleporter_Calc_Tunnel_Verts( Teleporter* teleporter )
{
#ifndef PARSEC_SERVER
	ASSERT( teleporter != NULL );
	
	// free any previous geometry
	if ( teleporter->tunnel_verts != NULL )
		FREEMEM( teleporter->tunnel_verts );
	
	int num_steps = ( teleporter->tunnel_spline_cords + 1 );
	int num_vtxs  = 4 * num_steps;
	Vertex3* vtxs;
	
	// allocate the space for the new vertices
	size_t allocsize = (size_t)num_vtxs * sizeof( Vertex3 );
	teleporter->tunnel_verts = vtxs = (Vertex3*)ALLOCMEM( allocsize );
	if ( vtxs == NULL )
		OUTOFMEM( 0 );
	memset( vtxs, 0, allocsize );
	
	Vector3 start_pos, end_pos;
	Vector3 start_tan, end_tan;
	
	// get the tangents
	FetchZVector( teleporter->ObjPosition,					&start_tan );
	FetchZVector( teleporter->child_object->ObjPosition,	&end_tan   );
	
	// scale the tangents
	VECMULS( &start_tan, &start_tan, teleporter->tunnel_tangent_scale1 );
	VECMULS( &end_tan,   &end_tan,   teleporter->tunnel_tangent_scale2 );
	
	// get the start/end pos
	//FetchTVector( teleporter->ObjPosition,					&start_pos );
	//FetchTVector( teleporter->child_object->ObjPosition,	&end_pos );

	// HACK to cover the different Z curve between OpenGL and our projection
	Xmatrx start_frame_ospc, end_frame_ospc;
	Xmatrx start_frame_wspc, end_frame_wspc;
	CalcOrthoInverse( teleporter->ObjPosition,				 start_frame_ospc );
	CalcOrthoInverse( teleporter->child_object->ObjPosition, end_frame_ospc   );
	start_frame_ospc[ 2 ][ 3 ] -= 6;
	end_frame_ospc  [ 2 ][ 3 ] += 6;
	CalcOrthoInverse( start_frame_ospc, start_frame_wspc );
	CalcOrthoInverse( end_frame_ospc, end_frame_wspc );
	FetchTVector( start_frame_wspc,	&start_pos );
	FetchTVector( end_frame_wspc,	&end_pos );
	
	// get the start and end frame
	Xmatrx start_frame, end_frame;
	MakeNonTranslationMatrx( teleporter->ObjPosition,				start_frame );
	MakeNonTranslationMatrx( teleporter->child_object->ObjPosition, end_frame );

	// norm the start/end frame
	NormMtx( start_frame );
	NormMtx( end_frame );
	
/*	// get the axis vects from start/end frame
	Vector3 start_axis[ 3 ], end_axis[ 3 ];
	FetchXVector( start_frame, &start_axis[ 0 ] );
	FetchYVector( start_frame, &start_axis[ 1 ] );
	FetchZVector( start_frame, &start_axis[ 2 ] );
	FetchXVector( end_frame, &end_axis[ 0 ] );
	FetchYVector( end_frame, &end_axis[ 1 ] );
	FetchZVector( end_frame, &end_axis[ 2 ] );
	
	// rotate start/end frame to be aligned to the positive z axis
	Vector3 zAxis;
	zAxis.X = GEOMV_0;
	zAxis.Y = GEOMV_0;
	zAxis.Z = GEOMV_1;

	// get the rotation axis
	Vector3 rot_axis_start, rot_axis_end;
	CrossProduct( &zAxis, &start_axis[ 2 ], &rot_axis_start );
	CrossProduct( &zAxis, &end_axis[ 2 ],   &rot_axis_end );

	int ignore_start = ( VctLenX( &rot_axis_start ) == GEOMV_0 );
	int ignore_end   = ( VctLenX( &rot_axis_end   ) == GEOMV_0 );

 	// norm the rotation axis
	if ( !ignore_start ) NormVctX( &rot_axis_start );
	if ( !ignore_end   ) NormVctX( &rot_axis_end );
	
	// get the rotation angle
	float alpha_start_rad = ignore_start ? 0.0f : acos( (double)DotProduct( &start_axis[ 2 ], &zAxis ) );
	float alpha_end_rad   = ignore_end   ? 0.0f : acos( (double)DotProduct( &end_axis[ 2 ],   &zAxis ) );

	// setup the transformations to rotate start/end frame
	Xmatrx start_rot_into_z, end_rot_into_z;
	if ( !ignore_start ) MatrxFromAngularDisplacement( start_rot_into_z, RAD_TO_BAMS( alpha_start_rad ), &rot_axis_start );
	if ( !ignore_end   ) MatrxFromAngularDisplacement( end_rot_into_z,   RAD_TO_BAMS( alpha_end_rad   ), &rot_axis_end   );

	// rotate the axis vects into Z
	Vector3 start_axis_rotated[ 3 ], end_axis_rotated[ 3 ];
	for( int axis = 0; axis < 3; axis++ ) {
		if ( !ignore_start ) MtxVctMULt( start_rot_into_z, &start_axis[ axis ], &start_axis_rotated[ axis ] );
		if ( !ignore_end   ) MtxVctMULt( end_rot_into_z,   &end_axis[ axis ],   &end_axis_rotated[ axis ] );
	}

	// store the rotated axis back into the start/end frame
	if ( !ignore_start ) {
		StoreXVector( start_frame, &start_axis_rotated[ 0 ] );
		StoreYVector( start_frame, &start_axis_rotated[ 1 ] );
		StoreZVector( start_frame, &start_axis_rotated[ 2 ] );
	}
	if ( !ignore_end ) {
		StoreXVector( end_frame, &end_axis_rotated[ 0 ] );
		StoreYVector( end_frame, &end_axis_rotated[ 1 ] );
		StoreZVector( end_frame, &end_axis_rotated[ 2 ] );
	}
	
*/
	// setup the boundary in object space
	Vector3 boundary[ 4 ];
	for( int i = 0; i < 4; i++ ) {
		boundary[ i ].X = teleporter->start_vtxlist[ i ].X;
		boundary[ i ].Y = teleporter->start_vtxlist[ i ].Y;
		boundary[ i ].Z = GEOMV_0;
	}
	
	// init the hermite arclen data
	Hermite_ArcLen* hermite_data;
	hermite_data = Hermite_ArcLen_InitData( num_steps * 10, &start_pos, &end_pos, &start_tan, &end_tan );
	
	Vector3 interp_last;
	Vector3 tangent_last;
	Xmatrx  frame_last;
	
	// hack to store the start/end frame for display
	memcpy( teleporter->start_frame, start_frame, sizeof( Xmatrx ) );
	memcpy( teleporter->end_frame,   end_frame,   sizeof( Xmatrx ) );

	if ( teleporter->spline_frames != NULL ) {
		FREEMEM( teleporter->spline_frames );
	}
	teleporter->spline_frames = (Xmatrx*)ALLOCMEM( (size_t)num_steps * sizeof( Xmatrx ) );

	Xmatrx xy_plane_transf;
	Xmatrx hermite_transf;
	Xmatrx cur_frame;
	MakeIdMatrx( xy_plane_transf );

	// we make 2 passes in interpolating the frame along the spline. 
	// the first pass is to get the the correction of the rotation in the xy plane
	// the second pass is to correct the frames for the rotation in the xy plane
	for( int pass = 0; pass < 2; pass++ ) {

		// get the correction for the rotation in the xy plane
		float xy_correction_alpha_rad = 0;
		Xmatrx  xy_frame_2_correct;
		if( pass == 1 ) {

			// store the last cur_frame as correction frame in the xy plane
			MakeNonTranslationMatrx( frame_last, xy_frame_2_correct );

			// get the angle between the last interpolated frame in the first run and the exit_frame as specified by the user
			Vector3 x_set, x_interp;
			FetchXVector( end_frame, &x_set );
			FetchXVector( frame_last, &x_interp );
			xy_correction_alpha_rad = acos( (double)DotProduct( &x_set, &x_interp ) );
			xy_correction_alpha_rad /= num_steps ;
		}

		// interpolate pos on spline and orientation with quaternion for calculating the
		// boundary points of the tunnel ( all coordinates are in world space )
		for( int step = 0; step < num_steps; step++ ) {

			// init the sub transforms to identity
			MakeIdMatrx( cur_frame );
			MakeIdMatrx( hermite_transf );
			
			// normalize s
			float s = (float)step / (float)( num_steps - 1);
			
#ifdef ALLOW_Z_ROT
			// get slerp frame
			//QuaternionSlerpFrames( xy_plane_transf, start_frame, end_frame, s );
			//ReOrthoNormMtx( xy_plane_transf );
#endif // ALLOW_Z_ROT

			// calculate the xy transform to be the correction between the set exit_frame and the last interpolated frame from the first pass
			if ( ( pass == 1 )  && ( step != 0 ) ) {
				MakeIdMatrx( xy_plane_transf );
				//QuaternionSlerpFrames( xy_plane_transf, xy_frame_2_correct, end_frame, s );

				ObjRotZ( xy_plane_transf, RAD_TO_BAMS( xy_correction_alpha_rad  ) );
				ReOrthoNormMtx( xy_plane_transf );
			}
			
			// calculate the interpolated pos at the center of the tunnel
			Vector3 interp;
			Hermite_ArcLen_Interpolate( hermite_data, s, &interp );
			StoreTVector( hermite_transf, &interp );

			// get the next interpolation point
			Vector3 interp_next;
			if ( step < ( num_steps - 1 ) ) {
				float s_next = (float)( step + 1 ) / (float)( num_steps - 1 );
				Hermite_ArcLen_Interpolate( hermite_data, s_next, &interp_next );
			} 

			// when not at step 0, we get the tangent at the current step and rotate the frame_last to be z aligned with the tangent		
			Vector3 tangent;
			if ( step == 0 ) {
				FetchZVector( start_frame, &tangent );

				Vector3 dummy;
				FetchXVector( start_frame, &dummy ); StoreXVector( hermite_transf, &dummy ); 
				FetchYVector( start_frame, &dummy ); StoreYVector( hermite_transf, &dummy ); 
				FetchZVector( start_frame, &dummy ); StoreZVector( hermite_transf, &dummy ); 
			} else {
				// calculate the tangent ( tangent is paralell to the connection from samplepoint i-1 to i+1 )
				if ( step < ( num_steps - 1 ) ) {
					Vector3 D1, D2;
					VECSUB( &D1, &interp,      &interp_last );
					VECSUB( &D2, &interp_next, &interp );
					VECADD( &tangent, &D1, &D2 );
					VECMULS( &tangent, &tangent, FLOAT_TO_GEOMV( 0.5f ) );
				} else {
					FetchZVector( end_frame, &tangent );
				}

				NormVctX( &tangent );

				// get the rotation axis
				Vector3 rot_axis;
				CrossProduct( &tangent, &tangent_last, &rot_axis );
				
				
				Vector3 frame_axis_last[ 3 ];
				FetchXVector( frame_last, &frame_axis_last[ 0 ] );
				FetchYVector( frame_last, &frame_axis_last[ 1 ] );
				FetchZVector( frame_last, &frame_axis_last[ 2 ] );
				
				// check if we have a decent rotation axis
				if( VctLenX( &rot_axis ) > 1e-5 ) {

					NormVctX( &rot_axis );

					// get the angle for rotation
					float alpha_rad = acos( (double)DotProduct( &tangent, &tangent_last ) );

					// get the rotation matrix
					Xmatrx rot_into_tangent;
					MatrxFromAngularDisplacement( rot_into_tangent, RAD_TO_BAMS( alpha_rad ), &rot_axis );

					// rotate the (frame_last) axis vects into Z
					Vector3 frame_axis_last_rotated[ 3 ];

					for( int axis = 0; axis < 3; axis++ ) {
						MtxVctMULt( rot_into_tangent, &frame_axis_last[ axis ], &frame_axis_last_rotated[ axis ] );
					}

					// store the rotated frame into the hermite transformation
					StoreXVector( hermite_transf, &frame_axis_last_rotated[ 0 ] );
					StoreYVector( hermite_transf, &frame_axis_last_rotated[ 1 ] );
					StoreZVector( hermite_transf, &frame_axis_last_rotated[ 2 ] );
				} else {

					// there is no rotation necessary
					// store the last frame into the hermite transformation
					StoreXVector( hermite_transf, &frame_axis_last[ 0 ] );
					StoreYVector( hermite_transf, &frame_axis_last[ 1 ] );
					StoreZVector( hermite_transf, &frame_axis_last[ 2 ] );
				}
			}

			// create current frame from transformation in plane ( rot around z ) and translation/rotation from hermite interpolation
			MtxMtxMUL( hermite_transf, xy_plane_transf, cur_frame );
			//memcpy( cur_frame, hermite_transf, sizeof( Xmatrx ) );

			memcpy( &teleporter->spline_frames[ step ], cur_frame, sizeof( Xmatrx ) );
				
			// transform boundary from object to world space
			for( int nCorner = 0; nCorner < 4; nCorner++ ) {
				MtxVctMUL( cur_frame, &boundary[ nCorner ], &vtxs[ step * 4 + nCorner ] );
			}

			// store pos, tangent and frame for next step
			memcpy( &interp_last,	&interp, sizeof( Vector3 ) );
			memcpy( &tangent_last,	&tangent, sizeof( Vector3 ) );
			memcpy( frame_last,		cur_frame, sizeof( Xmatrx ) );
		}
	}
	
	// store the total arclength of the spline
	teleporter->tunnel_spline_arclen = hermite_data->total_arc_len;
	
	// kill the hermite arclen data
	Hermite_ArcLen_KillData( hermite_data );

	// calculate the cullbox in objectspace of the teleporter entry
	CullBox3* cullbox = &teleporter->tunnel_cullbox;
	cullbox->minmax[ 0 ] =  INT_TO_GEOMV( INT_MAX );
	cullbox->minmax[ 1 ] =  INT_TO_GEOMV( INT_MAX );
	cullbox->minmax[ 2 ] =  INT_TO_GEOMV( INT_MAX );

	cullbox->minmax[ 3 ] = -INT_TO_GEOMV( INT_MAX );
	cullbox->minmax[ 4 ] = -INT_TO_GEOMV( INT_MAX );
	cullbox->minmax[ 5 ] = -INT_TO_GEOMV( INT_MAX );
	
	// get the transformation from world to teleporter entry objectspace
	Xmatrx world2object;
	CalcOrthoInverse( teleporter->ObjPosition, world2object );

	Vertex3 vtx_in_ospc;
	for( int nVertex = 0; nVertex < num_vtxs; nVertex++ ) {
		MtxVctMUL( world2object, &vtxs[ nVertex ], &vtx_in_ospc );

		cullbox->minmax[ 0 ] = min( cullbox->minmax[ 0 ], vtx_in_ospc.X );
		cullbox->minmax[ 1 ] = min( cullbox->minmax[ 1 ], vtx_in_ospc.Y );
		cullbox->minmax[ 2 ] = min( cullbox->minmax[ 2 ], vtx_in_ospc.Z );

		cullbox->minmax[ 3 ] = max( cullbox->minmax[ 3 ], vtx_in_ospc.X );
		cullbox->minmax[ 4 ] = max( cullbox->minmax[ 4 ], vtx_in_ospc.Y );
		cullbox->minmax[ 5 ] = max( cullbox->minmax[ 5 ], vtx_in_ospc.Z );
	}
#endif
}


// notification callback when spline props changed ----------------------------
//
PRIVATE 
int	TeleporterModify_SplinePropsChanged( GenObject* base )
{
	ASSERT( base != NULL );
	Teleporter *teleporter = (Teleporter *) base;
	
	// calculate the vertices for the tunnel
	Teleporter_Calc_Tunnel_Verts( teleporter );
	
	// clamp to the total # of segs
	if ( teleporter->tunnel_len > teleporter->tunnel_spline_cords )
		teleporter->tunnel_len = teleporter->tunnel_spline_cords;
	
	// reset the tunnel animation
	teleporter->tunnel_cur_anim_step = GEOMV_0;
	
	return TRUE;
}

// strip the quotations around a string ---------------------------------------
//
PRIVATE
void StripQuotations( char* str2modify )
{
	ASSERT( str2modify != NULL );

	char szBuffer[ 128 ];
	size_t len = strlen( str2modify );
	ASSERT( len < 128 );

	// remove trainling
	if ( str2modify[ len - 1 ] == '"' ) {
		str2modify[ len - 1 ] = 0;
	}
	// remove leading
	if ( str2modify[ 0 ] == '"' ) {
		char* src = str2modify + 1;
		char* dst = str2modify;
		for(; *src != 0; src++, dst++ ) {
			*dst = *src;
		}
		*dst = 0;
	}
}


// notification callback when texture props changed ---------------------------
//
PRIVATE
int TeleporterModify_TexPropsChanged( GenObject* base )
{
	ASSERT( base != NULL );
	Teleporter *teleporter = (Teleporter *) base;
	
	// eventually remove leadin/trainling quotations from the texture names
	//StripQuotations( teleporter->tex_name_interior );
	//StripQuotations( teleporter->tex_name_tunnel   );
#ifndef PARSEC_SERVER
	// get pointer to texture map
	teleporter->start_texmap = FetchTextureMap( teleporter->tex_name_interior );
	if ( teleporter->start_texmap == NULL ) {
		MSGOUT( telep_texture_not_found, teleporter->tex_name_interior );
	} else {

		// init the u/v deltas with some random value
		geomv_t uwidth  = INT_TO_GEOMV( 1L << teleporter->start_texmap->Width  );
		geomv_t vheight = INT_TO_GEOMV( 1L << teleporter->start_texmap->Height );

		teleporter->u_maxdelta[ 0 ] = uwidth  * teleporter->u_variation[ 0 ];
		teleporter->v_maxdelta[ 0 ] = vheight * teleporter->v_variation[ 0 ];
		teleporter->u_maxdelta[ 1 ] = uwidth  * teleporter->u_variation[ 1 ];
		teleporter->v_maxdelta[ 1 ] = vheight * teleporter->v_variation[ 1 ];

	}

	// get pointer to texture map
	teleporter->tunnel_texmap = FetchTextureMap( teleporter->tex_name_tunnel );
	if ( teleporter->tunnel_texmap == NULL ) {
		MSGOUT( telep_texture_not_found, teleporter->tex_name_tunnel );
	}
#endif

	return TRUE;
}

// init a quaternion from spherical coordinates -------------------------------
//
void QuaternionFromSpherical_f( Quaternion_f *quat, float deg_angle, float deg_latitude, float deg_longitude )
{
	ASSERT( quat != NULL );

	float sin_a    = sin( DEG_TO_RAD( deg_angle ) / 2 );
	float cos_a    = cos( DEG_TO_RAD( deg_angle ) / 2 );

	float sin_lat  = sin( DEG_TO_RAD( deg_latitude ) );
	float cos_lat  = cos( DEG_TO_RAD( deg_latitude ) );

	float sin_long = sin( DEG_TO_RAD( deg_longitude ) );
	float cos_long = cos( DEG_TO_RAD( deg_longitude ) );

	quat->X       = sin_a * cos_lat * sin_long;
	quat->Y       = sin_a * sin_lat;
	quat->Z       = sin_a * sin_lat * cos_long;
	quat->W       = cos_a;
}

// init a quaternion from spherical coordinates -------------------------------
//
void QuaternionFromSpherical( Quaternion_f *quat, float deg_angle, float deg_latitude, float deg_longitude )
{
	ASSERT( quat != NULL );
	
	geomv_t sin_a    = FLOAT_TO_GEOMV( sin( DEG_TO_RAD( deg_angle ) / 2 ) );
	geomv_t cos_a    = FLOAT_TO_GEOMV( cos( DEG_TO_RAD( deg_angle ) / 2 ) );
	
	geomv_t sin_lat  = FLOAT_TO_GEOMV( sin( DEG_TO_RAD( deg_latitude ) ) );
	geomv_t cos_lat  = FLOAT_TO_GEOMV( cos( DEG_TO_RAD( deg_latitude ) ) );
	
	geomv_t sin_long = FLOAT_TO_GEOMV( sin( DEG_TO_RAD( deg_longitude ) ) );
	geomv_t cos_long = FLOAT_TO_GEOMV( cos( DEG_TO_RAD( deg_longitude ) ) );
	
	quat->X       = sin_a * cos_lat * sin_long;
	quat->Y       = sin_a * sin_lat;
	quat->Z       = sin_a * sin_lat * cos_long;
	quat->W       = cos_a;
}

// calculate the transfomation for the exit rotation --------------------------
//
PRIVATE
void Teleporter_Rotation_Transform( float phi, float theta, Xmatrx trafo )
{

	ASSERT( trafo != NULL );

	if(phi < 0.0f || isnan(phi)) {
		phi=0.0f;
	}
	if(phi>360.0f){
		phi=360.0f;
	}

	if(theta < 0.0f || isnan(theta)) {
		theta=0.0f;
	}
	if(theta>360.0f){
		theta=360.0f;
	}
	// NOTE: formula to get the position on the 1-sphere from angular coordinates:
	//
	// x = sin( phi ) * cos( theta )
	// y = sin( phi ) * sin( theta )
	// z = cos( phi )
	
	// for the z_axis in the frame we must add 90 degs to phi
	float sin_phi   = sin( DEG_TO_RAD( phi ) );
	float cos_phi   = cos( DEG_TO_RAD( phi ) );
	float sin_theta = sin( DEG_TO_RAD( 90 ) + DEG_TO_RAD( theta ) );
	float cos_theta = cos( DEG_TO_RAD( 90 ) + DEG_TO_RAD( theta ) );
	float sin_phi2  = sin( DEG_TO_RAD( 90 ) + DEG_TO_RAD( phi ) );
	float cos_phi2  = cos( DEG_TO_RAD( 90 ) + DEG_TO_RAD( phi ) );
	
	Vector3 x_axis, y_axis, z_axis;
	z_axis.X = sin_phi * cos_theta;
	z_axis.Y = sin_phi * sin_theta;
	z_axis.Z = cos_phi; 
	
	y_axis.X = sin_phi2 * cos_theta;	
	y_axis.Y = sin_phi2 * sin_theta;
	y_axis.Z = cos_phi2; 
	NormVctX( &z_axis );
	NormVctX( &y_axis );
	
	CrossProduct( &y_axis, &z_axis, &x_axis );
	StoreXVector( trafo, &x_axis );
	StoreYVector( trafo, &y_axis );
	StoreZVector( trafo, &z_axis );
	
	ReOrthoNormMtx( trafo );
}

// notification callback when exit props changed ------------------------------
//
PRIVATE 
int	TeleporterModify_StartPropsChanged( GenObject* base )
{
	ASSERT( base != NULL );
	Teleporter *teleporter = (Teleporter *) base;

	if ( teleporter->start_rot_phi > 360.0f ) {
		teleporter->start_rot_phi -= 360.0f;
	}
	if ( teleporter->start_rot_phi < 0.0f ) {
		teleporter->start_rot_phi += 360.0f;
	}
	if ( teleporter->start_rot_theta > 360.0f ) {
		teleporter->start_rot_theta -= 360.0f;
	}
	if ( teleporter->start_rot_theta < 0.0f ) {
		teleporter->start_rot_theta += 360.0f;
	}
		
	// get the trafo for the start rotation
	Xmatrx NewTrans;
	MakeIdMatrx( NewTrans );
	Teleporter_Rotation_Transform( teleporter->start_rot_phi, teleporter->start_rot_theta, NewTrans );
	MakeNonTranslationMatrx( NewTrans, teleporter->ObjPosition );
	
	// set the translational part
	StoreTVector( teleporter->ObjPosition, &teleporter->start );

	// recalc all start dependend data in the teleporter exit 
	TeleporterModify_ExitPropsChanged( teleporter );

	return TRUE;
}


// notification callback when exit props changed ------------------------------
//
PRIVATE
int TeleporterModify_ExitPropsChanged( GenObject* base )
{
	ASSERT( base != NULL );
	Teleporter *teleporter = (Teleporter *) base;

	// modify the position of the teleporter exit
	Xmatrx ChildTrans;
	MakeIdMatrx( ChildTrans );

	// get the trafo for the exit rotation
	Teleporter_Rotation_Transform( teleporter->exit_rot_phi, teleporter->exit_rot_theta, ChildTrans );

	ChildTrans[ 0 ][ 3 ] = teleporter->exit_delta_x;
	ChildTrans[ 1 ][ 3 ] = teleporter->exit_delta_y;
	ChildTrans[ 2 ][ 3 ] = teleporter->exit_delta_z;
#ifndef PARSEC_SERVER
	MtxMtxMUL( teleporter->ObjPosition, ChildTrans, teleporter->child_object->ObjPosition );
	
	TeleporterModify_SplinePropsChanged( base );
#endif
	return TRUE;
}

// draw the interior of the teleporter entry gate -----------------------------
//
void Teleporter_Draw_Entry_Interior( Teleporter* teleporter )
{
#ifndef PARSEC_SERVER
	ASSERT( teleporter != NULL );

	// ensure we have the texture for the start interior	
	if ( teleporter->start_texmap == NULL ) {
		return;
	}
	
	// only draw interior if front facing
	{
		Vector3 telepnormal;
		FetchZVector( teleporter->ObjPosition, &telepnormal );
		
		Vertex3 teleppos;
		FetchTVector( teleporter->ObjPosition, &teleppos );
		
		// get the world->view transform
		Xmatrx CameraInWorldSpace;
		CalcOrthoInverse( ViewCamera, CameraInWorldSpace );
		//FIXME: we should have a GLOBAL storage for the camera in world space
		
		Vertex3 campos;
		FetchTVector( CameraInWorldSpace, &campos );
		
		geomv_t camdot   = -DOT_PRODUCT( &telepnormal, &campos );
		geomv_t telepdot = -DOT_PRODUCT( &telepnormal, &teleppos );

		if ( camdot < telepdot ) {
			return;
		}
	}
	
	// set vertex color to white with premultiplie alpha
	float alpha = ( (float)teleporter->start_tex_alpha ) / 255.0f;
	byte _red   = (int)(255.0 * alpha);
	byte _green = (int)(255.0 * alpha);
	byte _blue  = (int)(255.0 * alpha);
	byte _alpha = teleporter->start_tex_alpha;
	
	// get the width/height in pixels
	geomv_t uWidth  = INT_TO_GEOMV( 1L << teleporter->start_texmap->Width  );
	geomv_t vHeight = INT_TO_GEOMV( 1L << teleporter->start_texmap->Height );
	
	// setup transformation matrix; transform is world->view
	D_LoadIterMatrix( NULL );
	
	IterRectangle3 itrect;
	itrect.flags	 = ITERFLAG_NONDESTRUCTIVE | ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW | ITERFLAG_Z_TO_DEPTH /* | ITERFLAG_ONESIDED */;
	itrect.itertype  = iter_texrgba | iter_specularadd;
	itrect.raststate = rast_zcompare | rast_texwrap | rast_chromakeyoff;
	itrect.rastmask  = rast_nomask;
	itrect.texmap	 = teleporter->start_texmap;
	
	// calculate transformation matrix
	MtxMtxMUL( ViewCamera, teleporter->ObjPosition, DestXmatrx );
	int nVtx = 0;	
	for ( nVtx = 0; nVtx < 4; nVtx++ ) {
		MtxVctMUL( DestXmatrx, &teleporter->start_vtxlist[ nVtx ], (Vertex3*)&itrect.Vtxs[ nVtx ] );
		itrect.Vtxs[ nVtx ].W = GEOMV_1;
		itrect.Vtxs[ nVtx ].U = teleporter->u_delta[ 0 ];
		itrect.Vtxs[ nVtx ].V = teleporter->v_delta[ 0 ];
		itrect.Vtxs[ nVtx ].R = _red;
		itrect.Vtxs[ nVtx ].G = _green;
		itrect.Vtxs[ nVtx ].B = _blue;
		itrect.Vtxs[ nVtx ].A = _alpha;
	}	
	
	
	itrect.Vtxs[ 0 ].U += 0;
	itrect.Vtxs[ 0 ].V += 0;
	
	itrect.Vtxs[ 1 ].U += uWidth;
	itrect.Vtxs[ 1 ].V += 0;
	
	itrect.Vtxs[ 2 ].U += uWidth;
	itrect.Vtxs[ 2 ].V += vHeight;
	
	itrect.Vtxs[ 3 ].U += 0;
	itrect.Vtxs[ 3 ].V += vHeight;
	
	// draw quad
	D_DrawIterRectangle3( &itrect, 0x3f );
	
	// draw the second layer
	for ( nVtx = 0; nVtx < 4; nVtx++ ) {
		MtxVctMUL( DestXmatrx, &teleporter->start_vtxlist[ nVtx ], (Vertex3*)&itrect.Vtxs[ nVtx ] );
		itrect.Vtxs[ nVtx ].W = GEOMV_1;
		itrect.Vtxs[ nVtx ].U = teleporter->u_delta[ 1 ];
		itrect.Vtxs[ nVtx ].V = teleporter->v_delta[ 1 ];
	}
	
	itrect.Vtxs[ 0 ].U += 0;
	itrect.Vtxs[ 0 ].V += 0;
	
	itrect.Vtxs[ 1 ].U += uWidth;
	itrect.Vtxs[ 1 ].V += 0;
	
	itrect.Vtxs[ 2 ].U += uWidth;
	itrect.Vtxs[ 2 ].V += vHeight;
	
	itrect.Vtxs[ 3 ].U += 0;
	itrect.Vtxs[ 3 ].V += vHeight;
	
	//itrect.flags	 = ITERFLAG_NONDESTRUCTIVE | ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW | ITERFLAG_Z_TO_DEPTH;
	
	D_DrawIterRectangle3( &itrect, 0x3f );
	
	// restore identity transformation
	D_LoadIterMatrix( NULL );

#endif
}

// check whether tunnel is visible --------------------------------------------
//
PRIVATE 
int Teleporter_Check_Tunnel_Visibility( Teleporter* teleporter )
{
#ifndef PARSEC_SERVER
	// NOTE: the teleporter cullbox is given as an axis aligned bounding box
	//       relative to the teleporter entry

	CullBox3 cullbox_world;

	MtxVctMUL( teleporter->ObjPosition, (Vector3*)&teleporter->tunnel_cullbox.minmax[ 0 ], (Vector3*)&cullbox_world.minmax[ 0 ] );
	MtxVctMUL( teleporter->ObjPosition, (Vector3*)&teleporter->tunnel_cullbox.minmax[ 3 ], (Vector3*)&cullbox_world.minmax[ 3 ] );
	
	dword cullmask = 0x3f;
	int cull_result = CULL_BoxAgainstVolume( &cullbox_world, World_ViewVolume, &cullmask );

	return !cull_result;
#else
	return TRUE;
#endif
}


// draw the teleporters -------------------------------------------------------
//
PRIVATE
int Teleporter_Draw( void *param )
{
#ifndef PARSEC_SERVER
	ASSERT( param != NULL );
	Teleporter *teleporter = (Teleporter *) param;

	// determine visibility
	int entry_visible  = ( teleporter->VisibleFrame == CurVisibleFrame );
	int exit_visible   = ( teleporter->child_object->VisibleFrame == CurVisibleFrame );
	int tunnel_visible = entry_visible | exit_visible;

	if ( !tunnel_visible ) {
		// cull tunnel against frustum
		tunnel_visible = Teleporter_Check_Tunnel_Visibility( teleporter );
	}

	// check whether to draw the entry interior
	if ( entry_visible ) {
		Teleporter_Draw_Entry_Interior( teleporter );
	}

	// check whether the tunnel is visible
	if ( tunnel_visible ) {

		// draw the tunnel
		Teleporter_Draw_Tunnel( teleporter );

		// draw the spline connecting start and exit ( for debugging only )
		if ( teleporter->tunnel_spline != 0 ) {
			Teleporter_Debug_Show_Tunnel_Spline( teleporter );
		}
	}
#endif
	return TRUE;
}

// macro to set the properties of a itervertex --------------------------------
//
#define SET_ITER_VTX( iter_vtx, in_vect3, u, v, r, g, b, a ) \
	MtxVctMUL( DestXmatrx, (in_vect3), (Vector3*)(iter_vtx) ); \
	(iter_vtx)->W = GEOMV_1; \
	(iter_vtx)->U = u; \
	(iter_vtx)->V = v; \
	(iter_vtx)->R = r; \
	(iter_vtx)->G = g; \
	(iter_vtx)->B = b; \
(iter_vtx)->A = a;

// do a lerp between 2 vectors ------------------------------------------------
//
void VctLerp( Vector3* dst, Vector3* src1, Vector3* src2, float t )
{
	Vector3 diff;
	VECSUB( &diff, src2, src1 );

	dst->X = src1->X + ( t * diff.X );
	dst->Y = src1->Y + ( t * diff.Y );
	dst->Z = src1->Z + ( t * diff.Z );
}


// draw a part of the tunnel connecting teleporter entry with exit ------------
//
PRIVATE
void Teleporter_Draw_Tunnel( Teleporter* teleporter )
{
#ifndef PARSEC_SERVER
	ASSERT( teleporter != NULL );

	if ( teleporter->tunnel_texmap == NULL )
		return;
	
	//NOTE: start_seg and end_seg specify points on the spline ( seperator planes of the tunnel )
	int num_total_spline_segs	= ( teleporter->tunnel_spline_cords + 1 );

	int start_seg = (int)(teleporter->tunnel_cur_anim_step - teleporter->tunnel_len);
	int end_seg   = start_seg + teleporter->tunnel_len;

	if ( start_seg < 0 )
		start_seg = 0;

	if ( end_seg >= num_total_spline_segs ) {
		end_seg = num_total_spline_segs - 1;
	}
	
	// no tunnel to draw
	if ( start_seg == end_seg )
		return;
	
	// tunnel is vanishing at the teleporter exit
	if( start_seg == ( teleporter->tunnel_spline_cords + 1 ) ) {
		return;
	}
	
	int num_tunnel_cords		= ( end_seg - start_seg + 1 ); // we add a segment at the end of the tunnel, for smooth translation of the tunnel
	int num_tunnel_segs			= num_tunnel_cords + 1;
	int num_verts				= 4 * num_tunnel_segs;
	int start_offset			= start_seg * 4;
	int end_offset				= end_seg * 4;
	
	// calculate transformation matrix
	memcpy( DestXmatrx, ViewCamera, sizeof( Xmatrx ) );

	// get the width/height in pixels
	geomv_t uWidth  = INT_TO_GEOMV( 1L << teleporter->tunnel_texmap->Width  );
	geomv_t vHeight = INT_TO_GEOMV( 1L << teleporter->tunnel_texmap->Height );
	
	// calculate the u_delta ( u advance for one cord )
	geomv_t tunnel_height	= teleporter->start_vtxlist[ 2 ].Y - teleporter->start_vtxlist[ 0 ].Y;  
	geomv_t u_delta			= uWidth * ( ( teleporter->tunnel_spline_arclen / teleporter->tunnel_spline_cords ) / tunnel_height );
	
	// we calculate the first/last segment vertices by interpolation between the precalculated segments
	Vector3 first_seg_vtxs[ 4 ];
	Vector3 last_seg_vtxs [ 4 ];
	
	// calculate the interstep 
	geomv_t cur_interseg = ( num_total_spline_segs - 1 ) * teleporter->tunnel_t;

	// clamp to the max
	if ( cur_interseg > ( num_total_spline_segs - 1 ) )
		cur_interseg = ( num_total_spline_segs - 1 );
	
	// calculate the interstep fraction ( that is the fraction between 2 spline points )
	//geomv_t cur_interseg_frac	= cur_interseg - (geomv_t)start_seg;
	geomv_t cur_interseg_frac	= teleporter->tunnel_cur_anim_step - (geomv_t)floor( teleporter->tunnel_cur_anim_step );

	geomv_t u_delta_first_cord = 0;
	geomv_t	u_delta_last_cord  = 0;

	int last_cord_removed = FALSE;

	// determine whether start segment is fixed to teleporter start
	int move_start_seg = teleporter->tunnel_cur_anim_step >= teleporter->tunnel_len;

	// snap to a already calculated point
	if ( cur_interseg_frac < GEOMV_VANISHING ) {
		for( int i = 0; i < 4; i++ ) {
			memcpy( &first_seg_vtxs[ i ], &teleporter->tunnel_verts[ start_offset + i ], sizeof( Vector3 ) );
			memcpy( &last_seg_vtxs[ i ],  &teleporter->tunnel_verts[ end_offset   + i ], sizeof( Vector3 ) );
		}

		u_delta_first_cord = u_delta;
		u_delta_last_cord  = u_delta;

		// we do not draw the last cord as this has len 0
		num_tunnel_cords--;
		num_tunnel_segs--;

		last_cord_removed = TRUE;

	} else {

		// determine whether to do lerping of start/end
		int lerp_start = move_start_seg;
		int lerp_end   = ( end_seg < ( num_total_spline_segs - 1 ) );
		
		// get the lerped position for the first/last segment
		for( int i = 0; i < 4; i++ ) {
			if ( lerp_start ) {
				VctLerp( &first_seg_vtxs[ i ], &teleporter->tunnel_verts[ start_offset + i ], 
					&teleporter->tunnel_verts[ start_offset + 4 + i ], GEOMV_TO_FLOAT( cur_interseg_frac ) );
			} else {
				memcpy( &first_seg_vtxs[ i ],  &teleporter->tunnel_verts[ start_offset + i ], sizeof( Vector3 ) );
			}
			if( lerp_end ) {
				VctLerp( &last_seg_vtxs [ i ], &teleporter->tunnel_verts[ end_offset + i ], 
					&teleporter->tunnel_verts[ end_offset   + 4 + i ], GEOMV_TO_FLOAT( cur_interseg_frac ) );
			} else {
				memcpy( &last_seg_vtxs[ i ],  &teleporter->tunnel_verts[ end_offset + i ], sizeof( Vector3 ) );
			}
		}

		// get the u_delta for the first cord
		Vector3 diff;
		if ( lerp_start ) {
			VECSUB( &diff, &first_seg_vtxs[ 0 ], &teleporter->tunnel_verts[ start_offset + 4 ] );
			geomv_t len = VctLenX( &diff );
			u_delta_first_cord = uWidth * ( len / tunnel_height );
		} else {
			u_delta_first_cord = 0;
		}

		// get the u_delta for the last cord
		if( lerp_end ) {
			VECSUB( &diff, &last_seg_vtxs[ 0 ], &teleporter->tunnel_verts[ end_offset + 4 ] );
			geomv_t len = VctLenX( &diff );
			u_delta_last_cord = uWidth * ( len / tunnel_height );
		} else {
			u_delta_last_cord = u_delta;
		}
	}
	
	// create vertex array
	IterArray3 *itarray = (IterArray3 *) ALLOCMEM(
		(size_t)&((IterArray3*)0)->Vtxs[ num_verts ] );
	if ( itarray == NULL )
		OUTOFMEM( 0 );

	
	itarray->NumVerts	= num_verts;
	itarray->arrayinfo	= ITERARRAY_USE_COLOR | ITERARRAY_USE_TEXTURE | ITERARRAY_GLOBAL_TEXTURE;
	itarray->flags		= ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_DIV_UVW | ITERFLAG_Z_TO_DEPTH;
	itarray->itertype	= iter_texrgba | iter_specularadd; //iter_rgba | iter_alphablend;
	itarray->raststate	= rast_zcompare | rast_texclamp | rast_chromakeyoff;
	itarray->rastmask	= rast_nomask;
	itarray->texmap		= teleporter->tunnel_texmap;
	
	geomv_t u = 0;

	float alpha = ( (float)teleporter->tunnel_tex_alpha ) / 255.0f;
	float _fRed   = 0.0f;
	float _fGreen = 0.0f;
	float _fBlue  = 0.0f;
	
	float divisor = ( end_seg - start_seg ) / 3.0f;
	float _color_delta = ( 255.0 * alpha ) / divisor;
	
	float ramp_segs_start = ( divisor      );
	float ramp_segs_end   = ( divisor * 2.0);

	for( int nSegment = 0; nSegment < num_tunnel_segs; nSegment++ ) {
		int nVtx = nSegment * 4;

		float segment_frac = 0.0f;
		if ( nSegment == ( num_tunnel_segs - 1 ) ) {
			segment_frac = num_tunnel_segs - 2;
		} else if ( nSegment == 0 ) {
			segment_frac = 0;
		} else {
			segment_frac = ( nSegment - cur_interseg_frac );
		}

		// modify the color to do a fade in/out at the beginning and end of the tunnel
		if ( nSegment < ramp_segs_start ) {
			if ( move_start_seg ) {
				_fRed   = segment_frac * _color_delta;
				_fGreen = segment_frac * _color_delta;
				_fBlue  = segment_frac * _color_delta;
			} else {
				_fRed   = nSegment * _color_delta;
				_fGreen = nSegment * _color_delta;
				_fBlue  = nSegment * _color_delta;
			}
		} else if ( segment_frac > ramp_segs_end ) {
			_fRed   = ( ( num_tunnel_segs - 2 ) - segment_frac ) * _color_delta;
			_fGreen = ( ( num_tunnel_segs - 2 ) - segment_frac ) * _color_delta;
			_fBlue  = ( ( num_tunnel_segs - 2 ) - segment_frac ) * _color_delta;
		} else {
			_fRed   = ( 255.0f * alpha );
			_fGreen = ( 255.0f * alpha );
			_fBlue  = ( 255.0f * alpha );
		}

		byte _red   = _fRed   > 255.0 ? 255 : (byte)_fRed;
		byte _green = _fGreen > 255.0 ? 255 : (byte)_fGreen;
		byte _blue  = _fBlue  > 255.0 ? 255 : (byte)_fBlue;
		byte _alpha = teleporter->tunnel_tex_alpha;

		if ( nSegment == 0 ) {

			SET_ITER_VTX( &itarray->Vtxs[ nVtx + 0 ], &first_seg_vtxs[ 0 ], u, vHeight, _red, _green, _blue, _alpha );
			SET_ITER_VTX( &itarray->Vtxs[ nVtx + 1 ], &first_seg_vtxs[ 1 ], u, GEOMV_0, _red, _green, _blue, _alpha );
			SET_ITER_VTX( &itarray->Vtxs[ nVtx + 2 ], &first_seg_vtxs[ 2 ], u, vHeight, _red, _green, _blue, _alpha );
			SET_ITER_VTX( &itarray->Vtxs[ nVtx + 3 ], &first_seg_vtxs[ 3 ], u, GEOMV_0, _red, _green, _blue, _alpha );
			
			u += u_delta_first_cord;
		} else {

			if ( nSegment == ( num_tunnel_segs - 1 ) ) {

				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 0 ], &last_seg_vtxs[ 0 ], u, vHeight, _red, _green, _blue, _alpha );
				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 1 ], &last_seg_vtxs[ 1 ], u, GEOMV_0, _red, _green, _blue, _alpha );
				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 2 ], &last_seg_vtxs[ 2 ], u, vHeight, _red, _green, _blue, _alpha );
				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 3 ], &last_seg_vtxs[ 3 ], u, GEOMV_0, _red, _green, _blue, _alpha );
			
				//u += u_delta;
			} else {
				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 0 ], &teleporter->tunnel_verts[ start_offset + nVtx + 0 ], u, vHeight, _red, _green, _blue, _alpha );
				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 1 ], &teleporter->tunnel_verts[ start_offset + nVtx + 1 ], u, GEOMV_0, _red, _green, _blue, _alpha );
				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 2 ], &teleporter->tunnel_verts[ start_offset + nVtx + 2 ], u, vHeight, _red, _green, _blue, _alpha );
				SET_ITER_VTX( &itarray->Vtxs[ nVtx + 3 ], &teleporter->tunnel_verts[ start_offset + nVtx + 3 ], u, GEOMV_0, _red, _green, _blue, _alpha );

				if ( !last_cord_removed && ( nSegment == ( num_tunnel_segs - 1 - 1 ) ) ) {
					u += ( u_delta - u_delta_last_cord );
				} else {
					u += u_delta;
				}
			}
		}
	}
	
	size_t num_triindxs = num_tunnel_cords * 8 * 3;
	
	dword *vindxs = (dword *) ALLOCMEM( num_triindxs * sizeof( dword ) );
	if ( vindxs == NULL )
		OUTOFMEM( 0 );
	
	
	for( int nCord = 0; nCord < num_tunnel_cords; nCord++ ) {
		for( int nSide = 0; nSide < 4; nSide++ ) {
			vindxs[ ( nCord * 24 ) + ( nSide * 6 ) + 0 ] = ( nCord * 4 ) + ( nSide + 0 );
			vindxs[ ( nCord * 24 ) + ( nSide * 6 ) + 1 ] = ( nCord * 4 ) + ( nSide + 4 );
			vindxs[ ( nCord * 24 ) + ( nSide * 6 ) + 2 ] = ( nCord * 4 ) + 4 + ( ( nSide + 1 ) % 4 );
			
			vindxs[ ( nCord * 24 ) + ( nSide * 6 ) + 3 ] = ( nCord * 4 ) + ( nSide + 0 );
			vindxs[ ( nCord * 24 ) + ( nSide * 6 ) + 4 ] = ( nCord * 4 ) + 4 + ( ( nSide + 1 ) % 4 );
			vindxs[ ( nCord * 24 ) + ( nSide * 6 ) + 5 ] = ( nCord * 4 ) + ( nSide + 1 ) % 4;
		}
	}
	
	// setup transformation matrix
	D_LoadIterMatrix( NULL );
	
	// lock array
	D_LockIterArray3( itarray, 0, itarray->NumVerts );
	
	// draw indexed triangles in a single call (no far-plane clipping!)
	D_DrawIterArrayIndexed( ITERARRAY_MODE_TRIANGLES, num_triindxs , vindxs, 0x3d );
	
	// unlock array
	D_UnlockIterArray();
	
	// restore identity transformation
	D_LoadIterMatrix( NULL );

	// free vertex index array
	FREEMEM( vindxs );
	
	// free vertex array
	FREEMEM( itarray );
#endif
}


// draw a spline line between start & end point -------------------------------
//
void D_DrawSplineLine( int num_steps, const Xmatrx transform, 
					  const Vector3* start, const Vector3* end, 
					  const Vector3* start_tan, const Vector3* end_tan, dword mode )
{
#ifndef PARSEC_SERVER
	ASSERT( start		!= NULL );
	ASSERT( end			!= NULL );
	ASSERT( start_tan	!= NULL );
	ASSERT( end_tan		!= NULL );

	ASSERT( ( mode >= D_DRAWLINE_STYLE_DEFAULT ) && ( mode <= D_DRAWLINE_STYLE_STIPPLED ) );

	IterLine3 itline;
	itline.NumVerts  = 2;
	
	itline.flags	 = ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH | ITERFLAG_NONDESTRUCTIVE;
	itline.flags    |= ( ( mode & 0x7 ) << 12 ); // map line drawing styles to iter line drawin styles
	
	itline.itertype  = iter_rgba;
	itline.raststate = rast_zbuffer;
	itline.rastmask  = rast_nomask;
	
	itline.Vtxs[ 0 ].W	   = GEOMV_1;
	itline.Vtxs[ 0 ].R 	   = 255;
	itline.Vtxs[ 0 ].G 	   = 255;
	itline.Vtxs[ 0 ].B 	   = 255;
	itline.Vtxs[ 0 ].A 	   = 255;
	itline.Vtxs[ 0 ].flags = ITERVTXFLAG_NONE;
	
	itline.Vtxs[ 1 ].W	   = GEOMV_1;
	itline.Vtxs[ 1 ].R 	   = 255;
	itline.Vtxs[ 1 ].G 	   = 255;
	itline.Vtxs[ 1 ].B 	   = 255;
	itline.Vtxs[ 1 ].A 	   = itline.Vtxs[ 0 ].A;
	itline.Vtxs[ 1 ].flags = ITERVTXFLAG_NONE;
	
	// setup transformation matrix
	D_LoadIterMatrix( NULL );

	// init the hermite arclen data
	Hermite_ArcLen* hermite_data;
	hermite_data = Hermite_ArcLen_InitData( num_steps * 10, start, end, start_tan, end_tan );
	
	for( int step = 0; step < num_steps; step++ ) {
		
		float s = (float)step / (float)( num_steps - 1 );
		
		// calculate the interpolated pos
		Vector3 interp;
		Hermite_ArcLen_Interpolate( hermite_data, s, &interp );
		
		if( step == 0 ) {
			
			MtxVctMUL( transform, start,	(Vertex3*)&itline.Vtxs[ 0 ] );
			MtxVctMUL( transform, &interp,	(Vertex3*)&itline.Vtxs[ 1 ] );
			
		} else {
			memcpy( &itline.Vtxs[ 0 ], &itline.Vtxs[ 1 ], sizeof ( IterVertex3 ) );
			
			MtxVctMUL( transform, &interp,			(Vertex3*)&itline.Vtxs[ 1 ] );
		}

		if ( ( step % 2 ) == 0 ) {
			SETRGBA( &itline.Vtxs[ 0 ], 255,0,0,255 );
			SETRGBA( &itline.Vtxs[ 1 ], 255,0,0,255 );
		} else {
			SETRGBA( &itline.Vtxs[ 0 ], 255,255,255,255 );
			SETRGBA( &itline.Vtxs[ 1 ], 255,255,255,255 );
		}
		
		D_DrawIterLine3( &itline, 0x00 );
	}

	// kill the hermite arclen data
	Hermite_ArcLen_KillData( hermite_data );
	
	// restore identity transformation
	D_LoadIterMatrix( NULL );
#endif

}

// make a cube out of a cullbox -----------------------------------------------
//
PRIVATE
void CubeFromCullBox3( CullBox3* cullbox, Vertex3* vtxs ) 
{
	ASSERT( cullbox != NULL );
	ASSERT( vtxs    != NULL );
	
	for( int nVertex = 0; nVertex < 8; nVertex++ ) {
		vtxs[ nVertex ].X = cullbox->minmax[ 0 + ( ( nVertex % 4 ) / 2 ) * 3 ];
		vtxs[ nVertex ].Y = cullbox->minmax[ 1 + ( ( ( ( nVertex % 4 ) % 3 ) != 0 ) ? 3 : 0 ) ];
		vtxs[ nVertex ].Z = cullbox->minmax[ 2 + ( nVertex / 4 ) * 3 ];
	}
}



// show the spline connecting the start with the exit of the teleporter -------
//
PRIVATE
void Teleporter_Debug_Show_Tunnel_Spline( Teleporter* teleporter )
{
#ifndef PARSEC_SERVER
	ASSERT( teleporter != NULL );

	// get the tangents
	Vector3 start_tan, end_tan;
	FetchZVector( teleporter->ObjPosition,					&start_tan );
	FetchZVector( teleporter->child_object->ObjPosition,	&end_tan   );
	
	// scale the tangents
	VECMULS( &start_tan, &start_tan, teleporter->tunnel_tangent_scale1 );
	VECMULS( &end_tan,   &end_tan,   teleporter->tunnel_tangent_scale2 );

	// draw center of tunnel 
	if ( teleporter->tunnel_spline & 1 ) {
		// get the start/end pos
		Vector3 start_pos, end_pos;
		FetchTVector( teleporter->ObjPosition,					&start_pos );
		FetchTVector( teleporter->child_object->ObjPosition,	&end_pos );
		
		// draw the spline showing the center of the tunnel
		D_DrawSplineLine( teleporter->tunnel_spline_cords + 1, ViewCamera, &start_pos, &end_pos, 
			&start_tan, &end_tan, D_DRAWLINE_STYLE_ANTIALIASED );
	}
	
	// draw tunnel boundary
	if ( teleporter->tunnel_spline & 2 ) {
		
		// calculate the boundaries of the tunnel
		Vector3 boundary_start, boundary_end;
		
		for( int nCorner = 0; nCorner < 4; nCorner++ ) {
			
			// transform the start/end boundary points from object to world space
			MtxVctMUL( teleporter->ObjPosition,					&teleporter->start_vtxlist[ nCorner ], &boundary_start );
			MtxVctMUL( teleporter->child_object->ObjPosition,	&teleporter->start_vtxlist[ nCorner ], &boundary_end   );
			
			// draw the boundary spline
			D_DrawSplineLine( teleporter->tunnel_spline_cords + 1, ViewCamera, &boundary_start, &boundary_end, 
				&start_tan, &end_tan, D_DRAWLINE_STYLE_ANTIALIASED );
		}
	}
	
	// draw tunnel boundary rects
	if ( teleporter->tunnel_spline & 4 ) {

		ASSERT( teleporter->tunnel_verts != NULL );
		
		int num_steps = ( teleporter->tunnel_spline_cords + 1 );
		Vertex3* vtxs = teleporter->tunnel_verts;
	
		// setup transformation matrix
		D_LoadIterMatrix( NULL );
		
		for( int step = 0; step < num_steps; step++ ) {
			
			IterLine3 itline;
			itline.NumVerts  = 2;
			
			itline.flags	 = ITERFLAG_LS_ANTIALIASED | ITERFLAG_Z_DIV_XYZ | ITERFLAG_Z_TO_DEPTH | ITERFLAG_NONDESTRUCTIVE;
			itline.itertype  = iter_rgba;
			itline.raststate = rast_zbuffer;
			itline.rastmask  = rast_nomask;
			
			itline.Vtxs[ 0 ].W	   = GEOMV_1;
			itline.Vtxs[ 0 ].R 	   = 0;
			itline.Vtxs[ 0 ].G 	   = 255;
			itline.Vtxs[ 0 ].B 	   = 0;
			itline.Vtxs[ 0 ].A 	   = 255;
			itline.Vtxs[ 0 ].flags = ITERVTXFLAG_NONE;
			
			itline.Vtxs[ 1 ].W	   = GEOMV_1;
			itline.Vtxs[ 1 ].R 	   = 0;
			itline.Vtxs[ 1 ].G 	   = 255;
			itline.Vtxs[ 1 ].B 	   = 0;
			itline.Vtxs[ 1 ].A 	   = itline.Vtxs[ 0 ].A;
			itline.Vtxs[ 1 ].flags = ITERVTXFLAG_NONE;
			
			for( int nCorner = 0; nCorner < 4; nCorner++ ) {

				if( nCorner != 3 ) {
					
					MtxVctMUL( ViewCamera, &vtxs[ nCorner     ],	(Vertex3*)&itline.Vtxs[ 0 ] );
					MtxVctMUL( ViewCamera, &vtxs[ nCorner + 1 ],	(Vertex3*)&itline.Vtxs[ 1 ] );
					
				} else {
					memcpy( &itline.Vtxs[ 0 ], &itline.Vtxs[ 1 ], sizeof ( IterVertex3 ) );
					MtxVctMUL( ViewCamera, &vtxs[ 0 ], (Vertex3*)&itline.Vtxs[ 1 ] );
				}
				
				D_DrawIterLine3( &itline, 0x00 );
			}
		
			// advance to next rectangle
			vtxs += 4;
		}

		// restore identity transformation
		D_LoadIterMatrix( NULL );
	}

	// draw the frames for start/exit
	if ( teleporter->tunnel_spline & 8 ) {
		Vector3 pos;
		
		FetchTVector( teleporter->ObjPosition, &pos );
		D_FrameOfReference( teleporter->start_frame, &pos );

		FetchTVector( teleporter->child_object->ObjPosition, &pos );
		D_FrameOfReference( teleporter->end_frame, &pos );
		//Draw_FrameOfReference( teleporter->ObjPosition, NULL );
	}

	// draw the frames making up the spline 
	if ( teleporter->tunnel_spline & 16 ) {

		int num_steps = ( teleporter->tunnel_spline_cords + 1 );

		for( int step = 0; step < num_steps; step++ ) {
			D_FrameOfReference( teleporter->spline_frames[ step ], NULL );
		}
	}

	// draw the bounding box of the tunnel
	if ( teleporter->tunnel_spline & 32 ) {

		CullBox3* cullbox = &teleporter->tunnel_cullbox;

		Vector3 vtxs_ospc[ 8 ];
		CubeFromCullBox3( cullbox, vtxs_ospc );


		// transform from object to worldspace
		Vector3 vtxs[ 8 ];
		int nVertex = 0;
		for( nVertex = 0; nVertex < 8; nVertex++ ) {
			MtxVctMUL( teleporter->ObjPosition, &vtxs_ospc[ nVertex ], &vtxs[ nVertex ] );
		}

		colrgba_s color;
		SETRGBA( &color, 0, 0, 255, 255 );
		int nLine = 0;	
		for( nLine = 0; nLine < 8; nLine++ ) {
			D_LineWorld( &vtxs[ 4 * ( nLine / 4 ) + nLine % 4 ], &vtxs[ 4 * ( nLine / 4 ) + ( nLine + 1 ) % 4 ], &color );	
		}
		for( nLine = 0; nLine < 4; nLine++ ) {
			D_LineWorld( &vtxs[ nLine ], &vtxs[ 4 + nLine ], &color );
		}
	}
#endif

	return;
}

// type fields init function for teleporter -------------------------------------
//
PRIVATE
void TeleporterInitType( CustomObject *base )
{
	ASSERT( base != NULL );
	
	Teleporter *teleporter = (Teleporter *) base;
	
	teleporter->active					= TRUE;

	teleporter->exit_delta_x			= 0;
	teleporter->exit_delta_y			= 0;
	teleporter->exit_delta_z			= 100;
	
	teleporter->exit_rot_phi			= 0;
	teleporter->exit_rot_theta			= 0;

	teleporter->actoffset				= 10;
	teleporter->act_cone_angle			= 30;

	teleporter->u_variation[ 0 ]		= 0.05;		// percentage of texture space
	teleporter->v_variation[ 0 ]		= 0.05;
	teleporter->u_variation[ 1 ]		= 0.03;		// percentage of texture space
	teleporter->v_variation[ 1 ]		= 0.03;

	teleporter->start_tex_sec_layer		= 1;
	teleporter->start_tex_alpha			= 128;

	teleporter->tunnel_slerp			= 1;		// do slerp
	teleporter->tunnel_lerp				= 2;		// do hermite lerp

	teleporter->tunnel_tangent_scale1	= 300;
	teleporter->tunnel_tangent_scale2	= 300;

	teleporter->tunnel_anim_speed		= 2;

	teleporter->tunnel_spline			= FALSE;
	teleporter->tunnel_spline_cords		= 30;
	
	strcpy( teleporter->tex_name_interior, TELEPORTER_DEFAULT_TEX_NAME_INTERIOR );
	strcpy( teleporter->tex_name_tunnel,   TELEPORTER_DEFAULT_TEX_NAME_TUNNEL );

	teleporter->start_texmap			= NULL;
	teleporter->tunnel_texmap			= NULL;
	
	teleporter->start_vtxlist			= NULL;
	teleporter->start_texmap			= NULL;
	teleporter->tunnel_verts			= NULL;
	teleporter->tunnel_spline_arclen    = GEOMV_0;

	teleporter->tunnel_tex_alpha		= 128;
	teleporter->id				=0;
}

// teleporter constructor (class instantiation) ---------------------------------
//
PRIVATE
void TeleporterInstantiate( CustomObject *base )
{
	ASSERT( base != NULL );
	Teleporter *teleporter = (Teleporter *) base;
	
	teleporter->start_vtxlist	= (Vertex3*)ALLOCMEM( 4 * sizeof( Vertex3 ) );
	memset( teleporter->start_vtxlist, 0, 4 * sizeof( Vertex3 ) );

	// init the vertices making up the filled interior of the teleporter start
	teleporter->start_vtxlist[ 0 ].X = (geomv_t)-TELEP_BOUNDARY_HORIZ;
	teleporter->start_vtxlist[ 0 ].Y = (geomv_t)-TELEP_BOUNDARY_VERT;
	teleporter->start_vtxlist[ 0 ].Z = GEOMV_0;
	
	teleporter->start_vtxlist[ 1 ].X = (geomv_t)TELEP_BOUNDARY_HORIZ;
	teleporter->start_vtxlist[ 1 ].Y = (geomv_t)-TELEP_BOUNDARY_VERT;
	teleporter->start_vtxlist[ 1 ].Z = GEOMV_0;
	
	teleporter->start_vtxlist[ 2 ].X = (geomv_t)TELEP_BOUNDARY_HORIZ;
	teleporter->start_vtxlist[ 2 ].Y = (geomv_t)TELEP_BOUNDARY_VERT;
	teleporter->start_vtxlist[ 2 ].Z = GEOMV_0;
	
	teleporter->start_vtxlist[ 3 ].X = (geomv_t)-TELEP_BOUNDARY_HORIZ;
	teleporter->start_vtxlist[ 3 ].Y = (geomv_t)TELEP_BOUNDARY_VERT;
	teleporter->start_vtxlist[ 3 ].Z = GEOMV_0;
	
	// summon the child teleporter ( exit ) as plain geometry type
	dword objclass = OBJ_FetchObjectClassId( TELEPORTER_EXIT_MODEL_NAME );
	ASSERT( objclass != CLASS_ID_INVALID );
	if ( objclass >= (dword)NumObjClasses ) {
		CON_AddLine( telep_exit_class_not_found );
		return;
	}
	
	if ( objclass != CLASS_ID_INVALID ) {

		Xmatrx ChildTrans;
		MakeIdMatrx( ChildTrans );

		// translate the teleporter exit 
		ChildTrans[ 0 ][ 3 ] = teleporter->exit_delta_x;
		ChildTrans[ 1 ][ 3 ] = teleporter->exit_delta_y;
		ChildTrans[ 2 ][ 3 ] = teleporter->exit_delta_z;
		
		// get the trafo for the exit rotation
		Teleporter_Rotation_Transform( teleporter->exit_rot_phi, teleporter->exit_rot_theta, ChildTrans );

		Xmatrx ChildPos;
		MtxMtxMUL( teleporter->ObjPosition, ChildTrans, ChildPos );

#ifndef PARSEC_SERVER
		// summon a object from a class ( do not show stargate )
		teleporter->child_object = SummonObject( objclass, ChildPos );
#else
		teleporter->child_object = TheWorld->CreateObject( objclass, ChildPos, PLAYERID_SERVER );
#endif
	}

	// get the texture maps and calc the max u/v deltas
	TeleporterModify_TexPropsChanged( teleporter );

	// init the angles
	teleporter->u_phi[ 0 ] = DEG_TO_BAMS( RAND() % 360 );
	teleporter->v_phi[ 0 ] = DEG_TO_BAMS( RAND() % 360 );
	teleporter->u_phi[ 1 ] = DEG_TO_BAMS( RAND() % 360 );
	teleporter->v_phi[ 1 ] = DEG_TO_BAMS( RAND() % 360 );

	// init the delta angles
	teleporter->u_phi_delta[ 0 ] = DEG_TO_BAMS( 0.1 );
	teleporter->v_phi_delta[ 0 ] = DEG_TO_BAMS( 0.1 );
	teleporter->u_phi_delta[ 1 ] = DEG_TO_BAMS( 0.1 );
	teleporter->v_phi_delta[ 1 ] = DEG_TO_BAMS( 0.1 );

	// init the tunnel frame to the frame of the teleporter start
	memcpy( &teleporter->tunnel_frame, &teleporter->ObjPosition, sizeof( Xmatrx ) );

	// init the interpolation t
	//teleporter->tunnel_t = 0.0f;
	teleporter->tunnel_t = (float)RAND() / (float)0x7fff/*RAND_MAX*/;

	teleporter->spline_frames = NULL;
	
	// calculate the verts for the tunnel
	Teleporter_Calc_Tunnel_Verts( teleporter );

	// set the properties for the tunnel animation
	teleporter->tunnel_len			 = 10;
	teleporter->tunnel_cur_anim_step = GEOMV_0;

	// get the initial start position
	FetchTVector( teleporter->ObjPosition, &teleporter->start );

	// we need to get the angles out of the matrix
	geomv_t startx = teleporter->ObjPosition[0][2];
	geomv_t starty = teleporter->ObjPosition[1][2];
	geomv_t startz = teleporter->ObjPosition[1][2];

	// yay maths....
	// store the rotational spherical coords.
	teleporter->start_rot_phi			= atan(starty/startx);
	teleporter->start_rot_theta			= atan((sqrt(powf(startx,2)+powf(starty,2))/startz));
}

#ifndef PARSEC_SERVER
// callback type and flags ----------------------------------------------------
//
static int callback_type = CBTYPE_DRAW_CUSTOM_ITER | CBFLAG_REMOVE;
#endif


// teleporter destructor (instance destruction) ---------------------------------
//
PRIVATE
void TeleporterDestroy( CustomObject *base )
{
	ASSERT( base != NULL );
	Teleporter *teleporter = (Teleporter *) base;
#ifndef PARSEC_SERVER
	// stop any playing teleporter sound
	AUD_TeleporterOff( teleporter );
#endif	
	// destroy attached vertex info
	ASSERT( teleporter->start_vtxlist != NULL );
	ASSERT( teleporter->tunnel_verts  != NULL );
	
	FREEMEM( teleporter->start_vtxlist );
	FREEMEM( teleporter->tunnel_verts  );

	teleporter->start_vtxlist = NULL;
	teleporter->tunnel_verts  = NULL;

	if ( teleporter->spline_frames != NULL ) {
		FREEMEM( teleporter->spline_frames );
	}

#ifndef PARSEC_SERVER
	// ensure pending callbacks are destroyed to avoid
	// calling them with invalid pointers
	int numremoved = CALLBACK_DestroyCallback( callback_type, (void *) base );
	//ASSERT( numremoved <= 1 );
#endif
}


// teleporter animation callback ----------------------------------------------
//
PRIVATE
int TeleporterAnimate( CustomObject *base )
{
#ifndef PARSEC_SERVER
	ASSERT( base != NULL );
	Teleporter *teleporter = (Teleporter *) base;
	
	// do nothing if teleporter inactive
	if ( !teleporter->active )
		return TRUE;
	
	teleporter->u_phi[ 0 ] += teleporter->u_phi_delta[ 0 ] * CurScreenRefFrames;
	teleporter->v_phi[ 0 ] += teleporter->v_phi_delta[ 0 ] * CurScreenRefFrames;
	teleporter->u_phi[ 1 ] += teleporter->u_phi_delta[ 1 ] * CurScreenRefFrames;
	teleporter->v_phi[ 1 ] += teleporter->v_phi_delta[ 1 ] * CurScreenRefFrames;
	
	sincosval_s sincosv;
	GetSinCos( teleporter->u_phi[ 0 ], &sincosv );
	teleporter->u_delta[ 0 ] = GEOMV_MUL( teleporter->u_maxdelta[ 0 ], sincosv.cosval );
	GetSinCos( teleporter->v_phi[ 0 ], &sincosv );
	teleporter->v_delta[ 0 ] = GEOMV_MUL( teleporter->v_maxdelta[ 0 ], sincosv.sinval );
	GetSinCos( teleporter->u_phi[ 1 ], &sincosv );
	teleporter->u_delta[ 1 ] = GEOMV_MUL( teleporter->u_maxdelta[ 1 ], sincosv.cosval );
	GetSinCos( teleporter->v_phi[ 1 ], &sincosv );
	teleporter->v_delta[ 1 ] = GEOMV_MUL( teleporter->v_maxdelta[ 1 ], sincosv.sinval );
	
	// animate the tunnel frame
	Teleporter_Animate_Tunnel( teleporter );
	
	// register the drawing callback for drawing the interior of the teleporter
	CALLBACK_RegisterCallback( callback_type, Teleporter_Draw, (void *) base );
#endif
	return TRUE;
}

// animate the tunnel frame ---------------------------------------------------
//
PRIVATE
void Teleporter_Animate_Tunnel( Teleporter* teleporter )
{
#ifndef PARSEC_SERVER
	ASSERT( teleporter != NULL );

	// do nothing if teleporter inactive
	if ( !teleporter->active )
		return;

	// reset the interpolation if it was at the end of the animation
	if ( teleporter->tunnel_t == 1.0f ) {
		teleporter->tunnel_t = 0.0f;
	} else {
		// advance the t paramater
		teleporter->tunnel_t += (float)CurScreenRefFrames / ( (float)FRAME_MEASURE_TIMEBASE * (float)teleporter->tunnel_anim_speed );
		if ( teleporter->tunnel_t > 1.0f ) {
			teleporter->tunnel_t = 1.0f;
		}
	}
	
	int num_steps = ( teleporter->tunnel_spline_cords + 1 );

	teleporter->tunnel_cur_anim_step = ( num_steps - 1 + teleporter->tunnel_len ) * teleporter->tunnel_t;

	if ( teleporter->tunnel_cur_anim_step >= num_steps + teleporter->tunnel_len ) {
		teleporter->tunnel_cur_anim_step = num_steps - 1 + teleporter->tunnel_len;
	}

#ifdef ANIMATE_TUNNEL_FRAME
	
	// reset the interpolation if it was at the end of the animation
	if ( teleporter->tunnel_t == 1.0f ) {
		teleporter->tunnel_t = 0.0f;
		memcpy( teleporter->tunnel_frame, teleporter->ObjPosition, sizeof( Xmatrx ) );
	}

	// advance the t paramater
	teleporter->tunnel_t += (float)CurScreenRefFrames / ( (float)FRAME_MEASURE_TIMEBASE * (float)teleporter->tunnel_anim_speed );
	if ( teleporter->tunnel_t > 1.0f ) {
		teleporter->tunnel_t = 1.0f;
	}

	// get the position of the tunnel frame ( source )
	Vector3 tunnel_pos;
	FetchTVector( teleporter->tunnel_frame, &tunnel_pos ); 
	
	// get the position of the teleporter exit ( destination )
	Vector3 exit_pos;
	FetchTVector( teleporter->child_object->ObjPosition, &exit_pos );
	
	// check whether to do the SLERP
	if ( teleporter->tunnel_slerp == 1 ) {

		// we only want todo the quaterionien slerp on the orientations not on the poses
		Xmatrx start_frame, end_frame;
		MakeNonTranslationMatrx( teleporter->ObjPosition,				start_frame );
		MakeNonTranslationMatrx( teleporter->child_object->ObjPosition, end_frame );

		// get slerp frame
		QuaternionSlerpFrames( teleporter->tunnel_frame, start_frame, end_frame, teleporter->tunnel_t );
		
/*		// get source orientation quaternion
		Quaternion srcquat;
		QuaternionFromMatrx( &srcquat, start_frame );
		QuaternionMakeUnit( &srcquat );
		
		// get destination orientation quaternion
		Quaternion dstquat;
		QuaternionFromMatrx( &dstquat, end_frame );
		QuaternionMakeUnit( &dstquat );
		
		// do slerp from src to dst (filter output this frame)
		Quaternion slerpquat;
		QuaternionSlerp( &slerpquat, &srcquat, &dstquat, teleporter->tunnel_t );
		QuaternionMakeUnit( &slerpquat );
		
		// fill R part of view camera matrix (filtered orientation this frame)
		MatrxFromQuaternion( teleporter->tunnel_frame, &slerpquat );
*/
	}

	// check whether to do simple LERP
	if ( teleporter->tunnel_lerp == 1 ) {
		
		// get the position of the tunnel frame
		FetchTVector( teleporter->tunnel_frame, &tunnel_pos ); 
		
		// interpolation vector from last filtered to new filtered t
		float lerpx = GEOMV_TO_FLOAT( exit_pos.X - tunnel_pos.X ) * teleporter->tunnel_t;
		float lerpy = GEOMV_TO_FLOAT( exit_pos.Y - tunnel_pos.Y ) * teleporter->tunnel_t;
		float lerpz = GEOMV_TO_FLOAT( exit_pos.Z - tunnel_pos.Z ) * teleporter->tunnel_t;
		
		Vector3 deltatvec;
		deltatvec.X = FLOAT_TO_GEOMV( lerpx );
		deltatvec.Y = FLOAT_TO_GEOMV( lerpy );
		deltatvec.Z = FLOAT_TO_GEOMV( lerpz );
		
		VECADD( &tunnel_pos, &deltatvec, &tunnel_pos );
	} 
	
	// check whether to do Hermite LERP for the position
	if ( teleporter->tunnel_lerp == 2 ) {
		
		Vector3 T1;
		Vector3 T2;
		FetchZVector( teleporter->ObjPosition,				 &T1 );
		FetchZVector( teleporter->child_object->ObjPosition, &T2 );
		
		Vector3 teleporter_pos;
		FetchTVector( teleporter->ObjPosition, &teleporter_pos );
		
		VECMULS( &T1, &T1, teleporter->tunnel_tangent_scale1 );
		VECMULS( &T2, &T2, teleporter->tunnel_tangent_scale2 );
		
		Hermite_Interpolate( &tunnel_pos, teleporter->tunnel_t, &teleporter_pos, &exit_pos, &T1, &T2 );
	}
	
	// store the tunnel pos back to the tunnel frame
	StoreTVector( teleporter->tunnel_frame, &tunnel_pos );

#endif // ANIMATE_TUNNEL_FRAME
#endif
}


// check whether a ship is in range of a teleporter entry ---------------------
//
PUBLIC
int Teleporter_ShipInRange( Teleporter *teleporter, ShipObject *ship )
{
	ASSERT( teleporter != NULL );
	ASSERT( ship != NULL );
	
	//NOTE:
	// the ship is treated as a sphere for activation
	// range detection.
	
	Vector3 telepnormal;
	FetchZVector( teleporter->ObjPosition, &telepnormal );
	
	Vertex3 teleppos;
	FetchTVector( teleporter->ObjPosition, &teleppos );
	
	Vertex3 shippos;
	FetchTVector( ship->ObjPosition, &shippos );
	
	geomv_t shipdot  = -DOT_PRODUCT( &telepnormal, &shippos );
	geomv_t telepdot = -DOT_PRODUCT( &telepnormal, &teleppos );
	geomv_t distance = shipdot - telepdot;
	
	// not in range if ship in wrong halfspace ( already behind teleporter )
	if ( GEOMV_NEGATIVE( distance ) ) {
		return FALSE;
	}

	// check whether inside of boundingsphere around teleporter
	Vector3 telepship;
	VECSUB( &telepship, &shippos, &teleppos );

	geomv_t telepship_len = VctLenX( &telepship );
	if ( telepship_len > teleporter->BoundingSphere ) {
		return FALSE;
	}
	
	// inside the activation distance ?
	if ( distance < teleporter->actoffset ) {
		
		// check whether inside of cone
		Vector3 shipnormal;
		FetchZVector( ship->ObjPosition, &shipnormal );
		
		shipdot = DOT_PRODUCT( &telepnormal, &shipnormal );
		sincosval_s sincosv;
		GetSinCos( DEG_TO_BAMS( teleporter->act_cone_angle ), &sincosv );
		MSGOUT("%d %f %f",( shipdot >= sincosv.cosval ),shipdot,sincosv.cosval); //CrazySpence Debug
		return ( shipdot >= sincosv.cosval );
		
	} else {
		return FALSE;
	}
}



// teleporter collision callback ------------------------------------------------
//
PRIVATE
int TeleporterCollide( CustomObject *base )
{
	ASSERT( base != NULL );
	
	Teleporter *teleporter = (Teleporter *) base;
	
	// do nothing if teleporter inactive
	if ( !teleporter->active )
		return TRUE;
	
	// get the world->object transform
	Xmatrx World2Telep;
	CalcOrthoInverse( teleporter->ObjPosition, World2Telep );

#ifndef PARSEC_SERVER
	// determine whether MyShip is in audio range of teleporter
	Vector3 diff;
	Vertex3 teleppos;
	Vertex3 shippos;
	FetchTVector( teleporter->ObjPosition, &teleppos );
	FetchTVector( MyShip->ObjPosition, &shippos );
	VECSUB( &diff, &teleppos, &shippos );
	geomv_t distance = VctLenX( &diff );

	if ( distance < MAX_VOLUME_DISTANCE_TELEPORTER ) {
		AUD_Teleporter( teleporter );
	} else {
		AUD_TeleporterOff( teleporter );
	}
	
	
	// first check local ship
	if ( Teleporter_ShipInRange( teleporter, MyShip ) ) {
		
		// get the world->view transform
		Xmatrx CameraInWorldSpace;
		CalcOrthoInverse( ShipViewCamera, CameraInWorldSpace );
		
		// transform the camera to object space
		Xmatrx CameraInTelepSpace;
		MtxMtxMUL( World2Telep, CameraInWorldSpace, CameraInTelepSpace );
		
		// transform camera to world space ( using the teleporter exit frame )
		MtxMtxMUL( teleporter->child_object->ObjPosition, CameraInTelepSpace, CameraInWorldSpace );
		
		// transform camera to view space
		CalcOrthoInverse( CameraInWorldSpace, ShipViewCamera );
		
		CameraMoved = TRUE;
	}
	
#ifndef DONT_MOVE_REMOTE_SHIPS
	if(!NET_ProtocolGMSV()){
		// walk all ships and check for teleportings
		ShipObject *shippo = FetchFirstShip();
		for ( ; shippo; shippo = (ShipObject *) shippo->NextObj ) {
			if ( shippo != MyShip ) {
				if ( Teleporter_ShipInRange( teleporter, shippo ) ) {

					// transform the ship to (teleporter) object space
					Xmatrx ShipInTelepSpace;
					MtxMtxMUL( World2Telep, shippo->ObjPosition, ShipInTelepSpace );

					// transform ship to world space ( using the teleporter exit frame )
					MtxMtxMUL( teleporter->child_object->ObjPosition, ShipInTelepSpace, shippo->ObjPosition );
				}
			}
		}
	}
#endif // !DONT_MOVE_REMOTE_SHIPS
/* XXX: remove
#else // !PARSEC_SERVER
// TODO code the server side collision code
	// walk all ships and check for teleportings
	ShipObject *shippo = FetchFirstShip();
	for ( ; shippo; shippo = (ShipObject *) shippo->NextObj ) {
			if ( Teleporter_ShipInRange( teleporter, shippo ) ) {

				// transform the ship to (teleporter) object space
				Xmatrx ShipInTelepSpace;
				MtxMtxMUL( World2Telep, shippo->ObjPosition, ShipInTelepSpace );

				// transform ship to world space ( using the teleporter exit frame )
				MtxMtxMUL( teleporter->child_object->ObjPosition, ShipInTelepSpace, shippo->ObjPosition );
			}
	}*/
#endif // !PARSEC_SERVER
	
	return TRUE;
}

// handle persistency ---------------------------------------------------------
//
int TeleporterPersistFromStream( CustomObject* base, int tostream, void* rl )
{
#ifdef PARSEC_SERVER
	ASSERT( base != NULL );
	ASSERT( tostream == TRUE );
	Teleporter* teleporter = (Teleporter*) base;

	// determine size in packet
	size_t size = E_REList::RmEvGetSizeFromType(RE_TELEPORTER);;

	// write to RE list
	if ( rl != NULL ) {

		E_REList* pREList = (E_REList*)rl;

		RE_Teleporter* re_tlp = (RE_Teleporter*)pREList->NET_Allocate( RE_TELEPORTER );
		ASSERT( re_tlp != NULL );

		re_tlp->pos[ 0 ]	= teleporter->ObjPosition[ 0 ][ 3 ];
		re_tlp->pos[ 1 ]	= teleporter->ObjPosition[ 1 ][ 3 ];
		re_tlp->pos[ 2 ]	= teleporter->ObjPosition[ 2 ][ 3 ];

		re_tlp->dir[ 0 ]	= teleporter->ObjPosition[ 0 ][ 2 ];
		re_tlp->dir[ 1 ]	= teleporter->ObjPosition[ 1 ][ 2 ];
		re_tlp->dir[ 2 ]	= teleporter->ObjPosition[ 2 ][ 2 ];

		re_tlp->exit_delta_x = teleporter->exit_delta_x;
		re_tlp->exit_delta_y = teleporter->exit_delta_y;
		re_tlp->exit_delta_z = teleporter->exit_delta_z;

		re_tlp->id	= teleporter->id;

		re_tlp->exit_rot_phi = teleporter->exit_rot_phi;
		re_tlp->exit_rot_theta = teleporter->exit_rot_theta;

	}

	return size;
#else
	return 0;
#endif
}

PUBLIC void TeleporterPropsChanged(GenObject* base){
	TeleporterModify_StartPropsChanged(base);


}

// register object type for Teleporter ------------------------------------------
//
PRIVATE
void TeleporterRegisterCustomType()
{
	custom_type_info_s info;
	memset( &info, 0, sizeof( info ) );

	info.type_name			= "teleporter";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( Teleporter );
	info.type_template		= NULL;
	info.type_flags			= CUSTOM_TYPE_DEFAULT;
	info.callback_init		= TeleporterInitType;
	info.callback_instant	= TeleporterInstantiate;
	info.callback_destroy	= TeleporterDestroy;
	info.callback_animate	= TeleporterAnimate;
	info.callback_collide	= TeleporterCollide;
	info.callback_notify	= NULL;
	info.callback_persist   = TeleporterPersistFromStream;

	teleporter_type = OBJ_RegisterCustomType( &info );
	CON_RegisterCustomType( info.type_id, Teleporter_PropList );

	memset( &info, 0, sizeof( info ) );
	
	info.type_name			= "telep_exit";
	info.type_id			= 0x00000000;
	info.type_size			= sizeof( Teleporter );
	info.type_template		= NULL;
	info.type_flags			= CUSTOM_TYPE_DEFAULT | CUSTOM_TYPE_NOT_PERSISTANT;
	info.callback_init		= NULL;
	info.callback_instant	= NULL;
	info.callback_destroy	= NULL;
	info.callback_animate	= NULL;
	info.callback_collide	= NULL;
	info.callback_notify	= NULL;
	info.callback_persist   = NULL;

	OBJ_RegisterCustomType( &info );
}

// key table for "tp.create" command --------------------------------------------
//
key_value_s tp_command_keys[] = {
	{"id",			NULL, 	KEYVALFLAG_NONE },
	{ "pos",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "dir",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "expos",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "exdir",		NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,			NULL,	KEYVALFLAG_NONE				},
};

enum {
	KEY_TELEPORTER_ID,
	KEY_TELEPORTER_POS,
	KEY_TELEPORTER_DIR,
	KEY_TELEPORTER_EXPOS,
	KEY_TELEPORTER_EXDIR
};

#ifdef PARSEC_SERVER
// console command for specifying teleporters in game server mode --------------------------------
//
PRIVATE
int Cmd_TP_CREATE( char* tp_create_command )
{
	//NOTE:
	//CONCOM:
	// tp_create_command	::= 'tp.create' [<pos_spec>] [<dir_spec>] [expos_spec] [exdir_spec]
	// pos_spec			::= 'pos' '(' <float> <float> <float> ')'
	// dir_spec			::= 'dir' '(' <float> <float> <float> ')'
	// expos_spec	::= 'expos' '(' <float> <float> <float> ')'
	// exdir_spec			::= 'exdir' '(' <float> <float> <float> ')'


	ASSERT( tp_create_command != NULL );
	HANDLE_COMMAND_DOMAIN( tp_create_command );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( tp_command_keys,tp_create_command ) ) {
		return TRUE;
	}



	// parse position
	Vector3 pos_spec;
	if ( tp_command_keys[ KEY_TELEPORTER_POS ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_POS ], (float*)&pos_spec.X, 3, 3 ) ) {
			CON_AddLine( "position invalid" );
			return TRUE;
		}
	} else {
		//FIXME: constants
		pos_spec.X = ( RAND() % 1000 ) - 500;
		pos_spec.Y = ( RAND() % 1000 ) - 500;
		pos_spec.Z = ( RAND() % 1000 ) - 500;
		pos_spec.VisibleFrame = 0;
	}


	// parse exit position
	Vector3 expos_spec;
	if ( tp_command_keys[ KEY_TELEPORTER_EXPOS ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_EXPOS ], (float*)&expos_spec.X, 3, 3 ) ) {
			CON_AddLine( "position invalid" );
			return TRUE;
		}
	} else {
		//FIXME: constants
		expos_spec.X = ( RAND() % 1000 ) - 500;
		expos_spec.Y = ( RAND() % 1000 ) - 500;
		expos_spec.Z = ( RAND() % 1000 ) - 500;
		expos_spec.VisibleFrame = 0;
	}

	// parse direction
	Vector3 dir_spec;
	if ( tp_command_keys[ KEY_TELEPORTER_DIR ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_DIR ], (float*)&dir_spec.X, 3, 3 ) ) {
			CON_AddLine( "direction invalid" );
			return TRUE;
		}
		if(dir_spec.X == 0 && dir_spec.Y == 0 && dir_spec.Z==0)
			dir_spec.Z=1; // default to Z if all zero
	} else {
		// default to point in z direction
		dir_spec.X = 0.0f;
		dir_spec.Y = 0.0f;
		dir_spec.Z = 1.0f;
		dir_spec.VisibleFrame = 0;
	}

	// parse exit direction
		Vector3 exdir_spec;
		if ( tp_command_keys[ KEY_TELEPORTER_EXDIR ].value != NULL ) {
			if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_EXDIR ], (float*)&exdir_spec.X, 3, 3 ) ) {
				CON_AddLine( "direction invalid" );
				return TRUE;
			}
			if(exdir_spec.X == 0 && dir_spec.Y == 0 && exdir_spec.Z==0)
				exdir_spec.Z=1; // default to Z if all zero
		} else {
			// default to point in z direction
			exdir_spec.X = 0.0f;
			exdir_spec.Y = 0.0f;
			exdir_spec.Z = 1.0f;
			exdir_spec.VisibleFrame = 0;
		}

	// add the teleporter
	TheServer->AddTeleporter( &pos_spec, &dir_spec, &expos_spec, &exdir_spec );

	return TRUE;
}

// console command for specifying teleporters in game server mode --------------------------------
//
PRIVATE
int Cmd_TP_MODIFY( char* tp_mod_command )
{
	//NOTE:
	//CONCOM:
	// tp_modify_command	::= 'tp.mod' id_spec [<pos_spec>] [<dir_spec>] [expos_spec] [exdir_spec]
	// id_spec			::= 'id' <int>
	// pos_spec			::= 'pos' '(' <float> <float> <float> ')'
	// dir_spec			::= 'dir' '(' <float> <float> <float> ')'
	// expos_spec	::= 'expos' '(' <float> <float> <float> ')'
	// exdir_spec			::= 'exdir' '(' <float> <float> <float> ')'


	ASSERT( tp_mod_command != NULL );
	HANDLE_COMMAND_DOMAIN( tp_mod_command );

	int tp_id = 0;
	Vector3 pos_spec;
	Vector3 expos_spec;
	Vector3 dir_spec;
	Vector3 exdir_spec;

	Vector3 *pos, *expos, *dir, *exdir;

	bool pos_ch, expos_ch, dir_ch, exdir_ch;

	// scan out all values to keys
	if ( !ScanKeyValuePairs( tp_command_keys,tp_mod_command ) ) {
		return TRUE;
	}

	if(tp_command_keys[KEY_TELEPORTER_ID].value != NULL ) {
		if(!ScanKeyValueInt(&tp_command_keys[KEY_TELEPORTER_ID],&tp_id)) {
			CON_AddLine("Teleporter ID Invalid");
			return TRUE;
		}
	}

	// parse position
	if ( tp_command_keys[ KEY_TELEPORTER_POS ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_POS ], (float*)&pos_spec.X, 3, 3 ) ) {
			CON_AddLine( "position invalid" );
			return TRUE;
		}
		pos_ch = TRUE;
	} else {
		pos_ch = FALSE;
	}

	// parse exit position
	if ( tp_command_keys[ KEY_TELEPORTER_EXPOS ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_EXPOS ], (float*)&expos_spec.X, 3, 3 ) ) {
			CON_AddLine( "position invalid" );
			return TRUE;
		}
		expos_ch = TRUE;

	} else {
		expos_ch = FALSE;
	}
	// parse direction
	if ( tp_command_keys[ KEY_TELEPORTER_DIR ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_DIR ], (float*)&dir_spec.X, 3, 3 ) ) {
			CON_AddLine( "direction invalid" );
			return TRUE;
		}
		dir_ch = TRUE;
		if(dir_spec.X == 0 && dir_spec.Y == 0 && dir_spec.Z==0)
			dir_spec.Z=1; // default to Z if all zero
	} else {
		dir_ch = FALSE;
	}

	// parse exit direction
	if ( tp_command_keys[ KEY_TELEPORTER_EXDIR ].value != NULL ) {
		if ( !ScanKeyValueFloatList( &tp_command_keys[ KEY_TELEPORTER_EXDIR ], (float*)&exdir_spec.X, 3, 3 ) ) {
			CON_AddLine( "direction invalid" );
			return TRUE;
		}
		exdir_ch = TRUE;
		if(exdir_spec.X == 0 && exdir_spec.Y == 0 && exdir_spec.Z==0)
			exdir_spec.Z=1; // default to Z if all zero
	} else {
		exdir_ch = FALSE;
	}

	pos=(pos_ch) ? &pos_spec : NULL;
	expos=(expos_ch) ? &expos_spec : NULL;
	dir=(dir_ch) ? &dir_spec : NULL;
	exdir=(exdir_ch) ? &exdir_spec : NULL;

	// add the teleporter
	TheServer->ModTeleporter( tp_id, pos, dir, expos, exdir );

	return TRUE;
}

#endif //PARSEC_SERVER


// module registration function -----------------------------------------------
//
REGISTER_MODULE( G_TELEP )
{
	// register type
	TeleporterRegisterCustomType();

	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );
#ifdef PARSEC_SERVER
	// register "tp.create" command
	regcom.command	 = "tp.create";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_TP_CREATE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "tp.create" command
	regcom.command	 = "tp.mod";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_TP_MODIFY;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
#endif
}

