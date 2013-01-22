#include <QvUnknownNode.h>

QV_NODE_SOURCE(QvUnknownNode);

QvUnknownNode::QvUnknownNode()
{
    QV_NODE_CONSTRUCTOR(QvUnknownNode);

    className = NULL;

    // Set global field data to this instance's
    instanceFieldData = new QvFieldData;
    fieldData = instanceFieldData;
}

void
QvUnknownNode::setClassName(const char *name)
{
    className = strdup(name);
}

QvUnknownNode::~QvUnknownNode()
{
    if (className != NULL)
	free((void *) className);
}
