#ifndef  _QV_SUB_NODE_
#define  _QV_SUB_NODE_

#include <QvFieldData.h>
#include <QvNode.h>

#define QV_NODE_HEADER(className)					      \
  public:								      \
    className();						      \
    virtual ~className();						      \
    virtual void	traverse(QvState *state);			      \
  private:								      \
    static QvBool	firstInstance;					      \
    static QvFieldData	*fieldData;					      \
    virtual QvFieldData *getFieldData() { return fieldData; }

#define QV_NODE_SOURCE(className)					      \
    QvFieldData	       *className::fieldData;				      \
    QvBool		className::firstInstance = TRUE;

#define QV_NODE_CONSTRUCTOR(className)					      \
    if (fieldData == NULL)						      \
	fieldData = new QvFieldData;					      \
    else								      \
	firstInstance = FALSE;						      \
    isBuiltIn = FALSE;							      \

#define QV_NODE_IS_FIRST_INSTANCE() (firstInstance == TRUE)

#define QV_NODE_ADD_FIELD(fieldName)					      \
    if (firstInstance)							      \
	fieldData->addField(this, QV__QUOTE(fieldName), &this->fieldName);    \
    this->fieldName.setContainer(this);

#define QV_NODE_DEFINE_ENUM_VALUE(enumType,enumValue)			      \
    if (firstInstance)							      \
	fieldData->addEnumValue(QV__QUOTE(enumType),			      \
				QV__QUOTE(enumValue), enumValue)

#endif /* _QV_SUB_NODE_ */

