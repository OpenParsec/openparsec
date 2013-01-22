/*
 * PARSEC HEADER: s_telep.h
 */

#ifndef _S_TELEP_H_
#define _S_TELEP_H_


// teleporter limits ----------------------------------------------------------
//
#define TELEPORTER_MAX_TEX_NAME			63


// teleporter custom type structure -------------------------------------------

struct Teleporter : CustomObject {

	// object properties that can be modified via the console
	geomv_t				exit_delta_x;
	geomv_t				exit_delta_y;
	geomv_t				exit_delta_z;
	float				exit_rot_phi;
	float				exit_rot_theta;
	geomv_t				actoffset;					
	int					active;						// enable/disable the teleporter
	char				tex_name_interior[ TELEPORTER_MAX_TEX_NAME + 1 ];	// texture name of the teleporter start interior
	char				tex_name_tunnel  [ TELEPORTER_MAX_TEX_NAME + 1 ];	// texture name of the texture for the tunnel
	int					start_tex_sec_layer;		// enable/disable second texture layer at teleporter start
	geomv_t				u_variation[ 2 ];			
	geomv_t				v_variation[ 2 ];
	int					start_tex_alpha;			// alpha for the start interior texture 
	geomv_t				act_cone_angle;				// cone angle of the activation cone
	int					tunnel_slerp;				// 0/1 is SLERP on/off
	int					tunnel_lerp;				// 0/1/2 is LERP off/LERP normal/LERP Hermite way
	float				tunnel_anim_speed;			// animation speed in sec
	float				tunnel_tangent_scale1;		// tangent scale for hermite interpolation ( start tangent )
	float				tunnel_tangent_scale2;		// tangent scale for hermite interpolation ( end tangent )
	int					tunnel_spline;				// 2 bits toggling spline drawing for the center of the tunnel and the boundaries ( bit0 set -> center, bit1 set ->boundaries )
	int					tunnel_spline_cords;		// # of steps to use for the spline interpolation when drawing center/boundaries of tunnel
	int					tunnel_tex_alpha;			// alpha for the tunnel interior texture 
	int					tunnel_len;					// length of the tunnel in spline cords
	Vector3				start;						// coordinates of the teleporter start 
	float				start_rot_phi;				// start rotations ( phi )	
	float				start_rot_theta;			// start rotations ( theta )
	
	// internal data
	Vertex3*			start_vtxlist;				// vtxs in object space
	TextureMap*			start_texmap;
	TextureMap*			tunnel_texmap;
	geomv_t				u_delta[ 2 ];
	geomv_t				v_delta[ 2 ];
	bams_t				u_phi[ 2 ];
	bams_t				v_phi[ 2 ];
	bams_t				u_phi_delta[ 2 ];
	bams_t				v_phi_delta[ 2 ];
	geomv_t				u_maxdelta[ 2 ];
	geomv_t				v_maxdelta[ 2 ];
	GenObject*			child_object;
	Xmatrx				tunnel_frame;
	float				tunnel_t;
	Vertex3*			tunnel_verts;
	geomv_t				tunnel_cur_anim_step;
	geomv_t				tunnel_spline_arclen;
	CullBox3			tunnel_cullbox;

	// hacks :)
	Xmatrx				start_frame, end_frame;
	Xmatrx*				spline_frames;
};


// external functions

// (none)


#endif // _S_TELEP_H_


