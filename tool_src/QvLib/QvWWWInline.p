# 1 "QvWWWInline.cpp"



# 1 "QvWWWInline.h" 1



# 1 "QvSFEnum.h" 1



# 1 "QvString.h" 1



# 1 "QvBasic.h" 1









# 1 "/usr/include/sys/types.h" 1 3 4
 




















 
 











































# 1 "/usr/include/sys/cdefs.h" 1 3 4
 




















 
 
















































 






















# 121 "/usr/include/sys/cdefs.h" 3 4


 
















 

























# 67 "/usr/include/sys/types.h" 2 3 4


 
# 1 "/usr/include/machine/types.h" 1 3 4
 




















 







# 1 "/usr/include/ppc/types.h" 1 3 4
 




















 


 







































 



typedef	signed  char		int8_t;
typedef	unsigned char		u_int8_t;
typedef	short			int16_t;
typedef	unsigned short		u_int16_t;
typedef	int			int32_t;
typedef	unsigned int		u_int32_t;
typedef	long long		int64_t;
typedef	unsigned long long	u_int64_t;

typedef int32_t			register_t;


typedef int 			*intptr_t;
typedef unsigned long		*uintptr_t;



# 30 "/usr/include/machine/types.h" 2 3 4









# 70 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/machine/ansi.h" 1 3 4
 




















 
 










# 1 "/usr/include/ppc/ansi.h" 1 3 4
 





































 




















 






















# 34 "/usr/include/machine/ansi.h" 2 3 4









# 72 "/usr/include/sys/types.h" 2 3 4

# 1 "/usr/include/machine/endian.h" 1 3 4
 




















 







# 1 "/usr/include/ppc/endian.h" 1 3 4
 





















 





































 






 












extern "C" { 
unsigned long	htonl  (unsigned long)  ;
unsigned short	htons  (unsigned short)  ;
unsigned long	ntohl  (unsigned long)  ;
unsigned short	ntohs  (unsigned short)  ;
} 


 













# 116 "/usr/include/ppc/endian.h" 3 4



# 30 "/usr/include/machine/endian.h" 2 3 4









# 73 "/usr/include/sys/types.h" 2 3 4



typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		 
typedef	unsigned int	uint;		 


typedef	u_int64_t	u_quad_t;	 
typedef	int64_t		quad_t;
typedef	quad_t *	qaddr_t;

typedef	char *		caddr_t;	 
typedef	int32_t		daddr_t;	 
typedef	int32_t		dev_t;		 
typedef	u_int32_t	fixpt_t;	 
typedef	u_int32_t	gid_t;		 
typedef	u_int32_t	ino_t;		 
typedef	long		key_t;		 
typedef	u_int16_t	mode_t;		 
typedef	u_int16_t	nlink_t;	 
typedef	quad_t		off_t;		 
typedef	int32_t		pid_t;		 
typedef quad_t		rlim_t;		 
typedef	int32_t		segsz_t;	 
typedef	int32_t		swblk_t;	 
typedef	u_int32_t	uid_t;		 



 







typedef	unsigned long 	clock_t;




typedef	long unsigned int 	size_t;




typedef	int 	ssize_t;




typedef	long 	time_t;





 







typedef int32_t	fd_mask;






typedef	struct fd_set {
	fd_mask	fds_bits[((( 256  ) + ((  (sizeof(fd_mask) * 8 )  ) - 1)) / (  (sizeof(fd_mask) * 8 )  )) ];
} fd_set;







# 174 "/usr/include/sys/types.h" 3 4





struct _pthread_handler_rec
{
	void           (*routine)(void *);   
	void           *arg;                  
	struct _pthread_handler_rec *next;
};












typedef struct _opaque_pthread_t { long sig; struct _pthread_handler_rec  *cleanup_stack; char opaque[596 ];} *pthread_t;

typedef struct _opaque_pthread_attr_t { long sig; char opaque[36 ]; } pthread_attr_t;

typedef struct _opaque_pthread_mutexattr_t { long sig; char opaque[8 ]; } pthread_mutexattr_t;

typedef struct _opaque_pthread_mutex_t { long sig; char opaque[40 ]; } pthread_mutex_t;

typedef struct _opaque_pthread_condattr_t { long sig; char opaque[4 ]; } pthread_condattr_t;

typedef struct _opaque_pthread_cond_t { long sig;  char opaque[24 ]; } pthread_cond_t;

typedef struct { long sig; char opaque[4 ]; } pthread_once_t;



typedef unsigned long pthread_key_t;     


# 10 "QvBasic.h" 2


# 1 "/usr/include/libc.h" 1 3 4
 




















 






# 1 "/usr/include/stdio.h" 1 3 4
 




















 
























































 











typedef off_t fpos_t;








 





 
struct __sbuf {
	unsigned char *_base;
	int	_size;
};

 

























typedef	struct __sFILE {
	unsigned char *_p;	 
	int	_r;		 
	int	_w;		 
	short	_flags;		 
	short	_file;		 
	struct	__sbuf _bf;	 
	int	_lbfsize;	 

	 
	void	*_cookie;	 
	int	(*_close)  (void *)  ;
	int	(*_read)   (void *, char *, int)  ;
	fpos_t	(*_seek)   (void *, fpos_t, int)  ;
	int	(*_write)  (void *, const char *, int)  ;

	 
	struct	__sbuf _ub;	 
	unsigned char *_up;	 
	int	_ur;		 

	 
	unsigned char _ubuf[3];	 
	unsigned char _nbuf[1];	 

	 
	struct	__sbuf _lb;	 

	 
	int	_blksize;	 
	fpos_t	_offset;	 
} FILE;

extern "C" { 
extern FILE __sF[];
} 





	 











 















 




				 



 




















 


extern "C" { 
void	 clearerr  (FILE *)  ;
int	 fclose  (FILE *)  ;
int	 feof  (FILE *)  ;
int	 ferror  (FILE *)  ;
int	 fflush  (FILE *)  ;
int	 fgetc  (FILE *)  ;
int	 fgetpos  (FILE *, fpos_t *)  ;
char	*fgets  (char *, size_t, FILE *)  ;
FILE	*fopen  (const char *, const char *)  ;
int	 fprintf  (FILE *, const char *, ...)  ;
int	 fputc  (int, FILE *)  ;
int	 fputs  (const char *, FILE *)  ;
size_t	 fread  (void *, size_t, size_t, FILE *)  ;
FILE	*freopen  (const char *, const char *, FILE *)  ;
int	 fscanf  (FILE *, const char *, ...)  ;
int	 fseek  (FILE *, long, int)  ;
int	 fsetpos  (FILE *, const fpos_t *)  ;
long	 ftell  (FILE *)  ;
size_t	 fwrite  (const void *, size_t, size_t, FILE *)  ;
int	 getc  (FILE *)  ;
int	 getchar  (void)  ;
char	*gets  (char *)  ;

extern int sys_nerr;			 
extern const  char * const  sys_errlist[];

void	 perror  (const char *)  ;
int	 printf  (const char *, ...)  ;
int	 putc  (int, FILE *)  ;
int	 putchar  (int)  ;
int	 puts  (const char *)  ;
int	 remove  (const char *)  ;
int	 rename   (const char *, const char *)  ;
void	 rewind  (FILE *)  ;
int	 scanf  (const char *, ...)  ;
void	 setbuf  (FILE *, char *)  ;
int	 setvbuf  (FILE *, char *, int, size_t)  ;
int	 sprintf  (char *, const char *, ...)  ;
int	 sscanf  (const char *, const char *, ...)  ;
FILE	*tmpfile  (void)  ;
char	*tmpnam  (char *)  ;
int	 ungetc  (int, FILE *)  ;
int	 vfprintf  (FILE *, const char *, char * )  ;
int	 vprintf  (const char *, char * )  ;
int	 vsprintf  (char *, const char *, char * )  ;
} 

 






extern "C" { 
char	*ctermid  (char *)  ;
FILE	*fdopen  (int, const char *)  ;
int	 fileno  (FILE *)  ;
} 


 



