#include <stdarg.h>
#include <QvString.h>
#include <QvInput.h>
#include <QvReadError.h>

void
QvReadError::post(const QvInput *in, const char *formatString ...)
{
    char	buf[10000];
    va_list	ap;

    va_start(ap, formatString);
    vsprintf(buf, formatString, ap);
    va_end(ap);

    QvString locstr;
    in->getLocationString(locstr);
    fprintf(stderr, "VRML read error: %s\n%s\n", buf, locstr.getString());
}
