#include <QvMaterial.h>

QV_NODE_SOURCE(QvMaterial);

QvMaterial::QvMaterial()
{
    QV_NODE_CONSTRUCTOR(QvMaterial);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(ambientColor);
    QV_NODE_ADD_FIELD(diffuseColor);
    QV_NODE_ADD_FIELD(specularColor);
    QV_NODE_ADD_FIELD(emissiveColor);
    QV_NODE_ADD_FIELD(shininess);
    QV_NODE_ADD_FIELD(transparency);

    ambientColor.values[0]=ambientColor.values[1]=ambientColor.values[2] = 0.2;
    diffuseColor.values[0]=diffuseColor.values[1]=diffuseColor.values[2] = 0.2;
    specularColor.values[0]=specularColor.values[1]=specularColor.values[2]=0.;
    emissiveColor.values[0]=emissiveColor.values[1]=emissiveColor.values[2]=0.;
    shininess.values[0] = 0.2;
    transparency.values[0] = 0.0;
}

QvMaterial::~QvMaterial()
{
}
