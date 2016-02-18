/*
 * PARSEC HEADER: h_strmap.h
 */

#ifndef _H_STRMAP_H_
#define _H_STRMAP_H_


// external functions ---------------------------------------------------------
//
void 	MAP_FadeInStarmap();
void 	MAP_FadeOutStarmap();
void 	MAP_ActivateStarmap();
int 	MAP_ShowStarmap( void *param );
void 	MAP_KeyHandler();
void    MAP_JoyHandler();

// ----------------------------------------------------------------------------
//
struct starmap_bg_info_s {

	float		xpos;
	float		ypos;
	float		width;
	float		height;
	int			owidth;
	int			oheight;

	TextureMap*	texmap;
	const char*	texname;
};

#endif // _H_STRMAP_H_


