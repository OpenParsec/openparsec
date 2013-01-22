/*
 * PARSEC HEADER: do_font.h
 */

#ifndef _DO_FONT_H_
#define _DO_FONT_H_


// do_font.c implements the following functions
// --------------------------------------------
//	void D_SetWStrContext( char *charset, dword *geometry, char *outseg, dword width, dword height );
//	void D_WriteString( const char *string, ugrid_t putx, ugrid_t puty );
//	void D_WriteTrString( const char *string, ugrid_t putx, ugrid_t puty, dword brightx );
//	int  D_GetPStringWidth( char *string );
//	void D_TexfontWrite( const char *string, IterTexfont *itexfont );
//	int  D_TexfontGetWidth( const char *string, IterTexfont *itexfont );

// do_font.c implements the following variables
// --------------------------------------------
//	dword  Char16x16Geom[];
//	dword  Char08x08Geom[];
//	dword  Char04x09Geom[];
//	dword  CharGoldGeom[];
//	char * CharsetDataBase;


#endif // _DO_FONT_H_


