#ifndef  _QV_LEVEL_OF_DETAIL_
#define  _QV_LEVEL_OF_DETAIL_

#include <QvMFFloat.h>
#include <QvGroup.h>

class QvLevelOfDetail : public QvGroup {

    QV_NODE_HEADER(QvLevelOfDetail);

  public:
    // Fields
    QvMFFloat		screenArea;	// Areas to use for comparison
};

#endif /* _QV_LEVEL_OF_DETAIL_ */
