#ifndef  _QV_PERSPECTIVE_CAMERA_
#define  _QV_PERSPECTIVE_CAMERA_

#include <QvSFFloat.h>
#include <QvSFRotation.h>
#include <QvSFVec3f.h>
#include <QvSubNode.h>

class QvPerspectiveCamera : public QvNode {

    QV_NODE_HEADER(QvPerspectiveCamera);

  public:
    QvSFVec3f		position;	// Location of viewpoint
    QvSFRotation	orientation;	// Orientation (rotation with
					// respect to (0,0,-1) vector)
    QvSFFloat	    	focalDistance;	// Distance from viewpoint to
					// point of focus.
    QvSFFloat		heightAngle;	// Angle (in radians) of field
					// of view, in height direction
};

#endif /* _QV_PERSPECTIVE_CAMERA_ */