extern "C" { 
char	*fgetln  (FILE *, size_t *)  ;
int	 fpurge  (FILE *)  ;
int	 fseeko  (FILE *, fpos_t, int)  ;
fpos_t	ftello  (FILE *)  ;
int	 getw  (FILE *)  ;
int	 pclose  (FILE *)  ;
FILE	*popen  (const char *, const char *)  ;
int	 putw  (int, FILE *)  ;
void	 setbuffer  (FILE *, char *, int)  ;
int	 setlinebuf  (FILE *)  ;
char	*tempnam  (const char *, const char *)  ;
int	 snprintf  (char *, size_t, const char *, ...)  ;
int	 vsnprintf  (char *, size_t, const char *, char * )  ;
int	 vscanf  (const char *, char * )  ;
int	 vsscanf  (const char *, const char *, char * )  ;
FILE	*zopen  (const char *, const char *, int)  ;
} 

 






 


extern "C" { 
FILE	*funopen  (const void *,
		int (*)(void *, char *, int),
		int (*)(void *, const char *, int),
		fpos_t (*)(void *, fpos_t, int),
		int (*)(void *))  ;
} 




 


extern "C" { 
int	__srget  (FILE *)  ;
int	__svfscanf  (FILE *, const char *, char * )  ;
int	__swbuf  (int, FILE *)  ;
} 

 





static inline  int __sputc(int _c, FILE *_p) {
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
		return (*_p->_p++ = _c);
	else
		return (__swbuf(_c, _p));
}
# 379 "/usr/include/stdio.h" 3 4























# 29 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/standards.h" 1 3 4
 




















 














# 30 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/unistd.h" 1 3 4
 




















 






































 










# 1 "/usr/include/sys/unistd.h" 1 3 4
 




















 
 





































 


 












 
				 

				 

				 













 





 





 





 
















 




























 



# 72 "/usr/include/unistd.h" 2 3 4












extern "C" { 
  void
	 _exit  (int)  ;
int	 access  (const char *, int)  ;
unsigned int	 alarm  (unsigned int)  ;
int	 chdir  (const char *)  ;
int	 chown  (const char *, uid_t, gid_t)  ;
int	 close  (int)  ;
size_t	 confstr  (int, char *, size_t)  ;
int	 dup  (int)  ;
int	 dup2  (int, int)  ;
int	 execl  (const char *, const char *, ...)  ;
int	 execle  (const char *, const char *, ...)  ;
int	 execlp  (const char *, const char *, ...)  ;
int	 execv  (const char *, char * const *)  ;
int	 execve  (const char *, char * const *, char * const *)  ;
int	 execvp  (const char *, char * const *)  ;
pid_t	 fork  (void)  ;
long	 fpathconf  (int, int)  ;
char	*getcwd  (char *, size_t)  ;
gid_t	 getegid  (void)  ;
uid_t	 geteuid  (void)  ;
gid_t	 getgid  (void)  ;
int	 getgroups  (int, gid_t [])  ;
char	*getlogin  (void)  ;
pid_t	 getpgrp  (void)  ;
pid_t	 getpid  (void)  ;
pid_t	 getppid  (void)  ;
uid_t	 getuid  (void)  ;
int	 isatty  (int)  ;
int	 link  (const char *, const char *)  ;
off_t	 lseek  (int, off_t, int)  ;
long	 pathconf  (const char *, int)  ;
int	 pause  (void)  ;
int	 pipe  (int *)  ;
ssize_t	 read  (int, void *, size_t)  ;
int	 rmdir  (const char *)  ;
int	 setgid  (gid_t)  ;
int	 setpgid  (pid_t, pid_t)  ;
pid_t	 setsid  (void)  ;
int	 setuid  (uid_t)  ;
unsigned int	 sleep  (unsigned int)  ;
long	 sysconf  (int)  ;
pid_t	 tcgetpgrp  (int)  ;
int	 tcsetpgrp  (int, pid_t)  ;
char	*ttyname  (int)  ;
int	 unlink  (const char *)  ;
ssize_t	 write  (int, const void *, size_t)  ;

extern char *optarg;			 
extern int optind, opterr, optopt, optreset;
int	 getopt  (int, char * const [], const char *)  ;



struct timeval;				 

int	 acct  (const char *)  ;
int	 async_daemon  (void)  ;
char	*brk  (const char *)  ;
int	 chroot  (const char *)  ;
char	*crypt  (const char *, const char *)  ;
int	 des_cipher  (const char *, char *, long, int)  ;
int	 des_setkey  (const char *key)  ;
int	 encrypt  (char *, int)  ;
void	 endusershell  (void)  ;
int	 exect  (const char *, char * const *, char * const *)  ;
int	 fchdir  (int)  ;
int	 fchown  (int, int, int)  ;
int	 fsync  (int)  ;
int	 ftruncate  (int, off_t)  ;
int	 getdtablesize  (void)  ;
int	 getgrouplist  (const char *, int, int *, int *)  ;
long	 gethostid  (void)  ;
int	 gethostname  (char *, int)  ;
mode_t	 getmode  (const void *, mode_t)  ;
  int
	 getpagesize  (void)  ;
char	*getpass  (const char *)  ;
char	*getusershell  (void)  ;
char	*getwd  (char *)  ;			 
int	 initgroups  (const char *, int)  ;
int	 iruserok  (unsigned long, int, const char *, const char *)  ;
int	 mknod  (const char *, mode_t, dev_t)  ;
int	 mkstemp  (char *)  ;
char	*mktemp  (char *)  ;
int	 nfssvc  (int, void *)  ;
int	 nice  (int)  ;




# 1 "/usr/include/signal.h" 1 3 4
 




















 







































# 1 "/usr/include/sys/signal.h" 1 3 4
 




















 
 














































# 1 "/usr/include/machine/signal.h" 1 3 4
 

























# 1 "/usr/include/ppc/signal.h" 1 3 4
 




















 

























typedef int sig_atomic_t; 

 




 










typedef enum {
	REGS_SAVED_NONE,		 
	REGS_SAVED_CALLER,		 


	REGS_SAVED_ALL			 
} regs_saved_t;


 






struct sigcontext {
    int		sc_onstack;      
    int		sc_mask;         
	int		sc_ir;			 
    int		sc_psw;          
    int		sc_sp;      	 
	void	*sc_regs;		 
};



# 27 "/usr/include/machine/signal.h" 2 3 4









# 70 "/usr/include/sys/signal.h" 2 3 4
















































 













typedef unsigned int sigset_t;

 


struct	sigaction {

	void	(*sa_handler)(int);	 



	sigset_t sa_mask;		 
	int	sa_flags;		 
};








 








typedef	void (*sig_t)  (int)  ;	 

 


struct	sigaltstack {
	char	*ss_sp;		 
	int	ss_size;		 
	int	ss_flags;		 
};



 



struct	sigvec {
	void	(*sv_handler)();	 
	int	sv_mask;		 
	int	sv_flags;		 
};





 


struct	sigstack {
	char	*ss_sp;			 
	int	ss_onstack;		 
};

 





# 213 "/usr/include/sys/signal.h" 3 4







 



extern "C" { 
void	(*signal  (int, void (*)  (int)  )  )  (int)  ;
} 

# 62 "/usr/include/signal.h" 2 3 4



extern const  char * const  sys_signame[32 ];
extern const  char * const  sys_siglist[32 ];


extern "C" { 
int	raise  (int)  ;

int	kill  (pid_t, int)  ;
int	sigaction  (int, const struct sigaction *, struct sigaction *)  ;
int	sigaddset  (sigset_t *, int)  ;
int	sigdelset  (sigset_t *, int)  ;
int	sigemptyset  (sigset_t *)  ;
int	sigfillset  (sigset_t *)  ;
int	sigismember  (const sigset_t *, int)  ;
int	sigpending  (sigset_t *)  ;
int	sigprocmask  (int, const sigset_t *, sigset_t *)  ;
int	sigsuspend  (const sigset_t *)  ;

int	killpg  (pid_t, int)  ;
int	sigblock  (int)  ;
int	siginterrupt  (int, int)  ;
int	sigpause  (int)  ;
int	sigreturn  (struct sigcontext *)  ;
int	sigsetmask  (int)  ;
int	sigvec  (int, struct sigvec *, struct sigvec *)  ;
void	psignal  (unsigned int, const char *)  ;


} 

 







# 176 "/usr/include/unistd.h" 2 3 4


int	 profil  (char *, int, int, int)  ;
int	 rcmd  (char **, int, const char *,
		const char *, const char *, int *)  ;
char	*re_comp  (const char *)  ;
int	 re_exec  (const char *)  ;
int	 readlink  (const char *, char *, int)  ;
int	 reboot  (int)  ;
int	 revoke  (const char *)  ;
int	 rresvport  (int *)  ;
int	 ruserok  (const char *, int, const char *, const char *)  ;
char	*sbrk  (int)  ;
int	 select  (int, fd_set *, fd_set *, fd_set *, struct timeval *)  ;
int	 setegid  (gid_t)  ;
int	 seteuid  (uid_t)  ;
int	 setgroups  (int, const gid_t *)  ;
void	 sethostid  (long)  ;
int	 sethostname  (const char *, int)  ;
int	 setkey  (const char *)  ;
int	 setlogin  (const char *)  ;
void	*setmode  (const char *)  ;
int	 setpgrp  (pid_t pid, pid_t pgrp)  ;	 
int	 setregid  (gid_t, gid_t)  ;
int	 setreuid  (uid_t, uid_t)  ;
int	 setrgid  (gid_t)  ;
int	 setruid  (uid_t)  ;
void	 setusershell  (void)  ;
int	 swapon  (const char *)  ;
int	 symlink  (const char *, const char *)  ;
void	 sync  (void)  ;
int	 syscall  (int, ...)  ;
int	 truncate  (const char *, off_t)  ;
int	 ttyslot  (void)  ;
unsigned int	 ualarm  (unsigned int, unsigned int)  ;
int	 unwhiteout  (const char *)  ;
void	 usleep  (unsigned int)  ;
void	*valloc  (size_t)  ;			 
pid_t	 vfork  (void)  ;

extern char *suboptarg;			 
int	 getsubopt  (char **, char * const *, char **)  ;

 
int	getattrlist  (const char*,void*,void*,size_t,unsigned long)  ;
int	setattrlist  (const char*,void*,void*,size_t,unsigned long)  ;
int exchangedata  (const char*,const char*,unsigned long)  ;
int	checkuseraccess  (const char*,uid_t,gid_t*,int,int,unsigned long)  ;
int	getdirentriesattr  (int,void*,void*,size_t,unsigned long*,unsigned long*,unsigned long*,unsigned long)  ;
int	searchfs  (const char*,void*,void*,unsigned long,unsigned long,void*)  ;

int fsctl  (const char *,unsigned long,void*,unsigned long)  ;		



} 


