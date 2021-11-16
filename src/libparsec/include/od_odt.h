/*
 * PARSEC HEADER (OBJECT)
 * ODT/ODT2 Object File Format V1.70
 *
 * Copyright (c) Markus Hadwiger 1995-1999
 * All Rights Reserved.
 */

#ifndef _OD_ODT_H_
#define _OD_ODT_H_



//-----------------------------------------------------------------------------
// ODT1 FORMAT (ODT)                                                          -
//-----------------------------------------------------------------------------


// generic transformation matrix
typedef fixed_t	ODT_Xmatrx[ 3 ][ 4 ];		// [0 0 0 1] (line 4) omitted!


// 3-D vertex coordinates
struct ODT_Vertex3 {

	fixed_t	X;
	fixed_t	Y;
	fixed_t	Z;
	dword	Flags;	// padding to 16 bytes and flags
};


// 2-D point in unsigned integer coordinates (always >= 0)
struct ODT_UPoint {

	dword	X;		// unsigned!
	dword	Y;      // unsigned!
};


// 2-D coordinates after projection; not yet converted to screen coordinates
struct ODT_ProjPoint {

	fixed_t	X;
	fixed_t	Y;		// 8 bytes
};


// 2-D point in screen coordinates (used in vertex list for the edge tracer)
struct ODT_SPoint {

	long	X;		// signed!
	long	Y;		// signed!
};


// types for shading of faces
enum ODT_shadingtype_t {

	ODT_no_shad,			// lighting independent ambient color (no shading!)
	ODT_flat_shad,			// flat shading
	ODT_gouraud_shad,		// gouraud shading
	ODT_afftex_shad,		// affine texture mapping
	ODT_ipol1tex_shad,		// first order interpolated texture mapping (lin)
	ODT_ipol2tex_shad,		// second order interpolated texture mapping (quad)
	ODT_persptex_shad,		// perspective correct texmapping w/o interpolation
	ODT_material_shad,		// shade using material specification
	ODT_texmat_shad		// composite texture and material specification
};


// defines a single object-face (size is 128 bytes)
struct ODT_Face {

	char*			TexMap;			// 4 pointer to texture name
	ODT_UPoint*		TexEqui;		// 4 (u,v) correspondences (texture placement)
	dword			ColorRGB;		// 4 RGB color for direct color display
	dword			ColorIndx;		// 4 colorindex for palette mapped display
	dword			FaceNormalIndx; // 4 index of vertex which is the surface normal
	ODT_Xmatrx		TexXmatrx;		// 48 matrix for texture placement
	dword			_mtxscratch1;	// 4 scratchpad for matrix code
	ODT_Xmatrx		CurTexXmatrx;	// 48 current transformation screen -> texture
	dword			Shading;		// 4 shading type to apply to this face
	dword			_padto_128;		// 4
};

// defines a single object-face (size is 128 bytes)
struct ODT_Face_Hdr {

	dword			TexMap;			// 4 pointer to texture name
	dword			TexEqui;		// 4 (u,v) correspondences (texture placement)
	dword			ColorRGB;		// 4 RGB color for direct color display
	dword			ColorIndx;		// 4 colorindex for palette mapped display
	dword			FaceNormalIndx; // 4 index of vertex which is the surface normal
	ODT_Xmatrx		TexXmatrx;		// 48 matrix for texture placement
	dword			_mtxscratch1;	// 4 scratchpad for matrix code
	ODT_Xmatrx		CurTexXmatrx;	// 48 current transformation screen -> texture
	dword			Shading;		// 4 shading type to apply to this face
	dword			_padto_128;		// 4
};



// defines a single object-polygon (size is 16 bytes)
struct ODT_Poly {

	dword		NumVerts;		// number of vertices (no surface normal!)
	dword		FaceIndx;		// index of face this polygon belongs to
	dword*		VertIndxs; 		// list of vertexindexes comprising the polygon
	dword		_padto_16;
};

struct ODT_Poly_Hdr {
	dword		NumVerts;		// number of vertices (no surface normal!)
	dword		FaceIndx;		// index of face this polygon belongs to
	dword		VertIndxs; 		// list of vertexindexes comprising the polygon
	dword		_padto_16;
};


