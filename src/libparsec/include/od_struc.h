/*
 * PARSEC HEADER (OBJECT)
 * Generic Object Structures V1.80
 *
 * Copyright (c) Markus Hadwiger 1995-2001
 * All Rights Reserved.
 */

#ifndef _OD_STRUC_H_
#define _OD_STRUC_H_

#ifdef PARSEC_SERVER

// forward decls --------------------------------------------------------------
// 
class E_Distributable;

#endif // PARSEC_SERVER


// defines a texture map (size is 32 bytes) -----------------------------------
//
struct TextureMap {

	dword		Geometry;		// code for geometry of texture
	word		Width;			// log2( width in pixels )
	word		Height; 		// log2( height in pixels )
	word		Flags;			// flag word
	byte		LOD_small;		// smallest LOD level (power of two)
	byte		LOD_large;		// largest LOD level (power of two)
	dword		CompFormat;		// impl.-dependent compressed format spec
	char*		BitMap;			// pointer to bitmap (8 bit per texel)
	word*		TexPalette;		// pointer to color look up table for texture
	const char*	TexMapName;		// pointer to texture name
	dword		TexelFormat;	// texel format
};


// single texfont char (size is 8 bytes) --------------------------------------
//
struct texchar_s {

	short		tex_u;			// u coordinate in texture
	short		tex_v;			// v coordinate in texture
	short		tex_id;			// id of texture (relative to texfont)
	signed char	tex_ulead;		// u offset to first real column
	signed char	tex_ustep;		// u advance after drawing this char
};


// entire texfont descriptor (size is 16 bytes) -------------------------------
//
struct texfont_s {

	dword		flags;
	byte		height;			// height of chars (must be equal for all)
	byte		numtextures;	// may have been split up into several textures
	short		numtexchars;	// number of chars in this font
	texchar_s*	texchars;		// table of chars
	TextureMap*	texmap;			// corresponding font textures (array)
};


// single entry (frame) in table of textures ----------------------------------
//
struct texfrm_s {

	int			deltatime;		// delta time to next entry (frame)
	TextureMap*	texmap;			// texture map for this frame
};


// single entry (frame) in table of 2-D (image) transformations ---------------
//
struct xfofrm_s {

	int			deltatime;		// delta time to next entry (frame)
	imgtrafo_s*	imgtrafo;		// image transformation for this frame
};


// texture animation descriptor (size is 32 bytes) ----------------------------
//
struct texanim_s {

	texfrm_s*	tex_table;		// table of textures
	dword		tex_end;		// end index
	dword		tex_start;		// start index
	dword		tex_rep;		// repeat index

	xfofrm_s*	xfo_table;		// table of 2-D (image) transformations
	dword		xfo_end;		// end index
	dword		xfo_start;      // start index
	dword		xfo_rep;        // repeat index
};


// single entry (frame) in color animation table (size is 8 bytes) ------------
//
struct colfrm_s {

	int			deltatime;		// delta time to next entry (frame)
	colrgba_s	color;			// color for this frame
};


// how to combine the color sources in color animations -----------------------
//
enum {

	COLANIM_SOURCENOCOMBINE		= 0x0000,	// no source color combination
	COLANIM_SOURCEADD			= 0x0001,	// add source colors
	COLANIM_SOURCEMUL			= 0x0002,	// multiply source colors
	COLANIM_SOURCE_MASK			= 0x0003
};


// color animation descriptor (size is 16 bytes) ------------------------------
//
struct colanim_s {

	colfrm_s*	col_table0;		// color source 0
	colfrm_s*	col_table1;		// color source 1

	word		col_end0;		// table 0 end index (length-1)
	word		col_end1;		// table 1 end index (length-1)

	dword		col_flags;		// COLANIM_xx
};


// how to combine the source color with the face color ------------------------
//
enum {

	FACE_ANIM_BASEIGNORE		= 0x0000,	// ignore base (face) color
	FACE_ANIM_BASEADD			= 0x0001,	// add anim to base color
	FACE_ANIM_BASEMUL			= 0x0002,	// multiply base color by anim
	FACE_ANIM_BASE_MASK			= 0x0003
};