# 31 "/usr/include/libc.h" 2 3 4






# 1 "/usr/include/string.h" 1 3 4
 




















 

















































extern "C" { 
void	*memchr  (const void *, int, size_t)  ;
int	 memcmp  (const void *, const void *, size_t)  ;
void	*memcpy  (void *, const void *, size_t)  ;
void	*memmove  (void *, const void *, size_t)  ;
void	*memset  (void *, int, size_t)  ;
char	*strcat  (char *, const char *)  ;
char	*strchr  (const char *, int)  ;
int	 strcmp  (const char *, const char *)  ;
int	 strcoll  (const char *, const char *)  ;
char	*strcpy  (char *, const char *)  ;
size_t	 strcspn  (const char *, const char *)  ;
char	*strerror  (int)  ;
size_t	 strlen  (const char *)  ;
char	*strncat  (char *, const char *, size_t)  ;
int	 strncmp  (const char *, const char *, size_t)  ;
char	*strncpy  (char *, const char *, size_t)  ;
char	*strpbrk  (const char *, const char *)  ;
char	*strrchr  (const char *, int)  ;
size_t	 strspn  (const char *, const char *)  ;
char	*strstr  (const char *, const char *)  ;
char	*strtok  (char *, const char *)  ;
size_t	 strxfrm  (char *, const char *, size_t)  ;

 

int	 bcmp  (const void *, const void *, size_t)  ;
void	 bcopy  (const void *, void *, size_t)  ;
void	 bzero  (void *, size_t)  ;
int	 ffs  (int)  ;
char	*index  (const char *, int)  ;
void	*memccpy  (void *, const void *, int, size_t)  ;
char	*rindex  (const char *, int)  ;
int	 strcasecmp  (const char *, const char *)  ;
char	*strdup  (const char *)  ;
void	 strmode  (int, char *)  ;
int	 strncasecmp  (const char *, const char *, size_t)  ;
char	*strsep  (char **, const char *)  ;
void	 swab  (const void *, void *, size_t)  ;

} 


# 37 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/stdlib.h" 1 3 4
 




















 















































typedef	__wchar_t 	rune_t;

typedef	__wchar_t 	wchar_t;


typedef struct {
	int quot;		 
	int rem;		 
} div_t;

typedef struct {
	long quot;		 
	long rem;		 
} ldiv_t;










extern int __mb_cur_max;




extern "C" { 
  void
	 abort  (void)  ;
  int
	 abs  (int)  ;
int	 atexit  (void (*)(void))  ;
double	 atof  (const char *)  ;
int	 atoi  (const char *)  ;
long	 atol  (const char *)  ;
void	*bsearch  (const void *, const void *, size_t,
	    size_t, int (*)(const void *, const void *))  ;
void	*calloc  (size_t, size_t)  ;
  div_t
	 div  (int, int)  ;
  void
	 exit  (int)  ;
void	 free  (void *)  ;
char	*getenv  (const char *)  ;
  long
	 labs  (long)  ;
  ldiv_t
	 ldiv  (long, long)  ;
void	*malloc  (size_t)  ;
void	 qsort  (void *, size_t, size_t,
	    int (*)(const void *, const void *))  ;
int	 rand  (void)  ;
void	*realloc  (void *, size_t)  ;
void	 srand  (unsigned)  ;
double	 strtod  (const char *, char **)  ;
long	 strtol  (const char *, char **, int)  ;
unsigned long
	 strtoul  (const char *, char **, int)  ;
int	 system  (const char *)  ;

 
int	 mblen  (const char *, size_t)  ;
size_t	 mbstowcs  (wchar_t *, const char *, size_t)  ;
int	 wctomb  (char *, wchar_t)  ;
int	 mbtowc  (wchar_t *, const char *, size_t)  ;
size_t	 wcstombs  (char *, const wchar_t *, size_t)  ;


int	 putenv  (const char *)  ;
int	 setenv  (const char *, const char *, int)  ;



void	*alloca  (size_t)  ;		 
					 
char	*getbsize  (int *, long *)  ;
char	*cgetcap  (char *, char *, int)  ;
int	 cgetclose  (void)  ;
int	 cgetent  (char **, char **, char *)  ;
int	 cgetfirst  (char **, char **)  ;
int	 cgetmatch  (char *, char *)  ;
int	 cgetnext  (char **, char **)  ;
int	 cgetnum  (char *, char *, long *)  ;
int	 cgetset  (char *)  ;
int	 cgetstr  (char *, char *, char **)  ;
int	 cgetustr  (char *, char *, char **)  ;

int	 daemon  (int, int)  ;
char	*devname  (int, int)  ;
int	 getloadavg  (double [], int)  ;

char	*group_from_gid  (unsigned long, int)  ;
int	 heapsort  (void *, size_t, size_t,
	    int (*)(const void *, const void *))  ;
char	*initstate  (unsigned long, char *, long)  ;
int	 mergesort  (void *, size_t, size_t,
	    int (*)(const void *, const void *))  ;
int	 radixsort  (const unsigned char **, int, const unsigned char *,
	    unsigned)  ;
int	 sradixsort  (const unsigned char **, int, const unsigned char *,
	    unsigned)  ;
long	 random  (void)  ;
char	*realpath  (const char *, char resolved_path[])  ;
char	*setstate  (char *)  ;
void	 srandom  (unsigned long)  ;
char	*user_from_uid  (unsigned long, int)  ;

long long
	 strtoq  (const char *, char **, int)  ;
unsigned long long
	 strtouq  (const char *, char **, int)  ;

void	 unsetenv  (const char *)  ;

} 