// list of indexes of currently visible polygons ( DetermineObjVisibility() )
struct ODT_VisPolys {

	dword		NumVisPolys;	// number of polygon indexes in list
	dword		PolyIndxs[ 1 ];	// first element of polygon index list (array)
};


// node structure for bsp tree nodes (size is 16 bytes)
struct ODT_BSPNode {

	dword	Polygon;		// index of polygon contained in this node
	dword	Contained;		// index of first node in the same plane (list)
	dword	FrontTree;		// index of root-node of front-subtree
	dword	BackTree;		// index of root-node of back-subtree
};


// structure of graphical object contained in ODT file ------------------------
//
struct ODT_GenObject_Hdr {
	// this structure is used as a "header" to the file and describes
	// the data contained within via 32-bit offset variables.  In previous
	// code, the members listed as offset were pointers.  Here they will
	// be used to calculate the offset in the data to get the value from.
	// offsets are forced to dword to force a 32 bit value.
	
	dword			NextObj;			// 4 bytes: offset, should be 0 in the file
	dword			PrevObj;			// 4 bytes: offset, should be 0 in the file
	dword			NextVisObj;			// 4 bytes: offset, should be 0 in the file
	dword			ObjectNumber;		// 4 bytes: value, unique number of this objectinstance
	dword			HostObjNumber;		// 4 bytes: value, number this object has on its host
	dword			ObjectType; 		// 4 bytes: value, type this object belongs to
	dword			ObjectClass;		// 4 bytes: value, class this object belongs to
	dword			InstanceSize;		// 4 bytes: value, size of instance of this object class
	dword			NumVerts;			// 4 bytes: value, number of vertices w/ normals
	dword			NumPolyVerts;		// 4 bytes: value, number of vertices w/o normals
	dword			NumNormals; 		// 4 bytes: value, number of face normals
	dword			VertexList;			// 4 bytes: offset, list of all vertices in object space
	dword			X_VertexList;		// 4 bytes: offset, vertices transformed into view space
	dword			P_VertexList;		// 4 bytes: offset, vtxs projected onto view plane
	dword			S_VertexList;		// 4 bytes: offset, vtxs converted to screen coordinates
	dword			NumPolys;			// 4 bytes: value, number of polygons in this object
	dword			PolyList;			// 4 bytes: offset, list of polygons in this object
	dword			NumFaces;			// 4 bytes: value, number of faces in this object
	dword			FaceList;			// 4 bytes: offset, list of all faces
	dword			VisPolyList;		// 4 bytes: offset, indexes of currently visible polys
	fixed_t			FarthestZ;			// 4 bytes: value, currently farthest z of all vertices
	fixed_t			NearestZ;			// 4 bytes: value, currently nearest z of all vertices
	fixed_t			BoundingSphere; 	// 4 bytes: value, radius of bounding sphere
	fixed_t			BoundingSphere2;	// 4 bytes: value, radius squared of bounding sphere
	ODT_Vertex3 	BoundingBox[8]; 	// 128 bytes: value, vertices of bounding box in objectspace ???
	dword			BSPTree;			// 4 bytes: offset, pointer to root of bsp tree
	ODT_Vertex3 	LocalCameraLoc; 	// 16 bytes: value, location of camera in object space
	ODT_Vertex3 	PyrNormals[4];		// 64 bytes: value, normals of view pyramid in obj space
	dword			_mtxscratch1;		// 4 bytes: value, scratchpad for matrix code
	ODT_Xmatrx		ObjPosition;		// 48 bytes: value, location and orientation in worldsp.
	dword			_mtxscratch2;		// 4 bytes: value, scratchpad for matrix code
	ODT_Xmatrx		CurrentXmatrx;		// 48 bytes: value, current objspace -> viewspace xform

};


