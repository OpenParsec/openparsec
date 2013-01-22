#ifndef  _QV_ROTATION_
#define  _QV_ROTATION_

#include <QvSFRotation.h>
#include <QvSubNode.h>

class QvRotation : public QvNode {

    QV_NODE_HEADER(QvRotation);

  public:
    // Fields
    QvSFRotation	rotation;	// Rotation
};

#endif /* _QV_ROTATION_ */
