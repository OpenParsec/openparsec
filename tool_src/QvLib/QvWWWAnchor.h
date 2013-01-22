#ifndef  _QV_WWW_ANCHOR_
#define  _QV_WWW_ANCHOR_

#include <QvSFEnum.h>
#include <QvSFString.h>
#include <QvGroup.h>

class QvWWWAnchor : public QvGroup {

    QV_NODE_HEADER(QvWWWAnchor);

  public:

    enum Map {			// Map types:
	NONE,				// Leave URL name alone
	POINT,				// Add object coords to URL name
    };

    // Fields
    QvSFString		name;		// URL name
    QvSFEnum		map;		// How to map pick to URL name
};

#endif /* _QV_WWW_ANCHOR_ */
