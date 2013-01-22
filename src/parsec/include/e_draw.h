/*
 * PARSEC HEADER: e_draw.h
 */

#ifndef _E_DRAW_H_
#define _E_DRAW_H_


// colored screen rectangle info

struct colscreenrect_s {

	int			x;
	int			y;
	int			w;
	int 		h;

	dword		itertype;
	colrgba_s	color;
};


// textured screen rectangle info

struct texscreenrect_s {

	int			x;
	int			y;
	int			w;
	int 		h;

	int			scaled_w;
	int			scaled_h;

	int			texofsx;
	int			texofsy;

	int			alpha;

	dword		itertype;
	TextureMap*	texmap;
};


// external functions

void 	DRAW_PanelDecorations( sgrid_t putx, sgrid_t puty, int width, int height );
void	DRAW_ClippedTrRect( sgrid_t putx, sgrid_t puty, int width, int height, dword brightx );
void 	DRAW_ColoredScreenRect( colscreenrect_s *rect );
void 	DRAW_TexturedScreenRect( texscreenrect_s *rect, Rectangle2 *cliprect );


#endif // _E_DRAW_H_


