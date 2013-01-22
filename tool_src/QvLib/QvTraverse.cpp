#include <QvElement.h>
#include <QvNodes.h>
#include <QvState.h>
//#ifdef WIN32
#include <QvUnknownNode.h>
//#endif


//////////////////////////////////////////////////////////////////////////////
//
// Traversal code for all nodes. The default method (in QvNode) does
// nothing. Because traverse() is defined in the header for ALL node
// classes, each one has an implementation here.
//
//////////////////////////////////////////////////////////////////////////////

// For debugging
static int indent = 0;
static void
announce(const char *className)
{
    for (int i = 0; i < indent; i++)
	printf("\t");
    printf("Traversing a %s\n", className);
}
#define ANNOUNCE(className) announce(QV__QUOTE(className))

#define DEFAULT_TRAVERSE(className)					      \
void									      \
className::traverse(QvState *)						      \
{									      \
    ANNOUNCE(className);						      \
}

//////////////////////////////////////////////////////////////////////////////
//
// Groups.
//
//////////////////////////////////////////////////////////////////////////////

void
QvGroup::traverse(QvState *state)
{
    ANNOUNCE(QvGroup);
    indent++;
    for (int i = 0; i < getNumChildren(); i++)
	getChild(i)->traverse(state);
    indent--;
}

void
QvLevelOfDetail::traverse(QvState *state)
{
    ANNOUNCE(QvLevelOfDetail);
    indent++;

    // ??? In a real implementation, this would choose a child based
    // ??? on the projected screen areas.
    if (getNumChildren() > 0)
	getChild(0)->traverse(state);

    indent--;
}

void
QvSeparator::traverse(QvState *state)
{
    ANNOUNCE(QvSeparator);
    state->push();
    indent++;
    for (int i = 0; i < getNumChildren(); i++)
	getChild(i)->traverse(state);
    indent--;
    state->pop();
}

void
QvSwitch::traverse(QvState *state)
{
    ANNOUNCE(QvSwitch);
    indent++;

    int which = whichChild.value;

    if (which == QV_SWITCH_NONE)
	;

    else if (which == QV_SWITCH_ALL)
	for (int i = 0; i < getNumChildren(); i++)
	    getChild(i)->traverse(state);

    else
	if (which < getNumChildren())
	    getChild(which)->traverse(state);

    indent--;
}

void
QvTransformSeparator::traverse(QvState *state)
{
    ANNOUNCE(QvTransformSeparator);

    // We need to "push" just the transformation stack. We'll
    // accomplish this by just pushing a no-op transformation onto
    // that stack. When we "pop", we'll restore that stack to its
    // previous state.

    QvElement *markerElt = new QvElement;
    markerElt->data = this;
    markerElt->type = QvElement::NoOpTransform;
    state->addElement(QvState::TransformationIndex, markerElt);

    indent++;
    for (int i = 0; i < getNumChildren(); i++)
	getChild(i)->traverse(state);
    indent--;

    // Now do the "pop"
    while (state->getTopElement(QvState::TransformationIndex) != markerElt)
	state->popElement(QvState::TransformationIndex);
}

//////////////////////////////////////////////////////////////////////////////
//
// Properties.
//
//////////////////////////////////////////////////////////////////////////////

#define DO_PROPERTY(className, stackIndex)				      \
void									      \
className::traverse(QvState *state)					      \
{									      \
    ANNOUNCE(className);						      \
    QvElement *elt = new QvElement;					      \
    elt->data = this;							      \
    state->addElement(QvState::stackIndex, elt);			      \
}

#define DO_TYPED_PROPERTY(className, stackIndex, eltType)		      \
void									      \
className::traverse(QvState *state)					      \
{									      \
    ANNOUNCE(className);						      \
    QvElement *elt = new QvElement;					      \
    elt->data = this;							      \
    elt->type = QvElement::eltType;					      \
    state->addElement(QvState::stackIndex, elt);			      \
}


void QvCoordinate3::traverse(QvState *state)
{
    ANNOUNCE(QvCoordinate3);
    QvElement *elt = new QvElement;
    elt->data = this;
    state->addElement(QvState::Coordinate3Index, elt);

//[MSH_START]
/*
	for ( int i = 0; i < point.num; i++ )
		printf( "point[%d]=%f %f %f\n", i, point.values[ i*3 ], point.values[ i*3 + 1 ], point.values[ i*3 + 2 ] );
*/
//[MSH_END]

	printf( "---------------------------------------\n" );
}
//DO_PROPERTY(QvCoordinate3,		Coordinate3Index)
//--------------------------------------------