# 38 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/time.h" 1 3 4
 




















 































































struct tm {
	int	tm_sec;		 
	int	tm_min;		 
	int	tm_hour;	 
	int	tm_mday;	 
	int	tm_mon;		 
	int	tm_year;	 
	int	tm_wday;	 
	int	tm_yday;	 
	int	tm_isdst;	 
	long	tm_gmtoff;	 
	char	*tm_zone;	 
};

# 1 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 1 3


# 1 "/usr/include/ppc/limits.h" 1 3 4
 

































 














 















































# 3 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 2 3











 



 



 




 





 



 












 

 




 



 








 



 













 




 








 






# 100 "/usr/include/time.h" 2 3 4






extern "C" { 
char *asctime  (const struct tm *)  ;
clock_t clock  (void)  ;
char *ctime  (const time_t *)  ;
double difftime  (time_t, time_t)  ;
struct tm *gmtime  (const time_t *)  ;
struct tm *localtime  (const time_t *)  ;
time_t mktime  (struct tm *)  ;
size_t strftime  (char *, size_t, const char *, const struct tm *)  ;
time_t time  (time_t *)  ;


void tzset  (void)  ;



char *timezone  (int, int)  ;
void tzsetwall  (void)  ;

} 


# 39 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/gcc/darwin/2.95.2/g++/../stdarg.h" 1 3
 








































# 1 "/usr/include/gcc/darwin/2.95.2/g++/../va-ppc.h" 1 3
 


 
 



typedef char *__gnuc_va_list;


 





 




# 43 "/usr/include/gcc/darwin/2.95.2/g++/../va-ppc.h" 3



void va_end (__gnuc_va_list);		 


 












 





# 352 "/usr/include/gcc/darwin/2.95.2/g++/../va-ppc.h" 3

# 42 "/usr/include/gcc/darwin/2.95.2/g++/../stdarg.h" 2 3

# 131 "/usr/include/gcc/darwin/2.95.2/g++/../stdarg.h" 3












 
 













# 175 "/usr/include/gcc/darwin/2.95.2/g++/../stdarg.h" 3


 




 

 

 

typedef __gnuc_va_list va_list;
























# 40 "/usr/include/libc.h" 2 3 4



# 1 "/usr/include/sys/mount.h" 1 3 4
 




















 
 






































# 1 "/usr/include/sys/ucred.h" 1 3 4
 




















 
 





































# 1 "/usr/include/sys/param.h" 1 3 4
 




















 
 

























































 






# 1 "/usr/include/sys/syslimits.h" 1 3 4
 




















 

 































































# 88 "/usr/include/sys/param.h" 2 3 4













 
# 1 "/usr/include/machine/param.h" 1 3 4
 




















 







# 1 "/usr/include/ppc/param.h" 1 3 4
 




















 
 








 



























					 

 















		 



 




 


 













 







 














 











# 30 "/usr/include/machine/param.h" 2 3 4









# 102 "/usr/include/sys/param.h" 2 3 4

# 1 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 1 3
# 10 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 3

# 108 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 3

# 103 "/usr/include/sys/param.h" 2 3 4


 









 


 



























 






















				 



 












 











 





 






 







 

















 














# 61 "/usr/include/sys/ucred.h" 2 3 4


 


struct ucred {
	u_long	cr_ref;			 
	uid_t	cr_uid;			 
	short	cr_ngroups;		 
	gid_t	cr_groups[16  ];	 
};




# 88 "/usr/include/sys/ucred.h" 3 4



# 62 "/usr/include/sys/mount.h" 2 3 4


# 1 "/usr/include/sys/queue.h" 1 3 4
 




















 





































 




































































 






 




 
 






























# 184 "/usr/include/sys/queue.h" 3 4

 
















 













































# 260 "/usr/include/sys/queue.h" 3 4

 
















 













































 

















 



























# 378 "/usr/include/sys/queue.h" 3 4









# 395 "/usr/include/sys/queue.h" 3 4

















 














 

















# 453 "/usr/include/sys/queue.h" 3 4


# 463 "/usr/include/sys/queue.h" 3 4


# 473 "/usr/include/sys/queue.h" 3 4


# 483 "/usr/include/sys/queue.h" 3 4








# 502 "/usr/include/sys/queue.h" 3 4

# 548 "/usr/include/sys/queue.h" 3 4



# 64 "/usr/include/sys/mount.h" 2 3 4

# 1 "/usr/include/sys/lock.h" 1 3 4
 




















 
 








































# 74 "/usr/include/sys/lock.h" 3 4





# 1 "/usr/include/mach/boolean.h" 1 3 4
 




















 


 































 
 

























 
 
























 

 









 




# 1 "/usr/include/mach/machine/boolean.h" 1 3 4
 

























# 1 "/usr/include/mach/ppc/boolean.h" 1 3 4
 




















 


 












































 
 



















 
 
























 


 








typedef int		boolean_t;


# 27 "/usr/include/mach/machine/boolean.h" 2 3 4









# 127 "/usr/include/mach/boolean.h" 2 3 4



 

















# 79 "/usr/include/sys/lock.h" 2 3 4



struct slock{
	volatile unsigned int lock_data[10];
};





typedef struct slock	simple_lock_data_t;
typedef struct slock	*simple_lock_t;






 




struct lock__bsd__ {
	simple_lock_data_t
		lk_interlock;		 
	u_int	lk_flags;		 
	int	lk_sharecount;		 
	int	lk_waitcount;		 
	short	lk_exclusivecount;	 
	short	lk_prio;		 
	char	*lk_wmesg;		 
	int	lk_timo;		 
	pid_t	lk_lockholder;		 
	void	*lk_lockthread;		 
};
 








































 











 










 








 
















 





struct proc;

void	lockinit  (struct lock__bsd__ *, int prio, char *wmesg, int timo,
			int flags)  ;
int	lockmgr  (struct lock__bsd__ *, u_int flags,
			simple_lock_t, struct proc *p)  ;
int	lockstatus  (struct lock__bsd__ *)  ;


# 65 "/usr/include/sys/mount.h" 2 3 4

# 1 "/usr/include/net/radix.h" 1 3 4
 




















 









































 



struct radix_node {
	struct	radix_mask *rn_mklist;	 
	struct	radix_node *rn_p;	 
	short	rn_b;			 
	char	rn_bmask;		 
	u_char	rn_flags;		 



	union {
		struct {			 
			caddr_t	rn_Key;		 
			caddr_t	rn_Mask;	 
			struct	radix_node *rn_Dupedkey;
		} rn_leaf;
		struct {			 
			int	rn_Off;		 
			struct	radix_node *rn_L; 
			struct	radix_node *rn_R; 
		} rn_node;
	}		rn_u;





};








 



struct radix_mask {
	short	rm_b;			 
	char	rm_unused;		 
	u_char	rm_flags;		 
	struct	radix_mask *rm_mklist;	 
	union	{
		caddr_t	rmu_mask;		 
		struct	radix_node *rmu_leaf;	 
	}	rm_rmu;
	int	rm_refs;		 
};













typedef int walktree_f_t  (struct radix_node *, void *)  ;

