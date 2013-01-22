/*
 * PARSEC HEADER: e_supp.h
 */

#ifndef _E_SUPP_H_
#define _E_SUPP_H_


// external functions

void			InitRefFrameVars();
void			DoFrameTimeCalculations();
void			SaveScreenShot();

int				CeilPow2Exp( int number );

TextureMap*		FetchTextureMap( const char *texname );
texfont_s*		FetchTexfont( const char *fontname );

int				FetchBitmapId( const char *bmname );

int				EvictUnsupportedTextureFormatData();
int				ReloadFreedTextureBitmaps();

#ifdef PARSEC_SERVER
	#undef KillAllObjects
#endif // PARSEC_SERVER
void			KillAllObjects();

#endif // _E_SUPP_H_


