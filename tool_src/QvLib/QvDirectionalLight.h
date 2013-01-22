#ifndef  _QV_DIRECTIONAL_LIGHT_
#define  _QV_DIRECTIONAL_LIGHT_

#include <QvSFBool.h>
#include <QvSFColor.h>
#include <QvSFFloat.h>
#include <QvSFVec3f.h>
#include <QvSubNode.h>

class QvDirectionalLight : public QvNode {

    QV_NODE_HEADER(QvDirectionalLight);

  public:
    // Fields
    QvSFBool		on;		// Whether light is on
    QvSFFloat		intensity;	// Source intensity (0 to 1)
    QvSFColor		color;		// RGB source color
    QvSFVec3f		direction;	// Illumination direction vector
};

#endif /* _QV_DIRECTIONAL_LIGHT_ */
