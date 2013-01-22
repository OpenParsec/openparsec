//-----------------------------------------------------------------------------
//	BSPLIB HEADER: BRep.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _BREP_H_
#define _BREP_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BspObjectList.h"
#include "BoundingBox.h"
#include "SystemIO.h"
#include "Transform2.h"
#include "Transform3.h"

// qvlib header files
#include <QvState.h>
#include <QvSphere.h>
#include <QvCone.h>
#include <QvCube.h>
#include <QvCylinder.h>
#include <QvIndexedFaceSet.h>


BSPLIB_NAMESPACE_BEGIN


// b-rep class; used by vrml tree traversal to construct objects --------------
//
class BRep : public virtual SystemIO {

public:
	BRep( QvState *state );
	~BRep() { }

	void			BuildFromSpherePrimitive( const QvSphere& spherenode );
	void			BuildFromConePrimitive( const QvCone& spherenode );
	void			BuildFromCubePrimitive( const QvCube& spherenode );
	void			BuildFromCylinderPrimitive( const QvCylinder& spherenode );
	void			BuildFromIndexedFaceSet( const QvIndexedFaceSet& faceset );

public:
	static int		getTriangulation() { return do_triangulation; }
	static void		setTriangulation( int tri ) { do_triangulation = tri; }

	static int		getTessellation() { return tessellation_slices; }
	static void		setTessellation( int tes ) { tessellation_slices = tes; }

	static int		getMaterialFlag() { return use_material_spec; }
	static void		setMaterialFlag( int mfl ) { use_material_spec = mfl; }

	static int		getMirrorTextureVFlag() { return mirror_v_axis; }
	static void		setMirrorTextureVFlag( int mtv ) { mirror_v_axis = mtv; }

private:
	float*			FetchCoordinate3State( int &num );
	float*			FetchTextureCoordinate2State( int &num );
	float*			FetchNormalState( int &num );
	void			FetchTransformationState( Transform3& trafo );
	void			FetchTextureTransformationState( Transform2& trafo );
	int				FetchMaterialState( Material& mat, int indx );
	void			FetchMaterialBindingState( int& binding );
	void			FetchNormalBindingState( int& binding );
	void			FetchShapeHintsState();
	Texture*		CheckTexture2State();
	void			CreateIndexedFace( Texture *texture, int curindex, int numtexindexs, long *texindexs, int v1, int v2, int v3 );
	void			PostProcessObject();

private:
	static int		shapehint_vertexOrdering;
	static int		shapehint_shapeType;
	static int		shapehint_faceType;
	static float	shapehint_creaseAngle;

	static int		use_material_spec;
	static int		mirror_v_axis;
	static int		do_triangulation;
	static int		tessellation_slices;

private:
	QvState*		m_state;
	BspObject*		m_baseobject;
};


BSPLIB_NAMESPACE_END


#endif // _BREP_H_

