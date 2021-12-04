/*
 * PARSEC HEADER: d_font.h
 */

#ifndef _D_FONT_H_
#define _D_FONT_H_


// ----------------------------------------------------------------------------
// DRAWING SUBSYSTEM (D) FONT group                                           -
// ----------------------------------------------------------------------------


void D_SetWStrContext( char *charset, dword *geometry, char *outseg, dword width, dword height );
void D_WriteString( const char *string, ugrid_t putx, ugrid_t puty );
void D_WriteTrString( const char *string, ugrid_t putx, ugrid_t puty, dword brightx );
int  D_GetPStringWidth( char *string );
void D_TexfontWrite( const char *string, IterTexfont *itexfont );
int  D_TexfontGetWidth( const char *string, IterTexfont *itexfont );

// type for writestring function pointer --------------------------------------
//
typedef void (*WSFP)( const char *string, ugrid_t putx, ugrid_t puty, dword brightx );

#endif // _D_FONT_H_


