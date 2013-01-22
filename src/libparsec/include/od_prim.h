/*
 * PARSEC HEADER (OBJECT)
 * Primitive Types V1.80
 *
 * Copyright (c) Markus Hadwiger 1995-2001
 * All Rights Reserved.
 */

#ifndef _OD_PRIM_H_
#define _OD_PRIM_H_


// homogeneous floating-point coordinates of point in 2-space -----------------
//
struct Point2h_f {

	float	X;
	float	Y;
	float	W;
};


// homogeneous fixed-point coordinates of point in 2-space --------------------
//
struct Point2h_x {

	fixed_t	X;
	fixed_t	Y;
	fixed_t	W;
};


// homogeneous floating-point coordinates of point in 3-space -----------------
//
struct Point3h_f {

	float	X;
	float	Y;
	float	Z;
	float	W;
};


// homogeneous fixed-point coordinates of point in 3-space --------------------
//
struct Point3h_x {

	fixed_t	X;
	fixed_t	Y;
	fixed_t	Z;
	fixed_t	W;
};


// general quaternion ---------------------------------------------------------
//
struct Quaternion {

	geomv_t	W;	// q = w + xi + yj + zk
	geomv_t	X;	// q = ( w, (x,y,z) )
	geomv_t	Y;	// q = ( cos(theta/2), sin(theta/2)*(x,y,z) )
	geomv_t	Z;
};


// general quaternion with floating-point components --------------------------
//
struct Quaternion_f {

	float	W;	// q = w + xi + yj + zk
	float	X;	// q = ( w, (x,y,z) )
	float	Y;	// q = ( cos(theta/2), sin(theta/2)*(x,y,z) )
	float	Z;
};


// 2-D texture coordinates ----------------------------------------------------
//
struct TexCoord2 {

	geomv_t	U;
	geomv_t	V;
};


// 3-D vertex coordinates (used by object structures) -------------------------
//
struct Vertex3 {

	geomv_t	X;
	geomv_t	Y;
	geomv_t	Z;
	dword	VisibleFrame;	// ==CurVisibleFrame if vertex touched this frame
};


// 3-D vector -----------------------------------------------------------------
//
typedef Vertex3 Vector3;


// 2-D rectangle --------------------------------------------------------------
//
struct Rectangle2 {

	rastv_t	left;
	rastv_t	right;
	rastv_t	top;
	rastv_t	bottom;
};


// 3-D plane ------------------------------------------------------------------
//
struct Plane3 {

	geomv_t	X;		// normal[0]
	geomv_t	Y;		// normal[1]
	geomv_t	Z;		// normal[2]
	geomv_t	D;		// distance along normal
};
#if 0
#define PLANE_NORMAL(p)		 	((p)->D, (Vector3*)(p))		// crude type check which will give an error if it fails and a warning otherwise..
#else
#define PLANE_NORMAL(p)		 	((Vector3*)(p))		
#endif
#define PLANE_OFFSET(p)		 	((p)->D)
#define PLANE_AXIAL(p)		 	(DW32((p)->X)==0xf800ff00)	// signaling NaN
#define PLANE_CMPAXIS(p)	 	(DW32((p)->Y)&0x03)			// select coordinate
#define PLANE_AXISCOMP(p)		((p)->Z)					// single component
#define PLANE_MAKEAXIAL(p,c,i)	{DW32((p)->X)=0xf800ff00; DW32((p)->Y)=(i)&0x03; (p)->Z=(c);}


// 3-D sphere -----------------------------------------------------------------
//
struct Sphere3 {

	geomv_t	X;		// origin[0]
	geomv_t	Y;		// origin[1]
	geomv_t	Z;		// origin[2]
	geomv_t	R;		// radius or radius squared, depending on application
};


// 2-D bounding rectangle -----------------------------------------------------
//
struct CullBox2 {

	geomv_t	minmax[ 4 ];	// [0 1]=[minx miny] [3 4]=[maxx maxy]
};


// 3-D bounding box -----------------------------------------------------------
//
struct CullBox3 {

	geomv_t	minmax[ 6 ];	// [0 1 2]=[minx miny minz] [3 4 5]=[maxx maxy maxz]
};


// 3-D test points for trivial reject and accept of a bounding box ------------
//
struct ReAcPointBox3 {