struct radix_node_head {
	struct	radix_node *rnh_treetop;
	int	rnh_addrsize;		 
	int	rnh_pktsize;		 
	struct	radix_node *(*rnh_addaddr)	 
		 (void *v, void *mask,
		     struct radix_node_head *head, struct radix_node nodes[])  ;
	struct	radix_node *(*rnh_addpkt)	 
		 (void *v, void *mask,
		     struct radix_node_head *head, struct radix_node nodes[])  ;
	struct	radix_node *(*rnh_deladdr)	 
		 (void *v, void *mask, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_delpkt)	 
		 (void *v, void *mask, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_matchaddr)	 
		 (void *v, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_lookup)	 
		 (void *v, void *mask, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_matchpkt)	 
		 (void *v, struct radix_node_head *head)  ;
	int	(*rnh_walktree)			 
		 (struct radix_node_head *head, walktree_f_t *f, void *w)  ;
	int	(*rnh_walktree_from)		 
		 (struct radix_node_head *head, void *a, void *m,
		     walktree_f_t *f, void *w)  ;
	void	(*rnh_close)	 
		 (struct radix_node *rn, struct radix_node_head *head)  ;
	struct	radix_node rnh_nodes[3];	 
};















void	 rn_init  (void)  ;
int	 rn_inithead  (void **, int)  ;
int	 rn_refines  (void *, void *)  ;
struct radix_node
	 *rn_addmask  (void *, int, int)  ,
	 *rn_addroute  (void *, void *, struct radix_node_head *,
			struct radix_node [2])  ,
	 *rn_delete  (void *, void *, struct radix_node_head *)  ,
	 *rn_lookup  (void *v_arg, void *m_arg,
		        struct radix_node_head *head)  ,
	 *rn_match  (void *, struct radix_node_head *)  ;



# 66 "/usr/include/sys/mount.h" 2 3 4

# 1 "/usr/include/sys/socket.h" 1 3 4
 




















 
 
 






































 



 








 















					 



 














 


struct	linger {
	int	l_onoff;		 
	int	l_linger;		 
};

 




 






























					 
					 
					 












 



struct sockaddr {
	u_char	sa_len;			 
	u_char	sa_family;		 
	char	sa_data[14];		 
};


 



struct sockproto {
	u_short	sp_family;		 
	u_short	sp_protocol;		 
};


 








struct sockaddr_storage {
	u_char ss_len;		 
	u_char ss_family;	 
	char _ss_pad1[((sizeof(int64_t))  - sizeof(u_char) * 2) ];
	int64_t _ss_align;	 
	char _ss_pad2[(128  - sizeof(u_char) * 2 - ((sizeof(int64_t))  - sizeof(u_char) * 2)  - (sizeof(int64_t)) ) ];
};


 











































 










# 295 "/usr/include/sys/socket.h" 3 4

 



















 




 



struct msghdr {
	caddr_t	msg_name;		 
	u_int	msg_namelen;		 
	struct	iovec *msg_iov;		 
	u_int	msg_iovlen;		 
	caddr_t	msg_control;		 
	u_int	msg_controllen;		 
	int	msg_flags;		 
};

















 





struct cmsghdr {
	u_int	cmsg_len;		 
	int	cmsg_level;		 
	int	cmsg_type;		 
 
};

 


 





 











 




 


struct osockaddr {
	u_short	sa_family;		 
	char	sa_data[14];		 
};

 


struct omsghdr {
	caddr_t	msg_name;		 
	int	msg_namelen;		 
	struct	iovec *msg_iov;		 
	int	msg_iovlen;		 
	caddr_t	msg_accrights;		 
	int	msg_accrightslen;
};

 






# 427 "/usr/include/sys/socket.h" 3 4






extern "C" { 
int	accept  (int, struct sockaddr *, int *)  ;
int	bind  (int, const struct sockaddr *, int)  ;
int	connect  (int, const struct sockaddr *, int)  ;
int	getpeername  (int, struct sockaddr *, int *)  ;
int	getsockname  (int, struct sockaddr *, int *)  ;
int	getsockopt  (int, int, int, void *, int *)  ;
int	listen  (int, int)  ;
ssize_t	recv  (int, void *, size_t, int)  ;
ssize_t	recvfrom  (int, void *, size_t, int, struct sockaddr *, int *)  ;
ssize_t	recvmsg  (int, struct msghdr *, int)  ;
ssize_t	send  (int, const void *, size_t, int)  ;
ssize_t	sendto  (int, const void *,
	    size_t, int, const struct sockaddr *, int)  ;
ssize_t	sendmsg  (int, const struct msghdr *, int)  ;



int	setsockopt  (int, int, int, const void *, int)  ;
int	shutdown  (int, int)  ;
int	socket  (int, int, int)  ;
int	socketpair  (int, int, int, int *)  ;
} 



# 67 "/usr/include/sys/mount.h" 2 3 4


typedef struct fsid { int32_t val[2]; } fsid_t;	 

 





struct fid {
	u_short		fid_len;		 
	u_short		fid_reserved;		 
	char		fid_data[16 ];	 
};

 






struct statfs {
	short	f_otype;		 
	short	f_oflags;		 
	long	f_bsize;		 
	long	f_iosize;		 
	long	f_blocks;		 
	long	f_bfree;		 
	long	f_bavail;		 
	long	f_files;		 
	long	f_ffree;		 
	fsid_t	f_fsid;			 
	uid_t	f_owner;		 
	short	f_reserved1;	 
	short	f_type;			 
    long	f_flags;		 
	long    f_reserved2[2];	 
	char	f_fstypename[15 ];  
	char	f_mntonname[90 ];	 
	char	f_mntfromname[90 ]; 




	char	f_reserved3;	 
	long	f_reserved4[4];	 

};

 




struct  vnodelst  {	struct   vnode  *lh_first;	} ;

struct mount {
	struct {	struct  mount  *cqe_next;	struct  mount  *cqe_prev;	}  mnt_list;		 
	struct vfsops	*mnt_op;		 
	struct vfsconf	*mnt_vfc;		 
	struct vnode	*mnt_vnodecovered;	 
	struct vnodelst	mnt_vnodelist;		 
	struct lock__bsd__ mnt_lock;		 
	int		mnt_flag;		 
	int		mnt_kern_flag;		 
	int		mnt_maxsymlinklen;	 
	struct statfs	mnt_stat;		 
	qaddr_t		mnt_data;		 
};

 















 








 








 










 











 












 







 







 







 


struct fhandle {
	fsid_t	fh_fsid;	 
	struct	fid fh_fid;	 
};
typedef struct fhandle	fhandle_t;

 


struct export_args {
	int	ex_flags;		 
	uid_t	ex_root;		 
	struct	ucred ex_anon;		 
	struct	sockaddr *ex_addr;	 
	int	ex_addrlen;		 
	struct	sockaddr *ex_mask;	 
	int	ex_masklen;		 
};

 




struct vfsconf {
	struct	vfsops *vfc_vfsops;	 
	char	vfc_name[15 ];	 
	int	vfc_typenum;		 
	int	vfc_refcount;		 
	int	vfc_flags;		 
	int	(*vfc_mountroot)(void);	 
	struct	vfsconf *vfc_next;	 
};

# 361 "/usr/include/sys/mount.h" 3 4




extern "C" { 
int	fstatfs  (int, struct statfs *)  ;
int	getfh  (const char *, fhandle_t *)  ;
int	getfsstat  (struct statfs *, long, int)  ;
int	getmntinfo  (struct statfs **, int)  ;
int	mount  (const char *, const char *, int, void *)  ;
int	statfs  (const char *, struct statfs *)  ;
int	unmount  (const char *, int)  ;
} 



# 43 "/usr/include/libc.h" 2 3 4


# 1 "/usr/include/sys/wait.h" 1 3 4
 




















 
 

































 



 




 


























 












 

 







 





union wait {
	int	w_status;		 
	 


	struct {







		unsigned int	w_Filler:16,	 
				w_Retcode:8,	 
				w_Coredump:1,	 
				w_Termsig:7;	 

	} w_T;
	 




	struct {






		unsigned int	w_Filler:16,	 
				w_Stopsig:8,	 
				w_Stopval:8;	 

	} w_S;
};













extern "C" { 
struct rusage;	 

pid_t	wait  (int *)  ;
pid_t	waitpid  (pid_t, int *, int)  ;

pid_t	wait3  (int *, int, struct rusage *)  ;
pid_t	wait4  (pid_t, int *, int, struct rusage *)  ;

} 


