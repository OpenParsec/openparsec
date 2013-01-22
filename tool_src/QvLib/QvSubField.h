#ifndef  _QV_SUB_FIELD_
#define  _QV_SUB_FIELD_

#include <QvField.h>
#include <QvInput.h>

/////////////////////////////////////////////////////////////////////////////

#define QV_SFIELD_HEADER(className)					      \
  public:								      \
    className();							      \
    virtual ~className();						      \
    virtual QvBool readValue(QvInput *in)

/////////////////////////////////////////////////////////////////////////////

#define QV_MFIELD_HEADER(className)					      \
  public:								      \
    className();							      \
    virtual ~className();						      \
    virtual QvBool	read1Value(QvInput *in, int index);		      \
    void		allocValues(int newNum)

/////////////////////////////////////////////////////////////////////////////

#define QV_SFIELD_SOURCE(className)					      \
									      \
className::className()							      \
{									      \
}									      \
className::~className()							      \
{									      \
}

/////////////////////////////////////////////////////////////////////////////

#define QV_MFIELD_SOURCE(className, valueType, numValues)		      \
									      \
className::className()							      \
{									      \
    values = NULL;							      \
    /* Make room for 1 value to start */				      \
    allocValues(1);							      \
}									      \
									      \
className::~className()							      \
{									      \
    if (values != NULL)							      \
	free((char *) values);						      \
}									      \
									      \
void									      \
className::allocValues(int newNum)					      \
{									      \
    if (values == NULL) {						      \
	if (newNum > 0)							      \
	    values = (valueType *)					      \
		malloc(numValues * sizeof(valueType) * newNum);		      \
    }									      \
    else {								      \
	if (newNum > 0)							      \
	    values = (valueType *)					      \
		realloc(values, numValues * sizeof(valueType) * newNum);      \
	else {								      \
	    free((char *) values);					      \
	    values = NULL;						      \
	}								      \
    }									      \
    num = maxNum = newNum;						      \
}

#endif /* _QV_SUB_FIELD_ */