	Vector3	reject;			// in negative halfspace -> trivial reject
	Vector3	accept;			// in positive halfspace -> trivial accept
};


// same as ReAcPointsBox3 but with coordinate indexes for a CullBox3 ----------
//
struct ReAcIndexBox3 {

	int		reject[ 3 ];	// reject coordinate indexes
	int		accept[ 3 ];	// accept coordinate indexes
};


// 3-D cull plane (plane together with corresponding reject/accept indexes) ---
//
struct CullPlane3 {

	Plane3			plane;	// the indexes of reject/accept points are an
	ReAcIndexBox3	reacx;	// invariant property of each plane!
};


// bsp node including culling info --------------------------------------------
//
struct CullBSPNode {

	short	polygons[ 2 ];	// index of first front/back polygon
	short	numpolys[ 2 ];	// number of front/back polygons

	Plane3	plane;			// separator plane
	geomv_t	minmax[ 6 ];	// tree bounding box

	dword	flags;			// node/leaf/separator
	dword	visframe;		// visibility status (frame in which last visible)
	dword	subtrees[ 2 ];	// front- and back-subtree
};

#define NODE_FRONTPOLYGON(n)	((n)->polygons[1])
#define NODE_BACKPOLYGON(n)		((n)->polygons[0])
#define NODE_FRONTNUM(n)		((n)->numpolys[1])
#define NODE_BACKNUM(n)			((n)->numpolys[0])
#define NODE_FRONTSUBTREE(n)	((n)->subtrees[1])
#define NODE_BACKSUBTREE(n)		((n)->subtrees[0])
#define NODE_LEAF(n)			((n)->flags & 0x01)
#define NODE_SEPARATOR(n)		((n)->flags & 0x02)


// flags for iter vertices ----------------------------------------------------
//
enum {

	ITERVTXFLAG_NONE		= 0x00000000,	// default

	ITERVTXFLAG_LAYERS_BASE	= 0x00000000,
	ITERVTXFLAG_LAYERS_1	= 0x00000002,	// ITERVTXFLAG_LAYERS_xx specifies
	ITERVTXFLAG_LAYERS_2	= 0x00000003,	// the number of layers (stored in
	ITERVTXFLAG_LAYERS_3	= 0x00000004,	// IterLayer) apart from the base
	ITERVTXFLAG_LAYERS_4	= 0x00000005,	// layer (IterVertex). e.g., 2
	ITERVTXFLAG_LAYERS_5	= 0x00000006,	// means there is one fully used
	ITERVTXFLAG_LAYERS_6	= 0x00000007,	// IterLayer, 1 that there is one
	ITERVTXFLAG_LAYERS_7	= 0x00000008,	// half used IterLayer
	ITERVTXFLAG_LAYERS_8	= 0x00000009,	// lowest bit is number of
	ITERVTXFLAG_LAYERS_SLM	= 0x00000001,	// sublayers in last IterLayer - 1
	ITERVTXFLAG_LAYERS_MASK	= 0x0000000f,

	ITERVTXFLAG_RESTART		= 0x00000100,	// restart strip (line/triangle)
};


// 2-D vertex with iterative attributes ---------------------------------------
//
struct IterVertex2 {

	rastv_t	X;		// screenspace x
	rastv_t	Y;		// screenspace y
	rastv_t	Z;		// depth-buffer value

	geomv_t	W;		// homogeneous component
	geomv_t	U;		// texture u
	geomv_t	V;		// texture v

	byte	R;		// red component
	byte	G;		// green component
	byte	B;		// blue component
	byte	A;		// alpha component

	dword	flags;	// ITERVTXFLAG_xx
};


// 3-D vertex with iterative attributes ---------------------------------------
//
struct IterVertex3 {

	geomv_t	X;		// viewspace x
	geomv_t	Y;		// viewspace y
	geomv_t	Z;		// viewspace z

	geomv_t	W;		// homogeneous component
	geomv_t	U;		// texture u
	geomv_t	V;		// texture v

	byte	R;		// red component
	byte	G;		// green component
	byte	B;		// blue component
	byte	A;		// alpha component

	dword	flags;	// ITERVTXFLAG_xx
};


// 2-D vertex overlay with additional texture coordinates ---------------------
//
struct IterLayer2 {

