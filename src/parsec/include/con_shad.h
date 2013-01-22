/*
 * PARSEC HEADER: con_shad.h
 */

#ifndef _CON_SHAD_H_
#define _CON_SHAD_H_


// forward declarations
struct shader_s;
struct key_value_s;


// external functions

shader_s*		DetermineODTShader( key_value_s *shaderspec );

char*			GetShaderDescription( dword shaderid );

int				Cmd_DefShader( char *command );
int				Cmd_SetShader( char *command );


#endif // _CON_SHAD_H_