// instance data for current state of face animation (size is 32 bytes) -------
//
struct FaceAnimState {

// base info

	texanim_s*	TexAnim;		// texture animation
	colanim_s*	ColAnim;		// color animation
	dword		ColFlags;		// FACE_ANIM_xx
	colrgba_s	ColOutput;		// animated output color

// state info

	word		tex_pos;		// current position in texture table
	short		tex_time;		// time left until next advance in table
	word		xfo_pos;		// current position in trafo table
	short		xfo_time;		// time left until next advance in table

	word		col_pos0;		// current position in color source 0
	short		col_time0;		// time left until next advance in table
	word		col_pos1;		// current position in color source 1
	short		col_time1;		// time left until next advance in table
};


// extended face info flags ---------------------------------------------------
//
enum {

	FACE_EXT_NONE               = 0x0000,	// no animations at all
	FACE_EXT_ANIMATETEXTURES	= 0x0001,	// enable texture animation
	FACE_EXT_TRANSFORMTEXTURES	= 0x0002,	// enable texture xfo animation
	FACE_EXT_ANIMATECOLORS		= 0x0004,	// enable color animation
	FACE_EXT_ONESHOT			= 0x0010,	// one-shot animation
	FACE_EXT_ONESHOTRESTORE		= 0x0020,	// restore previous shader
	FACE_EXT_DISABLED			= 0x8000	// temporarily disabled
};


// optional extended face information (size is 8 bytes) -----------------------
//
struct FaceExtInfo {

	dword		Flags;			// FACE_EXT_xx
	dword		StateId;		// index into GenObject::FaceAnimStates[]
};


// face shading flags ---------------------------------------------------------
//
enum {

	FACE_SHADING_DEFAULT		= 0x0000,

	FACE_SHADING_FACECOLOR		= 0x0001,	// use face color (direct/indexed)
	FACE_SHADING_USECOLORINDEX	= 0x0002,	// indexed instead of direct color
	FACE_SHADING_TEXIPOLATE		= 0x0004,	// texture may be interpolated

	FACE_SHADING_ENABLETEXTURE	= 0x0010,	// shader uses texture
	FACE_SHADING_CONSTANTCOLOR	= 0x0020,	// shade with constant color
	FACE_SHADING_NOVERTEXALPHA	= 0x0040,	// shader must ignore vertex alpha

	FACE_SHADING_NODEPTHCMP		= 0x0100,	// disable depth compare
	FACE_SHADING_NODEPTHWRITE	= 0x0200,	// disable depth write

	FACE_SHADING_DRAW_SORTED	= 0x0400,	// draw in depth order
	FACE_SHADING_DRAW_LAST		= 0x0800,	// draw last (overlay)

	FACE_SHADING_NOBACKCULLING	= 0x1000,	// turn off backface culling
	FACE_SHADING_BACK_FIRST		= 0x2000	// draw backfaces, then frontfaces
};


// defines a single object-face (size is 128 bytes) ---------------------------
//
struct Face {

	TextureMap*	TexMap;			// pointer to texture description
	FaceExtInfo* ExtInfo;		// optional extended face info
	dword		ColorRGB;		// RGB color for direct color display
	dword		ColorIndx;		// colorindex for palette mapped display
	dword		FaceNormalIndx; // index of vertex which is the surface normal
	Xmatrx		TexXmatrx;		// matrix for texture placement
	dword		_mtxscratch1;	// scratchpad for matrix code
	Xmatrx		CurTexXmatrx;	// current transformation screen -> texture
	word		ShadingIter;	// shader to use for this face (iter_xx)
	word		ShadingFlags;	// shading flags (FACE_SHADING_xx)
	dword		VisibleFrame;	// ==CurVisibleFrame if face touched this frame
};


// polygon flags --------------------------------------------------------------
//
enum {

	POLYFLAG_DEFAULT		= 0x0000,	// vertex indexes only
	POLYFLAG_CORNERCOLORS	= 0x0001,	// RGBA color array follows
	POLYFLAG_WEDGEINDEXES	= 0x0002	// wedge index array follows
};


