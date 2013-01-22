#include <QvDB.h>
#include <QvInput.h>
#include <QvReadError.h>
#include <QvNode.h>

const char *QvDB::versionString = "Reference VRML Parser 1.0";

void
QvDB::init()
{
    QvNode::init();
}

QvBool
QvDB::read(QvInput *in, QvNode *&node)
{
    QvBool ret;

    ret = QvNode::read(in, node);

    if (ret && node == NULL && ! in->eof()) {
	char	c;
	in->get(c);
	QvReadError::post(in, "Extra characters ('%c') found in input", c);
	ret = FALSE;
    }

    return ret;
}
