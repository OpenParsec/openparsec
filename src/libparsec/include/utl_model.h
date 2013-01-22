/*
 * PARSEC HEADER: x_model.h
 */

#ifndef _X_MODEL_H_
#define _X_MODEL_H_


// ----------------------------------------------------------------------------
// MODEL GEOMETRY SUBSYSTEM                                                   -
// ----------------------------------------------------------------------------


// BSP group

int		BSP_FindColliderLine( CullBSPNode *tree, Vertex3 *v0, Vertex3 *v1, dword *colnode, geomv_t *colt );


// CULL group

void	CULL_ReAcIndexBox3( ReAcIndexBox3 *reac, Plane3 *plane );
void	CULL_ReAcPointBox3( ReAcPointBox3 *reac, CullBox3 *cullbox, Plane3 *plane );
void	CULL_MakeVolumeCullVolume( Plane3 *volume, CullPlane3 *cullvolume, dword cullmask );
int		CULL_BoxAgainstCullVolume( CullBox3 *cullbox, CullPlane3 *volume, dword *cullmask );
int		CULL_BoxAgainstVolume( CullBox3 *cullbox, Plane3 *volume, dword *cullmask );
int		CULL_SphereAgainstVolume( Sphere3 *sphere, Plane3 *volume, dword *cullmask );


// CLIP group

IterPolygon3*	CLIP_VolumeIterTriangle3( IterTriangle3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterTriangle3_UVW( IterTriangle3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterTriangle3_RGBA( IterTriangle3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterRectangle3( IterRectangle3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterRectangle3_UVW( IterRectangle3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterRectangle3_RGBA( IterRectangle3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterPolygon3( IterPolygon3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterPolygon3_UVW( IterPolygon3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_VolumeIterPolygon3_RGBA( IterPolygon3 *poly, Plane3 *volume, dword cullmask );
IterPolygon3*	CLIP_PlaneIterTriangle3( IterTriangle3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterTriangle3_UVW( IterTriangle3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterTriangle3_RGBA( IterTriangle3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterRectangle3( IterRectangle3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterRectangle3_UVW( IterRectangle3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterRectangle3_RGBA( IterRectangle3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterPolygon3( IterPolygon3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterPolygon3_UVW( IterPolygon3 *poly, Plane3 *plane );
IterPolygon3*	CLIP_PlaneIterPolygon3_RGBA( IterPolygon3 *poly, Plane3 *plane );

IterLine2*		CLIP_RectangleIterLine2( IterLine2 *line, Rectangle2 *rect );
IterLine3*		CLIP_PlaneIterLine3( IterLine3 *line, Plane3 *plane );

GenObject*		CLIP_VolumeGenObject( GenObject *clipobj, Plane3 *volume, dword cullmask );
GenObject*		CLIP_PlaneGenObject( GenObject *clipobj, Plane3 *plane );


#endif // _X_MODEL_H_


