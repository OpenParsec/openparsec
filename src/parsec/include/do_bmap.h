/*
 * PARSEC HEADER: do_bmap.h
 */

#ifndef _DO_BMAP_H_
#define _DO_BMAP_H_


// do_bmap.c implements the following functions
// --------------------------------------------
//	void D_PutTrBitmap( char *bitmap, dword width, dword height, ugrid_t putx, ugrid_t puty );
//	void D_PutTrMapBitmap( char *bitmap, char *maps, dword width, dword height, ugrid_t putx, ugrid_t puty, dword color );
//	void D_PutClipBitmap( char *bitmap, dword width, dword height, sgrid_t putx, sgrid_t puty );
//	void D_PutTrClipBitmap( char *bitmap, dword width, dword height, sgrid_t putx, sgrid_t puty );
//	void D_PutSTCBitmap( char *bitmap, dword srcw, dword srch, dword dstw, dword dsth, sgrid_t putx, sgrid_t puty );
//	void D_PutSTCBitmapZ( char *bitmap, dword srcw, dword srch, dword dstw, dword dsth, sgrid_t putx, sgrid_t puty, dword zvalue );
//	void D_DrawTrRect( ugrid_t putx, ugrid_t puty, dword width, dword height, dword brightx );

// do_bmap.c implements the following variables
// --------------------------------------------
//	char * GlobalColXlat;
//	char * BrightxMap;



#endif // _DO_BMAP_H_


