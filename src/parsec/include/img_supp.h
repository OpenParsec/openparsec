/*
 * PARSEC HEADER: img_supp.h
 */

#ifndef _IMG_SUPP_H_
#define _IMG_SUPP_H_


// external functions

dword	IMG_DetermineTexGeoAspect( dword width, dword height );
int		IMG_DetermineTexGeoAspectExt( dword width, dword height );
dword	IMG_DetermineTexGeoScale( dword maxside );

dword	IMG_CountMipmapTexels( dword texw, dword texh, dword lodlarge, dword lodsmall );


#endif // _IMG_SUPP_H_


