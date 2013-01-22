#ifndef  _QV_READ_ERROR
#define  _QV_READ_ERROR

class QvInput;

class QvReadError {
  public:
    static void		post(const QvInput *in, const char *formatString ...);
};

#endif /* _QV_READ_ERROR */