// defines a single object-polygon (size is 16 bytes) -------------------------
//
struct Poly {

	dword		NumVerts;		// number of vertices (no surface normal!)
	dword		FaceIndx;		// index of face this polygon belongs to
	dword*		VertIndxs; 		// list of vertexindexes comprising the polygon
	dword		Flags;			// POLYFLAG_xx
};


// node structure for bsp tree nodes (size is 16 bytes) -----------------------
//
struct BSPNode {

	dword		Polygon;		// index of polygon contained in this node
	dword		Contained;		// index of first node in the same plane (list)
	dword		FrontTree;		// index of root-node of front-subtree
	dword		BackTree;		// index of root-node of back-subtree
};


// vertex animation callback function type ------------------------------------
//
struct GenObject;
typedef int (*vtxanim_fpt)( GenObject *gobj, dword animid );


// instance data for current state of vertex animation (size is 128 bytes) ----
//
struct VtxAnimState {

// base info

	dword			NumVerts;			// number of vertices with normals
	dword			NumPolyVerts;		// number of vertices without normals
	dword			NumNormals; 		// number of face normals
	dword			VertexBase;			// base index for vertex subarray
	dword			NumPolys;			// number of polygons
	dword			PolyBase;			// base index for polygon subarray
	dword			NumFaces;			// number of faces
	dword			FaceBase;			// base index for face subarray
	dword			NumWedges;			// number of wedges
	dword			WedgeBase;			// base index for wedge subarray

	vtxanim_fpt		AnimCallback;		// callback function

// state info

	Xmatrx			CurrentXmatrx;		// current transformation matrix
	dword			StateInfo[ 9 ];		// custom animation state info
};


// allow pointer to particle cluster to be declared ---------------------------
//
struct objectbase_pcluster_s;
typedef objectbase_pcluster_s ObjPartCluster;


// forward declaration --------------------------------------------------------
//
struct CustomObject;


// wedge data flags -----------------------------------------------------------
//
enum {

	WEDGEFLAG_ENABLE_TEXCOORDS	= 0x0001
};


// structure of generic graphical object for specific lod ---------------------
//
struct GenLodObject {

	dword			NumVerts;			// number of vertices with normals
	dword			NumPolyVerts;		// number of vertices without normals
	dword			NumNormals; 		// number of face normals
	Vertex3*		VertexList;			// list of all vertices in object space
	Vertex3*		X_VertexList;		// vertices transformed into view space
	SPoint*			S_VertexList;		// vtxs converted to screen coordinates
	dword			NumPolys;			// number of polygons in this object
	Poly*			PolyList;			// list of polygons in this object
	dword			NumFaces;			// number of faces in this object
	Face*			FaceList;			// list of all faces
	dword*			VisPolyList;		// indexes of currently visible polys
	dword*			SortedPolyList;		// poly indexes sorted on attributes
	void*			AuxList;			// ...
	BSPNode*		BSPTree;			// pointer to root of bsp tree
	CullBSPNode*	AuxBSPTree;			// auxiliary bsp tree
	void*			AuxObject;			// object for api-optimized rendering
	dword			NumWedges;			// wedges==subvertices on attributes
	word			NumLayers;			// number of texcoordinates per wedge
	word			WedgeFlags;			// WEDGEFLAG_xx
	dword*			WedgeVertIndxs;		// vertices corresponding to wedges
	Vector3*		WedgeNormals;		// vertex/wedge normals
	colrgba_s*		WedgeColors;		// vertex/wedge colors
	TexCoord2*		WedgeTexCoords;		// vertex/wedge texture coordinates
	colrgba_s*		WedgeLighted;		// temp storage for lighted wedges
	colrgba_s*		WedgeSpecular;		// temp storage for specular colors
	colrgba_s*		WedgeFogged;		// temp storage for fogged wedges
	word			ActiveFaceAnims;	// active face anim state subrange
	word			ActiveVtxAnims;		// active vertex anim state subrange
};


// object lod descriptor (size is 16 bytes) -----------------------------------
//
struct GenLodInfo {