// structure of graphical object contained in ODT file ------------------------
//
struct ODT_GenObject {
	// this structure is used as a "header" to the file and describes
	// the data contained within via 32-bit offset variables.  In previous
	// code, these were pointers.
	// marking up with sizes in 32 bit land, for 64-bit conversion
	ODT_GenObject*	NextObj;			//  pointers to next and previous obj in
	ODT_GenObject*	PrevObj;			//	doubly linked objectinstance list
	ODT_GenObject*	NextVisObj;			// pointer to next obj in visible list
	dword			ObjectNumber;		// unique number of this objectinstance
	dword			HostObjNumber;		// number this object has on its host
	dword			ObjectType; 		// type this object belongs to
	dword			ObjectClass;		// class this object belongs to
	dword			InstanceSize;		// size of instance of this object class
	dword			NumVerts;			// number of vertices w/ normals
	dword			NumPolyVerts;		// number of vertices w/o normals
	dword			NumNormals; 		// number of face normals
	ODT_Vertex3*	VertexList;			// list of all vertices in object space
	ODT_Vertex3*	X_VertexList;		// vertices transformed into view space
	ODT_ProjPoint*	P_VertexList;		// vtxs projected onto view plane
	ODT_SPoint*		S_VertexList;		// vtxs converted to screen coordinates
	dword			NumPolys;			// number of polygons in this object
	ODT_Poly*		PolyList;			// list of polygons in this object
	dword			NumFaces;			// number of faces in this object
	ODT_Face*		FaceList;			// list of all faces
	ODT_VisPolys*	VisPolyList;		// indexes of currently visible polys
	fixed_t			FarthestZ;			// currently farthest z of all vertices
	fixed_t			NearestZ;			// currently nearest z of all vertices
	fixed_t			BoundingSphere; 	// radius of bounding sphere
	fixed_t			BoundingSphere2;	// radius squared of bounding sphere
	ODT_Vertex3 	BoundingBox[8]; 	// vertices of bounding box in objectspace ???
	ODT_BSPNode*	BSPTree;			// pointer to root of bsp tree
	ODT_Vertex3 	LocalCameraLoc; 	// location of camera in object space
	ODT_Vertex3 	PyrNormals[4];		// normals of view pyramid in obj space
	dword			_mtxscratch1;		// scratchpad for matrix code
	ODT_Xmatrx		ObjPosition;		// location and orientation in worldsp.
	dword			_mtxscratch2;		// scratchpad for matrix code
	ODT_Xmatrx		CurrentXmatrx;		// current objspace -> viewspace xform

};



//-----------------------------------------------------------------------------
// ODT2 FORMAT (OD2)                                                          -
//-----------------------------------------------------------------------------


// generic transformation matrix
typedef float	OD2_Xmatrx[ 3 ][ 4 ];		// [0 0 0 1] (line 4) omitted!


// 3-D vertex coordinates
struct OD2_Vertex3 {

	float	X;
	float	Y;
	float	Z;
	dword	Flags;	// padding to 16 bytes and flags
};


// types for shading of faces
enum OD2_shadingtype_t {

	OD2_shad_ambient	= 0x1000,	// fixed color (lighting independent ambient color)
	OD2_shad_flat		= 0x1001,	// flat shading
	OD2_shad_gouraud	= 0x1002,	// gouraud shading
	OD2_shad_afftex		= 0x2003,	// affine texture mapping
	OD2_shad_ipol1tex	= 0x2004,	// first order interpolated texture mapping (lin)
	OD2_shad_ipol2tex	= 0x2005,	// second order interpolated texture mapping (quad)
	OD2_shad_persptex	= 0x2006,	// perspective correct mapping without any interpolation
	OD2_shad_material	= 0x1007,	// use material specification
	OD2_shad_texmat		= 0x3008,	// textures modulated by material specification

	OD2_num_shadingtypes = 9,		// MUST SET THIS MANUALLY!!

	OD2_shadmask_base	= 0x00ff,	// mask for basic shading type
	OD2_shadmask_color	= 0x1000,	// mask to denote faces with attached color
	OD2_shadmask_texmap	= 0x2000	// mask to denote texture mapped faces
};


// types for color specification
enum OD2_colormodel_t {

	OD2_col_none,				// shading type has no associated color
	OD2_col_indexed,			// indexed color (color look up table index)
	OD2_col_rgb,				// color specified via separate R,G,B channels
	OD2_col_rgba,				// color specified via separate R,G,B,A channels
	OD2_col_material,			// use material specification

	OD2_num_colormodels
};


// defines a single object-face
struct OD2_Face {