# 45 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/sys/time.h" 1 3 4
 




















 
 









































 



struct timeval {
	int32_t	tv_sec;		 
	int32_t	tv_usec;	 
};

 


struct timespec {
	time_t	tv_sec;		 
	int32_t	tv_nsec;	 
};










struct timezone {
	int	tz_minuteswest;	 
	int	tz_dsttime;	 
};










 







# 121 "/usr/include/sys/time.h" 3 4

# 130 "/usr/include/sys/time.h" 3 4

 







struct	itimerval {
	struct	timeval it_interval;	 
	struct	timeval it_value;	 
};

 


struct clockinfo {
	int	hz;		 
	int	tick;		 
	int	tickadj;	 
	int	stathz;		 
	int	profhz;		 
};













extern "C" { 
int	adjtime  (const struct timeval *, struct timeval *)  ;
int	getitimer  (int, struct itimerval *)  ;
int	gettimeofday  (struct timeval *, struct timezone *)  ;
int	setitimer  (int, const struct itimerval *, struct itimerval *)  ;
int	settimeofday  (const struct timeval *, const struct timezone *)  ;
int	utimes  (const char *, const struct timeval *)  ;
} 





# 46 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/sys/times.h" 1 3 4
 




















 
 

















































struct tms {
	clock_t tms_utime;	 
	clock_t tms_stime;	 
	clock_t tms_cutime;	 
	clock_t tms_cstime;	 
};




extern "C" { 
clock_t	times  (struct tms *)  ;
} 


# 47 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/sys/resource.h" 1 3 4
 




















 
 





































 









 






struct	rusage {
	struct timeval ru_utime;	 
	struct timeval ru_stime;	 
	long	ru_maxrss;		 

	long	ru_ixrss;		 
	long	ru_idrss;		 
	long	ru_isrss;		 
	long	ru_minflt;		 
	long	ru_majflt;		 
	long	ru_nswap;		 
	long	ru_inblock;		 
	long	ru_oublock;		 
	long	ru_msgsnd;		 
	long	ru_msgrcv;		 
	long	ru_nsignals;		 
	long	ru_nvcsw;		 
	long	ru_nivcsw;		 

};

 
















struct orlimit {
	int32_t	rlim_cur;		 
	int32_t	rlim_max;		 
};

struct rlimit {
	rlim_t	rlim_cur;		 
	rlim_t	rlim_max;		 
};

 
struct loadavg {
	fixpt_t	ldavg[3];
	long	fscale;
};







extern "C" { 
int	getpriority  (int, int)  ;
int	getrlimit  (int, struct rlimit *)  ;
int	getrusage  (int, struct rusage *)  ;
int	setpriority  (int, int, int)  ;
int	setrlimit  (int, const struct rlimit *)  ;
} 



# 48 "/usr/include/libc.h" 2 3 4




# 1 "/usr/include/sys/stat.h" 1 3 4
 




















 
 














































struct ostat {
	u_int16_t st_dev;		 
	ino_t	  st_ino;		 
	mode_t	  st_mode;		 
	nlink_t	  st_nlink;		 
	u_int16_t st_uid;		 
	u_int16_t st_gid;		 
	u_int16_t st_rdev;		 
	int32_t	  st_size;		 
	struct	timespec st_atimespec;	 
	struct	timespec st_mtimespec;	 
	struct	timespec st_ctimespec;	 
	int32_t	  st_blksize;		 
	int32_t	  st_blocks;		 
	u_int32_t st_flags;		 
	u_int32_t st_gen;		 
};


struct stat {
	dev_t	  st_dev;		 
	ino_t	  st_ino;		 
	mode_t	  st_mode;		 
	nlink_t	  st_nlink;		 
	uid_t	  st_uid;		 
	gid_t	  st_gid;		 
	dev_t	  st_rdev;		 

	struct	timespec st_atimespec;	 
	struct	timespec st_mtimespec;	 
	struct	timespec st_ctimespec;	 








	off_t	  st_size;		 
	int64_t	  st_blocks;		 
	u_int32_t st_blksize;		 
	u_int32_t st_flags;		 
	u_int32_t st_gen;		 
	int32_t	  st_lspare;
	int64_t	  st_qspare[2];
};































































							 

							 




 









 




















extern "C" { 
int	chmod  (const char *, mode_t)  ;
int	fstat  (int, struct stat *)  ;
int	mkdir  (const char *, mode_t)  ;
int	mkfifo  (const char *, mode_t)  ;
int	stat  (const char *, struct stat *)  ;
mode_t	umask  (mode_t)  ;

int	chflags  (const char *, u_long)  ;
int	fchflags  (int, u_long)  ;
int	fchmod  (int, mode_t)  ;
int	lstat  (const char *, struct stat *)  ;

} 


# 52 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/sys/file.h" 1 3 4
 




















 
 





































# 1 "/usr/include/sys/fcntl.h" 1 3 4
 




















 
 











































 









 






 





 




























 


# 131 "/usr/include/sys/fcntl.h" 3 4


 













 









 



 













	







 


 









 




 


					 	

 



struct flock {
	off_t	l_start;	 
	off_t	l_len;		 
	pid_t	l_pid;		 
	short	l_type;		 
	short	l_whence;	 
};


 



struct radvisory {
       off_t   ra_offset;
       int     ra_count;
};



 






 

typedef struct fstore {
        u_int32_t fst_flags;	 
	int 	fst_posmode;	 
	off_t	fst_offset;	 
	off_t	fst_length;	 
	off_t   fst_bytesalloc;	 
} fstore_t;

 

typedef struct fbootstraptransfer {
  off_t fbt_offset;              
  size_t fbt_length;             
  void *fbt_buffer;              
} fbootstraptransfer_t;

 















struct log2phys {
	u_int32_t	l2p_flags;		 
	off_t		l2p_contigbytes;	 
	off_t		l2p_devoffset;	 
};









extern "C" { 
int	open  (const char *, int, ...)  ;
int	creat  (const char *, mode_t)  ;
int	fcntl  (int, int, ...)  ;

int	flock  (int, int)  ;

} 



# 61 "/usr/include/sys/file.h" 2 3 4



# 112 "/usr/include/sys/file.h" 3 4



# 53 "/usr/include/libc.h" 2 3 4


# 1 "/usr/include/sys/ioctl.h" 1 3 4
 




















 
 










































# 1 "/usr/include/sys/ttycom.h" 1 3 4
 




















 
 










































# 1 "/usr/include/sys/ioccom.h" 1 3 4
 




















 
 





































 










				 

				 

				 

				 

				 







 



# 66 "/usr/include/sys/ttycom.h" 2 3 4


 




 



struct winsize {
	unsigned short	ws_row;		 
	unsigned short	ws_col;		 
	unsigned short	ws_xpixel;	 
	unsigned short	ws_ypixel;	 
};














						 


						 

						 






						 






						 















































# 66 "/usr/include/sys/ioctl.h" 2 3 4


 




struct ttysize {
	unsigned short	ts_lines;
	unsigned short	ts_cols;
	unsigned short	ts_xxx;
	unsigned short	ts_yyy;
};





# 1 "/usr/include/sys/filio.h" 1 3 4
 




















 
 












































 









# 84 "/usr/include/sys/ioctl.h" 2 3 4

# 1 "/usr/include/sys/sockio.h" 1 3 4
 




















 
 







































 





























   
 
































                             


# 85 "/usr/include/sys/ioctl.h" 2 3 4






extern "C" { 
int	ioctl  (int, unsigned long, ...)  ;
} 



 










# 55 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/netinet/in.h" 1 3 4
 




















 





































 




 


























































 












































 





 
 




 


 












































 








 





 







 


struct in_addr {
	u_int32_t s_addr;
};

 












































 


struct sockaddr_in {
	u_char	sin_len;
	u_char	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};



 






struct ip_opts {
	struct	in_addr ip_dst;		 
	char	ip_opts[40];		 
};

 









































 






 


