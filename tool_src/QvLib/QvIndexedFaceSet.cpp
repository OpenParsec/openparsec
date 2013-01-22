#include <QvIndexedFaceSet.h>

QV_NODE_SOURCE(QvIndexedFaceSet);

QvIndexedFaceSet::QvIndexedFaceSet()
{
    QV_NODE_CONSTRUCTOR(QvIndexedFaceSet);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(coordIndex);
    QV_NODE_ADD_FIELD(materialIndex);
    QV_NODE_ADD_FIELD(normalIndex);
    QV_NODE_ADD_FIELD(textureCoordIndex);

    coordIndex.values[0] = 0;
    materialIndex.values[0] = QV_END_FACE_INDEX;
    normalIndex.values[0] = QV_END_FACE_INDEX;
    textureCoordIndex.values[0] = QV_END_FACE_INDEX;
}

QvIndexedFaceSet::~QvIndexedFaceSet()
{
}