	char*			TexMap;			// pointer to texture name
	dword			ColorRGB;		// RGB color for direct color display
	dword			ColorIndx;		// colorindex for palette mapped display
	dword			FaceNormalIndx; // index of vertex which is the surface normal
	OD2_Xmatrx		TexXmatrx;		// matrix for texture placement
	dword			Shading;		// shading type to apply to this face
	dword			ColorModel;		// type of color specification
};


// defines a single object-polygon
struct OD2_Poly {

	dword		NumVerts;		// number of vertices (no surface normal!)
	dword		FaceIndx;		// index of face this polygon belongs to
	dword*		VertIndxs; 		// list of vertexindexes comprising the polygon
};


// 3-D plane
struct OD2_Plane {

	float	X;					// normal[0]
	float	Y;					// normal[1]
	float	Z;					// normal[2]
	float	D;					// distance along normal
};


// axial bounding box
struct OD2_CullBox {

	float		mins[ 3 ];		// bounding box min-point
	float		maxs[ 3 ];		// bounding box max-point
};


// node structure for bsp tree nodes
struct OD2_BSPNode {

	dword		flags;			// node/leaf/separator

	short		frontpoly;		// index of first front polygon
	short		numfront;		// number of front polygons

	short		backpoly;		// index of first back polygon
	short		numback;		// number of back polygons

	short		fronttree;		// front-subtree
	short		backtree;		// back-subtree

	OD2_Plane	plane;			// separator plane
	OD2_CullBox	box;			// tree bounding box
};


// object node (part of node list attached to root or child in tree) ----------
//
struct OD2_Node {

	dword			NodeType;			// node type
	OD2_Node*		NextNode;			// next node in list

	dword			flags;
	dword			flags2;

	dword			InstanceSize;		// size of instance of this object class
};


// object node containing bsp tree --------------------------------------------
//
struct OD2_Node_BSP : OD2_Node {

	dword			NumVerts;			// number of vertices w/ normals
	dword			NumPolyVerts;		// number of vertices w/o normals
	dword			NumNormals; 		// number of face normals
	OD2_Vertex3*	VertexList;			// list of all vertices in object space

	dword			NumPolys;			// number of polygons in this object
	OD2_Poly*		PolyList;			// list of polygons in this object

	dword			NumFaces;			// number of faces in this object
	OD2_Face*		FaceList;			// list of all faces

	OD2_BSPNode*	BSPTree;			// pointer to root of bsp tree

	OD2_Vertex3		BoundingCenter;		// center of bounding sphere
	float			BoundingSphere; 	// radius of bounding sphere
	OD2_CullBox		BoundingBox;		// axial bounding box
};


// child (non-root) node in ODT2 object graph ---------------------------------
//
struct OD2_Child {

	dword			childflags;
	dword			childflags2;

	OD2_Node*		NodeList;			// first node in attached list
	OD2_Child*		Children[ 2 ];		// child nodes in object graph

	ODT_Xmatrx		NodeTrafo;	   		// object-space transformation (relative)
};


// ODT2 object's root node (includes file header) -----------------------------
//
struct OD2_Root {

	char			odt2[ 6 ];			// signature ("ODT2")
	byte			major;				// major revision
	byte			minor;				// minor revision

	dword			rootflags;
	dword			rootflags2;

	OD2_Node*		NodeList;			// first node in attached list
	OD2_Child*		Children[ 2 ];		// child nodes in object graph

	dword			ObjectType; 		// type this object belongs to
	dword			ObjectClass;		// class this object belongs to
	dword			InstanceSize;		// size of instance of this object class

	dword			NumVerts;			// number of vertices w/ normals
	dword			NumPolyVerts;		// number of vertices w/o normals
	dword			NumNormals; 		// number of face normals
	OD2_Vertex3*	VertexList;			// list of all vertices in object space

	dword			NumPolys;			// number of polygons in this object
	OD2_Poly*		PolyList;			// list of polygons in this object

	dword			NumFaces;			// number of faces in this object
	OD2_Face*		FaceList;			// list of all faces

	dword			NumTextures;		// number of textures

	OD2_Vertex3		BoundingCenter;		// center of bounding sphere
	float			BoundingSphere; 	// radius of bounding sphere
	OD2_CullBox		BoundingBox;		// axial bounding box
};


#endif // _OD_ODT_H_