struct ip_mreq {
	struct	in_addr imr_multiaddr;	 
	struct	in_addr imr_interface;	 
};

 







 








# 456 "/usr/include/netinet/in.h" 3 4

 























# 499 "/usr/include/netinet/in.h" 3 4

 

# 1 "/usr/include/netinet6/in6.h" 1 3 4
 

 




























 













































 





 

































 


struct in6_addr {
	union {
		u_int8_t   __u6_addr8[16];
		u_int16_t  __u6_addr16[8];
		u_int32_t  __u6_addr32[4];
	} __u6_addr;			 
};










 





struct sockaddr_in6 {
	u_int8_t	sin6_len;	 
	u_int8_t	sin6_family;	 
	u_int16_t	sin6_port;	 
	u_int32_t	sin6_flowinfo;	 
	struct in6_addr	sin6_addr;	 
	u_int32_t	sin6_scope_id;	 
};

 


# 166 "/usr/include/netinet6/in6.h" 3 4










 


# 199 "/usr/include/netinet6/in6.h" 3 4


 


























 













 








 








 









 







 

















 








 










 


# 335 "/usr/include/netinet6/in6.h" 3 4


















 






 








 



struct route_in6 {
	struct	rtentry *ro_rt;
	struct	sockaddr_in6 ro_dst;
};


 



 























				    
















 












 







 





 


struct ipv6_mreq {
	struct in6_addr	ipv6mr_multiaddr;
	u_int		ipv6mr_interface;
};

 


struct in6_pktinfo {
	struct in6_addr ipi6_addr;	 
	u_int ipi6_ifindex;		 
};

 








 








# 530 "/usr/include/netinet6/in6.h" 3 4

 





























 























# 611 "/usr/include/netinet6/in6.h" 3 4

# 640 "/usr/include/netinet6/in6.h" 3 4



# 666 "/usr/include/netinet6/in6.h" 3 4


extern "C" { 
struct cmsghdr;

extern int inet6_option_space  (int)  ;
extern int inet6_option_init  (void *, struct cmsghdr **, int)  ;
extern int inet6_option_append  (struct cmsghdr *, const u_int8_t *,
	int, int)  ;
extern u_int8_t *inet6_option_alloc  (struct cmsghdr *, int, int, int)  ;
extern int inet6_option_next  (const struct cmsghdr *, u_int8_t **)  ;
extern int inet6_option_find  (const struct cmsghdr *, u_int8_t **, int)  ;

extern size_t inet6_rthdr_space  (int, int)  ;
extern struct cmsghdr *inet6_rthdr_init  (void *, int)  ;
extern int inet6_rthdr_add  (struct cmsghdr *, const struct in6_addr *,
		unsigned int)  ;
extern int inet6_rthdr_lasthop  (struct cmsghdr *, unsigned int)  ;



extern int inet6_rthdr_segments  (const struct cmsghdr *)  ;
extern struct in6_addr *inet6_rthdr_getaddr  (struct cmsghdr *, int)  ;
extern int inet6_rthdr_getflags  (const struct cmsghdr *, int)  ;

extern int inet6_opt_init  (void *, size_t)  ;
extern int inet6_opt_append  (void *, size_t, int, u_int8_t,
				 size_t, u_int8_t, void **)  ;
extern int inet6_opt_finish  (void *, size_t, int)  ;
extern int inet6_opt_set_val  (void *, size_t, void *, int)  ;

extern int inet6_opt_next  (void *, size_t, int, u_int8_t *,
			       size_t *, void **)  ;
extern int inet6_opt_find  (void *, size_t, int, u_int8_t,
			  size_t *, void **)  ;
extern int inet6_opt_get_val  (void *, size_t, void *, int)  ;
extern size_t inet6_rth_space  (int, int)  ;
extern void *inet6_rth_init  (void *, int, int, int)  ;
extern int inet6_rth_add  (void *, const struct in6_addr *)  ;
extern int inet6_rth_reverse  (const void *, void *)  ;
extern int inet6_rth_segments  (const void *)  ;
extern struct in6_addr *inet6_rth_getaddr  (const void *, int)  ;
} 


# 502 "/usr/include/netinet/in.h" 2 3 4



# 514 "/usr/include/netinet/in.h" 3 4



# 56 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/arpa/inet.h" 1 3 4
 




















 





































 



extern "C" { 
unsigned long	 inet_addr  (const char *)  ;
int		 inet_aton  (const char *, struct in_addr *)  ;
unsigned long	 inet_lnaof  (struct in_addr)  ;
struct in_addr	 inet_makeaddr  (u_long , u_long)  ;
unsigned long	 inet_netof  (struct in_addr)  ;
unsigned long	 inet_network  (const char *)  ;
char		*inet_ntoa  (struct in_addr)  ;
} 


# 57 "/usr/include/libc.h" 2 3 4

# 1 "/usr/include/mach/machine/vm_types.h" 1 3 4
 

























# 1 "/usr/include/mach/ppc/vm_types.h" 1 3 4
 




















 


 














































 
 























 
 
























 


 












 










typedef unsigned int	natural_t;

 






typedef int		integer_t;









 



typedef	natural_t	vm_offset_t;

 




typedef	natural_t		vm_size_t;
typedef	unsigned long long	vm_double_size_t;

 


typedef unsigned int	space_t;



 






# 27 "/usr/include/mach/machine/vm_types.h" 2 3 4









# 58 "/usr/include/libc.h" 2 3 4


# 1 "/usr/include/mach/kern_return.h" 1 3 4
 




















 


 
























 

 











# 1 "/usr/include/mach/machine/kern_return.h" 1 3 4
 

























# 1 "/usr/include/mach/ppc/kern_return.h" 1 3 4
 




















 


 












































 
 






















 
 
























 


 











typedef	int		kern_return_t;


# 27 "/usr/include/mach/machine/kern_return.h" 2 3 4









# 64 "/usr/include/mach/kern_return.h" 2 3 4


 







		 



		 




		 





		 




		 



		 




		 




		 



		 





		 







		 



		 



		 



		 




		 



		 



		 



		 



		 



		 



		 




		 



		 





		 















		 







		 




		 




		 




		 




		 




		 




		 





		 



		 



		 


		

		 


		

		 



		 



		 




		 



		 



		 



		 




		 



		 



		 



		 



		 


		 



		 



# 60 "/usr/include/libc.h" 2 3 4


struct qelem {
        struct qelem *q_forw;
        struct qelem *q_back;
        char *q_data;
};

extern kern_return_t map_fd(int fd, vm_offset_t offset,
        vm_offset_t *addr, boolean_t find_space, vm_size_t numbytes);



# 12 "QvBasic.h" 2











typedef int QvBool;

 










 








# 4 "QvString.h" 2



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
    void		makeEmpty(QvBool freeOld = 1 );
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

    char		staticStorage[32 ];
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


# 4 "QvSFEnum.h" 2

# 1 "QvSubField.h" 1



# 1 "QvField.h" 1





class QvInput;
class QvNode;

class QvField {
  public:
    virtual ~QvField();

    void		setIgnored(QvBool ig)	{ flags.ignored = ig;   }
    QvBool		isIgnored() const	{ return flags.ignored; }

    QvBool		isDefault() const	{ return flags.hasDefault; }

    QvNode *		getContainer() const	{ return container; }

    void		setDefault(QvBool def)	{ flags.hasDefault = def; }
    void		setContainer(QvNode *cont);
    QvBool		read(QvInput *in, const QvName &name);

    QvField()		{ flags.hasDefault = 1 ; flags.ignored = 0 ; }

  public:

  private:
    struct {
	unsigned int hasDefault		: 1;  
	unsigned int ignored		: 1;  
    }			flags;

    QvNode		*container;

    static QvField *	createInstanceFromName(const QvName &className);
    virtual QvBool	readValue(QvInput *in) = 0;

friend class QvFieldData;
};

class QvSField : public QvField {
  public:
    virtual ~QvSField();

  protected:
    QvSField();

