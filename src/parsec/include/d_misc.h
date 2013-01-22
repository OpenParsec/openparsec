/*
 * PARSEC HEADER: d_misc.h
 */

#ifndef _D_MISC_H_
#define _D_MISC_H_


// ----------------------------------------------------------------------------
// DRAWING SUBSYSTEM (D) MISC group                                           -
// ----------------------------------------------------------------------------


// line drawing styles

#define D_DRAWLINE_STYLE_DEFAULT		0x0000
#define D_DRAWLINE_STYLE_ANTIALIASED	0x0001
#define D_DRAWLINE_STYLE_THICK			0x0002
#define D_DRAWLINE_STYLE_STIPPLED		0x0004
#define D_DRAWLINE_STYLE_CLOSE_STRIP	0x0008


// external functions

int  D_DrawSquare( ugrid_t xko, ugrid_t yko, visual_t col, dword siz );
int  D_DrawSquareZ( ugrid_t xko, ugrid_t yko, visual_t col, dword siz, dword zvalue );
void D_DrawRadarObj( ugrid_t xko, ugrid_t yko, visual_t col );
void D_DrawHorzBar( ugrid_t xko, ugrid_t yko, visual_t col, dword leng );
void D_DrawVertBar( ugrid_t xko, ugrid_t yko, visual_t col, dword leng );
void D_DrawLine( ugrid_t xko1, ugrid_t yko1, ugrid_t xko2, ugrid_t yko2, visual_t col1, visual_t col2, dword mode );
void D_DrawLineStrip( ugrid_t *vtxs, dword numvtxs, visual_t col, dword mode );


// subsystem independet external functions ------------------------------------
//
void D_LineWorld( const Vector3* start, const Vector3* end, const colrgba_s* color );
void D_FrameOfReference( const Xmatrx frame, const Vector3* transl );



#endif // D_MISC.H


