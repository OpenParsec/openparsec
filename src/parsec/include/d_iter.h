/*
 * PARSEC HEADER: d_iter.h
 */

#ifndef _D_ITER_H_
#define _D_ITER_H_


// ----------------------------------------------------------------------------
// DRAWING SUBSYSTEM (D) ITER group                                           -
// ----------------------------------------------------------------------------


void D_BiasIterDepth( int bias );
void D_LoadIterMatrix( Xmatrx matrix );
void D_DrawIterPoint2( IterPoint2 *itpoint );
void D_DrawIterPoint3( IterPoint3 *itpoint, dword cullmask );
void D_DrawIterLine2( IterLine2 *itline );
void D_DrawIterLine3( IterLine3 *itline, dword cullmask );
void D_DrawIterTriangle2( IterTriangle2 *ittri );
void D_DrawIterTriangle3( IterTriangle3 *ittri, dword cullmask );
void D_DrawIterRectangle2( IterRectangle2 *itrect );
void D_DrawIterRectangle3( IterRectangle3 *itrect, dword cullmask );
void D_DrawIterPolygon2( IterPolygon2 *itpoly );
void D_DrawIterPolygon3( IterPolygon3 *itpoly, dword cullmask );
void D_DrawIterTriStrip2( IterTriStrip2 *itstrip );
void D_DrawIterTriStrip3( IterTriStrip3 *itstrip, dword cullmask );
void D_LockIterArray2( IterArray2 *itarray, dword first, dword count );
void D_LockIterArray3( IterArray3 *itarray, dword first, dword count );
void D_UnlockIterArray();
void D_DrawIterArray( dword mode, dword first, dword count, dword cullmask );
void D_DrawIterArrayIndexed( dword mode, dword count, uint16 *indexes, dword cullmask );


#endif // _D_ITER_H_


