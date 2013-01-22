#ifndef  _QV_NODE_
#define  _QV_NODE_

#include <QvString.h>

class QvChildList;
class QvDict;
class QvFieldData;
class QvInput;
class QvNodeList;
class QvState;

class QvNode {

  public:
    enum Stage {
	FIRST_INSTANCE,		// First real instance being constructed
	PROTO_INSTANCE,		// Prototype instance being constructed
	OTHER_INSTANCE,		// Subsequent instance being constructed
    };

    QvFieldData	*fieldData;
    QvChildList	*children;
    QvBool	isBuiltIn;

    QvName		*objName;
    QvNode();
    virtual ~QvNode();

    const QvName &	getName() const;
    void		setName(const QvName &name);

    static void		init();
    static QvBool	read(QvInput *in, QvNode *&node);

    virtual QvFieldData *getFieldData() = 0;

    virtual void	traverse(QvState *state) = 0;

  protected:
    virtual QvBool	readInstance(QvInput *in);

  private:
    static QvDict	*nameDict;

    static void		addName(QvNode *, const char *);
    static void		removeName(QvNode *, const char *);
    static QvNode *	readReference(QvInput *in);
    static QvBool	readNode(QvInput *in, QvName &className,QvNode *&node);
    static QvBool	readNodeInstance(QvInput *in, const QvName &className,
					 const QvName &refName, QvNode *&node);
    static QvNode *	createInstance(QvInput *in, const QvName &className);
    static QvNode *	createInstanceFromName(const QvName &className);
    static void		flushInput(QvInput *in);
};

#endif /* _QV_NODE_ */
