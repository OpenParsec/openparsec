#ifndef  _QV_TEXTURE_2_
#define  _QV_TEXTURE_2_

#include <QvSFEnum.h>
#include <QvSFImage.h>
#include <QvSFString.h>
#include <QvSubNode.h>

class QvTexture2 : public QvNode {

    QV_NODE_HEADER(QvTexture2);

  public:
    enum Wrap {				// Texture wrap type
	REPEAT,
	CLAMP,
    };

    // Fields.
    QvSFString		filename;	// file to read texture from
    QvSFImage		image;		// The texture
    QvSFEnum		wrapS;
    QvSFEnum		wrapT;

    virtual QvBool	readInstance(QvInput *in);
    QvBool		readImage();
};

#endif /* _QV_TEXTURE_2_ */