DO_PROPERTY(QvMaterial,			MaterialIndex)
DO_PROPERTY(QvMaterialBinding,		MaterialBindingIndex)
DO_PROPERTY(QvNormal,			NormalIndex)
DO_PROPERTY(QvNormalBinding,		NormalBindingIndex)
DO_PROPERTY(QvShapeHints,		ShapeHintsIndex)
DO_PROPERTY(QvTextureCoordinate2,	TextureCoordinate2Index)
DO_PROPERTY(QvTexture2,			Texture2Index)
DO_PROPERTY(QvTexture2Transform,	Texture2TransformationIndex)

DO_TYPED_PROPERTY(QvDirectionalLight,	LightIndex, DirectionalLight)
DO_TYPED_PROPERTY(QvPointLight,		LightIndex, PointLight)
DO_TYPED_PROPERTY(QvSpotLight,		LightIndex, SpotLight)

DO_TYPED_PROPERTY(QvOrthographicCamera,	CameraIndex, OrthographicCamera)
DO_TYPED_PROPERTY(QvPerspectiveCamera,	CameraIndex, PerspectiveCamera)

DO_TYPED_PROPERTY(QvTransform,	     TransformationIndex, Transform)
DO_TYPED_PROPERTY(QvRotation,	     TransformationIndex, Rotation)
DO_TYPED_PROPERTY(QvMatrixTransform, TransformationIndex, MatrixTransform)
DO_TYPED_PROPERTY(QvTranslation,     TransformationIndex, Translation)
DO_TYPED_PROPERTY(QvScale,	     TransformationIndex, Scale)

//////////////////////////////////////////////////////////////////////////////
//
// Shapes.
//
//////////////////////////////////////////////////////////////////////////////

static void
printProperties(QvState *state)
{
    printf("--------------------------------------------------------------\n");
    state->print();
    printf("--------------------------------------------------------------\n");
}

#define DO_SHAPE(className)						      \
void									      \
className::traverse(QvState *state)					      \
{									      \
    ANNOUNCE(className);						      \
    printProperties(state);						      \
}

//[MSH_START]
/*
void QvCone::traverse(QvState *state)
{
	ANNOUNCE(QvCone);
	printf( "\tbottomRadius=%f\n", bottomRadius.value );
	printf( "\theight=%f\n", height.value );
	printf( "\tparts=%d\n", parts.value );
	printf( "---------------------------------------\n" );
	printProperties(state);
}
*/
//[MSH_END]



//[MSH_START]

//DO_SHAPE(QvSphere)
//DO_SHAPE(QvCone)
//DO_SHAPE(QvCube)
//DO_SHAPE(QvCylinder)

#include "BspObjectList.h"
#include "BRep.h"

void QvSphere::traverse(QvState *state)
{
	ANNOUNCE(QvSphere);
	BSPLIB_NAMESPACE::BRep brep( state );
	brep.BuildFromSpherePrimitive( *this );
}

void QvCone::traverse(QvState *state)
{
	ANNOUNCE(QvCone);
	BSPLIB_NAMESPACE::BRep brep( state );
	brep.BuildFromConePrimitive( *this );
}

void QvCube::traverse(QvState *state)
{
	ANNOUNCE(QvCube);
	BSPLIB_NAMESPACE::BRep brep( state );
	brep.BuildFromCubePrimitive( *this );
}


void QvCylinder::traverse(QvState *state)
{
	ANNOUNCE(QvCylinder);
	BSPLIB_NAMESPACE::BRep brep( state );
	brep.BuildFromCylinderPrimitive( *this );
}

void QvIndexedFaceSet::traverse(QvState *state)
{
	ANNOUNCE(QvIndexedFaceSet);
/*
	for ( int i = 0; i < coordIndex.num; i++ )
		printf( "coordIndex[%d]=%d\n", i, coordIndex.values[ i ] );
	printf( "---------------------------------------\n" );
*/
	// build b-rep out of this indexed face set and current state
	BSPLIB_NAMESPACE::BRep brep( state );
	brep.BuildFromIndexedFaceSet( *this );
}
//[MSH_END]

//DO_SHAPE(QvIndexedFaceSet)
//--------------------------------------------

DO_SHAPE(QvIndexedLineSet)
DO_SHAPE(QvPointSet)



//////////////////////////////////////////////////////////////////////////////
//
// WWW-specific nodes.
//
//////////////////////////////////////////////////////////////////////////////

// ???
DEFAULT_TRAVERSE(QvWWWAnchor)
DEFAULT_TRAVERSE(QvWWWInline)

//////////////////////////////////////////////////////////////////////////////
//
// Default traversal methods. These nodes have no effects during traversal.
//
//////////////////////////////////////////////////////////////////////////////

DEFAULT_TRAVERSE(QvInfo)
DEFAULT_TRAVERSE(QvUnknownNode)

//////////////////////////////////////////////////////////////////////////////

#undef ANNOUNCE
#undef DEFAULT_TRAVERSE
#undef DO_PROPERTY
#undef DO_SHAPE
#undef DO_TYPED_PROPERTY
