/*
 * PARSEC HEADER: e_shader.h
 */

#ifndef _E_SHADER_H_
#define _E_SHADER_H_


// shader spec

struct shader_s {

	char*		name;
	word		iter;
	word		flags;
	texanim_s*	texanim;
	colrgba_s	facecolor;
	colanim_s*	colanim;
	dword		colflags;
	dword		_pad32[ 2 ];
};


// special lod argument for SetFaceShader()

#define ACTIVE_LOD	(~0)


// argument specification for face shader overriding

enum {

	OVERRIDE_ARG_IDLIST,
	OVERRIDE_ARG_TEXTURE,
	OVERRIDE_ARG_GLOBAL,

	OVERRIDE_ARG_NUM_ARGS
};


// external functions

int			SetFaceShader( GenObject *obj, dword lod, dword faceid, shader_s *shader );

shader_s*	FetchShaderByName( const char *name, dword *shaderid );
shader_s*	FetchShaderById( dword shaderid );
int			RegisterShader( shader_s *shader );

void		OverrideShaderFaceByIdList( dword objclass, shader_s *shader, dword *faceids, int count );
void		OverrideShaderFaceByTexture( dword objclass, shader_s *shader, char *texname );
void		OverrideShaderFaceGlobal( dword objclass, shader_s *shader );
void		OverrideShaderFaceGlobalInstanced( GenObject *gobj, shader_s *shader );
void		OverrideShaderFaces( dword objclass, shader_s *shader, int arg );
void		OverrideShaderLodFaces( dword objclass, shader_s *shader, int lod, int arg );


// external variables

extern int			num_registered_shaders;
extern shader_s		registered_shaders[];

extern dword*		override_arg_faceids;
extern int			override_arg_count;
extern char*		override_arg_texname;


#endif // _E_SHADER_H_