  private:
    virtual QvBool	readValue(QvInput *in) = 0;
};

class QvMField : public QvField {

  public:
    int			num;		 
    int			maxNum;		 

     
    virtual ~QvMField();

  protected:
    QvMField();
    virtual void	makeRoom(int newNum);

  private:
    virtual void	allocValues(int num) = 0;
    virtual QvBool	readValue(QvInput *in);
    virtual QvBool	read1Value(QvInput *in, int index) = 0;
};


# 4 "QvSubField.h" 2

# 1 "QvInput.h" 1



# 1 "QvDict.h" 1





# 1 "QvPList.h" 1





class QvPList {
  public:
    QvPList();
    ~QvPList();
    void	append(void * ptr)
	{ if (nPtrs + 1 > ptrsSize) expand(nPtrs + 1);
	  ptrs[nPtrs++] = ptr; }
    int		find(const void *ptr) const;
    void	remove(int which);
    int		getLength() const		{ return (int) nPtrs;	}
    void	truncate(int start)
	{ nPtrs = start; }
    void *&	operator [](int i) const	{ return ptrs[i]; }

  private:
    void **	ptrs;
    int		nPtrs;
    int		ptrsSize;		
    void	setSize(int size)
	{ if (size > ptrsSize) expand(size); nPtrs = size; }
    void	expand(int size);
};


# 6 "QvDict.h" 2


class QvDictEntry {
  private:
    u_long		key;
    void *		value;
    QvDictEntry *	next;
    QvDictEntry(u_long k, void *v)	{ key = k; value = v; };

friend class QvDict;
};

class QvDict {
  public:
    QvDict( int entries = 251 );
    ~QvDict();
    void	clear();
    QvBool	enter(u_long key, void *value);
    QvBool	find(u_long key, void *&value) const;
    QvBool	remove(u_long key);

  private:
    int			tableSize;
    QvDictEntry *	*buckets;
    QvDictEntry *&	findEntry(u_long key) const;
};


# 4 "QvInput.h" 2



class QvNode;
class QvDB;

class QvInput {
 public:

    QvInput();
    ~QvInput();

    static float	isASCIIHeader(const char *string);
    void		setFilePointer(FILE *newFP);
    FILE *		getCurFile() const { return fp; }
    float		getVersion();
    QvBool		get(char &c);
    QvBool		read(char	    &c);
    QvBool		read(QvString       &s);
    QvBool		read(QvName	    &n, QvBool validIdent = 0 );
    QvBool		read(int	    &i);
    QvBool		read(unsigned int   &i);
    QvBool		read(short	    &s);
    QvBool		read(unsigned short &s);
    QvBool		read(long	    &l);
    QvBool		read(unsigned long  &l);
    QvBool		read(float	    &f);
    QvBool		read(double	    &d);
    QvBool		eof() const;
    void		getLocationString(QvString &string) const;
    void		putBack(char c);
    void		putBack(const char *string);
    void		addReference(const QvName &name, QvNode *node);
    QvNode *		findReference(const QvName &name) const;

  private:
    FILE		*fp;		 
    int			lineNum;	 
    float		version;	 
    QvBool		readHeader;	 
    QvBool		headerOk;	 
    QvDict		refDict;	 
    QvString		backBuf;
    int			backBufIndex;		

    QvBool		checkHeader();

    QvBool		skipWhiteSpace();

    QvBool		readInteger(long &l);
    QvBool		readUnsignedInteger(unsigned long &l);
    QvBool		readReal(double &d);
    QvBool		readUnsignedIntegerString(char *str);
    int			readDigits(char *string);
    int			readHexDigits(char *string);
    int			readChar(char *string, char charToRead);

friend class QvNode;
friend class QvDB;
};


# 5 "QvSubField.h" 2


 







 








 










 


# 71 "QvSubField.h"


# 5 "QvSFEnum.h" 2


class QvSFEnum : public QvSField {
  public:
    int value;
    public:	 QvSFEnum ();	virtual ~ QvSFEnum ();	virtual QvBool readValue(QvInput *in) ;

     
    void setEnums(int num, const int vals[], const QvName names[])
	{ numEnums = num; enumValues = vals; enumNames = names; }

    int			numEnums;	 
    const int		*enumValues;	 
    const QvName	*enumNames;	 

     
    QvBool		findEnumValue(const QvName &name, int &val) const;
};


# 37 "QvSFEnum.h"


# 4 "QvWWWInline.h" 2

# 1 "QvSFVec3f.h" 1





class QvSFVec3f : public QvSField {
  public:
    float value[3];
    public:	 QvSFVec3f ();	virtual ~ QvSFVec3f ();	virtual QvBool readValue(QvInput *in) ;
};


# 5 "QvWWWInline.h" 2

# 1 "QvGroup.h" 1



class QvChildList;
# 1 "QvSubNode.h" 1



# 1 "QvFieldData.h" 1







class QvField;
class QvInput;
class QvNode;

class QvFieldData {
  public:
    QvFieldData() {}
    ~QvFieldData();

    void		addField(QvNode *defObject, const char *fieldName,
				 const QvField *field);

    int			getNumFields() const	{ return fields.getLength(); }

    const QvName &	getFieldName(int index) const;

    QvField *		getField(const QvNode *object,
				 int index) const;

    void		addEnumValue(const char *typeName,
				     const char *valName, int val);
    void		getEnumData(const char *typeName, int &num,
				    const int *&vals, const QvName *&names);

    QvBool		read(QvInput *in, QvNode *object,
			     QvBool errorOnUnknownField = 1 ) const;

    QvBool		read(QvInput *in, QvNode *object,
			     const QvName &fieldName,
			     QvBool &foundName) const;

    QvBool		readFieldTypes(QvInput *in, QvNode *object);

  private:
    QvPList		fields;
    QvPList		enums;
};    


# 4 "QvSubNode.h" 2

# 1 "QvNode.h" 1





class QvChildList;
class QvDict;
class QvFieldData;
class QvInput;
class QvNodeList;
class QvState;

class QvNode {

  public:
    enum Stage {
	FIRST_INSTANCE,		 
	PROTO_INSTANCE,		 
	OTHER_INSTANCE,		 
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


# 5 "QvSubNode.h" 2



# 16 "QvSubNode.h"


























# 5 "QvGroup.h" 2


class QvGroup : public QvNode {

    public:	 QvGroup :: QvGroup ();	virtual ~ QvGroup ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    QvNode *		getChild(int index) const;
    int			getNumChildren() const;
    virtual QvChildList *getChildren() const;
    virtual QvBool	readInstance(QvInput *in);
    virtual QvBool	readChildren(QvInput *in);
};


# 6 "QvWWWInline.h" 2


class QvWWWInline : public QvGroup {

    public:	 QvWWWInline :: QvWWWInline ();	virtual ~ QvWWWInline ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFString          name;		 
    QvSFVec3f		bboxSize;	 
    QvSFVec3f		bboxCenter;	 
};


# 4 "QvWWWInline.cpp" 2


QvFieldData	       * QvWWWInline ::fieldData;	QvBool		 QvWWWInline ::firstInstance = 1 ; ;

QvWWWInline::QvWWWInline()
{
    if (fieldData == 0 )	fieldData = new QvFieldData;	else	firstInstance = 0 ;	isBuiltIn = 0 ; ;
    isBuiltIn = 1 ;

    if (firstInstance)	fieldData->addField(this, "name" , &this-> name ); this-> name .setContainer(this); ;
    if (firstInstance)	fieldData->addField(this, "bboxSize" , &this-> bboxSize ); this-> bboxSize .setContainer(this); ;
    if (firstInstance)	fieldData->addField(this, "bboxCenter" , &this-> bboxCenter ); this-> bboxCenter .setContainer(this); ;

    name.value = "";
    bboxSize.value[0] = bboxSize.value[0] = bboxSize.value[0] = 0.0;
    bboxCenter.value[0] = bboxCenter.value[0] = bboxCenter.value[0] = 0.0;
}

QvWWWInline::~QvWWWInline()
{
}
