/*
 * PARSEC - Shader Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
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

// local module header
#include "e_shader.h"

// proprietary module headers
#include "con_aux.h"
#include "con_main.h"
#include "e_colani.h"
#include "e_texani.h"
#include "obj_ctrl.h"



// string constants -----------------------------------------------------------
//
static char no_face_found[]			= "no face with this texture found.";
static char non_textured_face[]		= "face is not textured. shader disallowed.";
static char object_lod_invalid[]	= "lod invalid for this object.";
static char lod_faceid_invalid[]	= "faceid invalid for this lod.";
static char no_object_lods[]		= "object has no lods.";
static char no_auxlod_anim_states[]	= "cannot create new anim states for aux lods.";
static char no_faceanim_storage[]	= "object has no storage for faceanim shaders.";
static char too_many_faceanims[]	= "object has too many faceanim shaders.";


// maximum number of registered shaders ---------------------------------------
//
#define MAX_NUM_REGISTERED_SHADERS		128


// tables for registered shaders ----------------------------------------------
//
PUBLIC int		num_registered_shaders = 0;
PUBLIC shader_s	registered_shaders[ MAX_NUM_REGISTERED_SHADERS ];


// fetch already registered shader via name -----------------------------------
//
shader_s *FetchShaderByName( const char *name, dword *shaderid )
{
	ASSERT( name != NULL );

	for ( int sid = 0; sid < num_registered_shaders; sid++ ) {

		ASSERT( registered_shaders[ sid ].name != NULL );
		if ( stricmp( registered_shaders[ sid ].name, name ) == 0 ) {

			if ( shaderid != NULL )
				*shaderid = sid;
			return &registered_shaders[ sid ];
		}
	}

	return NULL;
}


// fetch already registered shader via id -------------------------------------
//
shader_s *FetchShaderById( dword shaderid )
{
	if ( shaderid < (dword)num_registered_shaders ) {
		return &registered_shaders[ shaderid ];
	}

	return NULL;
}


// register new shader --------------------------------------------------------
//
int RegisterShader( shader_s *shader )
{
	ASSERT( shader != NULL );

	if ( num_registered_shaders >= MAX_NUM_REGISTERED_SHADERS )
		return FALSE;

	// name is mandatory
	if ( shader->name == NULL )
		return FALSE;

	dword shaderid = num_registered_shaders;

	// check for already registered shader of same name
	if ( FetchShaderByName( shader->name, &shaderid ) != NULL ) {

		if ( !AUX_ENABLE_SHADER_OVERLOADING ) {
			return FALSE;
		}

	} else {

		// allocate new shader name
		char *name = (char *) ALLOCMEM( strlen( shader->name ) + 1 );
		if ( name == NULL )
			OUTOFMEM( 0 );
		strcpy( name, shader->name );

		ASSERT( registered_shaders[ shaderid ].name == NULL );
		registered_shaders[ shaderid ].name = name;
		num_registered_shaders++;
	}

	// store shader info
	registered_shaders[ shaderid ].iter		 = shader->iter;
	registered_shaders[ shaderid ].flags	 = shader->flags;
	registered_shaders[ shaderid ].texanim	 = shader->texanim;
	registered_shaders[ shaderid ].facecolor = shader->facecolor;
	registered_shaders[ shaderid ].colanim	 = shader->colanim;
	registered_shaders[ shaderid ].colflags	 = shader->colflags;

	return TRUE;
}


// table for determining whether shader needs constant color ------------------
//
static dword const_color_shader[] = {

	iter_constrgb,
	iter_constrgba,
	iter_texconsta,
	iter_texconstrgb,
	iter_texconstrgba,
	iter_constrgbtexa,
	iter_constrgbatexa,
	iter_texconstaspecrgb,
	iter_texconstrgbspeca,

	~0
};


// table for determining whether shader disables vertex alpha -----------------
//
static dword vtxalpha_disabled_shader[] = {

	iter_constrgb,
	iter_rgb,
	iter_texconstrgb,
	iter_texrgb,
	iter_constrgbtexa,
	iter_rgbtexa,

	~0
};


// enabling mask for error messages during face shader attaching --------------
//
enum {

	SFERRMSG_NONE				= 0x0000,
	SFERRMSG_NOTEXTURE			= 0x0001,
	SFERRMSG_FACEANIMMEM		= 0x0002,
	SFERRMSG_FACEANIMOVERFLOW	= 0x0004,
	SFERRMSG_NOOBJECTLODS		= 0x0008,
	SFERRMSG_INVALIDLOD			= 0x0010,
	SFERRMSG_INVALIDLODFACEID	= 0x0020,
	SFERRMSG_NOAUXLODANIMAPPEND	= 0x0040,
	SFERRMSG_ALL				= 0xffff
};

static int setfaceshader_error_msgs = SFERRMSG_NONE;


// set new shader for specified face if shader valid --------------------------
//
int SetFaceShader( GenObject *obj, dword lod, dword faceid, shader_s *shader )
{
	ASSERT( obj != NULL );

	if ( shader == NULL )
		return FALSE;

	// determine lod for shader attaching
	if ( lod == (dword)ACTIVE_LOD ) {

		// use currently active lod
		lod = obj->CurrentLod;

	} else {

		// check whether lods present
		if ( obj->NumLodObjects > 0 ) {

			ASSERT( obj->LodObjects != NULL );

			// check for invalid lod
			if ( lod >= obj->NumLodObjects ) {
				if ( setfaceshader_error_msgs & SFERRMSG_INVALIDLOD ) {
					setfaceshader_error_msgs &= ~SFERRMSG_INVALIDLOD;
					CON_AddLine( object_lod_invalid );
				}
				return FALSE;
			}

			// switch lod if necessary
			if ( lod != obj->CurrentLod ) {
				OBJ_SwitchObjectLod( obj, lod );
			}

			// check for invalid faceid (in this lod)
			if ( faceid >= obj->NumFaces ) {
				if ( setfaceshader_error_msgs & SFERRMSG_INVALIDLODFACEID ) {
					setfaceshader_error_msgs &= ~SFERRMSG_INVALIDLODFACEID;
					CON_AddLine( lod_faceid_invalid );
				}
				return FALSE;
			}

		} else {

			ASSERT( obj->CurrentLod == 0 );
			ASSERT( obj->LodObjects == NULL );

			// check for invalid lod
			if ( lod != 0 ) {
				if ( setfaceshader_error_msgs & SFERRMSG_NOOBJECTLODS ) {
					setfaceshader_error_msgs &= ~SFERRMSG_NOOBJECTLODS;
					CON_AddLine( no_object_lods );
				}
				return FALSE;
			}
		}
	}

	ASSERT( faceid < obj->NumFaces );
	Face *face = &obj->FaceList[ faceid ];

	word shadingiter  = shader->iter;
	word shadingflags = shader->flags;

	// set texture enabling flag manually
	shadingflags &= ~FACE_SHADING_ENABLETEXTURE;
	if ( ( shadingiter & iter_base_mask ) >= iter_texonly ) {

		// prevent specification of shader that needs
		// texture when the face is not textured
		if ( face->TexMap == NULL ) {
			if ( setfaceshader_error_msgs & SFERRMSG_NOTEXTURE ) {
				setfaceshader_error_msgs &= ~SFERRMSG_NOTEXTURE;
				CON_AddLine( non_textured_face );
			}
			return FALSE;
		}

		shadingflags |= FACE_SHADING_ENABLETEXTURE;
	}

	// set constant color flag manually
	shadingflags &= ~FACE_SHADING_CONSTANTCOLOR;
	dword cmpshader = shadingiter & iter_base_mask;
	unsigned int csh = 0;
	for ( csh = 0; const_color_shader[ csh ] != (dword)~0; csh++ ) {
		if ( cmpshader == const_color_shader[ csh ] ) {
			shadingflags |= FACE_SHADING_CONSTANTCOLOR;
		}
	}

	// set constant color if valid
	dword facecolor = 0xffffffff;
	if ( shadingflags & FACE_SHADING_FACECOLOR ) {
		facecolor = *(dword*)&shader->facecolor;
	}

	// set vertex alpha disabling manually
	shadingflags &= ~FACE_SHADING_NOVERTEXALPHA;
	for ( csh = 0; vtxalpha_disabled_shader[ csh ] != (dword)~0; csh++ ) {
		if ( cmpshader == vtxalpha_disabled_shader[ csh ] ) {
			shadingflags |= FACE_SHADING_NOVERTEXALPHA;
		}
	}

	// attach face animation
	FaceExtInfo *extinfo = NULL;
	if ( ( shader->texanim != NULL ) || ( shader->colanim != NULL ) ) {

		// face anim state array mandatory
		if ( obj->FaceAnimStates == NULL ) {
			if ( setfaceshader_error_msgs & SFERRMSG_FACEANIMMEM ) {
				setfaceshader_error_msgs &= ~SFERRMSG_FACEANIMMEM;
				CON_AddLine( no_faceanim_storage );
			}
			return FALSE;
		}

		// determine number of used anim states (relevant is lod 0)
		int usedfaceanims = obj->ActiveFaceAnims;
		if ( lod != 0 ) {
			ASSERT( obj->LodObjects != NULL );
			ASSERT( obj->LodObjects[ 0 ].LodObject != NULL );
			usedfaceanims = obj->LodObjects[ 0 ].LodObject->ActiveFaceAnims;
		}

		// check for equivalent anim state that can be used
		int stindx = 0;
		for ( stindx = 0; stindx < usedfaceanims; stindx++ ) {

			if ( shader->texanim != NULL ) {
				if ( obj->FaceAnimStates[ stindx ].TexAnim != shader->texanim )
					continue;
			}
			if ( shader->colanim != NULL ) {
				if ( obj->FaceAnimStates[ stindx ].ColAnim != shader->colanim )
					continue;
				if ( obj->FaceAnimStates[ stindx ].ColFlags != shader->colflags )
					continue;
			}
			break;
		}

		FaceAnimState *animstate = &obj->FaceAnimStates[ stindx ];

		// create new anim state if no reuse
		if ( stindx == usedfaceanims ) {

			// new anim states only possible for base lod (0)
			if ( lod != 0 ) {
				if ( setfaceshader_error_msgs & SFERRMSG_NOAUXLODANIMAPPEND ) {
					setfaceshader_error_msgs &= ~SFERRMSG_NOAUXLODANIMAPPEND;
					CON_AddLine( no_auxlod_anim_states );
				}
				return FALSE;
			}

			// check for depletion of available anim state storage
			if ( stindx >= obj->NumFaceAnims ) {
				if ( setfaceshader_error_msgs & SFERRMSG_FACEANIMOVERFLOW ) {
					setfaceshader_error_msgs &= ~SFERRMSG_FACEANIMOVERFLOW;
					CON_AddLine( too_many_faceanims );
				}
				return FALSE;
			}

			// init texture animation
			if ( shader->texanim != NULL ) {
				InitAnimStateTexAnim( animstate, shader->texanim );
			}

			// init color animation
			if ( shader->colanim != NULL ) {
				InitAnimStateColAnim( animstate, shader->colanim, shader->colflags );
			}
		}

		// ensure this anim state is active in this lod
		if ( stindx > obj->ActiveFaceAnims - 1 ) {
			obj->ActiveFaceAnims = stindx + 1;
		}

		// ensure active counter in lod object is valid
		if ( obj->NumLodObjects > 0 ) {
			ASSERT( obj->LodObjects != NULL );
			ASSERT( obj->LodObjects[ lod ].LodObject != NULL );
			obj->LodObjects[ lod ].LodObject->ActiveFaceAnims = obj->ActiveFaceAnims;
		}

		// extinfos are located right behind facelist
		extinfo = (FaceExtInfo *)&obj->FaceList[ obj->NumFaces ] + faceid;

		// create flags
		dword extflags = 0x0000;
		if ( shader->texanim != NULL )
			extflags |= FACE_EXT_ANIMATETEXTURES;
		if ( shader->colanim != NULL )
			extflags |= FACE_EXT_ANIMATECOLORS;

		// init extinfo
		extinfo->Flags   = extflags;
		extinfo->StateId = stindx;
	}

	// commit shader
	face->ShadingIter  = shadingiter;
	face->ShadingFlags = shadingflags;
	face->ExtInfo      = extinfo;
	face->ColorRGB     = facecolor;

	return TRUE;
}


// update instance data of face anims -----------------------------------------
//
PRIVATE
void UpdateFaceAnimInstances( GenObject *gobj )
{
	ASSERT( gobj != NULL );

	if ( gobj->FaceAnimStates == NULL )
		return;

	// determine corresponding list
	GenObject *objlist = NULL;
	switch ( gobj->ObjectType & TYPELISTMASK ) {

		case PSHIP_LIST_NO:
			objlist = FetchFirstShip();
			break;
		case LASER_LIST_NO:
			objlist = FetchFirstLaser();
			break;
		case MISSL_LIST_NO:
			objlist = FetchFirstMissile();
			break;
		case EXTRA_LIST_NO:
			objlist = FetchFirstExtra();
			break;
		case CUSTM_LIST_NO:
			objlist = FetchFirstCustom();
			break;
	}

	// scan entire list
	for ( ; objlist != NULL; objlist = objlist->NextObj ) {

		if ( objlist->ObjectClass == gobj->ObjectClass ) {

			// copy instance data
			objlist->ActiveFaceAnims = gobj->ActiveFaceAnims;
			ASSERT( objlist->FaceAnimStates != NULL );
			memcpy( objlist->FaceAnimStates, gobj->FaceAnimStates,
				sizeof( FaceAnimState ) * gobj->ActiveFaceAnims );
		}
	}

	// check for local ship update
	if ( MyShip->ObjectClass == gobj->ObjectClass ) {

		// copy instance data
		MyShip->ActiveFaceAnims = gobj->ActiveFaceAnims;
		ASSERT( MyShip->FaceAnimStates != NULL );
		memcpy( MyShip->FaceAnimStates, gobj->FaceAnimStates,
			sizeof( FaceAnimState ) * gobj->ActiveFaceAnims );
	}
}


// override shader in faces specified by id list ------------------------------
//
void OverrideShaderFaceByIdList( dword objclass, shader_s *shader, dword *faceids, int count )
{
	ASSERT( objclass < (dword)NumObjClasses );
	ASSERT( shader != NULL );
	ASSERT( faceids != NULL );

	setfaceshader_error_msgs = SFERRMSG_ALL;

	GenObject *gobj = ObjClasses[ objclass ];
	int faceidinval = FALSE;
	for ( int idx = 0; idx < count; idx++ ) {
		if ( faceids[ idx ] < gobj->NumFaces ) {
			SetFaceShader( gobj, ACTIVE_LOD, faceids[ idx ], shader );
		} else {
			faceidinval = TRUE;
		}
	}

	if ( faceidinval ) {
		CON_AddLine( lod_faceid_invalid );
	}

	setfaceshader_error_msgs = SFERRMSG_NONE;
}


// override shader in faces that are textured with the specified texture ------
//
void OverrideShaderFaceByTexture( dword objclass, shader_s *shader, char *texname )
{
	ASSERT( objclass < (dword)NumObjClasses );
	ASSERT( shader != NULL );
	ASSERT( texname != NULL );

	setfaceshader_error_msgs = SFERRMSG_ALL;

	int numchanged = 0;

	GenObject *gobj = ObjClasses[ objclass ];
	for ( dword faceid = 0; faceid < gobj->NumFaces; faceid++ ) {

		Face*		face   = &gobj->FaceList[ faceid ];
		TextureMap*	texmap = face->TexMap;
		if ( texmap == NULL )
			continue;

		ASSERT( texmap->TexMapName != NULL );
		if ( stricmp( texmap->TexMapName, texname ) == 0 ) {
			numchanged++;
			SetFaceShader( gobj, ACTIVE_LOD, faceid, shader );
		}
	}

	if ( numchanged == 0 ) {
		CON_AddLine( no_face_found );
	}

	setfaceshader_error_msgs = SFERRMSG_NONE;
}


// override shader in all faces of object class -------------------------------
//
void OverrideShaderFaceGlobal( dword objclass, shader_s *shader )
{
	ASSERT( objclass < (dword)NumObjClasses );
	ASSERT( shader != NULL );

	setfaceshader_error_msgs = SFERRMSG_ALL;

	GenObject *gobj = ObjClasses[ objclass ];
	for ( dword faceid = 0; faceid < gobj->NumFaces; faceid++ ) {
		SetFaceShader( gobj, ACTIVE_LOD, faceid, shader );
	}

	setfaceshader_error_msgs = SFERRMSG_NONE;
}


// override shader in instanced faces (no class modification) -----------------
//
void OverrideShaderFaceGlobalInstanced( GenObject *gobj, shader_s *shader )
{
	ASSERT( gobj != NULL );
	ASSERT( shader != NULL );

	setfaceshader_error_msgs = SFERRMSG_ALL;

	dword objclass = gobj->ObjectClass;
	if ( objclass == CLASS_ID_INVALID )
		return;

	// make sure we take the base lod for determining
	// whether face instances are already attached
	Face *basefaces = gobj->FaceList;
	if ( gobj->NumLodObjects > 0 ) {
		ASSERT( gobj->LodObjects != NULL );
		basefaces = gobj->LodObjects[ 0 ].LodObject->FaceList;
	}

	// need instanced face data if not already available
	ASSERT( objclass < (dword)NumObjClasses );
	if ( basefaces == ObjClasses[ objclass ]->FaceList ) {

		int numfaces = gobj->NumFaces;
		if ( gobj->NumLodObjects > 0 ) {
			for ( int lod = 1; lod < gobj->NumLodObjects; lod++ ) {
				numfaces += gobj->LodObjects[ lod ].LodObject->NumFaces;
			}
		}

		size_t facesize = sizeof( Face );
		if ( gobj->NumFaceAnims > 0 )
			facesize += sizeof( FaceExtInfo );
		size_t faceinstsize = numfaces * facesize;
		Face *faceinstances = (Face *) ALLOCMEM( faceinstsize );
		if ( faceinstances == NULL )
			OUTOFMEM( 0 );
		memcpy( faceinstances, basefaces, faceinstsize );

		// use as facelist of base lod
		if ( gobj->NumLodObjects > 0 ) {

			int faceskip = 0;
			for ( int lod = 0; lod < gobj->NumLodObjects; lod++ ) {

				Face *instlist = (Face *)( (char*)faceinstances + ( faceskip * facesize ) );
				gobj->LodObjects[ lod ].LodObject->FaceList = instlist;

				faceskip += gobj->LodObjects[ lod ].LodObject->NumFaces;
			}
			gobj->FaceList = gobj->LodObjects[ gobj->CurrentLod ].LodObject->FaceList;

		} else {

			gobj->FaceList = faceinstances;
		}
/*
		if ( gobj->NumVtxAnims > 0 ) {
			dword *baseinfo = (dword *) &gobj->VtxAnimStates[ gobj->NumVtxAnims ];
			baseinfo[ 8 ] = (dword) faceinstances;
		}
*/
	}

	// attach shader to instanced faces
	for ( dword faceid = 0; faceid < gobj->NumFaces; faceid++ ) {
		SetFaceShader( gobj, ACTIVE_LOD, faceid, shader );
	}

	setfaceshader_error_msgs = SFERRMSG_NONE;
}


