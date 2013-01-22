#ifndef  _QV_INDEXED_LINE_SET_
#define  _QV_INDEXED_LINE_SET_

#include <QvMFLong.h>
#include <QvSubNode.h>

#define QV_END_LINE_INDEX	(-1)

class QvIndexedLineSet : public QvNode {

    QV_NODE_HEADER(QvIndexedLineSet);

  public:
    // Fields:
    QvMFLong		coordIndex;		// Coordinate indices
    QvMFLong		materialIndex;		// Material indices
    QvMFLong		normalIndex;		// Surline normal indices
    QvMFLong		textureCoordIndex;	// Texture Coordinate indices
};

#endif /* _QV_INDEXED_LINE_SET_ */
