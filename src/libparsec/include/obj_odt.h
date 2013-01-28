/*
 * PARSEC HEADER: obj_odt.h
 */

#ifndef _OBJ_ODT_H_
#define _OBJ_ODT_H_



// object class loading flags

enum {

	OBJLOAD_NONE				= 0x0000,	// no additional info
	OBJLOAD_DEFAULT				= 0x8000,	// use appropriate default settings

	OBJLOAD_WEDGENORMALS		= 0x4001,	// calculate WedgeNormals
	OBJLOAD_WEDGECOLORS			= 0x4002,	// reserve storage for WedgeColors
	OBJLOAD_WEDGETEXCOORDS		= 0x4004,	// store explicit WedgeTexCoords
	OBJLOAD_WEDGELIGHTED		= 0x4008,	// reserve storage for WedgeLighted
	OBJLOAD_WEDGESPECULAR		= 0x4010,	// reserve storage for WedgeSpecular
	OBJLOAD_WEDGEFOGGED			= 0x4020,	// reserve storage for WedgeFogged

	OBJLOAD_WEDGE_INFO			= 0x4000,	// WedgeVertIndxs mandatory

	OBJLOAD_FACEANIMS			= 0x0040,	// FaceExtInfo and FaceAnimStates
	OBJLOAD_VTXANIMS			= 0x0080,	// VtxAnimStates

	OBJLOAD_POLYCORNERCOLORS	= 0x0100,	// reserve corner colors in polys
	OBJLOAD_POLYWEDGEINDEXES	= 0x0200	// reserve wedge indexes in polys
};


// maximum number of lods in an object

#define MAX_OBJECT_LODS			16


// additional configurable parameters for object loading

struct odt_loading_params_s {

	int		max_face_anim_states;
	int		max_vtx_anim_states;
	geomv_t	merge_normals_threshold;
};


// forward declaration
struct shader_s;


// external functions

int		OBJ_LoadODT( dword classid, dword flags, shader_s *shader );


// external variables

extern odt_loading_params_s odt_loading_params;


#endif // _OBJ_ODT_H_


