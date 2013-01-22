#ifndef  _QV_MF_LONG_
#define  _QV_MF_LONG_

#include <QvSubField.h>

class QvMFLong : public QvMField {
  public:
    long *values;
    QV_MFIELD_HEADER(QvMFLong);
};

#endif /* _QV_MF_LONG_ */