	geomv_t	U0;		// texture u (layer 0)
	geomv_t	V0;		// texture v (layer 0)
	geomv_t	W0;		// homogeneous component
	word	mode0;	//
	word	flags0;	//

	geomv_t	U1;		// texture u (layer 1)
	geomv_t	V1;		// texture v (layer 1)
	geomv_t	W1;		// homogeneous component
	word	mode1;	//
	word	flags1;	//
};


// 3-D vertex overlay with additional texture coordinates ---------------------
//
struct IterLayer3 {

	geomv_t	U0;		// texture u (layer 0)
	geomv_t	V0;		// texture v (layer 0)
	geomv_t	W0;		// homogeneous component
	word	mode0;	//
	word	flags0;	//

	geomv_t	U1;		// texture u (layer 1)
	geomv_t	V1;		// texture v (layer 1)
	geomv_t	W1;		// homogeneous component
	word	mode1;	//
	word	flags1;	//
};


// type for specification of iteration/use of vertex attributes ---------------
//
enum itertype_t {

	iter_constrgb			= 0x0000, // constant [rgb] attributes
	iter_constrgba			= 0x0001, // constant [rgba] attributes
	iter_rgb				= 0x0002, // iterated (rgb) attributes
	iter_rgba				= 0x0003, // iterated (rgba) attributes
	iter_texonly			= 0x0004, // apply texture without shading
	iter_texconsta			= 0x0005, // modulate texture with constant [a]
	iter_texconstrgb		= 0x0006, // modulate texture with constant [rgb]
	iter_texconstrgba		= 0x0007, // modulate texture with constant [rgba]
	iter_texa				= 0x0008, // modulate texture with iterated (a)
	iter_texrgb				= 0x0009, // modulate texture with iterated (rgb)
	iter_texrgba			= 0x000a, // modulate texture with iterated (rgba)
	iter_constrgbtexa		= 0x000b, // constant [rgb] with alpha-texture
	iter_constrgbatexa		= 0x000c, // constant [rgba] with alpha-texture
	iter_rgbtexa			= 0x000d, // iterated (rgb) with alpha-texture
	iter_rgbatexa			= 0x000e, // iterated (rgba) with alpha-texture
	iter_texblendrgba		= 0x000f, // blend texture with iterated (rgba)
	iter_texspeca			= 0x0010, // add (a) to texture (specular)
	iter_texspecrgb			= 0x0011, // add (rgb) to texture (specular)
	iter_texconstaspecrgb	= 0x0012, // add (rgb) to faded texture
	iter_texconstrgbspeca	= 0x0013, // add (a) to flat-shaded texture
	iter_texaspecrgb		= 0x0014, // add (rgb) to texture, modulate (a)
	iter_texrgbspeca		= 0x0015, // add (a) to texture, modulate (rgb)
	iter_base_mask			= 0x00ff,

	iter_overwrite			= 0x0000, // overwrite destination
	iter_alphablend			= 0x0100, // alpha blend with destination
	iter_modulate			= 0x0200, // modulate destination (multiply)
	iter_specularadd		= 0x0300, // add to destination (emissive/specular)
	iter_premulblend		= 0x0400, // blend with premultiplied alpha
	iter_additiveblend		= 0x0500, // additively blend with destination
	iter_compose_mask		= 0xff00,
	iter_compose_shift		= 8,
};


// type for specification of desired rasterizer state -------------------------
//
enum raststate_t {

	rast_nozcompare		= 0x0000,	// no depth compare
	rast_zcompare		= 0x0001,	// do depth compare
	rast_mask_zcompare	= 0x0001,	// don't change depth compare

	rast_nozwrite		= 0x0000,	// no depth-buffer write
	rast_zwrite			= 0x0002,	// do depth-buffer write
	rast_mask_zwrite	= 0x0002,	// don't change depth-buffer write

	rast_nozbuffer		= 0x0000,	// no standard depth-buffering
	rast_zbuffer		= 0x0003,	// do standard depth-buffering
	rast_mask_zbuffer	= 0x0003,	// don't change depth-buffering

	rast_texwrap		= 0x0000,	// no texture coordinate clamping
	rast_texclamp		= 0x0004,	// do texture coordinate clamping
	rast_mask_texclamp	= 0x0004,	// don't change coordinate clamp/wrap mode
	rast_mask_texwrap	= 0x0004,	// don't change coordinate clamp/wrap mode

