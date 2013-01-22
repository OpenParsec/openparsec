#include <QvWWWAnchor.h>

QV_NODE_SOURCE(QvWWWAnchor);

QvWWWAnchor::QvWWWAnchor()
{
    QV_NODE_CONSTRUCTOR(QvWWWAnchor);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(name);
    QV_NODE_ADD_FIELD(map);

    name.value = "";
    map.value = NONE;

    QV_NODE_DEFINE_ENUM_VALUE(Map, NONE);
    QV_NODE_DEFINE_ENUM_VALUE(Map, POINT);

    QV_NODE_SET_SF_ENUM_TYPE(map, Map);
}

QvWWWAnchor::~QvWWWAnchor()
{
}
