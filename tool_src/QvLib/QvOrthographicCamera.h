#ifndef  _QV_ORTHOGRAPHIC_CAMERA_
#define  _QV_ORTHOGRAPHIC_CAMERA_

#include <QvSFFloat.h>
#include <QvSFRotation.h>
#include <QvSFVec3f.h>
#include <QvSubNode.h>

class QvOrthographicCamera : public QvNode {

    QV_NODE_HEADER(QvOrthographicCamera);

  public:
    QvSFVec3f		position;	// Location of viewpoint
    QvSFRotation	orientation;	// Orientation (rotation with
					// respect to (0,0,-1) vector)
    QvSFFloat	    	focalDistance;	// Distance from viewpoint to
					// point of focus.
    QvSFFloat		height;		// Height of view volume
};

#endif /* _QV_ORTHOGRAPHIC_CAMERA_ */
