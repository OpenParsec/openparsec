#ifndef  _QV_DEBUG_ERROR
#define  _QV_DEBUG_ERROR

class QvDebugError {
  public:
    static void post(const char *methodName, const char *formatString ...);
};

#endif /* _QV_DEBUG_ERROR */
