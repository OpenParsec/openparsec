/*
 * PARSEC HEADER: e_colani.h
 */

#ifndef _E_COLANI_H_
#define _E_COLANI_H_


// colanim spec

struct colanimreg_s {

	char*		name;
	dword		_pad32[ 3 ];
	colanim_s	colanim;
};


// external functions

colanimreg_s*	FetchColAnim( char *name, dword *colanimid );
void			InitAnimStateColAnim( FaceAnimState *animstate, colanim_s *colanim, dword colflags );
int				RegisterColAnim( colanimreg_s *colanimref );


// external variables

extern int			num_registered_colanims;
extern colanimreg_s	registered_colanims[];


#endif // _E_COLANI_H_


