#include <stdarg.h>
#include <QvString.h>
#include <QvDebugError.h>

void
QvDebugError::post(const char *methodName, const char *formatString ...)
{
    char	buf[10000];
    va_list	ap;

    va_start(ap, formatString);
    vsprintf(buf, formatString, ap);
    va_end(ap);

    fprintf(stderr, "VRML error in %s(): %s\n", methodName, buf);
}
