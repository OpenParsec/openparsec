#include <QvTexture2.h>

QV_NODE_SOURCE(QvTexture2);

QvTexture2::QvTexture2()
{
    QV_NODE_CONSTRUCTOR(QvTexture2);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(filename);
    QV_NODE_ADD_FIELD(image);
    QV_NODE_ADD_FIELD(wrapS);
    QV_NODE_ADD_FIELD(wrapT);

    filename.value = "";
    image.size[0] = image.size[1] = 0.0;
    image.numComponents = 0;
    image.bytes = NULL;
    wrapS.value = REPEAT;
    wrapT.value = REPEAT;

    QV_NODE_DEFINE_ENUM_VALUE(Wrap, REPEAT);
    QV_NODE_DEFINE_ENUM_VALUE(Wrap, CLAMP);
    
    QV_NODE_SET_SF_ENUM_TYPE(wrapS, Wrap);
    QV_NODE_SET_SF_ENUM_TYPE(wrapT, Wrap);
}

QvTexture2::~QvTexture2()
{
}

QvBool
QvTexture2::readInstance(QvInput *in)
{
    QvBool readOK = QvNode::readInstance(in);

    if (readOK && ! filename.isDefault()) {
	if (! readImage())
	    readOK = FALSE;
	image.setDefault(TRUE);
    }

    return readOK;
}

QvBool
QvTexture2::readImage()
{
    // ???
    // ??? Read image from filename and store results in image field.
    // ???

    return TRUE;
}
