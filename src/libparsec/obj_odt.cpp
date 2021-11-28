/*
 * PARSEC - ODT File Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:44 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// C library
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "gd_heads.h"
#include "objstruc.h"
#include "od_odt.h"

// global externals
#include "globals.h"
#ifdef PARSEC_SERVER
#include "e_world_trans.h"
#endif // PARSEC_SERVER

// mathematics header
#include "utl_math.h"

// local module header
#include "obj_odt.h"

// proprietary module headers
#ifdef PARSEC_SERVER
#include "con_aux_sv.h"
#else // !PARSEC_SERVER
#include "con_aux.h"
#include "con_shad.h"
#include "e_shader.h"
//#include "e_supp.h"
#include "obj_ctrl.h"
#endif // !PARSEC_SERVER

#include "obj_clas.h"
#include "obj_type.h"
#include "sys_file.h"
#include "sys_swap.h"

// flags
//#define OD2_BBOX_VALID

// string constants -----------------------------------------------------------
//
static char object_not_found[] = "object \"%s\" not found";
static char object_readerror[] = "object \"%s\" readerror";
static char corrupt_object[] = "corrupt object file.";
static char no_object_mem[] = "not enough mem for object class-data.";

// alignment to enforce for object geometry data ------------------------------
//
#define OBJ_GEOMETRY_ALIGNMENT_VAL 0x1f
#define OBJ_GEOMETRY_ALIGNMENT_MASK (~OBJ_GEOMETRY_ALIGNMENT_VAL)

// default objload_xx flags ---------------------------------------------------
//
#define ODT_OBJLOAD_DEFAULT (OBJLOAD_WEDGENORMALS | OBJLOAD_WEDGELIGHTED | OBJLOAD_POLYWEDGEINDEXES)
#define OD2_OBJLOAD_DEFAULT (OBJLOAD_WEDGENORMALS | OBJLOAD_WEDGELIGHTED | OBJLOAD_POLYWEDGEINDEXES)

// map texture name to pointer ------------------------------------------------
//
PRIVATE
TextureMap *OBJODT_FetchTexture(char *texname)
{
#ifdef PARSEC_SERVER
	return NULL;
#else
	if (texname == NULL)
	{
		return NULL;
	}

	// make sure the texture name is all lower case since it is
	// not possible to specify upper case chars in the console
	strlwr(texname);

	TextureMap *FetchTextureMap(const char *texname);
	TextureMap *tmap = FetchTextureMap(texname);

	// error if no texture of specified name could be found
	if (tmap == NULL)
	{

		// display error message
		//		MSGOUT( "texture %s needed by object not found.", texname );

		// fall back on default texture if possible
		tmap = FetchTextureMap("texinval");
		if (tmap == NULL)
		{

			// hard exit if texture not found and default not available
			PERROR("OBJODT_FetchTexture(): texture not found: \"%s\".", texname);
		}
	}

	return tmap;
#endif // !PARSEC_SERVER
}

// tables for corners and wedges ----------------------------------------------
//
#define MAX_AVERAGE_POLYS_PER_VTX 8

// linear array subrange
struct range_info_s
{

	int start;
	int count;
};

// corner (vertex,poly)
struct corner_info_s
{

	int vertid;
	int polyid;
};

static range_info_s *corner_info_vtxrange = NULL;
static corner_info_s *corner_info_corners = NULL;
static int corner_info_num_corners;

// wedge (set of corners)
struct wedge_info_s
{

	int cornerstart;
	int cornercount;

	Vector3 normal;
};

static range_info_s *wedge_info_vtxrange = NULL;
static wedge_info_s *wedge_info_wedges = NULL;
static int wedge_info_num_wedges;

// generate corner info tables ------------------------------------------------
//
PRIVATE
void OBJODT_CreateVertexCornerInfo(GenObject *gobj)
{
	ASSERT(gobj != NULL);

	ASSERT(corner_info_vtxrange == NULL);
	ASSERT(corner_info_corners == NULL);

	int numverts = gobj->NumPolyVerts;
	int maxpolyids = numverts * MAX_AVERAGE_POLYS_PER_VTX;

	// alloc tables
	corner_info_vtxrange = (range_info_s *)ALLOCMEM(numverts * sizeof(range_info_s));
	if (corner_info_vtxrange == NULL)
		OUTOFMEM(0);
	corner_info_corners = (corner_info_s *)ALLOCMEM(maxpolyids * sizeof(corner_info_s));
	if (corner_info_corners == NULL)
		OUTOFMEM(0);

	int cornerbase = 0;
	int vidbase = gobj->NumNormals;

	//NOTE:
	// sort order is on vertices. i.e., all corners incident
	// with a specific vertex will be stored consecutively.

	// scan all vertices
	for (int vid = 0; vid < numverts; vid++)
	{

		corner_info_vtxrange[vid].start = cornerbase;
		corner_info_vtxrange[vid].count = 0;

		// scan all polys
		for (unsigned int pid = 0; pid < gobj->NumPolys; pid++)
		{

			dword *vindxs = gobj->PolyList[pid].VertIndxs;
			for (int indx = gobj->PolyList[pid].NumVerts; indx > 0; indx--)
			{
				if (*vindxs++ == (dword)(vid + vidbase))
				{
					ASSERT(cornerbase < maxpolyids);
					corner_info_corners[cornerbase].vertid = vid;
					corner_info_corners[cornerbase].polyid = pid;
					corner_info_vtxrange[vid].count++;
					cornerbase++;
				}
			}
		}
	}

	// store number of corners
	corner_info_num_corners = cornerbase;
}

// fetch normal corresponding to corner ---------------------------------------
//
PRIVATE
Vector3 *OBJODT_FetchCornerNormal(GenObject *gobj, int corner)
{
	ASSERT(gobj != NULL);
	ASSERT((dword)corner < (dword)corner_info_num_corners);

	dword cornerpoly = corner_info_corners[corner].polyid;
	ASSERT(cornerpoly < gobj->NumPolys);

	dword cornerface = gobj->PolyList[cornerpoly].FaceIndx;
	ASSERT(cornerface < gobj->NumFaces);

	dword cornernorm = gobj->FaceList[cornerface].FaceNormalIndx;
	ASSERT(cornernorm < gobj->NumNormals);

	return &gobj->VertexList[cornernorm];
}

// additional configurable parameters for object loading ----------------------
//
PUBLIC
odt_loading_params_s odt_loading_params = {

		0,										// maximum number of face anims (0==default)
		0,										// maximum number of vertex anims (0==default)
		FLOAT_TO_GEOMV(0.5f), // threshold for dot product of normals to merge
};

// generate wedge info tables (sets of corners) -------------------------------
//
PRIVATE
void OBJODT_CreateVertexWedgeInfo(GenObject *gobj)
{
	ASSERT(gobj != NULL);

	ASSERT(wedge_info_vtxrange == NULL);
	ASSERT(wedge_info_wedges == NULL);

	int numverts = gobj->NumPolyVerts;
	int maxwedges = corner_info_num_corners;

	// alloc tables
	wedge_info_vtxrange = (range_info_s *)ALLOCMEM(numverts * sizeof(range_info_s));
	if (wedge_info_vtxrange == NULL)
		OUTOFMEM(0);
	memset(wedge_info_vtxrange, 0, numverts * sizeof(range_info_s));

	wedge_info_wedges = (wedge_info_s *)ALLOCMEM(maxwedges * sizeof(wedge_info_s));
	if (wedge_info_wedges == NULL)
		OUTOFMEM(0);
	memset(wedge_info_wedges, 0, maxwedges * sizeof(wedge_info_s));

	//NOTE:
	// sort order is on vertices. i.e., all wedges incident
	// with a specific vertex will be stored consecutively.

	// scan all vertices
	int wedgebase = 0;
	for (int vid = 0; vid < numverts; vid++)
	{

		wedge_info_vtxrange[vid].start = wedgebase;
		wedge_info_vtxrange[vid].count = 0;

		// scan corners for this vertex
		int start = corner_info_vtxrange[vid].start;
		int beyond = corner_info_vtxrange[vid].count + start;

		int runs[32];

		// bring corners into wedge-order, determine number of wedges
		int cbase = start;
		int numwedges = 0;
		for (numwedges = 0; cbase < beyond; numwedges++)
		{

			int crun = 1;
			int cstop = beyond;

			// try to grow run
			int cscan = 0;
			for (cscan = cbase + 1; cscan < cstop;)
			{

				// merging allowed if at least one normal within threshold
				Vector3 *norm = OBJODT_FetchCornerNormal(gobj, cscan);
				int ccmp = 0;
				for (ccmp = cbase; ccmp < cbase + crun; ccmp++)
				{

					Vector3 *cmpnorm = OBJODT_FetchCornerNormal(gobj, ccmp);
					geomv_t ndiff = GEOMV_1 - DOT_PRODUCT(norm, cmpnorm);
					ABS_GEOMV(ndiff);
					if (ndiff < odt_loading_params.merge_normals_threshold)
					{

						// grow run, continue with next element
						crun++;
						cscan++;
						break;
					}
				}

				if (ccmp == cbase + crun)
				{

					// shorten test range
					cstop--;

					// swap non-mergeable corner beyond test range
					corner_info_s temp = corner_info_corners[cstop];
					corner_info_corners[cstop] = corner_info_corners[cscan];
					corner_info_corners[cscan] = temp;
				}
			}

			// store length of run (one wedge)
			ASSERT(numwedges < 32);
			runs[numwedges] = crun;

			// base for next run
			cbase += crun;
		}

		// merge runs (corners) into wedges
		int cornerstart = start;
		for (int wedge = 0; wedge < numwedges; wedge++)
		{

			wedge_info_wedges[wedgebase].cornerstart = cornerstart;
			wedge_info_wedges[wedgebase].cornercount = runs[wedge];

			wedge_info_wedges[wedgebase].normal.X = GEOMV_0;
			wedge_info_wedges[wedgebase].normal.Y = GEOMV_0;
			wedge_info_wedges[wedgebase].normal.Z = GEOMV_0;

			// sum all corner normals for this wedge
			int cornerbeyond = cornerstart + runs[wedge];
			for (int corner = cornerstart; corner < cornerbeyond; corner++)
			{

				Vector3 *cornernorm = OBJODT_FetchCornerNormal(gobj, corner);
				wedge_info_wedges[wedgebase].normal.X += cornernorm->X;
				wedge_info_wedges[wedgebase].normal.Y += cornernorm->Y;
				wedge_info_wedges[wedgebase].normal.Z += cornernorm->Z;
			}

			// normalize resulting normal (averaging implicit)
			NormVctX(&wedge_info_wedges[wedgebase].normal);

			cornerstart = cornerbeyond;

			wedge_info_vtxrange[vid].count++;
			wedgebase++;
		}
	}

	// set total number of wedges
	wedge_info_num_wedges = wedgebase;
}

// free storage for corner and wedge info tables ------------------------------
//
PRIVATE
void OBJODT_FreeVertexCornerWedgeInfo()
{
	// free corner info
	if (corner_info_vtxrange != NULL)
	{
		FREEMEM(corner_info_vtxrange);
		corner_info_vtxrange = NULL;
	}
	if (corner_info_corners != NULL)
	{
		FREEMEM(corner_info_corners);
		corner_info_corners = NULL;
	}

	// free wedge info
	if (wedge_info_vtxrange != NULL)
	{
		FREEMEM(wedge_info_vtxrange);
		wedge_info_vtxrange = NULL;
	}
	if (wedge_info_wedges != NULL)
	{
		FREEMEM(wedge_info_wedges);
		wedge_info_wedges = NULL;
	}
}

// pointer correction macro ---------------------------------------------------
//
#define CORRECT_POINTER(p) (((p) != NULL) ? ((char *)(p) + pdiff) : NULL)

// correct data pointers contained in lod object to new object base -----------
//
PRIVATE
void OBJODT_CorrectPointersLodObject(GenLodObject *lodobj, ptrdiff_t pdiff, int postcorrect)
{
	ASSERT(lodobj != NULL);

	Poly *polylist = lodobj->PolyList;
	Face *facelist = lodobj->FaceList;

	lodobj->VertexList = (Vertex3 *)CORRECT_POINTER(lodobj->VertexList);
	lodobj->X_VertexList = (Vertex3 *)CORRECT_POINTER(lodobj->X_VertexList);
	lodobj->S_VertexList = (SPoint *)CORRECT_POINTER(lodobj->S_VertexList);
	lodobj->PolyList = (Poly *)CORRECT_POINTER(lodobj->PolyList);
	lodobj->FaceList = (Face *)CORRECT_POINTER(lodobj->FaceList);
	lodobj->VisPolyList = (dword *)CORRECT_POINTER(lodobj->VisPolyList);
	lodobj->SortedPolyList = (dword *)CORRECT_POINTER(lodobj->SortedPolyList);
	lodobj->AuxList = (void *)CORRECT_POINTER(lodobj->AuxList);
	lodobj->BSPTree = (BSPNode *)CORRECT_POINTER(lodobj->BSPTree);
	lodobj->AuxBSPTree = (CullBSPNode *)CORRECT_POINTER(lodobj->AuxBSPTree);
	lodobj->AuxObject = (void *)CORRECT_POINTER(lodobj->AuxObject);
	lodobj->WedgeVertIndxs = (dword *)CORRECT_POINTER(lodobj->WedgeVertIndxs);
	lodobj->WedgeNormals = (Vector3 *)CORRECT_POINTER(lodobj->WedgeNormals);
	lodobj->WedgeColors = (colrgba_s *)CORRECT_POINTER(lodobj->WedgeColors);
	lodobj->WedgeTexCoords = (TexCoord2 *)CORRECT_POINTER(lodobj->WedgeTexCoords);
	lodobj->WedgeLighted = (colrgba_s *)CORRECT_POINTER(lodobj->WedgeLighted);
	lodobj->WedgeSpecular = (colrgba_s *)CORRECT_POINTER(lodobj->WedgeSpecular);
	lodobj->WedgeFogged = (colrgba_s *)CORRECT_POINTER(lodobj->WedgeFogged);

	// if correction takes place after relocation
	// the lists are already at their new positions
	if (postcorrect)
	{
		polylist = lodobj->PolyList;
		facelist = lodobj->FaceList;
	}

	// correct pointers contained in poly structures
	for (unsigned int pid = 0; pid < lodobj->NumPolys; pid++)
	{
		if (&polylist[pid] != NULL)
			polylist[pid].VertIndxs = (dword *)CORRECT_POINTER(polylist[pid].VertIndxs);
	}

	// correct pointers contained in face structures
	for (unsigned int fid = 0; fid < lodobj->NumFaces; fid++)
	{
		if (&facelist[fid] != NULL)
			facelist[fid].ExtInfo = (FaceExtInfo *)CORRECT_POINTER(facelist[fid].ExtInfo);
	}
}

// correct data pointers to new object base (base data) -----------------------
//
PRIVATE
void OBJODT_CorrectPointersBaseData(GenObject *obj, ptrdiff_t pdiff, int postcorrect)
{
	ASSERT(obj != NULL);

	GenLodInfo *lodinfo = obj->LodObjects;

	obj->LodObjects = (GenLodInfo *)CORRECT_POINTER(obj->LodObjects);
	obj->FaceAnimStates = (FaceAnimState *)CORRECT_POINTER(obj->FaceAnimStates);
	obj->VtxAnimStates = (VtxAnimState *)CORRECT_POINTER(obj->VtxAnimStates);

	// if correction takes place after relocation
	// the lod infos are already at their new positions
	if (postcorrect)
	{

		lodinfo = obj->LodObjects;

		// correct pointers contained in lod infos
		for (int lod = 0; lod < obj->NumLodObjects; lod++)
		{
			if (&lodinfo[lod] != NULL)
			{
				lodinfo[lod].LodObject = (GenLodObject *)CORRECT_POINTER(lodinfo[lod].LodObject);
				OBJODT_CorrectPointersLodObject(lodinfo[lod].LodObject, pdiff, TRUE);
			}
		}
	}
	else
	{

		// correct pointers contained in lod infos
		for (int lod = 0; lod < obj->NumLodObjects; lod++)
		{
			OBJODT_CorrectPointersLodObject(lodinfo[lod].LodObject, pdiff, FALSE);
			lodinfo[lod].LodObject = (GenLodObject *)CORRECT_POINTER(lodinfo[lod].LodObject);
		}
	}
}

// correct data pointers to new object base (geometry data) -------------------
//
PRIVATE
void OBJODT_CorrectPointersGeomData(GenObject *obj, ptrdiff_t pdiff, int postcorrect)
{

	ASSERT(obj != NULL);

	Poly *polylist = obj->PolyList;
	Face *facelist = obj->FaceList;

	obj->VertexList = (Vertex3 *)CORRECT_POINTER(obj->VertexList);
	obj->X_VertexList = (Vertex3 *)CORRECT_POINTER(obj->X_VertexList);
	obj->S_VertexList = (SPoint *)CORRECT_POINTER(obj->S_VertexList);
	obj->PolyList = (Poly *)CORRECT_POINTER(obj->PolyList);
	obj->FaceList = (Face *)CORRECT_POINTER(obj->FaceList);
	obj->VisPolyList = (dword *)CORRECT_POINTER(obj->VisPolyList);
	obj->SortedPolyList = (dword *)CORRECT_POINTER(obj->SortedPolyList);
	obj->AuxList = (void *)CORRECT_POINTER(obj->AuxList);
	obj->BSPTree = (BSPNode *)CORRECT_POINTER(obj->BSPTree);
	obj->AuxBSPTree = (CullBSPNode *)CORRECT_POINTER(obj->AuxBSPTree);
	obj->AuxObject = (void *)CORRECT_POINTER(obj->AuxObject);
	obj->WedgeVertIndxs = (dword *)CORRECT_POINTER(obj->WedgeVertIndxs);
	obj->WedgeNormals = (Vector3 *)CORRECT_POINTER(obj->WedgeNormals);
	obj->WedgeColors = (colrgba_s *)CORRECT_POINTER(obj->WedgeColors);
	obj->WedgeTexCoords = (TexCoord2 *)CORRECT_POINTER(obj->WedgeTexCoords);
	obj->WedgeLighted = (colrgba_s *)CORRECT_POINTER(obj->WedgeLighted);
	obj->WedgeSpecular = (colrgba_s *)CORRECT_POINTER(obj->WedgeSpecular);
	obj->WedgeFogged = (colrgba_s *)CORRECT_POINTER(obj->WedgeFogged);

	// if correction takes place after relocation
	// the lists are already at their new positions
	if (postcorrect)
	{
		polylist = obj->PolyList;
		facelist = obj->FaceList;
	}

	// correct pointers contained in poly structures
	for (unsigned int pid = 0; pid < obj->NumPolys; pid++)
	{
		if (polylist != NULL)
		{
			polylist[pid].VertIndxs = (dword *)CORRECT_POINTER(polylist[pid].VertIndxs);
		}
	}

	// correct pointers contained in face structures
	for (unsigned int fid = 0; fid < obj->NumFaces; fid++)
	{
		if (facelist != NULL)
		{
			facelist[fid].ExtInfo = (FaceExtInfo *)CORRECT_POINTER(facelist[fid].ExtInfo);
		}
	}
}

void OBJODT_DumpPoly(GenObject *obj)
{
	// correct pointers contained in poly structures
	for (unsigned int pid = 0; pid < obj->NumPolys; pid++)
	{
		if (obj->PolyList != NULL)
		{
			fprintf(stderr, "%d\t%d\t%p\n", obj->PolyList[pid].NumVerts, obj->PolyList[pid].FaceIndx, obj->PolyList[pid].VertIndxs);
			for (size_t i = 0; i < obj->PolyList[pid].NumVerts; i++)
			{
				fprintf(stderr, "%lu\t%d\n", i, obj->PolyList[pid].VertIndxs[i]);
			}
		}
	}
}

// store wedge indexes as poly corner info ------------------------------------
//
PRIVATE
void OBJODT_StorePolyWedgeIndexes(GenObject *gobj, dword flags)
{
	ASSERT(gobj != NULL);

	if ((flags & OBJLOAD_POLYWEDGEINDEXES) == 0)
		return;

	// determine where wedge indexes should be stored
	dword wbase = 1;
	if (flags & OBJLOAD_POLYCORNERCOLORS)
		wbase++;

	// augment polys with wedge indexes
	for (unsigned int pid = 0; pid < gobj->NumPolys; pid++)
	{

		dword *vertindxs = gobj->PolyList[pid].VertIndxs;
		dword *indxbeyond = vertindxs + gobj->PolyList[pid].NumVerts;
		dword *wedgeindxs = vertindxs + (gobj->PolyList[pid].NumVerts * wbase);

		// map all vertex indexes
		for (; vertindxs < indxbeyond; vertindxs++)
		{

			int windx = -1;
			int vindx = *vertindxs - gobj->NumNormals;

			// check all wedges of this vertex
			int wstart = wedge_info_vtxrange[vindx].start;
			int wbeyond = wedge_info_vtxrange[vindx].count + wstart;
			for (; wstart < wbeyond; wstart++)
			{

				// check all corners of this wedge
				int cstart = wedge_info_wedges[wstart].cornerstart;
				int cbeyond = wedge_info_wedges[wstart].cornercount + cstart;
				for (; cstart < cbeyond; cstart++)
				{

					if ((dword)corner_info_corners[cstart].polyid == pid)
					{
						windx = wstart;
						cstart = cbeyond;
						wstart = wbeyond;
					}
				}
			}

			// store corresponding wedge index
			ASSERT(windx != -1);
			*wedgeindxs++ = windx;
		}

		// specify that wedge index array is present
		gobj->PolyList[pid].Flags = POLYFLAG_WEDGEINDEXES;
	}
}

// create wedge data structures -----------------------------------------------
//
PRIVATE
GenObject *OBJODT_CreateWedgeData(GenObject *gobj, size_t *objmemsize, dword flags)
{
	ASSERT(gobj != NULL);
	ASSERT(objmemsize != NULL);

	if ((flags & OBJLOAD_WEDGE_INFO) == 0)
		return gobj;

	// don't override wedges from file
	// (always 0 right now)
	if (gobj->NumWedges > 0)
	{
		return gobj;
	}

	size_t basesize = *objmemsize;

	// create corner and wedge info,
	// determine number of wedges
	OBJODT_CreateVertexCornerInfo(gobj);
	OBJODT_CreateVertexWedgeInfo(gobj);
	dword numwedges = wedge_info_num_wedges;

	// GenObject::WedgeVertIndxs
	*objmemsize += sizeof(dword) * numwedges;

	// GenObject::WedgeNormals
	if (flags & OBJLOAD_WEDGENORMALS)
	{
		*objmemsize += sizeof(Vector3) * numwedges;
	}

	// GenObject::WedgeColors
	if (flags & OBJLOAD_WEDGECOLORS)
	{
		*objmemsize += sizeof(colrgba_s) * numwedges;
	}

	// GenObject::WedgeTexCoords
	if (flags & OBJLOAD_WEDGETEXCOORDS)
	{
		*objmemsize += sizeof(TexCoord2) * numwedges;
	}

	// GenObject::WedgeLighted
	if (flags & OBJLOAD_WEDGELIGHTED)
	{
		*objmemsize += sizeof(colrgba_s) * numwedges;
	}

	// GenObject::WedgeSpecular
	if (flags & OBJLOAD_WEDGESPECULAR)
	{
		*objmemsize += sizeof(colrgba_s) * numwedges;
	}

	// GenObject::WedgeFogged
	if (flags & OBJLOAD_WEDGEFOGGED)
	{
		*objmemsize += sizeof(colrgba_s) * numwedges;
	}

	// allocate memory for expanded object
	char *curobjmem = (char *)ALLOCMEM(*objmemsize);
	if (curobjmem == NULL)
		OUTOFMEM(no_object_mem);

	// copy original data
	memset(curobjmem, 0, *objmemsize);
	memcpy(curobjmem, gobj, basesize);

	GenObject *cobj = gobj;
	gobj = (GenObject *)curobjmem;

	// correct pointers
	ptrdiff_t pdiff = (char *)gobj - (char *)cobj;
	OBJODT_CorrectPointersBaseData(gobj, pdiff, TRUE);
	OBJODT_CorrectPointersGeomData(gobj, pdiff, TRUE);

	// free old object
	FREEMEM(cobj);
	char *heappos = curobjmem + basesize;

	// store number of wedges
	gobj->NumWedges = numwedges;

	// store pointers to wedge data
	gobj->WedgeVertIndxs = (dword *)heappos;
	heappos += sizeof(dword) * numwedges;

	if (flags & OBJLOAD_WEDGENORMALS)
	{
		gobj->WedgeNormals = (Vector3 *)heappos;
		heappos += sizeof(Vector3) * numwedges;
	}
	if (flags & OBJLOAD_WEDGECOLORS)
	{
		gobj->WedgeColors = (colrgba_s *)heappos;
		heappos += sizeof(colrgba_s) * numwedges;
	}
	if (flags & OBJLOAD_WEDGETEXCOORDS)
	{
		gobj->WedgeTexCoords = (TexCoord2 *)heappos;
		heappos += sizeof(TexCoord2) * numwedges;
	}
	if (flags & OBJLOAD_WEDGELIGHTED)
	{
		gobj->WedgeLighted = (colrgba_s *)heappos;
		heappos += sizeof(colrgba_s) * numwedges;
	}
	if (flags & OBJLOAD_WEDGESPECULAR)
	{
		gobj->WedgeSpecular = (colrgba_s *)heappos;
		heappos += sizeof(colrgba_s) * numwedges;
	}
	if (flags & OBJLOAD_WEDGEFOGGED)
	{
		gobj->WedgeFogged = (colrgba_s *)heappos;
		heappos += sizeof(colrgba_s) * numwedges;
	}

	// store wedge data
	for (unsigned int wedge = 0; wedge < numwedges; wedge++)
	{

		int vid = corner_info_corners[wedge_info_wedges[wedge].cornerstart].vertid;
		gobj->WedgeVertIndxs[wedge] = vid + gobj->NumNormals;

		if (flags & OBJLOAD_WEDGENORMALS)
		{
			gobj->WedgeNormals[wedge] = wedge_info_wedges[wedge].normal;
		}
		if (flags & OBJLOAD_WEDGECOLORS)
		{
			//TODO:
		}
		if (flags & OBJLOAD_WEDGETEXCOORDS)
		{
			//TODO:
		}
	}

	// store wedge indexes as poly corner info
	OBJODT_StorePolyWedgeIndexes(gobj, flags);

	// free temporary tables
	OBJODT_FreeVertexCornerWedgeInfo();

	return gobj;
}

// determine if face is texture mapped ----------------------------------------
//
PRIVATE
int ODT_FaceTextured(ODT_Face *face)
{
	return ((face->Shading == ODT_afftex_shad) ||
					(face->Shading == ODT_ipol1tex_shad) ||
					(face->Shading == ODT_ipol2tex_shad) ||
					(face->Shading == ODT_persptex_shad));
}

PRIVATE
int ODT_FaceTextured32(ODT_Face32 *face)
{
	return ((face->Shading == ODT_afftex_shad) ||
					(face->Shading == ODT_ipol1tex_shad) ||
					(face->Shading == ODT_ipol2tex_shad) ||
					(face->Shading == ODT_persptex_shad));
}

// these are global to reduce automatic variables in recursive traversal ------
//
static ODT_BSPNode *odt_tree;
static BSPNode *gen_tree;
static CullBSPNode *aux_tree;
static GenObject *base_obj;

static int odt_numnodes;
static int odt_numcontained;
static int odt_maxnodeid;

// recursively swap bsp tree and determine number of nodes --------------------
//
PRIVATE
void ODT_SwapBSPTree(int node, int inlist)
{
	if (node == 0)
		return;

	if (node > odt_maxnodeid)
		odt_maxnodeid = node;

	odt_tree[node].Polygon = SWAP_32(odt_tree[node].Polygon);
	odt_tree[node].Contained = SWAP_32(odt_tree[node].Contained);
	odt_tree[node].FrontTree = SWAP_32(odt_tree[node].FrontTree);
	odt_tree[node].BackTree = SWAP_32(odt_tree[node].BackTree);

	if (odt_tree[node].BackTree > 0)
	{
		ASSERT(!inlist);
		ODT_SwapBSPTree(odt_tree[node].BackTree, FALSE);
	}

	if (odt_tree[node].FrontTree > 0)
	{
		ASSERT(!inlist);
		ODT_SwapBSPTree(odt_tree[node].FrontTree, FALSE);
	}

	if (odt_tree[node].Contained > 0)
	{
		ODT_SwapBSPTree(odt_tree[node].Contained, TRUE);
	}

	if (inlist)
	{
		odt_numcontained++;
	}
	else
	{
		odt_numnodes++;
	}
}

// recursively build bsp tree -------------------------------------------------
//
PRIVATE
void ODT_BuildBSPTree(int node)
{
	if (node == 0)
		return;

	ASSERT((dword)node <= (dword)odt_maxnodeid);

	gen_tree[node].Polygon = odt_tree[node].Polygon;
	gen_tree[node].Contained = odt_tree[node].Contained;
	gen_tree[node].FrontTree = odt_tree[node].FrontTree;
	gen_tree[node].BackTree = odt_tree[node].BackTree;

	if (odt_tree[node].BackTree > 0)
	{
		ODT_BuildBSPTree(odt_tree[node].BackTree);
	}

	if (odt_tree[node].FrontTree > 0)
	{
		ODT_BuildBSPTree(odt_tree[node].FrontTree);
	}

	if (odt_tree[node].Contained > 0)
	{
		ODT_BuildBSPTree(odt_tree[node].Contained);
	}
}

// recursively build aux bsp tree ---------------------------------------------
//
PRIVATE
void ODT_BuildBSPTreeAux(int node)
{
	if (node == 0)
		return;

	ASSERT((dword)node <= (dword)odt_maxnodeid);

	dword polyid = odt_tree[node].Polygon;
	ASSERT(polyid < 32768);

	aux_tree[node].polygons[0] = 0;
	aux_tree[node].polygons[1] = (short)polyid;
	aux_tree[node].numpolys[0] = 0;
	aux_tree[node].numpolys[1] = 1;

	// node polygon
	ASSERT(polyid < base_obj->NumPolys);
	Poly *poly = &base_obj->PolyList[polyid];

	// dot node normal with first polygon vertex
	dword normalindx = base_obj->FaceList[poly->FaceIndx].FaceNormalIndx;
	geomv_t planeoffset = DOT_PRODUCT(&base_obj->VertexList[normalindx],
																		&base_obj->VertexList[*poly->VertIndxs]);
	// set explicit plane spec
	aux_tree[node].plane.X = base_obj->VertexList[normalindx].X;
	aux_tree[node].plane.Y = base_obj->VertexList[normalindx].Y;
	aux_tree[node].plane.Z = base_obj->VertexList[normalindx].Z;
	aux_tree[node].plane.D = planeoffset;

	//	aux_tree[ node ].minmax			= ?;

	aux_tree[node].flags = 0;
	aux_tree[node].visframe = 0;
	aux_tree[node].subtrees[0] = odt_tree[node].BackTree;
	aux_tree[node].subtrees[1] = odt_tree[node].FrontTree;

	if (odt_tree[node].BackTree > 0)
	{
		ODT_BuildBSPTreeAux(odt_tree[node].BackTree);
	}

	if (odt_tree[node].FrontTree > 0)
	{
		ODT_BuildBSPTreeAux(odt_tree[node].FrontTree);
	}

	//FIXME:
	// strip contained list
	//	if ( odt_tree[ node ].Contained > 0 ) {
	//		ODT_BuildBSPTreeAux( odt_tree[ node ].Contained );
	//	}
}

// convert odt shading spec into internal shader spec -------------------------
//
PRIVATE
void ODT_ConvertShading(GenObject *gobj, dword faceid, ODT_Face32 *odtface, shader_s *shader)
{
#ifdef PARSEC_CLIENT

	ASSERT(gobj != NULL);
	ASSERT(faceid < gobj->NumFaces);
	ASSERT(odtface != NULL);

	// allow overriding shader specified in file
	if (SetFaceShader(gobj, ACTIVE_LOD, faceid, shader))
		return;

	Face *face = &gobj->FaceList[faceid];

	switch (odtface->Shading)
	{

	case ODT_no_shad:
	case ODT_flat_shad:
	case ODT_gouraud_shad:
		face->ShadingIter = iter_rgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_USECOLORINDEX;
		break;

	case ODT_afftex_shad:
	case ODT_ipol1tex_shad:
	case ODT_ipol2tex_shad:
		face->ShadingIter = iter_texrgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_ENABLETEXTURE | FACE_SHADING_TEXIPOLATE;
		break;

	case ODT_persptex_shad:
		face->ShadingIter = iter_texrgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_ENABLETEXTURE;
		break;

		//		case ODT_material_shad:
		//		case ODT_texmat_shad:

	default:
		PANIC("invalid ODT shading specification.");
	}

#endif // PARSEC_CLIENT
}

// create (internal) object from (external) odt object ------------------------
//
PRIVATE
size_t ODT_CreateObject(ODT_GenObject32 *c32obj, dword flags, shader_s *shader)
{
	ASSERT(c32obj != NULL);

	// default flags may be requested
	if (flags == OBJLOAD_DEFAULT)
	{
		flags = ODT_OBJLOAD_DEFAULT;
	}
	ODT_GenObject *cobj = (ODT_GenObject *)ALLOCMEM(sizeof(ODT_GenObject));
	//
	//  Create a defined state
	memset(cobj, 0, sizeof(ODT_GenObject));

	// swap important header fields
	cobj->ObjectType = SWAP_32(c32obj->ObjectType);
	cobj->ObjectClass = SWAP_32(c32obj->ObjectClass);
	cobj->InstanceSize = SWAP_32(c32obj->InstanceSize);
	cobj->NumVerts = SWAP_32(c32obj->NumVerts);
	cobj->NumPolyVerts = SWAP_32(c32obj->NumPolyVerts);
	cobj->NumNormals = SWAP_32(c32obj->NumNormals);
	cobj->NumPolys = SWAP_32(c32obj->NumPolys);
	cobj->NumFaces = SWAP_32(c32obj->NumFaces);
	cobj->BoundingSphere2 = c32obj->BoundingSphere2;
	cobj->BoundingSphere = c32obj->BoundingSphere;


	//NOTE:
	// some very old ODT files have an invalid InstanceSize field.
	// never mind, we recalculate it anyway.

	ASSERT(cobj->NumVerts == cobj->NumPolyVerts + cobj->NumNormals);
	ASSERT(cobj->NumPolys >= cobj->NumFaces);

	// new base address for object data
	size_t newdatabase = (size_t)c32obj;

	// correct header relative pointers in object header to absolute pointers
	cobj->VertexList = (ODT_Vertex3 *)(SWAP_32((size_t)c32obj->pVertexList) + newdatabase);
	cobj->X_VertexList = (ODT_Vertex3 *)(SWAP_32((size_t)c32obj->pX_VertexList) + newdatabase);
	cobj->P_VertexList = (ODT_ProjPoint *)(SWAP_32((size_t)c32obj->pP_VertexList) + newdatabase);
	cobj->S_VertexList = (ODT_SPoint *)(SWAP_32((size_t)c32obj->pS_VertexList) + newdatabase);
	cobj->PolyList = (ODT_Poly *)(SWAP_32((size_t)c32obj->pPolyList) + newdatabase);
	cobj->FaceList = (ODT_Face *)(SWAP_32((size_t)c32obj->pFaceList) + newdatabase);
	cobj->VisPolyList = (ODT_VisPolys *)(SWAP_32((size_t)c32obj->pVisPolyList) + newdatabase);
	cobj->BSPTree = (ODT_BSPNode *)(SWAP_32((size_t)c32obj->pBSPTree) + newdatabase);

	// size of generic header plus size of type specific header
	size_t instancesize = OBJ_FetchTypeSize(cobj->ObjectType);

	// face anim state array may be included at end of instance data
	int numfaceanimstates = 0;
	ptrdiff_t faceanimstatebase = 0;
	if (flags & OBJLOAD_FACEANIMS)
	{

		// default is simply a fixed number of anim states,
		// since we do not know the number of textures
		numfaceanimstates = (odt_loading_params.max_face_anim_states > 0) ? odt_loading_params.max_face_anim_states : 16;

		// safeguard
		if (numfaceanimstates > 256)
		{
			numfaceanimstates = 256;
		}

		size_t alignmentpadding = ((instancesize + 3) & ~0x03) - instancesize;
		size_t sz_faceanimstates = numfaceanimstates * sizeof(FaceAnimState);

		faceanimstatebase = instancesize + alignmentpadding;
		instancesize += sz_faceanimstates + alignmentpadding;
	}

	// calc necessary alignment padding for data (area behind header)
	size_t alignmentpadding = ((instancesize + OBJ_GEOMETRY_ALIGNMENT_VAL) & OBJ_GEOMETRY_ALIGNMENT_MASK) - instancesize;
	ASSERT(alignmentpadding <= OBJ_GEOMETRY_ALIGNMENT_VAL);

	// swap bsp tree and count number of nodes
	odt_numnodes = 0;
	odt_numcontained = 0;
	odt_maxnodeid = 0;
	odt_tree = cobj->BSPTree;

	ODT_SwapBSPTree(1, FALSE);
	//	ASSERT( odt_maxnodeid == odt_numnodes + odt_numcontained ); //FIXME:

	//NOTE:
	// apparently some ODT files contain bsp trees where there
	// are unused nodes in the interior of the array. therefore,
	// the above assertion may indeed fail. should look into this.

	// calc some numbers not available in header
	int numpolyvindexs = ((size_t)cobj->FaceList - (size_t)cobj->PolyList -
												cobj->NumPolys * sizeof(ODT_Poly32)) /
											 sizeof(dword);
	int numodtbspnodes = odt_maxnodeid + 1;

	// reserve an extended face info for every face
	int numfaceexinfos = (numfaceanimstates > 0) ? cobj->NumFaces : 0;

	// determine how many dwords to reserve
	// for each corner in a polygon
	size_t cornersize = 1;
	if (flags & OBJLOAD_POLYCORNERCOLORS)
		cornersize++;
	if (flags & OBJLOAD_POLYWEDGEINDEXES)
		cornersize++;

	// determine sizes of data areas
	size_t sz_vertexlist = cobj->NumVerts * sizeof(Vertex3);
	size_t sz_xvertexlist = cobj->NumVerts * sizeof(Vertex3);
	size_t sz_svertexlist = cobj->NumVerts * sizeof(SPoint);
	size_t sz_polylist = cobj->NumPolys * sizeof(Poly);
	size_t sz_polyindexes = numpolyvindexs * sizeof(dword) * cornersize;
	size_t sz_facelist = cobj->NumFaces * sizeof(Face);
	size_t sz_faceextinfo = numfaceexinfos * sizeof(FaceExtInfo);
	size_t sz_vispolylist = cobj->NumPolys * sizeof(dword);
	size_t sz_bsptree = numodtbspnodes * sizeof(BSPNode);
	size_t sz_auxbsptree = numodtbspnodes * sizeof(CullBSPNode);

	// calc data size
	size_t datasize = 0;
	datasize += sz_vertexlist;	// VertexList
	datasize += sz_xvertexlist; // X_VertexList
	datasize += sz_svertexlist; // S_VertexList
	datasize += sz_polylist;		// PolyList
	datasize += sz_polyindexes; // +poly vertex/wedge indexes
	datasize += sz_facelist;		// FaceList
	datasize += sz_faceextinfo; // +extended face infos
	datasize += sz_vispolylist; // VisPolyList
	datasize += sz_bsptree;			// BSPTree
	datasize += sz_auxbsptree;	// AuxBSPTree

	// calc size of memory block the object will occupy
	size_t objmemsize = instancesize + alignmentpadding + datasize;

	// allocate memory for object
	char *curobjmem = (char *)ALLOCMEM(objmemsize);
	if (curobjmem == NULL)
		OUTOFMEM(no_object_mem);

	// preclear memory
	memset(curobjmem, 0, objmemsize);

	// init pointer to new class
	GenObject *gobj = (GenObject *)curobjmem;

	// init header fields
	gobj->NextObj = NULL;
	gobj->PrevObj = NULL;
	gobj->NextVisObj = NULL;

	gobj->ObjectNumber = 0;
	gobj->HostObjNumber = 0;
	gobj->ObjectType = cobj->ObjectType;
	gobj->ObjectClass = cobj->ObjectClass;
	gobj->InstanceSize = instancesize;

	gobj->NumVerts = cobj->NumVerts;
	gobj->NumPolyVerts = cobj->NumPolyVerts;
	gobj->NumNormals = cobj->NumNormals;
	gobj->NumPolys = cobj->NumPolys;
	gobj->NumFaces = cobj->NumFaces;

	gobj->VertexList = (Vertex3 *)((char *)gobj + instancesize + alignmentpadding);
	gobj->X_VertexList = (Vertex3 *)((char *)gobj->VertexList + sz_vertexlist);
	gobj->S_VertexList = (SPoint *)((char *)gobj->X_VertexList + sz_xvertexlist);
	gobj->PolyList = (Poly *)((char *)gobj->S_VertexList + sz_svertexlist);
	gobj->FaceList = (Face *)((char *)gobj->PolyList + sz_polylist + sz_polyindexes);
	gobj->VisPolyList = (dword *)((char *)gobj->FaceList + sz_facelist + sz_faceextinfo);
	gobj->SortedPolyList = NULL;
	gobj->AuxList = NULL;
	gobj->BSPTree = (BSPNode *)((char *)gobj->VisPolyList + sz_vispolylist);
	gobj->AuxBSPTree = (CullBSPNode *)((char *)gobj->BSPTree + sz_bsptree);
	//						= ( *)			( (char*)gobj->AuxBSPTree	+ sz_auxbsptree );

	gobj->NumFaceAnims = numfaceanimstates;
	gobj->ActiveFaceAnims = 0;
	gobj->FaceAnimStates = (flags & OBJLOAD_FACEANIMS) ? (FaceAnimState *)((char *)gobj + faceanimstatebase) : NULL;

	gobj->NumVtxAnims = 0;
	gobj->ActiveVtxAnims = 0;
	gobj->VtxAnimStates = NULL;

	gobj->BoundingSphere = FIXED_TO_GEOMV(SWAP_32(DW32(cobj->BoundingSphere)));
	gobj->BoundingSphere2 = FIXED_TO_GEOMV(SWAP_32(DW32(cobj->BoundingSphere2)));

	// axial bounding box
	geomv_t mins[3];
	geomv_t maxs[3];

	// init to swapped maximum extents
	for (int dim = 0; dim < 3; dim++)
	{
		mins[dim] = gobj->BoundingSphere;
		maxs[dim] = -gobj->BoundingSphere;
	}

	// init list of vertices
	ODT_Vertex3 *odtvtxs = cobj->VertexList;
	Vertex3 *genvtxs = gobj->VertexList;

	for (int vct = gobj->NumVerts; vct > 0; vct--, odtvtxs++, genvtxs++)
	{

		genvtxs->X = FIXED_TO_GEOMV(SWAP_32(DW32(odtvtxs->X)));
		genvtxs->Y = FIXED_TO_GEOMV(SWAP_32(DW32(odtvtxs->Y)));
		genvtxs->Z = FIXED_TO_GEOMV(SWAP_32(DW32(odtvtxs->Z)));
		genvtxs->VisibleFrame = 0;

		// determine min-max coordinates in each dimension for bounding box
		mins[0] = min(mins[0], genvtxs->X);
		maxs[0] = max(maxs[0], genvtxs->X);

		mins[1] = min(mins[1], genvtxs->Y);
		maxs[1] = max(maxs[1], genvtxs->Y);

		mins[2] = min(mins[2], genvtxs->Z);
		maxs[2] = max(maxs[2], genvtxs->Z);
	}

	// store bounding box via min-max vertices
	gobj->BoundingBox[0].X = mins[0];
	gobj->BoundingBox[0].Y = mins[1];
	gobj->BoundingBox[0].Z = mins[2];

	gobj->BoundingBox[1].X = maxs[0];
	gobj->BoundingBox[1].Y = maxs[1];
	gobj->BoundingBox[1].Z = maxs[2];

	// init list of polygons
	ODT_Poly *odtpolys = cobj->PolyList;
	ODT_Poly32 *odtpolys32 = (ODT_Poly32 *)(cobj->PolyList);
	Poly *genpolys = gobj->PolyList;
	dword *genindxs = (dword *)((char *)(gobj->PolyList) + sz_polylist);

	int numindxs = 0;
	for (int pct = gobj->NumPolys; pct > 0; pct--, odtpolys32++, genpolys++)
	{

		genpolys->NumVerts = SWAP_32(odtpolys32->NumVerts);
		genpolys->FaceIndx = SWAP_32(odtpolys32->FaceIndx);
		genpolys->VertIndxs = genindxs;
		genpolys->Flags = POLYFLAG_DEFAULT;
		//
		//  Only assignment on odtpolys
		odtpolys->VertIndxs = (dword *)(SWAP_32(odtpolys32->pVertIndxs) + newdatabase);

		// grab all vertex indexes
		dword *odtindxs = odtpolys->VertIndxs;
		for (int vict = genpolys->NumVerts; vict > 0; vict--, numindxs++)
		{
			ASSERT(numindxs < numpolyvindexs);
			*genindxs++ = SWAP_32(*odtindxs);
			odtindxs++;
		}

		// skip area reserved for additional corner info
		genindxs += genpolys->NumVerts * (cornersize - 1);
	}
	ASSERT(numindxs == numpolyvindexs);

	// init list of faces
	ODT_Face32 *odtfaces = (ODT_Face32 *)(cobj->FaceList);
	char *texmap;
	for (dword faceid = 0; faceid < gobj->NumFaces; faceid++, odtfaces++)
	{

		Face *face = &gobj->FaceList[faceid];

		// swap shading and texname fields
		odtfaces->Shading = SWAP_32(odtfaces->Shading);
		texmap = ODT_FaceTextured32(odtfaces) ? (char *)(SWAP_32((size_t)odtfaces->pTexMap) + newdatabase) : NULL;

		face->TexMap = OBJODT_FetchTexture(texmap);
		ODT_ConvertShading(gobj, faceid, odtfaces, shader);

		face->ExtInfo = NULL;

		face->ColorRGB = SWAP_32(odtfaces->ColorRGB);
		// set unspecified color to white
		if (face->ColorRGB == 0)
			face->ColorRGB = 0xffffffff;

		face->ColorIndx = SWAP_32(odtfaces->ColorIndx);
		face->FaceNormalIndx = SWAP_32(odtfaces->FaceNormalIndx);
		face->VisibleFrame = VISFRAME_NEVER;

		face->TexXmatrx[0][0] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[0][0])));
		face->TexXmatrx[0][1] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[0][1])));
		face->TexXmatrx[0][2] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[0][2])));
		face->TexXmatrx[0][3] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[0][3])));
		face->TexXmatrx[1][0] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[1][0])));
		face->TexXmatrx[1][1] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[1][1])));
		face->TexXmatrx[1][2] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[1][2])));
		face->TexXmatrx[1][3] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[1][3])));
		face->TexXmatrx[2][0] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[2][0])));
		face->TexXmatrx[2][1] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[2][1])));
		face->TexXmatrx[2][2] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[2][2])));
		face->TexXmatrx[2][3] = FIXED_TO_GEOMV(SWAP_32(DW32(odtfaces->TexXmatrx[2][3])));
	}

	// init bsp tree
	odt_tree = cobj->BSPTree;
	gen_tree = gobj->BSPTree;
	aux_tree = gobj->AuxBSPTree;
	base_obj = gobj;

	ODT_BuildBSPTree(1);
	ODT_BuildBSPTreeAux(1);

	// create wedge data structures
	gobj = OBJODT_CreateWedgeData(gobj, &objmemsize, flags);

	// enter object into class array
	ObjClasses[gobj->ObjectClass] = gobj;

	// init class and type data of object
	OBJ_InitClass(gobj->ObjectClass);
	//    OBJODT_DumpPoly(gobj);

	// return mem size of object
	return objmemsize;
}

// table of textures used by an object ----------------------------------------
//
struct texused_s
{

	dword num;
	TextureMap **table;

} textures_used;

// presort object faces on certain attributes ---------------------------------
//
PRIVATE
void OD2_PreSortAttributes(GenObject *obj)
{
	ASSERT(obj != NULL);

#ifndef PARSEC_SERVER
	// sort polygons on texture
	if (AUX_OBJ_SORT_POLYS_ON_TEXTURE)
	{

		dword *sortedindexes = obj->SortedPolyList;

		// sort textured faces
		for (unsigned int tid = 0; tid < textures_used.num; tid++)
		{

			// map to compare with
			TextureMap *curmap = textures_used.table[tid];

			// process polygon list
			Poly *plist = obj->PolyList;
			for (unsigned int pid = 0; pid < obj->NumPolys; pid++, plist++)
			{

				// store index if same texture
				if (obj->FaceList[plist->FaceIndx].TexMap == curmap)
				{
					*sortedindexes++ = pid;
				}
				ASSERT((sortedindexes - obj->SortedPolyList) <= (int)obj->NumPolys);
			}
		}

		// append faces without textures
		Poly *plist = obj->PolyList;
		for (unsigned int pid = 0; pid < obj->NumPolys; pid++, plist++)
		{

			// store index if no texture
			if (obj->FaceList[plist->FaceIndx].TexMap == NULL)
			{
				*sortedindexes++ = pid;
			}
			ASSERT((sortedindexes - obj->SortedPolyList) <= (int)obj->NumPolys);
		}

		ASSERT((sortedindexes - obj->SortedPolyList) == (int)obj->NumPolys);
	}
	else
#endif // !PARSEC_SERVER
	{
		// ensure the uninitialized list won't be used
		obj->SortedPolyList = NULL;
	}
}

// determine if face is texture mapped ----------------------------------------
//
PRIVATE
int OD2_FaceTextured(OD2_Face *face)
{
	return ((face->Shading == (OD2_shad_afftex & OD2_shadmask_base)) ||
					(face->Shading == (OD2_shad_ipol1tex & OD2_shadmask_base)) ||
					(face->Shading == (OD2_shad_ipol2tex & OD2_shadmask_base)) ||
					(face->Shading == (OD2_shad_persptex & OD2_shadmask_base)));
}

PRIVATE
int OD2_FaceTextured32(OD2_Face32 *face)
{
	return ((face->Shading == (OD2_shad_afftex & OD2_shadmask_base)) ||
					(face->Shading == (OD2_shad_ipol1tex & OD2_shadmask_base)) ||
					(face->Shading == (OD2_shad_ipol2tex & OD2_shadmask_base)) ||
					(face->Shading == (OD2_shad_persptex & OD2_shadmask_base)));
}

// convert od2 shading spec into internal shader spec -------------------------
//
PRIVATE
void OD2_ConvertShading(GenObject *gobj, dword faceid, OD2_Face *odtface, shader_s *shader)
{
#ifdef PARSEC_CLIENT

	ASSERT(gobj != NULL);
	ASSERT(faceid < gobj->NumFaces);
	ASSERT(odtface != NULL);

	// allow overriding shader specified in file
	if (SetFaceShader(gobj, ACTIVE_LOD, faceid, shader))
		return;

	Face *face = &gobj->FaceList[faceid];

	switch (odtface->Shading)
	{

	case (OD2_shad_ambient & OD2_shadmask_base):
	case (OD2_shad_flat & OD2_shadmask_base):
	case (OD2_shad_gouraud & OD2_shadmask_base):
		face->ShadingIter = iter_rgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_USECOLORINDEX;
		break;

	case (OD2_shad_afftex & OD2_shadmask_base):
	case (OD2_shad_ipol1tex & OD2_shadmask_base):
	case (OD2_shad_ipol2tex & OD2_shadmask_base):
		face->ShadingIter = iter_texrgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_ENABLETEXTURE | FACE_SHADING_TEXIPOLATE;
		break;

	case (OD2_shad_persptex & OD2_shadmask_base):
		face->ShadingIter = iter_texrgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_ENABLETEXTURE;
		break;

		//		case ( OD2_shad_material & OD2_shadmask_base ):
		//		case ( OD2_shad_texmat   & OD2_shadmask_base ):

	default:
		PANIC("invalid OD2 shading specification.");
	}
#endif // PARSEC_CLIENT
}

PRIVATE
void OD2_ConvertShading32(GenObject *gobj, dword faceid, OD2_Face32 *odtface, shader_s *shader)
{
#ifdef PARSEC_CLIENT

	ASSERT(gobj != NULL);
	ASSERT(faceid < gobj->NumFaces);
	ASSERT(odtface != NULL);

	// allow overriding shader specified in file
	if (SetFaceShader(gobj, ACTIVE_LOD, faceid, shader))
		return;

	Face *face = &gobj->FaceList[faceid];

	switch (odtface->Shading)
	{

	case (OD2_shad_ambient & OD2_shadmask_base):
	case (OD2_shad_flat & OD2_shadmask_base):
	case (OD2_shad_gouraud & OD2_shadmask_base):
		face->ShadingIter = iter_rgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_USECOLORINDEX;
		break;

	case (OD2_shad_afftex & OD2_shadmask_base):
	case (OD2_shad_ipol1tex & OD2_shadmask_base):
	case (OD2_shad_ipol2tex & OD2_shadmask_base):
		face->ShadingIter = iter_texrgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_ENABLETEXTURE | FACE_SHADING_TEXIPOLATE;
		break;

	case (OD2_shad_persptex & OD2_shadmask_base):
		face->ShadingIter = iter_texrgb | iter_overwrite;
		face->ShadingFlags = FACE_SHADING_ENABLETEXTURE;
		break;

		//		case ( OD2_shad_material & OD2_shadmask_base ):
		//		case ( OD2_shad_texmat   & OD2_shadmask_base ):

	default:
		PANIC("invalid OD2 shading specification.");
	}
#endif // PARSEC_CLIENT
}

// converts a float as contained in odt2 file into native geomv_t -----------
//
INLINE
geomv_t OD2_Geomv_in(float value)
{
	dword tmp = SWAP_32(DW32(value));
	return FLOAT_TO_GEOMV(*(float *)&tmp);
}

// create (internal) object from (external) odt2 object -----------------------
//
PRIVATE
size_t OD2_CreateObject(OD2_Root32 *cobj32, dword flags, shader_s *shader)
{
	ASSERT(cobj32 != NULL);

	// default flags may be requested
	if (flags == OBJLOAD_DEFAULT)
	{
		flags = OD2_OBJLOAD_DEFAULT;
	}

	// currently only version 1.0 valid
	if ((cobj32->major != 1) || (cobj32->minor != 0))
	{
		return FALSE;
	}
	//
	//  Create headerobject for od2_root
	OD2_Root *cobj = (OD2_Root *)ALLOCMEM(sizeof(OD2_Root));

	// swap important header fields
	cobj->rootflags = SWAP_32(cobj32->rootflags);
	cobj->rootflags2 = SWAP_32(cobj32->rootflags2);
	cobj->InstanceSize = SWAP_32(cobj32->InstanceSize);
	cobj->NumVerts = SWAP_32(cobj32->NumVerts);
	cobj->NumPolyVerts = SWAP_32(cobj32->NumPolyVerts);
	cobj->NumNormals = SWAP_32(cobj32->NumNormals);
	cobj->NumPolys = SWAP_32(cobj32->NumPolys);
	cobj->NumFaces = SWAP_32(cobj32->NumFaces);
	cobj->NumTextures = SWAP_32(cobj32->NumTextures);
	cobj->ObjectClass = SWAP_32(cobj32->ObjectClass);
	cobj->ObjectType = SWAP_32(cobj32->ObjectType);
	cobj->BoundingSphere = cobj32->BoundingSphere;

	ASSERT(cobj->InstanceSize == sizeof(OD2_Root));
	ASSERT(cobj->NumVerts >= cobj->NumPolyVerts + cobj->NumNormals);
	ASSERT(cobj->NumPolys >= cobj->NumFaces);

	// new base address for object data
	size_t newdatabase = (size_t)cobj32;

	// correct header relative pointers in object header to absolute pointers
	cobj->NodeList = (OD2_Node *)(SWAP_32((size_t)cobj32->pNodeList) + newdatabase);
	cobj->Children[0] = (OD2_Child *)(SWAP_32((size_t)cobj32->pChildren[0]) + newdatabase);
	cobj->Children[1] = (OD2_Child *)(SWAP_32((size_t)cobj32->pChildren[1]) + newdatabase);
	cobj->VertexList = (OD2_Vertex3 *)(SWAP_32((size_t)cobj32->pVertexList) + newdatabase);
	cobj->PolyList = (OD2_Poly *)(SWAP_32((size_t)cobj32->pPolyList) + newdatabase);
	cobj->FaceList = (OD2_Face *)(SWAP_32((size_t)cobj32->pFaceList) + newdatabase);

	// size of generic header plus size of type specific header
	size_t instancesize = OBJ_FetchTypeSize(cobj->ObjectType);

	// face anim state array may be included at end of instance data
	int numfaceanimstates = 0;
	ptrdiff_t faceanimstatebase = 0;
	if (flags & OBJLOAD_FACEANIMS)
	{

		// default is the number of textures
		numfaceanimstates = (odt_loading_params.max_face_anim_states > 0) ? odt_loading_params.max_face_anim_states : cobj->NumTextures;

		// safeguard
		if (numfaceanimstates > 256)
		{
			numfaceanimstates = 256;
		}

		size_t alignmentpadding = ((instancesize + 3) & ~0x03) - instancesize;
		size_t sz_faceanimstates = numfaceanimstates * sizeof(FaceAnimState);

		faceanimstatebase = instancesize + alignmentpadding;
		instancesize += sz_faceanimstates + alignmentpadding;
	}

	// vertex anim state array may be included at end of instance data
	int numvtxanimstates = 0;
	ptrdiff_t vtxanimstatebase = 0;
	if (flags & OBJLOAD_VTXANIMS)
	{

		// default is simply a fixed number
		numvtxanimstates = (odt_loading_params.max_vtx_anim_states > 0) ? odt_loading_params.max_vtx_anim_states : 4;

		// safeguard
		if (numvtxanimstates > 64)
		{
			numvtxanimstates = 64;
		}

		// number of additional base object info states
		int numbases = 1;

		// need space for every lod (instance data will be taken from lod 0)
		lodinfo_s *lodinfo = ObjectInfo[cobj->ObjectClass].lodinfo;
		if (lodinfo != NULL)
		{
			ASSERT(lodinfo->numlods > 0);
			numvtxanimstates *= lodinfo->numlods;
			numbases = lodinfo->numlods;
		}

		// reserve additional anim states for base object infos (beyond)
		size_t alignmentpadding = ((instancesize + 3) & ~0x03) - instancesize;
		size_t sz_vtxanimstates = (numvtxanimstates + numbases) * sizeof(VtxAnimState);

		vtxanimstatebase = instancesize + alignmentpadding;
		instancesize += sz_vtxanimstates + alignmentpadding;
	}

	// calc necessary alignment padding for data (area behind header)
	size_t alignmentpadding = ((instancesize + OBJ_GEOMETRY_ALIGNMENT_VAL) & OBJ_GEOMETRY_ALIGNMENT_MASK) - instancesize;
	ASSERT(alignmentpadding <= OBJ_GEOMETRY_ALIGNMENT_VAL);

	// calc some numbers not available in header
	int numpolyvindexs = ((size_t)cobj->FaceList - (size_t)cobj->PolyList -
												cobj->NumPolys * sizeof(OD2_Poly32)) /
											 sizeof(dword);

	// reserve an extended face info for every face
	int numfaceexinfos = (numfaceanimstates > 0) ? cobj->NumFaces : 0;

	// determine how many dwords to reserve
	// for each corner in a polygon
	size_t cornersize = 1;
	if (flags & OBJLOAD_POLYCORNERCOLORS)
		cornersize++;
	if (flags & OBJLOAD_POLYWEDGEINDEXES)
		cornersize++;

	// determine sizes of data areas
	size_t sz_vertexlist = cobj->NumVerts * sizeof(Vertex3);
	size_t sz_xvertexlist = cobj->NumVerts * sizeof(Vertex3);
	size_t sz_svertexlist = cobj->NumVerts * sizeof(SPoint);
	size_t sz_sortedpolylist = cobj->NumPolys * sizeof(dword);
	size_t sz_polylist = cobj->NumPolys * sizeof(Poly);
	size_t sz_polyindexes = numpolyvindexs * sizeof(dword) * cornersize;
	size_t sz_facelist = cobj->NumFaces * sizeof(Face);
	size_t sz_faceextinfo = numfaceexinfos * sizeof(FaceExtInfo);
	size_t sz_vispolylist = cobj->NumPolys * sizeof(dword);

	// calc data size
	size_t datasize = 0;
	datasize += sz_vertexlist;		 // VertexList
	datasize += sz_xvertexlist;		 // X_VertexList
	datasize += sz_svertexlist;		 // S_VertexList
	datasize += sz_sortedpolylist; // SortedPolyList
	datasize += sz_polylist;			 // PolyList
	datasize += sz_polyindexes;		 // +poly vertex/wedge indexes
	datasize += sz_facelist;			 // FaceList
	datasize += sz_faceextinfo;		 // +extended face infos
	datasize += sz_vispolylist;		 // VisPolyList

	// calc size of memory block the object will occupy
	size_t objmemsize = instancesize + alignmentpadding + datasize;

	// allocate memory for object
	char *curobjmem = (char *)ALLOCMEM(objmemsize);
	if (curobjmem == NULL)
		OUTOFMEM(no_object_mem);

	// preclear memory
	memset(curobjmem, 0, objmemsize);

	// init pointer to new class
	GenObject *gobj = (GenObject *)curobjmem;

	// init header fields
	gobj->NextObj = NULL;
	gobj->PrevObj = NULL;
	gobj->NextVisObj = NULL;

	gobj->ObjectNumber = 0;
	gobj->HostObjNumber = 0;
	gobj->ObjectType = cobj->ObjectType;
	gobj->ObjectClass = cobj->ObjectClass;
	gobj->InstanceSize = instancesize;

	gobj->NumVerts = cobj->NumVerts;
	gobj->NumPolyVerts = cobj->NumPolyVerts;
	gobj->NumNormals = cobj->NumNormals;
	gobj->NumPolys = cobj->NumPolys;
	gobj->NumFaces = cobj->NumFaces;

	gobj->VertexList = (Vertex3 *)((char *)gobj + instancesize + alignmentpadding);
	gobj->X_VertexList = (Vertex3 *)((char *)gobj->VertexList + sz_vertexlist);
	gobj->S_VertexList = (SPoint *)((char *)gobj->X_VertexList + sz_xvertexlist);
	gobj->SortedPolyList = (dword *)((char *)gobj->S_VertexList + sz_svertexlist);
	gobj->PolyList = (Poly *)((char *)gobj->SortedPolyList + sz_sortedpolylist);
	gobj->FaceList = (Face *)((char *)gobj->PolyList + sz_polylist + sz_polyindexes);
	gobj->VisPolyList = (dword *)((char *)gobj->FaceList + sz_facelist + sz_faceextinfo);
	//						= ( *)			( (char*)gobj->VisPolyList		+ sz_vispolylist );
	gobj->AuxList = NULL;
	gobj->BSPTree = NULL;
	gobj->AuxBSPTree = NULL;

	gobj->NumFaceAnims = numfaceanimstates;
	gobj->ActiveFaceAnims = 0;
	gobj->FaceAnimStates = (flags & OBJLOAD_FACEANIMS) ? (FaceAnimState *)((char *)gobj + faceanimstatebase) : NULL;

	gobj->NumVtxAnims = numvtxanimstates;
	gobj->ActiveVtxAnims = 0;
	gobj->VtxAnimStates = (flags & OBJLOAD_VTXANIMS) ? (VtxAnimState *)((char *)gobj + vtxanimstatebase) : NULL;

	dword tmp = SWAP_32(DW32(cobj->BoundingSphere));
	float boundrad = *(float *)&tmp;
	gobj->BoundingSphere = FLOAT_TO_GEOMV(boundrad);
	gobj->BoundingSphere2 = FLOAT_TO_GEOMV(boundrad * boundrad);

	// axial bounding box
	geomv_t mins[3];
	geomv_t maxs[3];

#ifdef OD2_BBOX_VALID

	// take bbox from file
	for (int dim = 0; dim < 3; dim++)
	{
		mins[dim] = FLOAT_TO_GEOMV(cobj->BoundingBox.mins[dim]);
		maxs[dim] = FLOAT_TO_GEOMV(cobj->BoundingBox.maxs[dim]);
	}

#else

	// init bbox to swapped maximum extents
	for (int dim = 0; dim < 3; dim++)
	{
		mins[dim] = gobj->BoundingSphere;
		maxs[dim] = -gobj->BoundingSphere;
	}

#endif

	// init list of vertices
	OD2_Vertex3 *odtvtxs = cobj->VertexList;
	Vertex3 *genvtxs = gobj->VertexList;

	for (int vct = gobj->NumVerts; vct > 0; vct--, odtvtxs++, genvtxs++)
	{

		genvtxs->X = OD2_Geomv_in(odtvtxs->X);
		genvtxs->Y = OD2_Geomv_in(odtvtxs->Y);
		genvtxs->Z = OD2_Geomv_in(odtvtxs->Z);
		genvtxs->VisibleFrame = 0;

#ifndef OD2_BBOX_VALID

		mins[0] = min(mins[0], genvtxs->X);
		maxs[0] = max(maxs[0], genvtxs->X);

		mins[1] = min(mins[1], genvtxs->Y);
		maxs[1] = max(maxs[1], genvtxs->Y);

		mins[2] = min(mins[2], genvtxs->Z);
		maxs[2] = max(maxs[2], genvtxs->Z);
#endif
	}

	// store bounding box via min-max vertices
	gobj->BoundingBox[0].X = mins[0];
	gobj->BoundingBox[0].Y = mins[1];
	gobj->BoundingBox[0].Z = mins[2];

	gobj->BoundingBox[1].X = maxs[0];
	gobj->BoundingBox[1].Y = maxs[1];
	gobj->BoundingBox[1].Z = maxs[2];

	// init list of polygons
	OD2_Poly32 *odtpolys32 = (OD2_Poly32 *)(cobj->PolyList);
	Poly *genpolys = gobj->PolyList;
	dword *genindxs = (dword *)((char *)gobj->PolyList + sz_polylist);

	int numindxs = 0;
	for (int pct = gobj->NumPolys; pct > 0; pct--, odtpolys32++, genpolys++)
	{

		genpolys->NumVerts = SWAP_32(odtpolys32->NumVerts);
		genpolys->FaceIndx = SWAP_32(odtpolys32->FaceIndx);
		genpolys->VertIndxs = genindxs;
		genpolys->Flags = POLYFLAG_DEFAULT;

		// grab all vertex indexes
		dword *odtindxs = (dword *)(SWAP_32((size_t)odtpolys32->pVertIndxs) + newdatabase);
		for (int vict = genpolys->NumVerts; vict > 0; vict--, numindxs++)
		{
			ASSERT(numindxs < numpolyvindexs);
			*genindxs = SWAP_32(*odtindxs);
			genindxs++;
			odtindxs++;
		}

		// skip area reserved for additional corner info
		genindxs += genpolys->NumVerts * (cornersize - 1);
	}
	ASSERT(numindxs == numpolyvindexs);

	// create temporary texture table
	textures_used.num = cobj->NumTextures;
	textures_used.table = (textures_used.num > 0) ? (TextureMap **)
																											ALLOCMEM(textures_used.num * sizeof(TextureMap *))
																								: NULL;

	// init list of faces
	OD2_Face32 *odtfaces = (OD2_Face32 *)(cobj->FaceList);
	char *texmap;

	unsigned int numtexturesused = 0;
	for (dword faceid = 0; faceid < gobj->NumFaces; faceid++, odtfaces++)
	{

		Face *face = &gobj->FaceList[faceid];

		// swap shading and texname fields
		odtfaces->Shading = SWAP_32(odtfaces->Shading);
		texmap = OD2_FaceTextured32(odtfaces) ? (char *)(SWAP_32((size_t)odtfaces->pTexMap) + newdatabase) : NULL;

		face->TexMap = OBJODT_FetchTexture(texmap);
		OD2_ConvertShading32(gobj, faceid, odtfaces, shader);

		face->ExtInfo = NULL;

		face->ColorRGB = SWAP_32(odtfaces->ColorRGB);
		// set unspecified color to white
		if (face->ColorRGB == 0)
			face->ColorRGB = 0xffffffff;

		face->ColorIndx = SWAP_32(odtfaces->ColorIndx);
		face->FaceNormalIndx = SWAP_32(odtfaces->FaceNormalIndx);
		face->VisibleFrame = VISFRAME_NEVER;

		face->TexXmatrx[0][0] = OD2_Geomv_in(odtfaces->TexXmatrx[0][0]);
		face->TexXmatrx[0][1] = OD2_Geomv_in(odtfaces->TexXmatrx[0][1]);
		face->TexXmatrx[0][2] = OD2_Geomv_in(odtfaces->TexXmatrx[0][2]);
		face->TexXmatrx[0][3] = OD2_Geomv_in(odtfaces->TexXmatrx[0][3]);
		face->TexXmatrx[1][0] = OD2_Geomv_in(odtfaces->TexXmatrx[1][0]);
		face->TexXmatrx[1][1] = OD2_Geomv_in(odtfaces->TexXmatrx[1][1]);
		face->TexXmatrx[1][2] = OD2_Geomv_in(odtfaces->TexXmatrx[1][2]);
		face->TexXmatrx[1][3] = OD2_Geomv_in(odtfaces->TexXmatrx[1][3]);
		face->TexXmatrx[2][0] = OD2_Geomv_in(odtfaces->TexXmatrx[2][0]);
		face->TexXmatrx[2][1] = OD2_Geomv_in(odtfaces->TexXmatrx[2][1]);
		face->TexXmatrx[2][2] = OD2_Geomv_in(odtfaces->TexXmatrx[2][2]);
		face->TexXmatrx[2][3] = OD2_Geomv_in(odtfaces->TexXmatrx[2][3]);

		if (face->TexMap != NULL)
		{

			// enter texture into list if not used before
			ASSERT(textures_used.table != NULL);
			unsigned int tid = 0;
			for (tid = 0; tid < numtexturesused; tid++)
				if (textures_used.table[tid] == face->TexMap)
					break;
			if (tid == numtexturesused)
			{
				ASSERT(numtexturesused < textures_used.num);
				textures_used.table[tid] = face->TexMap;
				numtexturesused++;
			}
		}
	}

#ifdef PARSEC_CLIENT

	// this will only fire if at least two textures are missing (the first
	// missing texture will count texinvalid and therefore not change the count)
	if (numtexturesused != textures_used.num)
	{
		MSGOUT("using %d fewer textures for object than specified.", (textures_used.num - numtexturesused));
	}

#endif // PARSEC_CLIENT

	// create wedge data structures
	gobj = OBJODT_CreateWedgeData(gobj, &objmemsize, flags);

	// perform presorting for face attributes
	OD2_PreSortAttributes(gobj);

	// free temporary texture table
	if (textures_used.table != NULL)
	{
		FREEMEM(textures_used.table);
		textures_used.table = NULL;
	}

	// enter object into class array
	ObjClasses[gobj->ObjectClass] = gobj;

	// init class and type data of object
	OBJ_InitClass(gobj->ObjectClass);

	// return mem size of object
	return objmemsize;
}

// temporary tables to store info about object lods ---------------------------
//
static GenObject *object_lod_classes[MAX_OBJECT_LODS];
static size_t object_lod_sizes[MAX_OBJECT_LODS];

#ifdef PARSEC_SERVER
//FIXME: HACK: this function must be moved from OBJ_CTRL to a shared module

// switch object detail level -------------------------------------------------
//
void OBJ_SwitchObjectLod(GenObject *obj, dword lod)
{
	ASSERT(obj != NULL);
	ASSERT(lod < obj->NumLodObjects);

	// store active lod
	obj->CurrentLod = lod;

	// retrieve source geometry
	ASSERT(obj->LodObjects != NULL);
	GenLodObject *lodobj = obj->LodObjects[lod].LodObject;

	// switch geometry
	ASSERT(lodobj != NULL);
	obj->NumVerts = lodobj->NumVerts;
	obj->NumPolyVerts = lodobj->NumPolyVerts;
	obj->NumNormals = lodobj->NumNormals;
	obj->VertexList = lodobj->VertexList;
	obj->X_VertexList = lodobj->X_VertexList;
	obj->S_VertexList = lodobj->S_VertexList;
	obj->NumPolys = lodobj->NumPolys;
	obj->PolyList = lodobj->PolyList;
	obj->NumFaces = lodobj->NumFaces;
	obj->FaceList = lodobj->FaceList;
	obj->VisPolyList = lodobj->VisPolyList;
	obj->SortedPolyList = lodobj->SortedPolyList;
	obj->AuxList = lodobj->AuxList;
	obj->BSPTree = lodobj->BSPTree;
	obj->AuxBSPTree = lodobj->AuxBSPTree;
	obj->AuxObject = lodobj->AuxObject;
	obj->NumWedges = lodobj->NumWedges;
	obj->NumLayers = lodobj->NumLayers;
	obj->WedgeFlags = lodobj->WedgeFlags;
	obj->WedgeVertIndxs = lodobj->WedgeVertIndxs;
	obj->WedgeNormals = lodobj->WedgeNormals;
	obj->WedgeColors = lodobj->WedgeColors;
	obj->WedgeTexCoords = lodobj->WedgeTexCoords;
	obj->WedgeLighted = lodobj->WedgeLighted;
	obj->WedgeSpecular = lodobj->WedgeSpecular;
	obj->WedgeFogged = lodobj->WedgeFogged;
	obj->ActiveFaceAnims = lodobj->ActiveFaceAnims;
	obj->ActiveVtxAnims = lodobj->ActiveVtxAnims;
}
#endif // PARSEC_SERVER

// merge lod objects into single object ---------------------------------------
//
PRIVATE
size_t InitClassFromODTLods(dword classid)
{
	ASSERT(classid < MAX_DISTINCT_OBJCLASSES);

	lodinfo_s *lodinfo = ObjectInfo[classid].lodinfo;
	ASSERT(lodinfo != NULL);

	int numlods = lodinfo->numlods;
	ASSERT((dword)numlods <= MAX_OBJECT_LODS);

	// determine resulting size
	size_t objmemsize = 0;

	size_t baseinstancesize = object_lod_classes[0]->InstanceSize;
	size_t lodspecsize = numlods * (sizeof(GenLodInfo) + sizeof(GenLodObject));
	objmemsize += baseinstancesize + lodspecsize;
	int lod = 0;
	for (lod = 0; lod < numlods; lod++)
	{

		GenObject *curobj = object_lod_classes[lod];
		ASSERT(curobj != NULL);

		ASSERT(curobj->NumLodObjects == 0);
		ASSERT(curobj->LodObjects == NULL);

		size_t alignmentpadding = ((objmemsize + OBJ_GEOMETRY_ALIGNMENT_VAL) & OBJ_GEOMETRY_ALIGNMENT_MASK) - objmemsize;
		objmemsize += alignmentpadding;

		size_t instancesize = curobj->InstanceSize;
		alignmentpadding = ((instancesize + OBJ_GEOMETRY_ALIGNMENT_VAL) & OBJ_GEOMETRY_ALIGNMENT_MASK) - instancesize;

		objmemsize += object_lod_sizes[lod] - instancesize - alignmentpadding;
	}

	// allocate memory for object
	char *curobjmem = (char *)ALLOCMEM(objmemsize);
	if (curobjmem == NULL)
		OUTOFMEM(no_object_mem);
	memset(curobjmem, 0, objmemsize);

	GenObject *gobj = (GenObject *)curobjmem;
	GenLodInfo *genlodinfo = (GenLodInfo *)(curobjmem + baseinstancesize);
	GenLodObject *genlodobject = (GenLodObject *)(curobjmem + baseinstancesize + numlods * sizeof(GenLodInfo));

	// initialize object
	memcpy(gobj, object_lod_classes[0], baseinstancesize);

	ptrdiff_t pdiff = (char *)gobj - (char *)object_lod_classes[0];
	OBJODT_CorrectPointersBaseData(gobj, pdiff, TRUE);

	gobj->NumLodObjects = numlods;
	gobj->LodObjects = genlodinfo;

	size_t curfillofs = baseinstancesize + lodspecsize;
	for (lod = 0; lod < numlods; lod++)
	{

		GenObject *curobj = object_lod_classes[lod];
		ASSERT(curobj != NULL);

		size_t alignmentpadding = ((curfillofs + OBJ_GEOMETRY_ALIGNMENT_VAL) & OBJ_GEOMETRY_ALIGNMENT_MASK) - curfillofs;
		curfillofs += alignmentpadding;

		size_t instancesize = curobj->InstanceSize;
		alignmentpadding = ((instancesize + OBJ_GEOMETRY_ALIGNMENT_VAL) & OBJ_GEOMETRY_ALIGNMENT_MASK) - instancesize;
		// correct pointers
		pdiff = (char *)gobj - (char *)curobj;
		pdiff += curfillofs - instancesize - alignmentpadding;
		OBJODT_CorrectPointersGeomData(curobj, pdiff, FALSE);

		// copy pointers
		GenLodObject *lodobj = &genlodobject[lod];

		lodobj->NumVerts = curobj->NumVerts;
		lodobj->NumPolyVerts = curobj->NumPolyVerts;
		lodobj->NumNormals = curobj->NumNormals;
		lodobj->VertexList = curobj->VertexList;
		lodobj->X_VertexList = curobj->X_VertexList;
		lodobj->S_VertexList = curobj->S_VertexList;
		lodobj->NumPolys = curobj->NumPolys;
		lodobj->PolyList = curobj->PolyList;
		lodobj->NumFaces = curobj->NumFaces;
		lodobj->FaceList = curobj->FaceList;
		lodobj->VisPolyList = curobj->VisPolyList;
		lodobj->SortedPolyList = curobj->SortedPolyList;
		lodobj->AuxList = curobj->AuxList;
		lodobj->BSPTree = curobj->BSPTree;
		lodobj->AuxBSPTree = curobj->AuxBSPTree;
		lodobj->AuxObject = curobj->AuxObject;
		lodobj->NumWedges = curobj->NumWedges;
		lodobj->NumLayers = curobj->NumLayers;
		lodobj->WedgeFlags = curobj->WedgeFlags;
		lodobj->WedgeVertIndxs = curobj->WedgeVertIndxs;
		lodobj->WedgeNormals = curobj->WedgeNormals;
		lodobj->WedgeColors = curobj->WedgeColors;
		lodobj->WedgeTexCoords = curobj->WedgeTexCoords;
		lodobj->WedgeLighted = curobj->WedgeLighted;
		lodobj->WedgeSpecular = curobj->WedgeSpecular;
		lodobj->WedgeFogged = curobj->WedgeFogged;
		lodobj->ActiveFaceAnims = curobj->ActiveFaceAnims;
		lodobj->ActiveVtxAnims = curobj->ActiveVtxAnims;

		// copy data
		size_t datasize = object_lod_sizes[lod] - instancesize - alignmentpadding;
		memcpy(curobjmem + curfillofs, (char *)curobj + instancesize + alignmentpadding, datasize);
		curfillofs += datasize;

		// store info
		genlodinfo[lod].Flags = 0x0000;
		genlodinfo[lod].MagTreshold = lodinfo->lodmags[lod];
		genlodinfo[lod].MinTreshold = lodinfo->lodmins[lod];
		genlodinfo[lod].LodObject = lodobj;
	}

	// init base to lod 0
	// (mandatory due to pointer correction)
	OBJ_SwitchObjectLod(gobj, 0);

	// set new class pointer
	ObjClasses[classid] = gobj;

	// return mem size of object
	return objmemsize;
}

// load a single object from an odt file, insert into global class table ------
//
PRIVATE
size_t InitClassFromODT(dword classid, dword flags, shader_s *shader)
{
	ASSERT(classid < MAX_DISTINCT_OBJCLASSES);

	// determine file size
	size_t odtobjsize = SYS_GetFileLength(ObjectInfo[classid].file);
	if (odtobjsize == (dword)-1)
		FERROR(object_not_found, ObjectInfo[classid].file);
	if (odtobjsize < sizeof(ODT_GenObject))
		PERROR(corrupt_object);

	// allocate temporary memory for odt object
	char *odtobjmem = (char *)ALLOCMEM(odtobjsize);
	if (odtobjmem == NULL)
		OUTOFMEM(no_object_mem);

	FILE *fp = SYS_fopen(ObjectInfo[classid].file, "rb");
	if (fp == NULL)
		FERROR(object_not_found, ObjectInfo[classid].file);

	// read odt data in one chunk
	if (SYS_fread(odtobjmem, 1, odtobjsize, fp) != odtobjsize)
		FERROR(object_readerror, ObjectInfo[classid].file);

	if (SYS_fclose(fp) != 0)
		FERROR(object_readerror, ObjectInfo[classid].file);

	// determine whether file is ODT2
	int isodt2 = (strcmp(odtobjmem, "ODT2") == 0);

	// actual object mem size
	size_t objmemsize = 0;

	if (isodt2)
	{

		// init important header fields
		OD2_Root32 *cobj = (OD2_Root32 *)odtobjmem;

		cobj->ObjectType = ObjectInfo[classid].type;
		cobj->ObjectClass = classid;

		// convert odt2 to internal object format
		objmemsize = OD2_CreateObject(cobj, flags, shader);
	}
	else
	{

		// init important header fields
		ODT_GenObject32 *cobj = (ODT_GenObject32 *)odtobjmem;

		cobj->ObjectNumber = 0;
		cobj->HostObjNumber = 0;
		cobj->ObjectType = ObjectInfo[classid].type;
		cobj->ObjectClass = classid;

		// convert odt to internal object format
		objmemsize = ODT_CreateObject(cobj, flags, shader);
	}

	// memory for loaded data is temporary
	FREEMEM(odtobjmem);

	// return actual object mem size
	return objmemsize;
}

// load object from odt file --------------------------------------------------
//
int OBJ_LoadODT(dword classid, dword flags, shader_s *shader)
{
	ASSERT(classid < MAX_DISTINCT_OBJCLASSES);

	//NOTE:
	// object type of specified object class must already
	// be valid in the global ObjectInfo[] table before
	// calling this function.

	if (ObjectInfo[classid].lodinfo == NULL)
	{

		// simply load one object class
		return (InitClassFromODT(classid, flags, shader) > 0);
	}
	else
	{

		// fetch info table
		lodinfo_s *lodinfo = ObjectInfo[classid].lodinfo;
		int numlods = lodinfo->numlods;
		if (numlods > MAX_OBJECT_LODS)
		{
			ASSERT(0);
			numlods = MAX_OBJECT_LODS;
		}

		// save base filename
		char *savebase = ObjectInfo[classid].file;

		// load object classes for all lods
		int lod = 0;
		for (lod = 0; lod < numlods; lod++)
		{

			// read lod file, init class
			ObjectInfo[classid].file = lodinfo->filetab[lod];
			size_t objmemsize = InitClassFromODT(classid, flags, shader);
			if (objmemsize == 0)
			{
				// return which lod failed
				return -lod;
			}

			object_lod_classes[lod] = ObjClasses[classid];
			object_lod_sizes[lod] = objmemsize;
		}

		// restore base filename
		ObjectInfo[classid].file = savebase;

		// merge lod objects into single object
		size_t objmemsize = InitClassFromODTLods(classid);

		// free single objects
		for (lod = 0; lod < numlods; lod++)
		{
			FREEMEM(object_lod_classes[lod]);
		}

		return (objmemsize > 0);
	}
}