	rast_chromakeyoff	= 0x0000,	// no chroma key compare
	rast_chromakeyon	= 0x0008,	// do chroma key compare
	rast_mask_chromakey	= 0x0008,	// don't change chroma keying

	rast_mask_mipmap	= 0x0010,	// don't change mip-mapping
	rast_mask_texfilter	= 0x0020,	// don't change texture filtering

	rast_default		= 0x0000,	// nozbuffer/texwrap/chromakeyoff
	rast_nomask			= 0x0000,	// don't mask anything
	rast_maskall		= 0xffff,	// mask everything
};


// type for specification of texture combine function -------------------------
//
enum texcombstate_t {

	texcomb_decal		= 0x0000,	// decal mapping (single texture)
	texcomb_trifilter  	= 0x0001,	// trilinearly filtered mip mapping
	texcomb_modulate   	= 0x0002,	// modulate texture with another texture
	texcomb_specularadd	= 0x0003,	// add texture to another texture
	texcomb_detail		= 0x0004,	// blend textures of two detail levels
};


// flags for iter primitives --------------------------------------------------
//
enum {

	ITERFLAG_NONE			= 0x0000,	// don't touch anything (draw as is)
	ITERFLAG_BACKFACES		= 0x0001,	// draw backfaces instead of frontfaces
	ITERFLAG_ONESIDED		= 0x0002,	// single-sided primitive
	ITERFLAG_PLANESTRIP		= 0x0004,	// only one plane for entire strip
	ITERFLAG_VERTEXREFS		= 0x0008,	// vertex pointers, no embedded vertices

	ITERFLAG_Z_DIV_XYZ		= 0x0010,	// divide xyz by z before rasterizing
	ITERFLAG_Z_DIV_UVW		= 0x0020,	// divide uvw by z before rasterizing
	ITERFLAG_Z_TO_DEPTH		= 0x0040,	// create depth buffer z (lerp-depth)
	ITERFLAG_W_NOT_ONE		= 0x0080,	// w is not one (projective mapping)

	ITERFLAG_NONDESTRUCTIVE	= 0x0100,	// don't alter original vertex data

	ITERFLAG_PS_DEFAULT		= 0x0000,	// point style: default
	ITERFLAG_PS_ANTIALIASED	= 0x1000,	// point style: antialiased

	ITERFLAG_LS_DEFAULT		= 0x0000,	// line style: default
	ITERFLAG_LS_ANTIALIASED	= 0x1000,	// line style: antialiased
	ITERFLAG_LS_THICK		= 0x2000,	// line style: thick
	ITERFLAG_LS_STIPPLED	= 0x4000,	// line style: stippled
	ITERFLAG_LS_CLOSE_STRIP	= 0x8000,	// line style: line-loop
};


// forward declarations
struct TextureMap;
struct texfont_s;


// 2-D point (point-set) with iterative vertex attributes ---------------------
//
struct IterPoint2 {

	word		flags;
	short		NumVerts;	// number of points
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	float		pointsize;	// size in pixels
	IterVertex2	Vtxs[ 1 ];	// alloc( (size_t)&((IterPoint2*)0)->Vtxs[ n ] )
};


// 3-D point (point-set) with iterative vertex attributes ---------------------
//
struct IterPoint3 {

	word		flags;
	short		NumVerts;	// number of points
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	float		pointsize;	// size in pixels
	IterVertex3	Vtxs[ 1 ];	// alloc( (size_t)&((IterPoint3*)0)->Vtxs[ n ] )
};


// 2-D line (line-strip) with iterative vertex attributes ---------------------
//
struct IterLine2 {

	word		flags;
	short		NumVerts;	// number of lines + 1
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex2	Vtxs[ 2 ];	// alloc( (size_t)&((IterLine2*)0)->Vtxs[ n ] )
};


// 3-D line (line-strip) with iterative vertex attributes ---------------------
//
struct IterLine3 {

	word		flags;
	short		NumVerts;	// number of lines + 1
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex3	Vtxs[ 2 ];	// alloc( (size_t)&((IterLine3*)0)->Vtxs[ n ] )
};


