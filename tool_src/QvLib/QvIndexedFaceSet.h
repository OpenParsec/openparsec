#ifndef  _QV_INDEXED_FACE_SET_
#define  _QV_INDEXED_FACE_SET_

#include <QvMFLong.h>
#include <QvSubNode.h>

#define QV_END_FACE_INDEX	(-1)

class QvIndexedFaceSet : public QvNode {

    QV_NODE_HEADER(QvIndexedFaceSet);

  public:
    // Fields:
    QvMFLong		coordIndex;		// Coordinate indices
    QvMFLong		materialIndex;		// Material indices
    QvMFLong		normalIndex;		// Surface normal indices
    QvMFLong		textureCoordIndex;	// Texture Coordinate indices
};

#endif /* _QV_INDEXED_FACE_SET_ */
