#include "math.h"

#include <QvPerspectiveCamera.h>

QV_NODE_SOURCE(QvPerspectiveCamera);

QvPerspectiveCamera::QvPerspectiveCamera()
{
    QV_NODE_CONSTRUCTOR(QvPerspectiveCamera);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(position);
    QV_NODE_ADD_FIELD(orientation);
    QV_NODE_ADD_FIELD(focalDistance);
    QV_NODE_ADD_FIELD(heightAngle);

    position.value[0] = 0.0;
    position.value[1] = 0.0;
    position.value[2] = 1.0;
    orientation.axis[0] = 0.0;
    orientation.axis[1] = 0.0;
    orientation.axis[2] = 1.0;
    orientation.angle = 0.0;
    focalDistance.value = 5.0;
    heightAngle.value = M_PI_4; // 45 degrees
}

QvPerspectiveCamera::~QvPerspectiveCamera()
{
}
