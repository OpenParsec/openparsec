/*
 * PARSEC HEADER: e_texani.h
 */

#ifndef _E_TEXANI_H_
#define _E_TEXANI_H_


// texanim spec

struct texanimreg_s {

	char*		name;
//	dword		_pad32[ 3 ];
//	colanim_s	colanim;
};


// external functions

texanimreg_s*	FetchTexAnim( char *name, dword *texanimid );
void			InitAnimStateTexAnim( FaceAnimState *animstate, texanim_s *texanim );
int				RegisterTexAnim( texanimreg_s *texanimref );


// external variables

extern int			num_registered_texanims;
extern texanimreg_s	registered_texanims[];


#endif // _E_TEXANI_H_


