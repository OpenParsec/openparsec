/*
 * PARSEC HEADER: ro_poly.h
 */

#ifndef _RO_POLY_H_
#define _RO_POLY_H_


// external functions

void	RO_SolidFillPoly( SVertexExList *svertexlist, Face *face, int colorsvalid );
void	RO_TexMapPoly( SVertexExList *svertexlist, Face *face, int colorsvalid );
int		RO_RenderPolygon( GenObject *baseobj, dword polyid );


#endif // _RO_POLY_H_


