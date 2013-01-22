#ifndef  _QV_INFO_
#define  _QV_INFO_

#include <QvSFString.h>
#include <QvSubNode.h>

class QvInfo : public QvNode {

    QV_NODE_HEADER(QvInfo);

  public:
    // Fields
    QvSFString		string;		// Info string
};

#endif /* _QV_INFO_ */
