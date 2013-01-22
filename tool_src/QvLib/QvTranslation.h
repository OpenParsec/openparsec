#ifndef  _QV_TRANSLATION_
#define  _QV_TRANSLATION_

#include <QvSFVec3f.h>
#include <QvSubNode.h>

class QvTranslation : public QvNode {

    QV_NODE_HEADER(QvTranslation);

  public:
    // Fields
    QvSFVec3f		translation;	// Translation vector
};

#endif /* _QV_TRANSLATION_ */
