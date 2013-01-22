#include <QvOrthographicCamera.h>

QV_NODE_SOURCE(QvOrthographicCamera);

QvOrthographicCamera::QvOrthographicCamera()
{
    QV_NODE_CONSTRUCTOR(QvOrthographicCamera);
    isBuiltIn = TRUE;

    QV_NODE_ADD_FIELD(position);
    QV_NODE_ADD_FIELD(orientation);
    QV_NODE_ADD_FIELD(focalDistance);
    QV_NODE_ADD_FIELD(height);

    position.value[0] = 0.0;
    position.value[1] = 0.0;
    position.value[2] = 1.0;
    orientation.axis[0] = 0.0;
    orientation.axis[1] = 0.0;
    orientation.axis[2] = 1.0;
    orientation.angle = 0.0;
    focalDistance.value = 5.0;
    height.value = 2.0;
}

QvOrthographicCamera::~QvOrthographicCamera()
{
}
