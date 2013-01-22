#ifdef WIN32
#include <QvSFString.h>
#endif
#include <QvWWWInline.h>

QV_NODE_SOURCE(QvWWWInline);

QvWWWInline::QvWWWInline()
{
    QV_NODE_CONSTRUCTOR(QvWWWInline);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(name);
    QV_NODE_ADD_FIELD(bboxSize);
    QV_NODE_ADD_FIELD(bboxCenter);

    name.value = "";
    bboxSize.value[0] = bboxSize.value[0] = bboxSize.value[0] = 0.0;
    bboxCenter.value[0] = bboxCenter.value[0] = bboxCenter.value[0] = 0.0;
}

QvWWWInline::~QvWWWInline()
{
}
