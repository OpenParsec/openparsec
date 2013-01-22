#ifndef _QV_STRING_
#define _QV_STRING_

#include <QvBasic.h>
#include <string.h>

class QvString {
  public:
    QvString()				{ string = staticStorage; 
					  string[0] = '\0'; }
    QvString(const char *str)		{ string = staticStorage;
					  *this = str; }
    QvString(const QvString &str)	{ string = staticStorage;
					  *this = str.string; }
    ~QvString();
    u_long		hash()		{ return QvString::hash(string); }
    int			getLength() const	{ return strlen(string); }
    void		makeEmpty(QvBool freeOld = TRUE);
    const char *	getString() const	{ return string; }
    QvString &		operator =(const char *str);
    QvString &		operator =(const QvString &str)
	{ return (*this = str.string); }
    QvString &		operator +=(const char *str);
    int			operator !() const { return (string[0] == '\0'); }
    friend int		operator ==(const QvString &str, const char *s);
   
    friend int		operator ==(const char *s, const QvString &str)
	{ return (str == s); }
    
    friend int		operator ==(const QvString &str1, const QvString &str2)
	{ return (str1 == str2.string); }
    friend int		operator !=(const QvString &str, const char *s);
    
    friend int		operator !=(const char *s, const QvString &str)
	{ return (str != s); }
    friend int		operator !=(const QvString &str1,
				    const QvString &str2)
	{ return (str1 != str2.string); }
    static u_long	hash(const char *s);    	
  private:
    char		*string;
    int			storageSize;
#define QV_STRING_STATIC_STORAGE_SIZE		32
    char		staticStorage[QV_STRING_STATIC_STORAGE_SIZE];
    void		expand(int bySize);
};

class QvNameEntry {
 public:
    QvBool		isEmpty() const   { return (string[0] == '\0'); }
    QvBool		isEqual(const char *s) const
	{ return (string[0] == s[0] && ! strcmp(string, s)); }
 private:
    static int		nameTableSize;
    static QvNameEntry	**nameTable;
    static struct QvNameChunk *chunk;		
    const char		*string;
    u_long		hashValue;
    QvNameEntry		*next;			
    static void		initClass();
    QvNameEntry(const char *s, u_long h, QvNameEntry *n)
	{ string = s; hashValue = h; next = n; }
    static const QvNameEntry *	insert(const char *s);

friend class QvName;
};

class QvName {
  public:
    QvName();
    QvName(const char *s)		{ entry = QvNameEntry::insert(s); }
    QvName(const QvString &s)	{ entry = QvNameEntry::insert(s.getString()); }

    QvName(const QvName &n)			{ entry = n.entry; }
    ~QvName()					{}
    const char		*getString() const	{ return entry->string; }
    int			getLength() const   { return strlen(entry->string); }
    static QvBool 	isIdentStartChar(char c);
    static QvBool	isIdentChar(char c);
    static QvBool 	isNodeNameStartChar(char c);
    static QvBool	isNodeNameChar(char c);
    int			operator !() const   { return entry->isEmpty(); }
    friend int		operator ==(const QvName &n, const char *s)
	{ return n.entry->isEqual(s); }
    friend int		operator ==(const char *s, const QvName &n)
	{ return n.entry->isEqual(s); }
    
    friend int 		operator ==(const QvName &n1, const QvName &n2)
	{ return n1.entry == n2.entry; }
    friend int		operator !=(const QvName &n, const char *s)
	{ return ! n.entry->isEqual(s); }
    friend int		operator !=(const char *s, const QvName &n)
	{ return ! n.entry->isEqual(s); }
    
    friend int 		operator !=(const QvName &n1, const QvName &n2)
	{ return n1.entry != n2.entry; }
  private:
    const QvNameEntry	*entry;
};

#endif /* _QV_STRING_ */