// 2-D triangle with iterative vertex attributes ------------------------------
//
struct IterTriangle2 {

	word		flags;
	short		NumVerts;	// need not be valid (implicitly 3)
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex2	Vtxs[ 3 ];
};


// 3-D triangle with iterative vertex attributes ------------------------------
//
struct IterTriangle3 {

	word		flags;
	short		NumVerts;	// need not be valid (implicitly 3)
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex3	Vtxs[ 3 ];
};


// 2-D rectangle with iterative vertex attributes -----------------------------
//
struct IterRectangle2 {

	word		flags;
	short		NumVerts;	// need not be valid (implicitly 4)
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex2	Vtxs[ 4 ];
};


// 3-D rectangle with iterative vertex attributes -----------------------------
//
struct IterRectangle3 {

	word		flags;
	short		NumVerts;	// need not be valid (implicitly 4)
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex3	Vtxs[ 4 ];
};


// 2-D polygon (n-gon) with iterative vertex attributes -----------------------
//
struct IterPolygon2 {

	word		flags;
	short		NumVerts;
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex2	Vtxs[ 1 ];	// alloc( (size_t)&((IterPolygon2*)0)->Vtxs[ n ] )
};


// 3-D polygon (n-gon) with iterative vertex attributes -----------------------
//
struct IterPolygon3 {

	word		flags;
	short		NumVerts;
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex3	Vtxs[ 1 ];	// alloc( (size_t)&((IterPolygon3*)0)->Vtxs[ n ] )
};


// 2-D triangle strip with iterative vertex attributes ------------------------
//
struct IterTriStrip2 {

	word		flags;
	short		NumVerts;	// number of triangles + 2
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// array; need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex2	Vtxs[ 1 ];	// alloc( (size_t)&((IterTriStrip2*)0)->Vtxs[ n ] )
};


// 3-D triangle strip with iterative vertex attributes ------------------------
//
struct IterTriStrip3 {

	word		flags;
	short		NumVerts;	// number of triangles + 2
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	Plane3*		plane;		// array; need only be valid if ITERFLAG_ONESIDED
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex3	Vtxs[ 1 ];	// alloc( (size_t)&((IterTriStrip3*)0)->Vtxs[ n ] )
};


// flags for contents of iterated arrays --------------------------------------
//
enum {

	ITERARRAY_USE_COLOR			= 0x0001,
	ITERARRAY_USE_TEXTURE		= 0x0002,
	ITERARRAY_GLOBAL_TEXTURE	= 0x0100,
};


// primitive types for iterated array drawing ---------------------------------
//
enum {

	ITERARRAY_MODE_TRIANGLES	= 0x0000,
	ITERARRAY_MODE_TRISTRIP		= 0x0001,
	ITERARRAY_MODE_TRIFAN		= 0x0002,
	ITERARRAY_MODE_QUADS		= 0x0003,
	ITERARRAY_MODE_QUADSTRIP	= 0x0004,

	ITERARRAY_MODE_NUM_MODES	// last!
};


// 2-D vertex array with iterative vertex attributes --------------------------
//
struct IterArray2 {

	word		flags;
	short		NumVerts;	// array size
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	dword		arrayinfo;	// flags for array content (ITERARRAY_xx)
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex2	Vtxs[ 1 ];	// alloc( (size_t)&((IterArray2*)0)->Vtxs[ n ] )
};


// 3-D vertex array with iterative vertex attributes --------------------------
//
struct IterArray3 {

	word		flags;
	short		NumVerts;	// array size
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	dword		arrayinfo;	// flags for array content (ITERARRAY_xx)
	TextureMap*	texmap;		// need only be valid if itertype >= iter_texonly
	IterVertex3	Vtxs[ 1 ];	// alloc( (size_t)&((IterArray3*)0)->Vtxs[ n ] )
};


// descriptor for rendering with a texfont ------------------------------------
//
struct IterTexfont {

	dword		flags;
	dword		itertype;	// itertype_t
	word		raststate;	// raststate_t (state)
	word		rastmask;	// raststate_t (mask)
	texfont_s*	texfont;	// texfont (contains no rendering or color info)
	IterVertex2	Vtxs[ 4 ];	// output location, color, and scale factors
};


#endif // _OD_PRIM_H_