// temporary storage for face identification ----------------------------------
//
PUBLIC dword*	override_arg_faceids = NULL;
PUBLIC int		override_arg_count;
PUBLIC char*	override_arg_texname;


// override shader in specified faces -----------------------------------------
//
void OverrideShaderFaces( dword objclass, shader_s *shader, int arg )
{
	ASSERT( objclass < (dword)NumObjClasses );
	ASSERT( shader != NULL );
	ASSERT( arg >= OVERRIDE_ARG_IDLIST );
	ASSERT( arg < OVERRIDE_ARG_NUM_ARGS );

	switch ( arg ) {

		case OVERRIDE_ARG_IDLIST:
			OverrideShaderFaceByIdList( objclass, shader, override_arg_faceids, override_arg_count );
			break;

		case OVERRIDE_ARG_TEXTURE:
			OverrideShaderFaceByTexture( objclass, shader, override_arg_texname );
			break;

		case OVERRIDE_ARG_GLOBAL:
			OverrideShaderFaceGlobal( objclass, shader );
			break;
	}
}


// override shader in specified faces of specified lod ------------------------
//
void OverrideShaderLodFaces( dword objclass, shader_s *shader, int lod, int arg )
{
	ASSERT( objclass < (dword)NumObjClasses );
	ASSERT( shader != NULL );

	GenObject *gobj = ObjClasses[ objclass ];

	if ( lod >= 0 ) {

		if ( gobj->NumLodObjects > 0 ) {
			ASSERT( lod < gobj->NumLodObjects );
			if ( lod != gobj->CurrentLod ) {
				OBJ_SwitchObjectLod( gobj, lod );
			}
		}

		OverrideShaderFaces( objclass, shader, arg );

	} else {

		// negative lod specifies multiple lods
		dword maxlod = 1 - lod;
		ASSERT( maxlod <= gobj->NumLodObjects );

		for ( lod = 0; lod < (int)maxlod; lod++ ) {
			if ( lod != gobj->CurrentLod )
				OBJ_SwitchObjectLod( gobj, lod );
			OverrideShaderFaces( objclass, shader, arg );
		}
	}

	// make sure instances are valid
	if ( gobj->CurrentLod != 0 )
		OBJ_SwitchObjectLod( gobj, 0 );
	UpdateFaceAnimInstances( gobj );
}



