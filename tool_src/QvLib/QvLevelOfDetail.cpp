#include <QvLevelOfDetail.h>

QV_NODE_SOURCE(QvLevelOfDetail);

QvLevelOfDetail::QvLevelOfDetail()
{
    QV_NODE_CONSTRUCTOR(QvLevelOfDetail);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(screenArea);

    screenArea.values[0] = 0;
}

QvLevelOfDetail::~QvLevelOfDetail()
{
}