	dword			Flags;
	geomv_t			MagTreshold;		// switching thresholds with hysteresis
	geomv_t			MinTreshold;		// (small->large, large->small)
	GenLodObject*	LodObject;			// actual geometry for this lod
};


// structure of generic graphical object --------------------------------------
//
struct GenObject {

	GenObject*		NextObj;			// next object in list; _if_ the list
	GenObject*		PrevObj;			// is doubly linked: previous object
	GenObject*		NextVisObj;			// pointer to next obj in visible list
	dword			ObjectNumber;		// unique number of this objectinstance
	dword			HostObjNumber;		// number this object has on its host
	dword			ObjectType; 		// type this object belongs to
	dword			ObjectClass;		// class this object belongs to
	size_t			InstanceSize;		// size of instance of this object class
	dword			NumVerts;			// number of vertices with normals
	dword			NumPolyVerts;		// number of vertices without normals
	dword			NumNormals; 		// number of face normals
	Vertex3*		VertexList;			// list of all vertices in object space
	Vertex3*		X_VertexList;		// vertices transformed into view space
	SPoint*			S_VertexList;		// vtxs converted to screen coordinates
	dword			NumPolys;			// number of polygons in this object
	Poly*			PolyList;			// list of polygons in this object
	dword			NumFaces;			// number of faces in this object
	Face*			FaceList;			// list of all faces
	dword			NumVisPolys;		// number of polygon indexes in vislist
	dword*			VisPolyList;		// indexes of currently visible polys
	dword*			SortedPolyList;		// poly indexes sorted on attributes
	void*			AuxList;			// ...
	geomv_t			BoundingSphere; 	// radius of bounding sphere
	geomv_t			BoundingSphere2;	// radius squared of bounding sphere
	Vertex3 		BoundingBox[ 2 ]; 	// min-max vertices of bounding box
	BSPNode*		BSPTree;			// pointer to root of bsp tree
	ObjPartCluster*	AttachedPClusters;	// list of attached particle clusters
	CustomObject*	NotifyCustmObjects;	// customobj list for callback_notify()
	dword			VisibleFrame;		// ==CurVisibleFrame if object visible
	CullBSPNode*	AuxBSPTree;			// auxiliary bsp tree
	dword			CullMask;			// last cull mask set by culling code
	void*			AuxObject;			// object for api-optimized rendering
	dword			NumWedges;			// wedges==subvertices on attributes
	word			NumLayers;			// number of texcoordinates per wedge
	word			WedgeFlags;			// WEDGEFLAG_xx
	dword*			WedgeVertIndxs;		// vertices corresponding to wedges
	Vector3*		WedgeNormals;		// vertex/wedge normals
	colrgba_s*		WedgeColors;		// vertex/wedge colors
	TexCoord2*		WedgeTexCoords;		// vertex/wedge texture coordinates
	colrgba_s*		WedgeLighted;		// temp storage for lighted wedges
	colrgba_s*		WedgeSpecular;		// temp storage for specular colors
	colrgba_s*		WedgeFogged;		// temp storage for fogged wedges
	word			CurrentLod;			// currently active detail level
	word			NumLodObjects;		// number of available detail levels
	GenLodInfo*		LodObjects;			// detail level descriptors
	word			NumFaceAnims;		// number of face anim states (maximum)
	word			ActiveFaceAnims;	// active state subrange (from start)
	FaceAnimState*	FaceAnimStates;		// face anim states (must be instanced!)
	word			NumVtxAnims;		// number of vtx anim states (maximum)
	word			ActiveVtxAnims;		// active state subrange (from start)
	VtxAnimState*	VtxAnimStates;		// vtx anim states (must be instanced!)
	dword			_mtxscratch1;		// scratchpad for matrix code
	Xmatrx			ObjPosition;		// location and orientation in worldsp.
	dword			_mtxscratch2;		// scratchpad for matrix code
	Xmatrx			CurrentXmatrx;		// current objspace -> viewspace xform
#ifdef PARSEC_SERVER
	E_Distributable*	pDist;				// pointer to the E_Distributable attached to this engine object
#endif // PARSEC_SERVER
};


#endif // _OD_STRUC_H_


