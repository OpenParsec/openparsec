# 1 "QvTraverse.cpp"
# 1 "QvElement.h" 1



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

 










 








# 4 "QvElement.h" 2


class QvNode;

 
 
 
 
 
 
 
 
 

class QvElement {

  public:

    enum NodeType {
	 
	Unknown,

	 
	OrthographicCamera,
	PerspectiveCamera,

	 
	DirectionalLight,
	PointLight,
	SpotLight,

	 
	NoOpTransform,		 
	MatrixTransform,
	Rotation,
	Scale,
	Transform,
	Translation,

	 
	NumNodeTypes,
    };

    static const char *nodeTypeNames[NumNodeTypes];	 

    int		depth;		 
    QvElement	*next;		 
    QvNode	*data;		 
    NodeType	type;		 

    QvElement();
    virtual ~QvElement();

     
    virtual void	print();
};


# 1 "QvTraverse.cpp" 2

# 1 "QvNodes.h" 1



# 1 "QvCone.h" 1



# 1 "QvSFBitMask.h" 1



# 1 "QvSFEnum.h" 1



# 1 "QvString.h" 1






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


# 4 "QvSFBitMask.h" 2


class QvSFBitMask : public QvSFEnum {
  public:
     
    public:	 QvSFBitMask ();	virtual ~ QvSFBitMask ();	virtual QvBool readValue(QvInput *in) ;
};


# 4 "QvCone.h" 2

# 1 "QvSFFloat.h" 1





class QvSFFloat : public QvSField {
  public:
    float value;
    public:	 QvSFFloat ();	virtual ~ QvSFFloat ();	virtual QvBool readValue(QvInput *in) ;
};


# 5 "QvCone.h" 2

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


























# 6 "QvCone.h" 2


class QvCone : public QvNode {

    public:	 QvCone :: QvCone ();	virtual ~ QvCone ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:

    enum Part {			 
	SIDES	= 0x01,			 
	BOTTOM	= 0x02,			 
	ALL	= 0x03		 	 
    };

     
    QvSFBitMask		parts;		 
    QvSFFloat		bottomRadius;	 
    QvSFFloat		height;		 
};


# 4 "QvNodes.h" 2

# 1 "QvCoordinate3.h" 1



# 1 "QvMFVec3f.h" 1





class QvMFVec3f : public QvMField {
  public:
    float *values;
    public:	 QvMFVec3f ();	virtual ~ QvMFVec3f ();	virtual QvBool	read1Value(QvInput *in, int index);	void		allocValues(int newNum) ;
};


# 4 "QvCoordinate3.h" 2



class QvCoordinate3 : public QvNode {

    public:	 QvCoordinate3 :: QvCoordinate3 ();	virtual ~ QvCoordinate3 ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvMFVec3f		point;		 
};


# 5 "QvNodes.h" 2

# 1 "QvCube.h" 1






class QvCube : public QvNode {

    public:	 QvCube :: QvCube ();	virtual ~ QvCube ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFFloat		width;		 
    QvSFFloat		height;		 
    QvSFFloat		depth;		 
};


# 6 "QvNodes.h" 2

# 1 "QvCylinder.h" 1







class QvCylinder : public QvNode {

    public:	 QvCylinder :: QvCylinder ();	virtual ~ QvCylinder ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:

    enum Part {			 
	SIDES	= 0x01,			 
	TOP	= 0x02,			 
	BOTTOM	= 0x04,			 
	ALL	= 0x07,			 
    };

     
    QvSFBitMask		parts;		 
    QvSFFloat		radius;		 
    QvSFFloat		height;		 
};


# 7 "QvNodes.h" 2

# 1 "QvDirectionalLight.h" 1



# 1 "QvSFBool.h" 1





class QvSFBool : public QvSField {
  public:
    QvBool value;
    public:	 QvSFBool ();	virtual ~ QvSFBool ();	virtual QvBool readValue(QvInput *in) ;
};


# 4 "QvDirectionalLight.h" 2

# 1 "QvSFColor.h" 1





class QvSFColor : public QvSField {
  public:
    float value[3];
    public:	 QvSFColor ();	virtual ~ QvSFColor ();	virtual QvBool readValue(QvInput *in) ;
};


# 5 "QvDirectionalLight.h" 2


# 1 "QvSFVec3f.h" 1





class QvSFVec3f : public QvSField {
  public:
    float value[3];
    public:	 QvSFVec3f ();	virtual ~ QvSFVec3f ();	virtual QvBool readValue(QvInput *in) ;
};


# 7 "QvDirectionalLight.h" 2



class QvDirectionalLight : public QvNode {

    public:	 QvDirectionalLight :: QvDirectionalLight ();	virtual ~ QvDirectionalLight ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFBool		on;		 
    QvSFFloat		intensity;	 
    QvSFColor		color;		 
    QvSFVec3f		direction;	 
};


# 8 "QvNodes.h" 2

# 1 "QvGroup.h" 1



class QvChildList;


class QvGroup : public QvNode {

    public:	 QvGroup :: QvGroup ();	virtual ~ QvGroup ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    QvNode *		getChild(int index) const;
    int			getNumChildren() const;
    virtual QvChildList *getChildren() const;
    virtual QvBool	readInstance(QvInput *in);
    virtual QvBool	readChildren(QvInput *in);
};


# 9 "QvNodes.h" 2

# 1 "QvIndexedFaceSet.h" 1



# 1 "QvMFLong.h" 1





class QvMFLong : public QvMField {
  public:
    long *values;
    public:	 QvMFLong ();	virtual ~ QvMFLong ();	virtual QvBool	read1Value(QvInput *in, int index);	void		allocValues(int newNum) ;
};


# 4 "QvIndexedFaceSet.h" 2





class QvIndexedFaceSet : public QvNode {

    public:	 QvIndexedFaceSet :: QvIndexedFaceSet ();	virtual ~ QvIndexedFaceSet ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvMFLong		coordIndex;		 
    QvMFLong		materialIndex;		 
    QvMFLong		normalIndex;		 
    QvMFLong		textureCoordIndex;	 
};


# 10 "QvNodes.h" 2

# 1 "QvIndexedLineSet.h" 1








class QvIndexedLineSet : public QvNode {

    public:	 QvIndexedLineSet :: QvIndexedLineSet ();	virtual ~ QvIndexedLineSet ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvMFLong		coordIndex;		 
    QvMFLong		materialIndex;		 
    QvMFLong		normalIndex;		 
    QvMFLong		textureCoordIndex;	 
};


# 11 "QvNodes.h" 2

# 1 "QvInfo.h" 1



# 1 "QvSFString.h" 1





class QvSFString : public QvSField {
  public:
    QvString value;
    public:	 QvSFString ();	virtual ~ QvSFString ();	virtual QvBool readValue(QvInput *in) ;
};


# 4 "QvInfo.h" 2



class QvInfo : public QvNode {

    public:	 QvInfo :: QvInfo ();	virtual ~ QvInfo ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFString		string;		 
};


# 12 "QvNodes.h" 2

# 1 "QvLevelOfDetail.h" 1



# 1 "QvMFFloat.h" 1





class QvMFFloat : public QvMField {
  public:
    float *values;
    public:	 QvMFFloat ();	virtual ~ QvMFFloat ();	virtual QvBool	read1Value(QvInput *in, int index);	void		allocValues(int newNum) ;
};


# 4 "QvLevelOfDetail.h" 2



class QvLevelOfDetail : public QvGroup {

    public:	 QvLevelOfDetail :: QvLevelOfDetail ();	virtual ~ QvLevelOfDetail ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvMFFloat		screenArea;	 
};


# 13 "QvNodes.h" 2

# 1 "QvMaterial.h" 1



# 1 "QvMFColor.h" 1





class QvMFColor : public QvMField {
  public:
    float *values;			 
    public:	 QvMFColor ();	virtual ~ QvMFColor ();	virtual QvBool	read1Value(QvInput *in, int index);	void		allocValues(int newNum) ;
};


# 4 "QvMaterial.h" 2




class QvMaterial : public QvNode {

    public:	 QvMaterial :: QvMaterial ();	virtual ~ QvMaterial ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvMFColor		ambientColor;	 
    QvMFColor		diffuseColor;	 
    QvMFColor		specularColor;	 
    QvMFColor		emissiveColor;	 
    QvMFFloat		shininess;	 
    QvMFFloat		transparency;	 
};


# 14 "QvNodes.h" 2

# 1 "QvMaterialBinding.h" 1






class QvMaterialBinding : public QvNode {

    public:	 QvMaterialBinding :: QvMaterialBinding ();	virtual ~ QvMaterialBinding ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    enum Binding {
	DEFAULT,
	NONE,
	OVERALL,
	PER_PART,
	PER_PART_INDEXED,
	PER_FACE,
	PER_FACE_INDEXED,
	PER_VERTEX,
	PER_VERTEX_INDEXED,
    };

     
    QvSFEnum		value;			
};


# 15 "QvNodes.h" 2

# 1 "QvMatrixTransform.h" 1



# 1 "QvSFMatrix.h" 1





class QvSFMatrix : public QvSField {
  public:
    float value[4][4];
    public:	 QvSFMatrix ();	virtual ~ QvSFMatrix ();	virtual QvBool readValue(QvInput *in) ;
};


# 4 "QvMatrixTransform.h" 2



class QvMatrixTransform : public QvNode {

    public:	 QvMatrixTransform :: QvMatrixTransform ();	virtual ~ QvMatrixTransform ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFMatrix		matrix;		 
};


# 16 "QvNodes.h" 2

# 1 "QvNormal.h" 1






class QvNormal : public QvNode {

    public:	 QvNormal :: QvNormal ();	virtual ~ QvNormal ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvMFVec3f		vector;		 
};


# 17 "QvNodes.h" 2

# 1 "QvNormalBinding.h" 1






class QvNormalBinding : public QvNode {

    public:	 QvNormalBinding :: QvNormalBinding ();	virtual ~ QvNormalBinding ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    enum Binding {
	DEFAULT,
	NONE,
	OVERALL,
	PER_PART,
	PER_PART_INDEXED,
	PER_FACE,
	PER_FACE_INDEXED,
	PER_VERTEX,
	PER_VERTEX_INDEXED,
    };

     
    QvSFEnum		value;		 
};


# 18 "QvNodes.h" 2

# 1 "QvOrthographicCamera.h" 1




# 1 "QvSFRotation.h" 1





class QvSFRotation : public QvSField {
  public:
    float axis[3];
    float angle;
    public:	 QvSFRotation ();	virtual ~ QvSFRotation ();	virtual QvBool readValue(QvInput *in) ;
};


# 5 "QvOrthographicCamera.h" 2




class QvOrthographicCamera : public QvNode {

    public:	 QvOrthographicCamera :: QvOrthographicCamera ();	virtual ~ QvOrthographicCamera ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    QvSFVec3f		position;	 
    QvSFRotation	orientation;	 
					 
    QvSFFloat	    	focalDistance;	 
					 
    QvSFFloat		height;		 
};


# 19 "QvNodes.h" 2

# 1 "QvPerspectiveCamera.h" 1








class QvPerspectiveCamera : public QvNode {

    public:	 QvPerspectiveCamera :: QvPerspectiveCamera ();	virtual ~ QvPerspectiveCamera ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    QvSFVec3f		position;	 
    QvSFRotation	orientation;	 
					 
    QvSFFloat	    	focalDistance;	 
					 
    QvSFFloat		heightAngle;	 
					 
};


# 20 "QvNodes.h" 2

# 1 "QvPointLight.h" 1









class QvPointLight : public QvNode {

    public:	 QvPointLight :: QvPointLight ();	virtual ~ QvPointLight ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFBool		on;		 
    QvSFFloat		intensity;	 
    QvSFColor		color;		 
    QvSFVec3f		location;	 
};


# 21 "QvNodes.h" 2

# 1 "QvPointSet.h" 1



# 1 "QvSFLong.h" 1





class QvSFLong : public QvSField {
  public:
    long value;
    public:	 QvSFLong ();	virtual ~ QvSFLong ();	virtual QvBool readValue(QvInput *in) ;
};


# 4 "QvPointSet.h" 2





class QvPointSet : public QvNode {

    public:	 QvPointSet :: QvPointSet ();	virtual ~ QvPointSet ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFLong		startIndex;	 
    QvSFLong		numPoints;	 
};


# 22 "QvNodes.h" 2

# 1 "QvRotation.h" 1






class QvRotation : public QvNode {

    public:	 QvRotation :: QvRotation ();	virtual ~ QvRotation ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFRotation	rotation;	 
};


# 23 "QvNodes.h" 2

# 1 "QvScale.h" 1






class QvScale : public QvNode {

    public:	 QvScale :: QvScale ();	virtual ~ QvScale ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFVec3f		scaleFactor;	 
};


# 24 "QvNodes.h" 2

# 1 "QvSeparator.h" 1








class QvSeparator : public QvGroup {

    public:	 QvSeparator :: QvSeparator ();	virtual ~ QvSeparator ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    enum CullEnabled {		 
	OFF,			 
	ON,			 
	AUTO,			 
    };

     
    QvSFEnum renderCulling;
};


# 25 "QvNodes.h" 2

# 1 "QvShapeHints.h" 1







class QvShapeHints : public QvNode {

    public:	 QvShapeHints :: QvShapeHints ();	virtual ~ QvShapeHints ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    enum VertexOrdering {
	UNKNOWN_ORDERING,
	CLOCKWISE,
	COUNTERCLOCKWISE,
    };

    enum ShapeType {
	UNKNOWN_SHAPE_TYPE,
	SOLID,
    };

    enum FaceType {
	UNKNOWN_FACE_TYPE,
	CONVEX,
    };

     
    QvSFEnum		vertexOrdering;	 
    QvSFEnum		shapeType;	 
    QvSFEnum		faceType;	 
    QvSFFloat		creaseAngle;	 
};


# 26 "QvNodes.h" 2

# 1 "QvSphere.h" 1






class QvSphere : public QvNode {

    public:	 QvSphere :: QvSphere ();	virtual ~ QvSphere ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFFloat		radius;		 
};



# 27 "QvNodes.h" 2

# 1 "QvSpotLight.h" 1









class QvSpotLight : public QvNode {

    public:	 QvSpotLight :: QvSpotLight ();	virtual ~ QvSpotLight ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFBool	on;		 
    QvSFFloat	intensity;	 
    QvSFColor	color;		 
    QvSFVec3f	location;	 
    QvSFVec3f	direction;	 
    QvSFFloat	dropOffRate;	 
				 
				 
    QvSFFloat	cutOffAngle;	 
				 
				 
};


# 28 "QvNodes.h" 2

# 1 "QvSwitch.h" 1









class QvSwitch : public QvGroup {

    public:	 QvSwitch :: QvSwitch ();	virtual ~ QvSwitch ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFLong		whichChild;	 
};


# 29 "QvNodes.h" 2

# 1 "QvTexture2.h" 1




# 1 "QvSFImage.h" 1





class QvSFImage : public QvSField {
  public:
    short		size[2];	 
    int			numComponents;	 
    unsigned char *	bytes;		 
    public:	 QvSFImage ();	virtual ~ QvSFImage ();	virtual QvBool readValue(QvInput *in) ;
};


# 5 "QvTexture2.h" 2




class QvTexture2 : public QvNode {

    public:	 QvTexture2 :: QvTexture2 ();	virtual ~ QvTexture2 ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
    enum Wrap {				 
	REPEAT,
	CLAMP,
    };

     
    QvSFString		filename;	 
    QvSFImage		image;		 
    QvSFEnum		wrapS;
    QvSFEnum		wrapT;

    virtual QvBool	readInstance(QvInput *in);
    QvBool		readImage();
};


# 30 "QvNodes.h" 2

# 1 "QvTexture2Transform.h" 1




# 1 "QvSFVec2f.h" 1





class QvSFVec2f : public QvSField {
  public:
    float value[2];
    public:	 QvSFVec2f ();	virtual ~ QvSFVec2f ();	virtual QvBool readValue(QvInput *in) ;
};


# 5 "QvTexture2Transform.h" 2



class QvTexture2Transform : public QvNode {

    public:	 QvTexture2Transform :: QvTexture2Transform ();	virtual ~ QvTexture2Transform ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFVec2f		translation;	 
    QvSFFloat		rotation;	 
    QvSFVec2f		scaleFactor;	 
    QvSFVec2f		center;	         
};


# 31 "QvNodes.h" 2

# 1 "QvTextureCoordinate2.h" 1



# 1 "QvMFVec2f.h" 1





class QvMFVec2f : public QvMField {
  public:
    float *values;
    public:	 QvMFVec2f ();	virtual ~ QvMFVec2f ();	virtual QvBool	read1Value(QvInput *in, int index);	void		allocValues(int newNum) ;
};


# 4 "QvTextureCoordinate2.h" 2



class QvTextureCoordinate2 : public QvNode {

    public:	 QvTextureCoordinate2 :: QvTextureCoordinate2 ();	virtual ~ QvTextureCoordinate2 ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvMFVec2f		point;		 
};


# 32 "QvNodes.h" 2

# 1 "QvTransform.h" 1







class QvTransform : public QvNode {

    public:	 QvTransform :: QvTransform ();	virtual ~ QvTransform ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFVec3f		translation;	  
    QvSFRotation	rotation;	  
    QvSFVec3f		scaleFactor;	  
    QvSFRotation	scaleOrientation; 
    QvSFVec3f		center;	          
};


# 33 "QvNodes.h" 2

# 1 "QvTransformSeparator.h" 1





class QvTransformSeparator : public QvGroup {

    public:	 QvTransformSeparator :: QvTransformSeparator ();	virtual ~ QvTransformSeparator ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

     
};


# 34 "QvNodes.h" 2

# 1 "QvTranslation.h" 1






class QvTranslation : public QvNode {

    public:	 QvTranslation :: QvTranslation ();	virtual ~ QvTranslation ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFVec3f		translation;	 
};


# 35 "QvNodes.h" 2

# 1 "QvWWWAnchor.h" 1







class QvWWWAnchor : public QvGroup {

    public:	 QvWWWAnchor :: QvWWWAnchor ();	virtual ~ QvWWWAnchor ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:

    enum Map {			 
	NONE,				 
	POINT,				 
    };

     
    QvSFString		name;		 
    QvSFEnum		map;		 
};


# 36 "QvNodes.h" 2

# 1 "QvWWWInline.h" 1







class QvWWWInline : public QvGroup {

    public:	 QvWWWInline :: QvWWWInline ();	virtual ~ QvWWWInline ();	virtual void	traverse(QvState *state);	private:	static QvBool	firstInstance;	static QvFieldData	*fieldData;	virtual QvFieldData *getFieldData() { return fieldData; } ;

  public:
     
    QvSFString		name;		 
    QvSFVec3f		bboxSize;	 
    QvSFVec3f		bboxCenter;	 
};


# 37 "QvNodes.h" 2



# 2 "QvTraverse.cpp" 2

# 1 "QvState.h" 1





 
# 1 "../BspLib/VrmlFile.h" 1
 
 
 
 
 
 




 
# 1 "../BspLib/BspLibDefs.h" 1
 
 
 
 
 
 





 
# 1 "/usr/include/ctype.h" 1 3 4
 




















 













































# 1 "/usr/include/runetype.h" 1 3 4
 




















 






















































 


typedef struct {
	rune_t		min;		 
	rune_t		max;		 
	rune_t		map;		 
	unsigned long	*types;		 
} _RuneEntry;

typedef struct {
	int		nranges;	 
	_RuneEntry	*ranges;	 
} _RuneRange;

typedef struct {
	char		magic[8];	 
	char		encoding[32];	 

	rune_t		(*sgetrune)
	     (const char *, unsigned int, char const **)  ;
	int		(*sputrune)
	     (rune_t, char *, unsigned int, char **)  ;
	rune_t		invalid_rune;

	unsigned long	runetype[(1 <<8 ) ];
	rune_t		maplower[(1 <<8 ) ];
	rune_t		mapupper[(1 <<8 ) ];

	 




	_RuneRange	runetype_ext;
	_RuneRange	maplower_ext;
	_RuneRange	mapupper_ext;

	void		*variable;	 
	int		variable_len;	 
} _RuneLocale;



extern _RuneLocale _DefaultRuneLocale;
extern _RuneLocale *_CurrentRuneLocale;


# 68 "/usr/include/ctype.h" 2 3 4










































 
extern "C" { 
unsigned long	___runetype  (__wchar_t )  ;
__wchar_t 	___tolower  (__wchar_t )  ;
__wchar_t 	___toupper  (__wchar_t )  ;
} 

 









static inline  int
__istype(__wchar_t  c, unsigned long f)
{
	return((((c & (~((1 <<8 )  - 1)) ) ? ___runetype(c) :
	    _CurrentRuneLocale->runetype[c]) & f) ? 1 : 0);
}

static inline  int
__isctype(__wchar_t  c, unsigned long f)
{
	return((((c & (~((1 <<8 )  - 1)) ) ? 0 :
	    _DefaultRuneLocale.runetype[c]) & f) ? 1 : 0);
}

 

static inline  __wchar_t 
toupper(__wchar_t  c)
{
	return((c & (~((1 <<8 )  - 1)) ) ?
	    ___toupper(c) : _CurrentRuneLocale->mapupper[c]);
}

static inline  __wchar_t 
tolower(__wchar_t  c)
{
	return((c & (~((1 <<8 )  - 1)) ) ?
	    ___tolower(c) : _CurrentRuneLocale->maplower[c]);
}


# 166 "/usr/include/ctype.h" 3 4



# 13 "../BspLib/BspLibDefs.h" 2

# 1 "/usr/include/errno.h" 1 3 4
 




















# 1 "/usr/include/sys/errno.h" 1 3 4
 




















 
 












































extern "C" { 
extern int * __error  (void)  ;

} 


 














					 


























 



 






 
















 

















 






 





 


















 






 














# 22 "/usr/include/errno.h" 2 3 4


# 14 "../BspLib/BspLibDefs.h" 2

# 1 "/usr/include/limits.h" 1 3 4
 




















 

 





































# 1 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 1 3
# 10 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 3

# 108 "/usr/include/gcc/darwin/2.95.2/g++/../machine/limits.h" 3

# 62 "/usr/include/limits.h" 2 3 4















































# 15 "../BspLib/BspLibDefs.h" 2

# 1 "/usr/include/math.h" 1 3 4
 




















 
 













 




 


















extern int signgam;


enum fdversion {fdlibm_ieee = -1, fdlibm_svid, fdlibm_xopen, fdlibm_posix};




 





 
extern  enum fdversion   _fdlib_version ;






# 91 "/usr/include/math.h" 3 4




 


















extern "C" { 
 


extern   double acos  (double)  ;
extern   double asin  (double)  ;
extern   double atan  (double)  ;
extern   double atan2  (double, double)  ;
extern   double cos  (double)  ;
extern   double sin  (double)  ;
extern   double tan  (double)  ;

extern   double cosh  (double)  ;
extern   double sinh  (double)  ;
extern   double tanh  (double)  ;

extern   double exp  (double)  ;
extern double frexp  (double, int *)  ;
extern   double ldexp  (double, int)  ;
extern   double log  (double)  ;
extern   double log10  (double)  ;
extern double modf  (double, double *)  ;

extern   double pow  (double, double)  ;
extern   double sqrt  (double)  ;

extern   double ceil  (double)  ;
extern   double fabs  (double)  ;
extern   double floor  (double)  ;
extern   double fmod  (double, double)  ;


extern   double erf  (double)  ;
extern   double erfc  (double)  ;
extern double gamma  (double)  ;
extern   double hypot  (double, double)  ;
extern   int isinf  (double)  ;
extern   int isnan  (double)  ;
extern    int finite  (double)  ;
extern   double j0  (double)  ;
extern   double j1  (double)  ;
extern   double jn  (int, double)  ;
extern double lgamma  (double)  ;
extern   double y0  (double)  ;
extern   double y1  (double)  ;
extern   double yn  (int, double)  ;


extern   double acosh  (double)  ;
extern   double asinh  (double)  ;
extern   double atanh  (double)  ;
extern   double cbrt  (double)  ;
extern   double logb  (double)  ;
extern   double nextafter  (double, double)  ;
extern   double remainder  (double, double)  ;
extern   double scalb  (double, int)  ;





 


extern   double significand  (double)  ;

 


extern   double copysign  (double, double)  ;
extern   int ilogb  (double)  ;
extern   double rint  (double)  ;
extern   double scalbn  (double, int)  ;

 


extern double cabs();
extern   double drem  (double, double)  ;
extern   double expm1  (double)  ;
extern   double log1p  (double)  ;

 









} 


# 16 "../BspLib/BspLibDefs.h" 2






 

















 

















 





 



 
typedef unsigned char			byte;
typedef unsigned short			word;
typedef unsigned long			dword;

 
typedef long					fixed_t;
typedef float					float_t;
typedef double					hprec_t;

 










 
 
namespace BspLib  { 


 
struct ColorRGB {
	byte R;
	byte G;
	byte B;
};

 
struct ColorRGBA {
	byte R;
	byte G;
	byte B;
	byte A;
};

 
struct ColorRGBf {
	float R;
	float G;
	float B;
};

 
struct ColorRGBAf {
	float R;
	float G;
	float B;
	float A;
};


 
 








class EpsAreas {

public:
	static double eps_scalarproduct;		 
	static double eps_vanishcomponent;		 
	static double eps_vanishdenominator;	 
	static double eps_planethickness;		 
	static double eps_pointonline;			 
	static double eps_pointonlineseg;		 
	static double eps_vertexmergearea;		 
};


} 




# 12 "../BspLib/VrmlFile.h" 2

# 1 "../BspLib/BoundingBox.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/BspObject.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/SystemIO.h" 1
 
 
 
 
 
 




 


 





namespace BspLib  { 


 
 
class SystemIO {

public:

	 
	enum {
		SYSTEM_IO_OK		= 0x0000,
		SYSTEM_IO_ERROR		= 0x0001,
		END_OF_FILE			= 0x0002,
		FILE_NOT_FOUND		= 0x0003,
		FILE_READ_ERROR		= 0x0004,
		FILE_WRITE_ERROR	= 0x0005,
		FILE_CLOSED			= 0x0006,
		FILE_ALREADY_OPEN	= 0x0007,
	};

	 
	enum {
		CRITERR_ALLOW_RET	= 0x0001,
	};

	 
	enum {
		CHECK_ERRORS		= 0x0001,
	};

public:
	static void InfoMessage( char *message );
	static void ErrorMessage( char *message );
	static void HandleCriticalError( int flags = 0 );
	static void SetProgramName( const char *name );
	static void SetInfoMessageCallback( void (*callb)(char*) );
	static void SetErrorMessageCallback( void (*callb)(char*) );
	static void SetCriticalErrorCallback( int (*callb)(int) );





public:
	static const char	version_str[];

private:
	static const char	*program_name;
	static void			(*infomessage_callback)(char*);
	static void			(*errormessage_callback)(char*);
	static int			(*criticalerror_callback)(int);






protected:


 
class FilePtr {

public:
	FilePtr( const char *fname, const char *mode ) { fp = fopen( fname, mode ); }
	~FilePtr() { if ( fp ) fclose( fp ); }

	operator FILE*() { return fp; }

protected:
	FILE *fp;

};  


 
class FileAccess : public FilePtr {

public:
	FileAccess( const char *fname, const char *mode, int flags = CHECK_ERRORS );
	~FileAccess();

	int ReOpen( const char *fname, const char *mode, int flags = CHECK_ERRORS );
	int	Close();

	int Read( void *buffer, size_t size, size_t count, int flags = 0 );
	int Write( void *buffer, size_t size, size_t count, int flags = 0 );

	char *ReadLine( char *line, int maxlen, int flags = 0 );
	char *WriteLine( char *line, int flags = 0 );

	int Status() { return status; }

	char *getFileName() { return filename; }

private:
	void IOError( int errorcode );

private:
	char *filename;
	int  status;

};  


 
class StrScratch {

public:
	StrScratch() { line = new char[ 1024 ]; }
	~StrScratch() { delete line; }

	operator char*() { return line; }

private:
	char *line;

};  


};  


 
 
class String {

public:
	String() { data = 0 ; }
	String( const char *src );
	~String() { delete data; }

	String( const String& copyobj );
	String& operator =( const String& copyobj );

	operator char*() { return data; }

	int IsNULL() const { return ( data == 0  ); }
	int IsEmpty() const { return data ? ( *data == '\0' ) : 1 ; }

	int getLength() const { return data ? strlen( data ) : 0; }

private:
	char *data;
};


} 




# 13 "../BspLib/BspObject.h" 2

# 1 "../BspLib/Chunk.h" 1
 
 
 
 
 
 




 




 
 

 


 
 


namespace BspLib  { 


 
template<class T> class Chunk;


 
 
template<class T>
class ChunkRep : public virtual SystemIO {

	friend class Chunk<T>;

	 
	enum {
		E_INVALIDINDX
	};

	void		Error( int err ) const;

private:
	ChunkRep( int chunksize = 0 );
	~ChunkRep();

	int			AddElement( const T& element );
	T&			FetchElement( int index );

	int			getNumElements() const;

private:
	int			ref_count;
	int			numelements;
	int			maxnumelements;
	T*			elements;
	ChunkRep*	next;

	static const int CHUNK_SIZE;
};


 
 
template<class T>
ChunkRep<T>::ChunkRep( int chunksize ) : ref_count( 0 )
{
	int challocsize	= chunksize > 0 ? chunksize : CHUNK_SIZE;
	elements		= new T[ challocsize ];
	maxnumelements	= challocsize;
	numelements		= 0;
	next			= 0 ;
}


 
 
template<class T>
ChunkRep<T>::~ChunkRep()
{
	delete[] elements;
	delete next;
}


 
 
template<class T>
void ChunkRep<T>::Error( int err ) const
{
	{
	StrScratch message;
	sprintf( message, "***ERROR*** in object of a class of template Chunk" );

	switch ( err ) {

	case E_INVALIDINDX:
		sprintf( message + strlen( message ), ": [Invalid index]" );
		break;
	}

	ErrorMessage( message );
	}

	HandleCriticalError();
}


 
 
template<class T>
T& ChunkRep<T>::FetchElement( int index )
{

# 133 "../BspLib/Chunk.h"


	if ( index >= numelements )
		Error( E_INVALIDINDX );

	 
	return elements[ index ];



}


 
 
template<class T>
int ChunkRep<T>::AddElement( const T& element )
{

# 185 "../BspLib/Chunk.h"


	 
	if ( numelements == maxnumelements ) {


		maxnumelements *= 2;



		T *temp = new T[ maxnumelements ];




		for ( int i = 0; i < numelements; i++ )
			temp[ i ] = elements[ i ];

		delete[] elements;
		elements = temp;
	}

	elements[ numelements ] = element;
	return numelements++;



}


 
 
template<class T>
int ChunkRep<T>::getNumElements() const
{
	int num = 0;
	for ( const ChunkRep *vlist = this; vlist; vlist = vlist->next )
		num += vlist->numelements;
	return num;
}


 
 
template<class T>
class Chunk {

public:
	Chunk( int chunksize = 0 ) { rep = new ChunkRep<T>( chunksize ); rep->ref_count = 1; }
	~Chunk() { if ( --rep->ref_count == 0 ) delete rep; }

	Chunk( const Chunk& copyobj );
	Chunk& operator =( const Chunk& copyobj );

	T&				operator []( int index ) { return rep->FetchElement( index ); }
	T&				FetchElement( int index ) { return rep->FetchElement( index ); }
	int				AddElement( const T& element ) { return rep->AddElement( element ); }

	int				getNumElements() const { return rep->getNumElements(); }

private:
	ChunkRep<T>*	rep;
};


 
 
template<class T>
Chunk<T>::Chunk( const Chunk<T>& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}


 
 
template<class T>
Chunk<T>& Chunk<T>::operator =( const Chunk<T>& copyobj )
{
	if ( &copyobj != this ) {
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;
		rep->ref_count++;
	}
	return *this;
}


} 




# 14 "../BspLib/BspObject.h" 2

# 1 "../BspLib/Face.h" 1
 
 
 
 
 
 




 


# 1 "../BspLib/Material.h" 1
 
 
 
 
 
 




 




namespace BspLib  { 


 
 
class Material {

public:
	Material() { }
	~Material() { }

	ColorRGBA	getAmbientColor()	{ return ambientcolor; }
	ColorRGBA	getDiffuseColor()	{ return diffusecolor; }
	ColorRGBA	getSpecularColor()	{ return specularcolor; }
	ColorRGBA	getEmissiveColor()	{ return emissivecolor; }

	void		setAmbientColor( ColorRGBA col )  { ambientcolor = col; ambientcolor.A = 255; }
	void		setDiffuseColor( ColorRGBA col )  { diffusecolor = col; diffusecolor.A = 255; }
	void		setSpecularColor( ColorRGBA col ) { specularcolor = col; specularcolor.A = 255; }
	void		setEmissiveColor( ColorRGBA col ) { emissivecolor = col; emissivecolor.A = 255; }

	float		getShininess()		{ return shininess; }
	float		getTransparency()	{ return transparency; }

	void		setShininess( float s )		{ shininess = s; }
	void		setTransparency( float t )	{ transparency = t; }

private:
	ColorRGBA	ambientcolor;	 
	ColorRGBA	diffusecolor;
	ColorRGBA	specularcolor;
	ColorRGBA	emissivecolor;
	float		shininess;
	float		transparency;
};


} 




# 14 "../BspLib/Face.h" 2

# 1 "../BspLib/Plane.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/Vertex.h" 1
 
 
 
 
 
 




 



namespace BspLib  { 


 
 
class Vertex3 {

	friend class Vector3;
	friend class LineSeg3;

	friend int operator ==( const Vertex3 v1, const Vertex3 v2 );
	friend int operator !=( const Vertex3 v1, const Vertex3 v2 );

	friend Vector3 operator -( const Vertex3 v1, const Vertex3 v2 );

	friend Vertex3 operator +( const Vertex3 v1, const Vector3 v2 );
	friend Vertex3 operator +( const Vector3 v1, const Vertex3 v2 );
	friend Vertex3 operator -( const Vertex3 v1, const Vector3 v2 );

public:
	Vertex3( hprec_t x = 0, hprec_t y = 0, hprec_t z = 0, hprec_t w = 1.0 );
	~Vertex3() { }

	Vertex3& operator +=( const Vector3 v );
	Vertex3& operator -=( const Vector3 v );

	int  IsInVicinity( const Vertex3& vertex ) const;
	void ChangeAxes( const char *xchangecmd, const Vertex3& ads );

	hprec_t getX() const { return X; }
	hprec_t getY() const { return Y; }
	hprec_t getZ() const { return Z; }
 	hprec_t getW() const { return W; }

	void setX( hprec_t x ) { X = x; }
	void setY( hprec_t y ) { Y = y; }
	void setZ( hprec_t z ) { Z = z; }
	void setW( hprec_t w ) { W = w; }

private:
	void ChangeAxis( const char xchar, const hprec_t src );

protected:
	hprec_t X;
	hprec_t Y;
	hprec_t Z;
	hprec_t W;
};


 
 
class Vector3 : public Vertex3 {

	friend class LineSeg3;

	friend Vector3 operator -( const Vertex3 v1, const Vertex3 v2 );

	friend Vertex3 operator +( const Vertex3 v1, const Vector3 v2 );
	friend Vertex3 operator +( const Vector3 v1, const Vertex3 v2 );
	friend Vertex3 operator -( const Vertex3 v1, const Vector3 v2 );

	friend Vector3 operator +( const Vector3 v1, const Vector3 v2 );
	friend Vector3 operator -( const Vector3 v1, const Vector3 v2 );

	friend Vector3 operator *( const Vector3 v1, double t );
	friend Vector3 operator *( double t, const Vector3 v1 );

public:
	Vector3( hprec_t x = 0, hprec_t y = 0, hprec_t z = 0, hprec_t w = 1.0 )
		: Vertex3( x, y, z, w ) { }
	 
	Vector3( const Vertex3& v1, const Vertex3& v2 )
		: Vertex3( v2.X - v1.X, v2.Y - v1.Y, v2.Z - v1.Z ) { }
	 
	Vector3( const Vector3& v1, const Vector3& v2 );
	 
	Vector3( const Vertex3& vtx ) { *(Vertex3*)this = vtx; }
	~Vector3() { }

	Vector3& operator +=( const Vector3 v );
	Vector3& operator -=( const Vector3 v );

	Vector3& operator *=( double t );

	int			Normalize();
	int			Homogenize();
	int			IsNullVector() const;
	hprec_t		VecLength() const;
	hprec_t		DotProduct( const Vector3& vect ) const;
	void		CrossProduct( const Vector3& vect1, const Vector3& vect2 );
	void		CreateDirVec( const Vertex3& vertex1, const Vertex3& vertex2 );
};


 
 
class Vertex2 {

	friend class Vector2;
	friend class LineSeg2;

	friend int operator ==( const Vertex2 v1, const Vertex2 v2 );
	friend int operator !=( const Vertex2 v1, const Vertex2 v2 );

	friend Vector2 operator -( const Vertex2 v1, const Vertex2 v2 );

	friend Vertex2 operator +( const Vertex2 v1, const Vector2 v2 );
	friend Vertex2 operator +( const Vector2 v1, const Vertex2 v2 );
	friend Vertex2 operator -( const Vertex2 v1, const Vector2 v2 );

public:
	Vertex2( hprec_t x = 0, hprec_t y = 0, hprec_t w = 1.0 ) { X = x; Y = y; W = w; }
	 
	Vertex2( const Vector2& vect );
	~Vertex2() { }

	Vertex2& operator +=( const Vector2 v );
	Vertex2& operator -=( const Vector2 v );

	int  IsInVicinity( const Vertex2& vertex ) const;
	void InitFromVertex3( const Vertex3& vertex );

	hprec_t getX() const { return X; }
	hprec_t getY() const { return Y; }
 	hprec_t getW() const { return W; }

	void setX( hprec_t x ) { X = x; }
	void setY( hprec_t y ) { Y = y; }
	void setW( hprec_t w ) { W = w; }

protected:
	hprec_t X;
	hprec_t Y;
	hprec_t W;
};


 
 
class Vector2 : public Vertex2 {

	friend class LineSeg2;

	friend Vector2 operator -( const Vertex2 v1, const Vertex2 v2 );

	friend Vertex2 operator +( const Vertex2 v1, const Vector2 v2 );
	friend Vertex2 operator +( const Vector2 v1, const Vertex2 v2 );
	friend Vertex2 operator -( const Vertex2 v1, const Vector2 v2 );

	friend Vector2 operator +( const Vector2 v1, const Vector2 v2 );
	friend Vector2 operator -( const Vector2 v1, const Vector2 v2 );

	friend Vector2 operator *( const Vector2 v1, double t );
	friend Vector2 operator *( double t, const Vector2 v1 );

public:
	Vector2( hprec_t x = 0, hprec_t y = 0, hprec_t w = 1.0 )
		: Vertex2( x, y, w ) { }
	 
	Vector2( const Vertex2& v1, const Vertex2& v2 )
		: Vertex2( v2.X - v1.X, v2.Y - v1.Y ) { }
	 
	Vector2( const Vertex2& vtx ) { *(Vertex2*)this = vtx; }
	~Vector2() { }

	Vector2& operator +=( const Vector2 v );
	Vector2& operator -=( const Vector2 v );

	Vector2& operator *=( double t );

	int			Normalize();
	int			Homogenize();
	int			IsNullVector() const;
	hprec_t		VecLength() const;
	hprec_t		DotProduct( const Vector2& vect ) const;
	void		CreateDirVec( const Vertex2& vertex, const Vertex2& dirvec );
};


} 


 
# 1 "../BspLib/Vector.h" 1
 
 
 
 
 
 




 
# 1 "../BspLib/Vector3.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/Vertex3.h" 1
 
 
 
 
 
 




 



namespace BspLib  { 


 
inline Vertex3::Vertex3( hprec_t x, hprec_t y, hprec_t z, hprec_t w )
{
	X = x;
	Y = y;
	Z = z;
	W = w;
}

 
inline int operator ==( const Vertex3 v1, const Vertex3 v2 )
{
	return ( ( v1.X == v2.X ) && ( v1.Y == v2.Y ) && ( v1.Z == v2.Z ) && ( v1.W == v2.W ) );
}

 
inline int operator !=( const Vertex3 v1, const Vertex3 v2 )
{
	return ( ( v1.X != v2.X ) || ( v1.Y != v2.Y ) || ( v1.Z != v2.Z ) || ( v1.W != v2.W ) );
}

 
inline Vertex3& Vertex3::operator +=( const Vector3 v )
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;

	return *this;
}

 
inline Vertex3& Vertex3::operator -=( const Vector3 v )
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;

	return *this;
}

 
inline int Vertex3::IsInVicinity( const Vertex3& vertex ) const
{
	return ( ( fabs( X - vertex.X ) < EpsAreas::eps_vertexmergearea  ) &&
			 ( fabs( Y - vertex.Y ) < EpsAreas::eps_vertexmergearea  ) &&
			 ( fabs( Z - vertex.Z ) < EpsAreas::eps_vertexmergearea  ) );
}


} 




# 13 "../BspLib/Vector3.h" 2



namespace BspLib  { 


 
inline Vector3 operator *( const Vector3 v1, double t )
{
	return Vector3( v1.X * t, v1.Y * t, v1.Z * t, 1.0 );
}

 
inline Vector3 operator *( double t, const Vector3 v1 )
{
	return Vector3( v1.X * t, v1.Y * t, v1.Z * t, 1.0 );
}

 
inline Vector3::Vector3( const Vector3& v1, const Vector3& v2 )
{
	CrossProduct( v1, v2 );
}

 
inline Vector3& Vector3::operator +=( const Vector3 v )
{
	X += v.X;
	Y += v.Y;
	Z += v.Z;
	return *this;
}

 
inline Vector3& Vector3::operator -=( const Vector3 v )
{
	X -= v.X;
	Y -= v.Y;
	Z -= v.Z;
	return *this;
}

 
inline Vector3& Vector3::operator *=( double t )
{
	X *= t;
	Y *= t;
	Z *= t;
	return *this;
}

 
inline int Vector3::Normalize()
{
	if ( IsNullVector() )
		return 0 ;

	double oonorm = 1 / VecLength();
	X = X * oonorm;
	Y = Y * oonorm;
	Z = Z * oonorm;
	W = 1.0;
	return 1 ;
}

 
inline int Vector3::Homogenize()
{
	if ( fabs( W ) < EpsAreas::eps_vanishdenominator  )
		return 0 ;

	double oow = 1 / W;
	X = X * oow;
	Y = Y * oow;
	Z = Z * oow;
	W = 1.0;
	return 1 ;
}

 
inline int Vector3::IsNullVector() const
{
	 
	return ( ( fabs( X ) < EpsAreas::eps_vanishcomponent  ) && ( fabs( Y ) < EpsAreas::eps_vanishcomponent  ) && ( fabs( Z ) < EpsAreas::eps_vanishcomponent  ) );
}

 
inline hprec_t Vector3::VecLength() const
{
	return sqrt( X * X + Y * Y + Z * Z );
}

 
inline hprec_t Vector3::DotProduct( const Vector3& vect ) const
{
	return ( vect.X * X ) + ( vect.Y * Y ) + ( vect.Z * Z );
}

 
inline void Vector3::CrossProduct( const Vector3& vect1, const Vector3& vect2 )
{
	X = ( vect1.Y * vect2.Z ) - ( vect1.Z * vect2.Y );
	Y = ( vect1.Z * vect2.X ) - ( vect1.X * vect2.Z );
	Z = ( vect1.X * vect2.Y ) - ( vect1.Y * vect2.X );
	W = 1.0;
}

 
inline void Vector3::CreateDirVec( const Vertex3& vertex1, const Vertex3& vertex2 )
{
	X = vertex2.X - vertex1.X;
	Y = vertex2.Y - vertex1.Y;
	Z = vertex2.Z - vertex1.Z;
	W = 1.0;
}


} 




# 12 "../BspLib/Vector.h" 2

# 1 "../BspLib/Vector2.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/Vertex2.h" 1
 
 
 
 
 
 




 



namespace BspLib  { 


 
inline Vertex2::Vertex2( const Vector2& vect )
{
	X = vect.X;
	Y = vect.Y;
	W = vect.W;
}

 
inline int operator ==( const Vertex2 v1, const Vertex2 v2 )
{
	return ( ( v1.X == v2.X ) && ( v1.Y == v2.Y ) && ( v1.W == v2.W ) );
}

 
inline int operator !=( const Vertex2 v1, const Vertex2 v2 )
{
	return ( ( v1.X != v2.X ) || ( v1.Y != v2.Y ) || ( v1.W != v2.W ) );
}

 
inline Vertex2& Vertex2::operator +=( const Vector2 v )
{
	X += v.X;
	Y += v.Y;

	return *this;
}

 
inline Vertex2& Vertex2::operator -=( const Vector2 v )
{
	X -= v.X;
	Y -= v.Y;

	return *this;
}

 
inline int Vertex2::IsInVicinity( const Vertex2& vertex ) const
{
	return ( ( fabs( X - vertex.X ) < EpsAreas::eps_vertexmergearea  ) &&
			 ( fabs( Y - vertex.Y ) < EpsAreas::eps_vertexmergearea  ) );
}

 
inline void Vertex2::InitFromVertex3( const Vertex3& vertex )
{
	X = vertex.getX();
	Y = vertex.getY();
	W = vertex.getZ();
}


} 




# 13 "../BspLib/Vector2.h" 2



namespace BspLib  { 


 
inline Vector2 operator *( const Vector2 v1, double t )
{
	return Vector2( v1.X * t, v1.Y * t, 1.0 );
}

 
inline Vector2 operator *( double t, const Vector2 v1 )
{
	return Vector2( v1.X * t, v1.Y * t, 1.0 );
}

 
inline Vector2& Vector2::operator +=( const Vector2 v )
{
	X += v.X;
	Y += v.Y;
	return *this;
}

 
inline Vector2& Vector2::operator -=( const Vector2 v )
{
	X -= v.X;
	Y -= v.Y;
	return *this;
}

 
inline Vector2& Vector2::operator *=( double t )
{
	X *= t;
	Y *= t;
	return *this;
}

 
inline int Vector2::Normalize()
{
	if ( IsNullVector() )
		return 0 ;

	double oonorm = 1 / VecLength();
	X = X * oonorm;
	Y = Y * oonorm;
	W = 1.0;
	return 1 ;
}

 
inline int Vector2::Homogenize()
{
	if ( fabs( W ) < EpsAreas::eps_vanishdenominator  )
		return 0 ;

	double oow = 1 / W;
	X = X * oow;
	Y = Y * oow;
	W = 1.0;
	return 1 ;
}

 
inline int Vector2::IsNullVector() const
{
	 
	return ( ( fabs( X ) < EpsAreas::eps_vanishcomponent  ) && ( fabs( Y ) < EpsAreas::eps_vanishcomponent  ) );
}

 
inline hprec_t Vector2::VecLength() const
{
	return sqrt( X * X + Y * Y );
}

 
inline hprec_t Vector2::DotProduct( const Vector2& vect ) const
{
	return ( vect.X * X ) + ( vect.Y * Y );
}

 
inline void Vector2::CreateDirVec( const Vertex2& vertex1, const Vertex2& vertex2 )
{
	X = vertex2.X - vertex1.X;
	Y = vertex2.Y - vertex1.Y;
	W = 1.0;
}


} 




# 13 "../BspLib/Vector.h" 2



namespace BspLib  { 


 
inline Vertex3 operator +( const Vertex3 v1, const Vector3 v2 )
{
	return Vertex3( v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z, 1.0 );
}

 
inline Vertex3 operator +( const Vector3 v1, const Vertex3 v2 )
{
	return Vertex3( v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z, 1.0 );
}

 
inline Vector3 operator -( const Vertex3 v1, const Vertex3 v2 )
{
	return Vector3( v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z, 1.0 );
}

 
inline Vertex3 operator -( const Vertex3 v1, const Vector3 v2 )
{
	return Vertex3( v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z, 1.0 );
}

 
inline Vector3 operator +( const Vector3 v1, const Vector3 v2 )
{
	return Vector3( v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z, 1.0 );
}

 
inline Vector3 operator -( const Vector3 v1, const Vector3 v2 )
{
	return Vector3( v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z, 1.0 );
}


 


 
inline Vertex2 operator +( const Vertex2 v1, const Vector2 v2 )
{
	return Vertex2( v1.X + v2.X, v1.Y + v2.Y, 1.0 );
}

 
inline Vertex2 operator +( const Vector2 v1, const Vertex2 v2 )
{
	return Vertex2( v1.X + v2.X, v1.Y + v2.Y, 1.0 );
}

 
inline Vector2 operator -( const Vertex2 v1, const Vertex2 v2 )
{
	return Vector2( v1.X - v2.X, v1.Y - v2.Y, 1.0 );
}

 
inline Vertex2 operator -( const Vertex2 v1, const Vector2 v2 )
{
	return Vertex2( v1.X - v2.X, v1.Y - v2.Y, 1.0 );
}

 
inline Vector2 operator +( const Vector2 v1, const Vector2 v2 )
{
	return Vector2( v1.X + v2.X, v1.Y + v2.Y, 1.0 );
}

 
inline Vector2 operator -( const Vector2 v1, const Vector2 v2 )
{
	return Vector2( v1.X - v2.X, v1.Y - v2.Y, 1.0 );
}


} 




# 199 "../BspLib/Vertex.h" 2

# 1 "../BspLib/LineSeg2.h" 1
 
 
 
 
 
 




 




namespace BspLib  { 


 
 
class LineSeg2 {

public:
	LineSeg2( const Vertex2& basevtx, const Vector2& dirvec );
	~LineSeg2() { }

	int PointOnLineSeg( const Vertex2& vertex ) const;

private:
	Vertex2	m_basevtx;
	Vector2 m_dirvec;
};

 
inline LineSeg2::LineSeg2( const Vertex2& basevtx, const Vector2& dirvec )
{
	m_basevtx = basevtx;
	m_dirvec  = dirvec;
}


} 




# 200 "../BspLib/Vertex.h" 2

# 1 "../BspLib/LineSeg3.h" 1
 
 
 
 
 
 




 




namespace BspLib  { 


 
 
class LineSeg3 {

public:
	LineSeg3( const Vertex3& basevtx, const Vector3& dirvec );
	~LineSeg3() { }

	int PointOnLineSeg( const Vertex3& vertex ) const;

private:
	Vertex3	m_basevtx;
	Vector3 m_dirvec;
};

 
inline LineSeg3::LineSeg3( const Vertex3& basevtx, const Vector3& dirvec )
{
	m_basevtx = basevtx;
	m_dirvec  = dirvec;
}


} 




# 201 "../BspLib/Vertex.h" 2

# 1 "../BspLib/VertexChunk.h" 1
 
 
 
 
 
 




 




namespace BspLib  { 


 
 
class VertexChunkRep : public virtual SystemIO {

	friend class VertexChunk;

private:
	VertexChunkRep( int chunksize = 0 );
	~VertexChunkRep();

	Vertex3&		FetchVertex( int index );
	int				AddVertex( Vertex3 vertex );
	int				FindVertex( const Vertex3& vertex );
	int				FindCloseVertex( const Vertex3& vertex, int skip = -1 );
	int				CheckVertices( int verbose );

	int				getNumElements() const;

private:
	void			ChangeAxis( char xchar, hprec_t src );
	int				VerticesEqual( Vertex3 v1, Vertex3 v2 );

private:
	int				ref_count;

	int				numvertices;
	int				maxnumvertices;
	Vertex3*		vertices;
	VertexChunkRep*	next;

	int				nummultvertices;
	static int		eliminatedoublets;
	static const int CHUNK_SIZE;
};

 
inline VertexChunkRep::VertexChunkRep( int chunksize ) : ref_count( 0 )
{
	int challocsize	= chunksize > 0 ? chunksize : CHUNK_SIZE;
	vertices		= new Vertex3[ challocsize ];
	maxnumvertices	= challocsize;
	numvertices		= 0;
	next			= 0 ;
}

 
inline VertexChunkRep::~VertexChunkRep()
{
	delete next;
	delete[] vertices;
}

 
inline int VertexChunkRep::VerticesEqual( Vertex3 v1, Vertex3 v2 )
{
 
	return ( v1 == v2 );
}


 
 
class VertexChunk {

public:
	VertexChunk( int chunksize = 0 ) { rep = new VertexChunkRep( chunksize ); rep->ref_count = 1; }
	~VertexChunk() { if ( --rep->ref_count == 0 ) delete rep; }

	VertexChunk( const VertexChunk& copyobj );
	VertexChunk& operator =( const VertexChunk& copyobj );

	Vertex3& operator []( int index ) { return rep->FetchVertex( index ); }
	Vertex3& FetchVertex( int index ) { return rep->FetchVertex( index ); }

	int	AddVertex( const Vertex3 vertex ) { return rep->AddVertex( vertex ); }
	int	FindVertex( const Vertex3& vertex ) { return rep->FindVertex( vertex ); }
	int	FindCloseVertex( Vertex3 vertex, int skip = -1 ) { return rep->FindCloseVertex( vertex, skip ); }
	int	CheckVertices( int verbose ) { return rep->CheckVertices( verbose ); }

	int	getNumElements() const { return rep->getNumElements(); }

private:
	VertexChunkRep*	rep;
};

 
inline VertexChunk::VertexChunk( const VertexChunk& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

 
inline VertexChunk& VertexChunk::operator =( const VertexChunk& copyobj )
{
	if ( &copyobj != this ) {
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;
		rep->ref_count++;
	}
	return *this;
}


} 




# 202 "../BspLib/Vertex.h" 2





# 13 "../BspLib/Plane.h" 2



namespace BspLib  { 


 
 
class Plane {

	enum {
		NORMAL_VALID	= 0x0001,
		OFFSET_VALID	= 0x0002,
		PLANE_VALID		= NORMAL_VALID | OFFSET_VALID,
	};

public:
	Plane() : m_valid( 0 ) { }
	Plane( const Vector3& pnormal );
	Plane( const Vector3& pnormal, double poffset );
	Plane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 );
	~Plane() { }

	int			InitPlane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 );
	int			CalcPlaneOffset( const Vertex3& vtx );

	Vector3		getPlaneNormal() const { return m_normal; }
	void		setPlaneNormal( const Vector3& pnormal ) { m_normal = pnormal; }

	double		getPlaneOffset() const { return m_offset; }
	void		setPlaneOffset( double poffset ) { m_offset = poffset; }

	int			NormalValid() const { return ( ( m_valid & NORMAL_VALID ) == NORMAL_VALID ); }
	int			PlaneValid() const { return ( ( m_valid & PLANE_VALID ) == PLANE_VALID ); }

	void		ApplyScaleFactor( double sfac );
	void		EliminateDirectionality();

	int			PointContained( const Vertex3& point ) const;
	int			PointInPositiveHalfspace( const Vertex3& point ) const;
	int			PointInNegativeHalfspace( const Vertex3& point ) const;

private:
	Vector3		m_normal;	 
	double		m_offset;	 
	int			m_valid;	 
};

 
inline Plane::Plane( const Vector3& pnormal )
{
	m_normal = pnormal;
	m_offset = 0.0;
	m_valid  = NORMAL_VALID;
}

 
inline Plane::Plane( const Vector3& pnormal, double poffset )
{
	m_normal = pnormal;
	m_offset = poffset;
	m_valid	 = PLANE_VALID;
}

 
inline Plane::Plane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 )
{
	 
	 
	m_normal.CrossProduct( v3 - v1, v2 - v1 );

	 
	m_valid = m_normal.Normalize() ? PLANE_VALID : 0;

	 
	m_offset = m_normal.DotProduct( v1 );
}

 
inline int Plane::InitPlane( const Vertex3& v1, const Vertex3& v2, const Vertex3& v3 )
{
	*this = Plane( v1, v2, v3 );
	return m_valid;
}

 
inline int Plane::CalcPlaneOffset( const Vertex3& vtx )
{
	 
	m_offset = m_normal.DotProduct( vtx );
	return ( m_valid |= OFFSET_VALID );
}

 
inline void Plane::ApplyScaleFactor( double sfac )
{
	 
	 
	m_offset *= sfac;
}

 
inline void Plane::EliminateDirectionality()
{
	if ( m_offset < 0.0 ) {
		m_normal *= -1.0;
		m_offset  = -m_offset;
	}
}

 
inline int Plane::PointContained( const Vertex3& point ) const
{
	return ( fabs( m_normal.DotProduct( point ) - m_offset ) < EpsAreas::eps_planethickness  );
}

 
inline int Plane::PointInPositiveHalfspace( const Vertex3& point ) const
{
	return ( m_normal.DotProduct( point ) - m_offset >= EpsAreas::eps_planethickness  );
}

 
inline int Plane::PointInNegativeHalfspace( const Vertex3& point ) const
{
	return ( m_normal.DotProduct( point ) - m_offset <= - EpsAreas::eps_planethickness  );
}


} 




# 15 "../BspLib/Face.h" 2


# 1 "../BspLib/Texture.h" 1
 
 
 
 
 
 




 





namespace BspLib  { 





 
 
class Texture : public virtual SystemIO {

public:
	Texture( int w = -1, int h = -1, char *tname = 0 , char *fname = 0  );
	~Texture() { }

	int			getWidth() const { return m_width; }
	int			getHeight() const { return m_height; }
	const char*	getName() const { return m_name; }
	const char*	getFile() const { return m_file; }

	void		setWidth( int w ) { m_width = w; }
	void		setHeight( int h ) { m_height = h; }
	void		setName( const char *tname );
	void		setFile( const char *fname );

	void		WriteInfo( FILE *fp );

private:
	int			m_width;
	int			m_height;
	char		m_name[ 31  + 1 ];
	char		m_file[ 1024  + 1 ];
};

 
typedef Chunk<Texture> TextureChunk;


} 




# 17 "../BspLib/Face.h" 2

# 1 "../BspLib/TriMapping.h" 1
 
 
 
 
 
 




 





namespace BspLib  { 


 
 
class TriMapping : public virtual SystemIO {

	void		Error() const;

public:
	TriMapping() { }
	~TriMapping() { }

	Vertex2&	getMapXY( int indx ) { if ( indx > 2 ) Error(); return map_xy[ indx ]; }
	Vertex2&	getMapUV( int indx ) { if ( indx > 2 ) Error(); return map_uv[ indx ]; }

private:
	Vertex2		map_xy[ 3 ];	 
	Vertex2		map_uv[ 3 ];	 
};


} 




# 18 "../BspLib/Face.h" 2




namespace BspLib  { 


 
 
class Face : public virtual SystemIO {

	void	Error() const;

public:

	 
	enum ShadingType {
		no_shad			= 0x1000,	 
		flat_shad		= 0x1001,	 
		gouraud_shad	= 0x1002,	 
		afftex_shad		= 0x2003,	 
		ipol1tex_shad	= 0x2004,	 
		ipol2tex_shad	= 0x2005,	 
		persptex_shad	= 0x2006,	 
		material_shad	= 0x1007,	 
		texmat_shad		= 0x3008,	 
		num_shading_types = 9,		 
		base_mask		= 0x00ff,	 
		color_mask		= 0x1000,	 
		texmap_mask		= 0x2000,	 
	};

	 
	enum ColorModel {
		no_col,					 
		indexed_col,			 
		rgb_col,				 
		material_col,			 
		num_color_models
	};

public:
	Face();
	~Face() { delete faceplane; delete facematerial; delete facemapping; delete texturename; }

	Face( const Face& copyobj );
	Face& operator =( const Face& copyobj );

	int			getId() const { return faceid; }
	void		setId( int id ) { faceid = id; }

	Material	getMaterial() const { if ( facematerial == 0  ) Error(); return *facematerial; }
	Plane		getPlane() const { if ( faceplane == 0  ) Error(); return *faceplane; }

	Vector3		getPlaneNormal() const;
	void		setPlaneNormal( const Vector3& normal );

	int			getShadingType() const { return shadingtype; }
	void		setShadingType( int type );

	int			getColorType() const { return colortype; }
	void		getColorIndex( dword& col ) const { col = facecolor_indx; }
	void		getColorRGBA( ColorRGBA& col ) const { col = facecolor_rgba; }
	void		getColorChannels( float& r, float& g, float& b ) const;
	const char *getTextureName() { return texturename; }

	void		setFaceColor( dword col );
	void		setFaceColor( ColorRGBA col );
	void		setTextureName( const char *tname );

	int			NormalValid() const { return faceplane ? faceplane->NormalValid() : 0 ; }
	int			PlaneValid() const { return faceplane ? faceplane->PlaneValid() : 0 ; }
	int			MaterialAttached() const { return ( facematerial != 0  ); }
	int			MappingAttached() const { return ( facemapping != 0  ); }
	int			FaceTexMapped() const { return ( ( shadingtype & texmap_mask ) != 0 ); }

	void		CalcPlane( const Vertex3& vertex1, const Vertex3& vertex2, const Vertex3& vertex3 );
	void		AttachNormal( const Vector3& normal );
	void		AttachPlane( Plane *plane );
	void		AttachMaterial( Material *mat );

	int			ConvertColorIndexToRGB( char *palette, int changemode );

	Vertex2&	MapXY( int indx );
	Vertex2&	MapUV( int indx );

	void		WriteFaceInfo( FILE *fp ) const;
	void		WriteNormalInfo( FILE *fp ) const;
	void		WriteMappingInfo( FILE *fp ) const;

public:
	static int	GetTypeIndex( const char *type );
	static int	GetColorModelIndex( const char *type );

public:
	static const char*	material_strings[];	 

private:
	static const char*	prop_strings[];		 
	static const int	prop_ids[];			 
	static const char*	color_strings[];	 
	static const int	color_ids[];		 

private:
	int			faceid;			 
	Plane*		faceplane;		 

	int			shadingtype;	 
	int			colortype;		 

	dword		facecolor_indx;	 
	ColorRGBA	facecolor_rgba;	 
	Material*	facematerial;	 
	TriMapping*	facemapping;	 
	char*		texturename;	 
};

 
typedef Chunk<Face> FaceChunk;

 
inline Face::Face()
{
	faceid		 = -1;		 
	shadingtype  = -1;		 
	colortype	 = -1;		 
	faceplane	 = 0 ;	 
	facematerial = 0 ;	 
	facemapping	 = 0 ;	 
	texturename	 = 0 ;	 
}

 
inline Vector3 Face::getPlaneNormal() const
{
	 
		if ( ( faceplane == 0  ) || !faceplane->NormalValid() )
			Error();
	  ;
	return faceplane->getPlaneNormal();
}

 
inline void Face::setPlaneNormal( const Vector3& normal )
{
	 
		if ( ( faceplane == 0  ) || !faceplane->NormalValid() )
			Error();
	  ;
	faceplane->setPlaneNormal( normal );
}

 
inline void Face::setShadingType( int type )
{
	 
		if ( ( type & base_mask ) >= num_shading_types )
			Error();
	  ;
	shadingtype = type;
}

 
inline void Face::setFaceColor( dword col )
{
	facecolor_indx	= col;
	colortype		= indexed_col;
}

 
inline void Face::setFaceColor( ColorRGBA col )
{
	facecolor_rgba	= col;
	colortype		= rgb_col;
}

 
inline void Face::getColorChannels( float& r, float& g, float& b ) const
{
	r = facecolor_rgba.R / 255.0f;
	g = facecolor_rgba.G / 255.0f;
	b = facecolor_rgba.B / 255.0f;
}

 
inline Vertex2& Face::MapXY( int indx )
{
	if ( facemapping == 0  )
		facemapping = new TriMapping;
	return facemapping->getMapXY( indx );
}

 
inline Vertex2& Face::MapUV( int indx )
{
	if ( facemapping == 0  )
		facemapping = new TriMapping;
	return facemapping->getMapUV( indx );
}


} 




# 15 "../BspLib/BspObject.h" 2

# 1 "../BspLib/Mapping.h" 1
 
 
 
 
 
 




 






namespace BspLib  { 





 
 
class Mapping : public virtual SystemIO {

	void		Error( int vertindx );

public:
	Mapping() { numvertexindxs = 0; }
	~Mapping() { }

	void		InsertFaceVertex( int vindx ) { facevertexindxs[ numvertexindxs++ ] = vindx; }
	void		SetCorrespondence( int vindx, Vertex2 vertex );
	Vertex2		FetchMapPoint( int vertindx );

	int			getNumVertices() { return numvertexindxs; }
	void		setNumVertices( int num ) { numvertexindxs = num; }
	void		setMappingCoordinates( int k, const Vertex2& vertex);

	void		WriteMappingInfo( FILE *fp ) const;

private:
	int			numvertexindxs;
	int			facevertexindxs[ 32  ];
	Vertex2		mappingcoordinates[ 32  ];
};

 
typedef Chunk<Mapping> MappingChunk;

 
inline void Mapping::setMappingCoordinates( int k, const Vertex2& vertex)
{
	if ( k < 32  )
		mappingcoordinates[ k ] = vertex;
	else
		Error( k );
}

 
inline void Mapping::SetCorrespondence( int vindx, Vertex2 vertex )
{
	if ( numvertexindxs < 32  ) {
		facevertexindxs[ numvertexindxs ] = vindx;
		mappingcoordinates[ numvertexindxs ] = vertex;
		numvertexindxs++;
	} else {
		Error( numvertexindxs );
	}
}

 
inline Vertex2 Mapping::FetchMapPoint( int vertindx )
{
	for ( int i = 0; i < numvertexindxs; i++ )
		if ( facevertexindxs[ i ] == vertindx )
			return mappingcoordinates[ i ];
	Error( vertindx );
	return Vertex2( 0, 0 );	 
}


} 




# 16 "../BspLib/BspObject.h" 2

# 1 "../BspLib/PolygonList.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/Polygon.h" 1
 
 
 
 
 
 




 







namespace BspLib  { 


class BoundingBox;


 
 
class VIndx {

public:
	VIndx( int indx = -1, VIndx *next = 0  ) { vertindx = indx; nextvertindx = next; }
	~VIndx() { delete nextvertindx; }

	int		getIndx() const { return vertindx; }
	void	setIndx( int indx ) { vertindx = indx; }

	VIndx*	getNext() const { return nextvertindx; }
	void	setNext( VIndx *next ) { nextvertindx = next; }

private:
	int		vertindx;			 
	VIndx*	nextvertindx;		 
};


class BSPNode;
class BspObject;


 
 
class Polygon : public virtual SystemIO {

	friend class PolygonListRep;

	void	Error() const;

	 
	enum {
		POLY_IN_FRONT_SUBSPACE,
		POLY_IN_BACK_SUBSPACE,
		POLY_STRADDLES_SPLITTER,
		POLY_IN_SAME_PLANE,
	};

	 
	enum {
		FRONT_SUBSPACE	= 1,
		BACK_SUBSPACE	= -1
	};

public:

	 
	enum {
		SPLITTERCRIT_FIRST_POLY		= 0x0000,	 
		SPLITTERCRIT_SAMPLE_FIRST_N	= 0x0101,	 
		SPLITTERCRIT_SAMPLE_ALL		= 0x0002,	 
		SPLITTERCRIT_RANDOM_SAMPLE	= 0x0103,	 
		SPLITTERCRITMASK_SAMPLESIZ	= 0x0100,	 
	};

	 
	enum {
		MESSAGE_SPLITTING_POLYGON				= 0x0001,
		MESSAGE_STARTVERTEX_IN_SPLITTER_PLANE	= 0x0002,
		MESSAGE_VERTEX_IN_SPLITTER_PLANE		= 0x0004,
		MESSAGE_NEW_SPLITVERTEX					= 0x0008,
		MESSAGE_REUSING_SPLITVERTEX				= 0x0010,
		MESSAGE_TRACEVERTEX_INSERTED			= 0x0020,
		MESSAGE_SPLITTING_QUADRILATERAL			= 0x0040,
		MESSAGE_INVOCATION						= 0x0080,
		MESSAGE_CHECKING_POLYGON_PLANES			= 0x0100,
		MESSAGEMASK_DISPLAY_ALL					= 0xffff
	};

public:
	Polygon( BspObject *bobj, int pno = 0, int fno = 0, Polygon *next = 0 , int num = 1 );
	~Polygon() { delete vertindxs; delete nextpolygon; }

	Polygon*	NewPolygon();						 
	Polygon*	InsertPolygon( Polygon *poly );		 
	Polygon*	DeleteHead();						 
	Polygon*	FindPolygon( int id );				 
	int			SumVertexNumsEntireList();			 

	void		PrependNewVIndx( int indx );		 
	void		AppendNewVIndx( int indx = -1 );	 
	void		AppendVIndx( VIndx *vindx );		 
	VIndx*		UnlinkLastVIndx();					 

	void		CalcPlaneNormals();					 
	void		CheckEdges();						 
	Polygon*	CheckPlanesAndMappings();			 

	 
	void		CalcBoundingBox( BoundingBox* &boundingbox );

	 
	int			CheckIntersection( Polygon *testpoly );
	 
	int			NormalDirectionSimilar( Polygon *testpoly );
	 
	void		SplitPolygon( Polygon *poly, Polygon* &frontsubspace, Polygon* &backsubspace );
	 
	BSPNode*	PartitionSpace();

	 
	void		CorrectBase( BspObject *newbaseobj, int vertexindxbase, int faceidbase, int polygonidbase );
	void		CorrectBaseByTable( BspObject *newbaseobj, int *vtxindxmap, int faceidbase, int polygonidbase );

	int			getId() const { return polygonno; }
	void		setId( int pno ) { polygonno = pno; }

	int			getFaceId() const { return faceno; }
	void		setFaceId( int fno ) { faceno = fno; }

	int			getNumPolygons() const { return numpolygons; }
	void		setNumPolygons( int len ) { numpolygons = len; }

	Polygon*	getNext() const { return nextpolygon; }
	void		setNext( Polygon *next ) { nextpolygon = next; }

	int			getNumVertices() const { return numvertindxs; }

	VIndx*		getVList() const { return vertindxs; }
	BspObject*	getBaseObject() const { return baseobject; }

	VertexChunk& getVertexList();
	FaceChunk&	getFaceList();

	int			HasArea() const;

	int			getFirstVertexIndx() const;
	int			getSecondVertexIndx() const;
	int			getThirdVertexIndx() const;

	Vertex3		getFirstVertex() const;
	Vertex3		getSecondVertex() const;
	Vertex3		getThirdVertex() const;

	Plane		getPlane() const { return ((Polygon *const)this)->getFaceList()[ faceno ].getPlane(); }
	Vector3		getPlaneNormal() const { return ((Polygon *const)this)->getFaceList()[ faceno ].getPlaneNormal(); }

	void		FillVertexIndexArray( dword *arr ) const;		 
	void		WriteVertexList( FILE *fp, int cr ) const;		 
	void		WritePolyList( FILE *fp ) const;				 

	int			CalcSplitterTestProbability();

public:
	static int	getSplitterSelection() { return splitter_crit; }
	static void	setSplitterSelection( int criterion ) { splitter_crit = criterion; }

	static int	getSampleSize() { return sample_size; }
	static void	setSampleSize( int siz ) { sample_size = siz; }

	static int	getTriangulationFlag() { return triangulate_all; }
	static void setTriangulationFlag( int flag ) { triangulate_all = flag; }

	static int	getNormalizeVectorsFlag() { return normalize_vectors; }
	static void setNormalizeVectorsFlag( int flag ) { normalize_vectors = flag; }

	static int	getDisplayMessagesFlag() { return display_messages; }
	static void setDisplayMessagesFlag( int flag ) { display_messages = flag; }

	static void	ResetCallCount() { partition_callcount = 0; }

private:
	static int	splitter_crit;		 
	static int	sample_size;		 
	static int	triangulate_all;	 
	static int	normalize_vectors;	 
	static int	display_messages;	 
	static int	partition_callcount; 
	static int	test_probability;	 
	static char	str_scratchpad[];	 

private:
	int			polygonno;			 
	int			faceno;				 
	int			numpolygons;		 
	int			numvertindxs;		 
	VIndx*		vertindxs;			 
	VIndx*		vindxinsertpos;		 
	BspObject*	baseobject;			 
	Polygon*	nextpolygon;		 
};

 
inline int Polygon::HasArea() const
{
	 
	 
	return ( vertindxs && vertindxs->getNext() && vertindxs->getNext()->getNext() );
}

 
inline int Polygon::getFirstVertexIndx() const
{
	 
		if ( vertindxs == 0  )
			Error();
	  ;
	return vertindxs->getIndx();
}
inline Vertex3 Polygon::getFirstVertex() const
{
	 
		if ( vertindxs == 0  )
			Error();
	  ;
	return ((Polygon *const)this)->getVertexList()[ vertindxs->getIndx() ];
}

 
inline int Polygon::getSecondVertexIndx() const
{
	 
		if ( ( vertindxs == 0  ) || ( vertindxs->getNext() == 0  ) )
			Error();
	  ;
	return vertindxs->getNext()->getIndx();
}
inline Vertex3 Polygon::getSecondVertex() const
{
	 
		if ( ( vertindxs == 0  ) || ( vertindxs->getNext() == 0  ) )
			Error();
	  ;
	return ((Polygon *const)this)->getVertexList()[ vertindxs->getNext()->getIndx() ];
}

 
inline int Polygon::getThirdVertexIndx() const
{
	 
		if ( ( vertindxs == 0  ) ||
			 ( vertindxs->getNext() == 0  ) ||
			 ( vertindxs->getNext()->getNext() == 0  ) )
			Error();
	  ;
	return vertindxs->getNext()->getNext()->getIndx();
}
inline Vertex3 Polygon::getThirdVertex() const
{
	 
		if ( ( vertindxs == 0  ) ||
			 ( vertindxs->getNext() == 0  ) ||
			 ( vertindxs->getNext()->getNext() == 0  ) )
			Error();
	  ;
	return ((Polygon *const)this)->getVertexList()[ vertindxs->getNext()->getNext()->getIndx() ];
}


} 




# 13 "../BspLib/PolygonList.h" 2




namespace BspLib  { 


class BSPNode;
class BspObject;


 
 
class PolygonListRep : public virtual SystemIO {

	friend class PolygonList;

	 
	enum {
		E_NEWVINDX,
		E_APPENDVINDX,
		E_GETNUMVERTXS
	};

	 
	void		Error( int err ) const;

private:
	PolygonListRep( BspObject *bobj );
	~PolygonListRep() { delete list; }

	void		MergeLists( PolygonListRep *mergelist );

	Polygon*	InitList( Polygon *listhead );

	 
	void		InvalidateList() { list = 0 ; numpolygons = 0; }

	Polygon*	FetchHead() { return list; }
	Polygon*	UnlinkHead();
	Polygon*	DeleteHead();
	Polygon*	FindPolygon( int id ) { return list ? list->FindPolygon( id ) : 0 ; }

	Polygon*	NewPolygon();
	Polygon*	InsertPolygon( Polygon *poly );

	void		PrependNewVIndx( int indx = -1 ) { if ( list == 0  ) Error( E_NEWVINDX ); list->PrependNewVIndx( indx ); }
	void		AppendNewVIndx( int indx = -1 ) { if ( list == 0  ) Error( E_NEWVINDX ); list->AppendNewVIndx( indx ); }
	void		AppendVIndx( VIndx *vindx ) { if ( list == 0  ) Error( E_APPENDVINDX ); list->AppendVIndx( vindx ); }
	Polygon*	CalcPlaneNormals();
	Polygon*	CheckPolygonPlanes();
	BSPNode*	PartitionSpace();

	void		WritePolyList( FILE *fp, int no ) const;

	int			getNumElements() const { return numpolygons; }
	int			getNumVertices() const { if( list == 0  ) Error( E_GETNUMVERTXS ); return list->getNumVertices(); }

private:
	int			ref_count;			 
	int			numpolygons;		 
	BspObject*	baseobject;			 
	Polygon*	list;				 
};

 
inline PolygonListRep::PolygonListRep( BspObject *bobj ) : ref_count( 0 )
{
	baseobject	= bobj;
	list		= 0 ;
	numpolygons	= 0;
}


 
 
class PolygonList {

public:
	PolygonList( BspObject *bobj ) { rep = new PolygonListRep( bobj ); rep->ref_count = 1; }
	~PolygonList() { if ( --rep->ref_count == 0 ) delete rep; }

	PolygonList( const PolygonList& copyobj );
	PolygonList& operator =( const PolygonList& copyobj );

	void		MergeLists( PolygonList *mergelist ) { if ( mergelist ) rep->MergeLists( mergelist->rep ); }

	Polygon*	InitList( Polygon *listhead ) { return rep->InitList( listhead ); }
	void		InvalidateList() { rep->InvalidateList(); }

	Polygon*	FetchHead() { return rep->FetchHead(); }
	Polygon*	UnlinkHead() { return rep->UnlinkHead(); }
	Polygon*	DeleteHead() { return rep->DeleteHead(); }
	Polygon*	FindPolygon( int id ) { return rep->FindPolygon( id ); }

	Polygon*	NewPolygon() { return rep->NewPolygon(); }
	Polygon*	InsertPolygon( Polygon *poly ) { return rep->InsertPolygon( poly ); }

	void		PrependNewVIndx( int indx = -1 ) { rep->PrependNewVIndx( indx ); }
	void		AppendNewVIndx( int indx = -1 ) { rep->AppendNewVIndx( indx ); }
	void		AppendVIndx( VIndx *vindx ) { rep->AppendVIndx( vindx ); }
	Polygon*	CalcPlaneNormals() { return rep->CalcPlaneNormals(); }
	Polygon*	CheckPolygonPlanes() { return rep->CheckPolygonPlanes(); }
	BSPNode*	PartitionSpace() { return rep->PartitionSpace(); }

	void		WritePolyList( FILE *fp, int no ) const { rep->WritePolyList( fp, no ); }

	int			getNumElements() const { return rep->getNumElements(); }
	int			getNumVertices() const { return rep->getNumVertices(); }

private:
	PolygonListRep*	rep;
};

 
inline PolygonList::PolygonList( const PolygonList& copyobj )
{
	rep = copyobj.rep;	 
	rep->ref_count++;	 
}

 
inline PolygonList& PolygonList::operator =( const PolygonList& copyobj )
{
	if ( &copyobj != this ) {
		 
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;	 
		rep->ref_count++;	 
	}
	return *this;
}


} 




# 17 "../BspLib/BspObject.h" 2

# 1 "../BspLib/BSPTree.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/BspNode.h" 1
 
 
 
 
 
 




 





namespace BspLib  { 


class BoundingBox;
class BspObject;


 
 
class BSPNode {

public:

	 
	enum {
		OUTPUT_OLD_STYLE,
		OUTPUT_KEY_VALUE_STYLE
	};

public:
	BSPNode( BSPNode *front = 0 , BSPNode *back = 0 ,
			 Polygon *poly = 0 , Polygon *backpoly = 0 ,
			 Plane *sep = 0 , BoundingBox *box = 0 , int num = -1 );
	~BSPNode();

public:
	void		NumberBSPNodes( int& curno );
	void		SumVertexNums( int& vtxnum );
	void		CorrectPolygonBases( BspObject *newbaseobj, int vertexindxbase, int faceidbase, int polygonidbase );
	void		CorrectPolygonBasesByTable( BspObject *newbaseobj, int *vtxindxmap, int faceidbase, int polygonidbase );
	void		WriteBSPTree( FILE *fp );
	void		CheckEdges();
	void		FetchFacePolygons( int facno, PolygonList& facepolylist );
	Polygon*	FetchBSPPolygon( int polyno );
	void		CalcBoundingBoxes();
	void		CalcSeparatorPlanes();

	int			getNodeNumber() const { return nodenumber; }
	Polygon*	getPolygon() { return polygon; }
	Polygon*	getBackPolygon() { return backpolygon; }
	BSPNode*	getFrontSubtree() const { return frontsubtree; }
	BSPNode*	getBackSubtree() const { return backsubtree; }

	Plane*		getSeparatorPlane() { return separatorplane; }
	void		setSeparatorPlane( Plane *sep ) { separatorplane = sep; }

	BoundingBox*getBoundingBox() { return boundingbox; }
	void		setBoundingBox( BoundingBox *box ) { boundingbox = box; }

private:
	void		GrowBoundingBox( BSPNode *othernode );

public:
	static int	getOutputFormat() { return outputformat; }
	static void	setOutputFormat( int format ) { outputformat = format; }

private:
	static int	outputformat;	 

private:
	int			nodenumber;		 
	Polygon*	polygon;		 
	Polygon*	backpolygon;	 
	Plane*		separatorplane;	 
	BoundingBox*boundingbox;	 
	BSPNode*	frontsubtree;	 
	BSPNode*	backsubtree;	 
};


 
 
class BSPNodeFlat {

public:
	BSPNodeFlat( int front = 0, int back = 0,
				 Polygon *poly = 0 , int clist = 0, int blist = 0,
				 Plane *sep = 0 , BoundingBox *box = 0 , int num = -1 );
	~BSPNodeFlat() {   }

public:
	void		InitNode( int front, int back, Polygon *poly, int clist, int blist,
						  Plane *sep = 0 , BoundingBox *box = 0 , int num = -1 );

	void		ApplyScaleFactor( double sfac );

	int			getNodeNumber() const { return nodenumber; }
	Polygon*	getPolygon() { return polygon; }
	int			getContainedList() const { return containedlistindx; }
	int			getBackList() const { return backlistindx; }
	int			getFrontSubTree() const { return frontsubtreeindx; }
	int			getBackSubTree() const { return backsubtreeindx; }

	Plane*		getSeparatorPlane() { return separatorplane; }
	void		setSeparatorPlane( Plane *sep ) { separatorplane = sep; }

	BoundingBox*getBoundingBox() { return boundingbox; }
	void		setBoundingBox( BoundingBox *box ) { boundingbox = box; }

private:
	int			nodenumber;			 
	Polygon*	polygon;			 
	Plane*		separatorplane;		 
	BoundingBox*boundingbox;		 
	int			containedlistindx;	 
	int			backlistindx;		 
	int			frontsubtreeindx;	 
	int			backsubtreeindx;	 
};


} 




# 13 "../BspLib/BSPTree.h" 2



namespace BspLib  { 


 
 
class BSPTreeRep {

	friend class BSPTree;

private:
	BSPTreeRep() : ref_count( 0 ) { root = 0 ; }
	~BSPTreeRep() { delete root; }

	BSPNode*	InitTree( BSPNode *rootnode );
	BSPNode*	getRoot() { return root; }

	void		InvalidateTree();

	int			TreeEmpty() const { return ( root == 0  ); }

private:
	int			ref_count;
	BSPNode*	root;
};

 
inline BSPNode *BSPTreeRep::InitTree( BSPNode *rootnode )
{
	delete root;
	return ( root = rootnode );
}

 
inline void BSPTreeRep::InvalidateTree()
{
	root = 0 ;
}


 
 
class BSPTree {

public:
	BSPTree() {	rep = new BSPTreeRep; rep->ref_count = 1; }
	~BSPTree() { if ( --rep->ref_count == 0 ) delete rep; }

	BSPTree( const BSPTree& copyobj );
	BSPTree& operator =( const BSPTree& copyobj );

	 
	BSPNode*	operator->() { return rep->getRoot(); }

	BSPNode*	InitTree( BSPNode *rootnode ) { return rep->InitTree( rootnode ); }
	BSPNode*	getRoot() { return rep->getRoot(); }

	void		InvalidateTree() { rep->InvalidateTree(); }

	int			TreeEmpty() const { return rep->TreeEmpty(); }

private:
	BSPTreeRep*	rep;
};

 
inline BSPTree::BSPTree( const BSPTree& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

 
inline BSPTree& BSPTree::operator =( const BSPTree& copyobj )
{
	if ( &copyobj != this ) {
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;
		rep->ref_count++;
	}
	return *this;
}


 
 
class BSPTreeFlatRep {

	friend class BSPTreeFlat;

private:
	BSPTreeFlatRep() : ref_count( 0 ) { root = 0 ; numnodes = 0; nodestorage = 0; }
	~BSPTreeFlatRep() { delete[] root; }

	BSPNodeFlat*	AppendNode( int front, int back, Polygon *poly, int clist, int blist );
	BSPNodeFlat*	FetchNodePerId( int id );
	BSPNode*		BuildBSPTree( int nodenum );
	BSPNodeFlat*	getRoot() { return root; }

	int				TreeEmpty() const { return ( root == 0  ); }
	int				getNumNodes() const { return numnodes; }

	void			ApplyScaleFactor( double sfac );
	void			DestroyTree() { delete[] root; root = 0 ; numnodes = 0; nodestorage = 0; }

private:
	int				ref_count;
	int				numnodes;
	int				nodestorage;
	BSPNodeFlat*	root;
};


 
 
class BSPTreeFlat {

public:
	BSPTreeFlat() {	rep = new BSPTreeFlatRep; rep->ref_count = 1; }
	~BSPTreeFlat() { if ( --rep->ref_count == 0 ) delete rep; }

	BSPTreeFlat( const BSPTreeFlat& copyobj );
	BSPTreeFlat& operator =( const BSPTreeFlat& copyobj );

	BSPNodeFlat*	AppendNode( int front, int back, Polygon *poly, int clist, int blist )
		{ return rep->AppendNode( front, back, poly, clist, blist ); }
	BSPNodeFlat*	FetchNodePerId( int id ) { return rep->FetchNodePerId( id ); }
	BSPNode*		BuildBSPTree( int nodenum ) { return rep->BuildBSPTree( nodenum ); }
	BSPNodeFlat*	getRoot() { return rep->getRoot(); }

	int				TreeEmpty() const { return rep->TreeEmpty(); }
	int				getNumNodes() const { return rep->getNumNodes(); }

	void			ApplyScaleFactor( double sfac ) { rep->ApplyScaleFactor( sfac ); }
	void			DestroyTree() { rep->DestroyTree(); }

private:
	BSPTreeFlatRep*	rep;
};

 
inline BSPTreeFlat::BSPTreeFlat( const BSPTreeFlat& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

 
inline BSPTreeFlat& BSPTreeFlat::operator =( const BSPTreeFlat& copyobj )
{
	if ( &copyobj != this ) {
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;
		rep->ref_count++;
	}
	return *this;
}


} 




# 18 "../BspLib/BspObject.h" 2


# 1 "../BspLib/Transform3.h" 1
 
 
 
 
 
 




 




namespace BspLib  { 


 
 
class Transform3 {

	friend Transform3 operator *( const Transform3& trafo1, const Transform3& trafo2 );

public:
	Transform3();
	Transform3( const float trafo[4][4] );
	Transform3( const double trafo[4][4] );
	~Transform3() { }

	 
	Vector3		TransformVector3( const Vector3& vec ) const;

	 
	Transform3&	Concat( const Transform3& cattrafo );
	Transform3&	ConcatR( const Transform3& cattrafo );

	 
	Transform3&	LoadIdentity();
	Transform3&	LoadRotation( double angle, double x, double y, double z );
	Transform3&	LoadScale( double x, double y, double z );
	Transform3&	LoadTranslation( double x, double y, double z );

	 
	Transform3&	Rotate( double angle, double x, double y, double z );
	Transform3&	Scale( double x, double y, double z );
	Transform3&	Translate( double x, double y, double z );

	 
	Transform3&	RotateR( double angle, double x, double y, double z );
	Transform3&	ScaleR( double x, double y, double z );
	Transform3&	TranslateR( double x, double y, double z );

	 
	Vector3		FetchTranslation() const;
	Vector3		ExtractTranslation();

private:
	 
	double	m_matrix[4][4];
};

 
inline Transform3& Transform3::LoadIdentity()
{
	memset( m_matrix, 0, sizeof( m_matrix ) );

	m_matrix[ 0 ][ 0 ] = 1.0;
	m_matrix[ 1 ][ 1 ] = 1.0;
	m_matrix[ 2 ][ 2 ] = 1.0;
	m_matrix[ 3 ][ 3 ] = 1.0;

	return *this;
}

 
inline Transform3::Transform3()
{
 
}

 
inline Transform3::Transform3( const double trafo[4][4] )
{
	 
	 
	 
	memcpy( m_matrix, trafo, sizeof( m_matrix ) );
}

 
inline Transform3::Transform3( const float trafo[4][4] )
{
	 
	 
	 
	for ( int i = 0; i < 16; i++ )
		((double *)m_matrix)[ i ] = (double) ((float *)trafo)[ i ];
}


} 




# 20 "../BspLib/BspObject.h" 2




namespace BspLib  { 


class BoundingBox;


 
 
class BspObjectInfo {

	friend class BspObject;

private:
	BspObjectInfo();
	~BspObjectInfo() { }

public:
	int	getNumVertices() const		{ return numvertices; }
	int	getNumPolygons() const		{ return numpolygons; }
	int	getNumFaces() const			{ return numfaces; }

	int	getInputVertices() const	{ return numvertices_in; }
	int	getInputPolygons() const	{ return numpolygons_in; }
	int	getInputFaces() const		{ return numfaces_in; }

	int getBspPolygons() const		{ return numbsppolygons; }
	int getPreBspPolygons() const	{ return numpolygons_before_bsp; }

	int	getNumTextures() const		{ return numtextures; }
	int	getNumMappings() const		{ return numcorrespondences; }
	int	getNumNormals() const		{ return numnormals; }
	int	getNumTexturedFaces() const	{ return numtexmappedfaces; }

	int getNumTraceVertices() const	{ return numtracevertices; }
	int getNumMultiVertices() const	{ return nummultvertices; }
	int getNumSplitQuads() const	{ return numsplitquadrilaterals; }

protected:
	int numvertices;				 
	int numpolygons;				 
	int numfaces;					 
	int numtextures;				 
	int numcorrespondences;			 

	int numnormals;					 
	int numtexmappedfaces;			 

	int numvertices_in;				 
	int numpolygons_in;				 
	int numfaces_in;				 

	int numbsppolygons;				 
	int numpolygons_before_bsp;		 

	int numtracevertices;			 
	int nummultvertices;			 
	int numsplitquadrilaterals;		 
};


 
 
class BspObject : public BspObjectInfo, public virtual SystemIO {

	friend class BoundingBox;
	friend class BspObjectListRep;
	friend class Face;
	friend class ObjectBSPNode;
	friend class Polygon;

	friend class VrmlFile;

public:
	BspObject();
	~BspObject() { delete objectname; delete next; }

	BspObject( const BspObject& copyobj );
	BspObject& operator =( const BspObject& copyobj );

	 
	 
	void			MergeObjects( BspObject *mergeobj );

	void			CollapseObjectList();		 

	BoundingBox*	BuildBoundingBoxList();		 

	BSPNode*		BuildBSPTree();				 
	BSPNode*		BuildBSPTreeFromFlat();		 

	int				BspTreeAvailable();			 
	int				BSPTreeAvailable();			 
	int				BSPTreeFlatAvailable();		 

	 
	void			CalcBoundingBoxes();		 
	void			CalcSeparatorPlanes();		 
	void			CheckEdges();				 
	void			CheckPolygonPlanes();		 
	void			CheckVertices( int verbose );  
	void			CalcPlaneNormals();			 
	void			CheckParsedData();			 
	void			UpdateAttributeNumbers();	 

	int				ConvertColorIndexesToRGB( char *palette, int changemode );

	 
	void			CalcBoundingBox( Vertex3& minvertex, Vertex3& maxvertex );

	 
	void			DisplayStatistics();

	 
	 
	 
	virtual int		WriteVertexList( FileAccess& fp ) { return 0 ; }
	virtual int		WritePolygonList( FileAccess& fp ) { return 0 ; }
	virtual int		WriteFaceList( FileAccess& fp ) { return 0 ; }
	virtual int		WriteFaceProperties( FileAccess& fp ) { return 0 ; }
	virtual int		WriteTextureList( FileAccess& fp ) { return 0 ; }
	virtual int		WriteMappingList( FileAccess& fp ) { return 0 ; }
	virtual int		WriteNormals( FileAccess& fp ) { return 0 ; }
	virtual int		WriteBSPTree( FileAccess& fp ) { return 0 ; }

	 
	VertexChunk&	getVertexList()		{ return vertexlist; }
	PolygonList&	getPolygonList()	{ return polygonlist; }
	FaceChunk&		getFaceList()		{ return facelist; }
	TextureChunk&	getTextureList()	{ return texturelist; }
	MappingChunk&	getMappingList()	{ return mappinglist; }
	BSPTree&		getBSPTree()		{ return bsptree; }
	BSPTreeFlat&	getBSPTreeFlat()	{ return bsptreeflat; }

	 
	char*			getObjectName()	const { return objectname; }
	void			setObjectName( char *name );

	 
	Vertex3			getCenterInWorldSpace() const { return objecttrafo.FetchTranslation(); }
	Transform3		getObjectTransformation() const { return objecttrafo; }
	void			setObjectTransformation( const Transform3& ot ) { objecttrafo = ot; }

	 
	void			ApplyScale( double scalefac );

	 
	void			ApplyCenter();

	 
	void			ApplyTransformation();

	 
	BspObject*		getNext() const { return next; }

private:
	void			InconsistencyError( const char *err );

public:
	static int		getEliminateDoubletsOnMergeFlag() { return check_vertex_doublets_on_merge; }
	static void		setEliminateDoubletsOnMergeFlag( int flag ) { check_vertex_doublets_on_merge = flag; }

private:
	static int		check_vertex_doublets_on_merge;

protected:
	 
	VertexChunk		vertexlist;		 
	PolygonList		polygonlist;	 
	FaceChunk		facelist;		 
	TextureChunk	texturelist;	 
	MappingChunk	mappinglist;	 
	BSPTree			bsptree;		 
	BSPTreeFlat		bsptreeflat;	 
	Transform3		objecttrafo;	 

	 
	char*			objectname;

	 
	BspObject*		next;
};

 
inline BspObject::BspObject( const BspObject& copyobj ) :
	BspObjectInfo( copyobj ),
	vertexlist( copyobj.vertexlist ),
	polygonlist( copyobj.polygonlist ),
	facelist( copyobj.facelist ),
	texturelist( copyobj.texturelist ),
	mappinglist( copyobj.mappinglist ),
	bsptree( copyobj.bsptree ),
	bsptreeflat( copyobj.bsptreeflat ),
	objecttrafo( copyobj.objecttrafo )
{
	 
	objectname = 0 ;
	setObjectName( copyobj.objectname );

	 
	next = 0 ;
}

 
inline BspObject& BspObject::operator =( const BspObject& copyobj )
{
	if ( &copyobj != this ) {

		 
		*(BspObjectInfo *)this = copyobj;

		 
		vertexlist	= copyobj.vertexlist;
		polygonlist	= copyobj.polygonlist;
		facelist	= copyobj.facelist;
		texturelist	= copyobj.texturelist;
		mappinglist	= copyobj.mappinglist;
		bsptree		= copyobj.bsptree;
		bsptreeflat	= copyobj.bsptreeflat;
		objecttrafo	= copyobj.objecttrafo;

		 
		setObjectName( copyobj.objectname );

		 
		next = 0 ;
	}
	return *this;
}


} 




# 13 "../BspLib/BoundingBox.h" 2




namespace BspLib  { 


 
 
class BoundingBox {

	friend class ObjectBSPNode;

public:
	BoundingBox() { containedobject = 0 ; nextbox = 0 ; }
	BoundingBox( BspObject *cobj, BoundingBox *next = 0  );
	BoundingBox( const Vertex3& minvert, const Vertex3& maxvert, BoundingBox *next = 0  );
	~BoundingBox() { delete nextbox; }

	void			ApplyScaleFactor( double sfac );
	void			GrowBoundingBox( BoundingBox *otherbox );
	void			BoundingBoxListUnion( BoundingBox& unionbox );
	ObjectBSPNode*	PartitionSpace();

	Vertex3			getMinVertex() const { return minvertex; }
	Vertex3			getMaxVertex() const { return maxvertex; }
	BspObject*		getContainedObject() const { return containedobject; }
	BoundingBox*	getNext() const { return nextbox; }

private:
	Vertex3			minvertex;
	Vertex3			maxvertex;
	BspObject*		containedobject;
	BoundingBox*	nextbox;
};

 
inline BoundingBox::BoundingBox( BspObject *cobj, BoundingBox *next )
{
	if ( ( containedobject = cobj ) != 0  ) {
		cobj->CalcBoundingBox( minvertex, maxvertex );
	}
	nextbox = next;
}

 
inline BoundingBox::BoundingBox( const Vertex3& minvert, const Vertex3& maxvert, BoundingBox *next )
{
	minvertex		= minvert;
	maxvertex		= maxvert;
	containedobject	= 0 ;
	nextbox			= next;
}


} 




# 13 "../BspLib/VrmlFile.h" 2

# 1 "../BspLib/InputData3D.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/BspObjectList.h" 1
 
 
 
 
 
 




 


# 1 "../BspLib/ObjectBSPTree.h" 1
 
 
 
 
 
 




 

# 1 "../BspLib/ObjectBSPNode.h" 1
 
 
 
 
 
 




 






namespace BspLib  { 


 
 
class ObjectBSPNode {

public:
	ObjectBSPNode( ObjectBSPNode *front = 0 , ObjectBSPNode *back = 0 , Plane *sep = 0 , BoundingBox *bbox = 0 , int id = -1 );
	~ObjectBSPNode() { delete separatorplane; delete boundingbox; delete frontsubtree; delete backsubtree; }

	void			NumberBSPNodes( int& curno );
	void			WriteBSPTree( FILE *fp ) const;

	BspObject*		CreateObjectList();
	BspObject*		CreateMergedBSPTree();

	void			DeleteNodeBspObjects();

	int				getNodeNumber() const { return nodenumber; }
	Plane*			getSeparatorPlane() const { return separatorplane; }
	BoundingBox*	getBoundingBox() const { return boundingbox; }
	ObjectBSPNode*	getFrontSubtree() const { return frontsubtree; }
	ObjectBSPNode*	getBackSubtree() const { return backsubtree; }

private:
	void			MergeTreeNodeObjects( BspObject *newobject );
	BSPNode*		CreateUnifiedBSPTree();

private:
	int				nodenumber;		 
	Plane*			separatorplane;	 
	BoundingBox*	boundingbox;	 
	ObjectBSPNode*	frontsubtree;	 
	ObjectBSPNode*	backsubtree;	 
};

 
inline ObjectBSPNode::ObjectBSPNode( ObjectBSPNode *front, ObjectBSPNode *back, Plane *sep, BoundingBox *bbox, int id )
{
	frontsubtree	= front;
	backsubtree		= back;
	separatorplane	= sep;
	boundingbox		= bbox;
	nodenumber		= id;
}


} 




# 13 "../BspLib/ObjectBSPTree.h" 2



namespace BspLib  { 


 
 
class ObjectBSPTreeRep {

	friend class ObjectBSPTree;

private:
	ObjectBSPTreeRep() : ref_count( 0 ) { root = 0 ; }
	~ObjectBSPTreeRep() { delete root; }

	void			KillTree() { delete root; root = 0 ; }
	ObjectBSPNode*	InitTree( ObjectBSPNode *rootnode );
	ObjectBSPNode*	getRoot() { return root; }

	int				TreeEmpty() const { return ( root == 0  ); }

private:
	int				ref_count;
	ObjectBSPNode*	root;
};

 
inline ObjectBSPNode *ObjectBSPTreeRep::InitTree( ObjectBSPNode *rootnode )
{
	delete root;
	return ( root = rootnode );
}


 
 
class ObjectBSPTree {

public:
	ObjectBSPTree() { rep = new ObjectBSPTreeRep; rep->ref_count = 1; }
	~ObjectBSPTree() { if ( --rep->ref_count == 0 ) delete rep; }

	ObjectBSPTree( const ObjectBSPTree& copyobj );
	ObjectBSPTree& operator =( const ObjectBSPTree& copyobj );

	 
	ObjectBSPNode*	operator->() { return rep->getRoot(); }

	void			KillTree() { rep->KillTree(); }
	ObjectBSPNode*	InitTree( ObjectBSPNode *rootnode ) { return rep->InitTree( rootnode ); }
	ObjectBSPNode*	getRoot() { return rep->getRoot(); }

	int				TreeEmpty() const { return rep->TreeEmpty(); }

private:
	ObjectBSPTreeRep* rep;
};

 
inline ObjectBSPTree::ObjectBSPTree( const ObjectBSPTree& copyobj )
{
	rep = copyobj.rep;
	rep->ref_count++;
}

 
inline ObjectBSPTree& ObjectBSPTree::operator =( const ObjectBSPTree& copyobj )
{
	if ( &copyobj != this ) {
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;
		rep->ref_count++;
	}
	return *this;
}


} 




# 14 "../BspLib/BspObjectList.h" 2



namespace BspLib  { 


 
 
class BspObjectListRep {

	friend class BspObjectList;

public:
	BspObjectListRep() : ref_count( 0 ) { list = 0 ; }
	~BspObjectListRep() { delete list; }

	BspObject*		CreateNewObject();				 
	BspObject*		InsertObject( BspObject *obj );	 

	BoundingBox*	BuildBoundingBoxList();			 

	int				CountListObjects();				 

	int				PrepareObjectBSPTree( ObjectBSPTree& objbsptree );
	int				MergeObjectBSPTree( ObjectBSPTree& objbsptree );

	int				CollapseObjectList();			 

	int				ProcessObjects( int flags );	 

	int				BspTreeAvailable() { return list ? list->BspTreeAvailable() : 0 ; }
	int				BSPTreeAvailable() { return list ? list->BSPTreeAvailable() : 0 ; }
	int				BSPTreeFlatAvailable() { return list ? list->BSPTreeFlatAvailable() : 0 ; }

	BspObject*		getListHead() const { return list; }

private:
	int				ref_count;	 
	BspObject*		list;		 
};


 
 
class BspObjectList {

public:

	 
	enum {
		CHECK_PLANES			= 0x0001,
		BUILD_BSP				= 0x0002,
		CHECK_VERTICES			= 0x0004,
		CHECK_EDGES				= 0x0008,
		BUILD_BSP_WITH_CHECKS	= 0x000F,
		DISPLAY_STATS			= 0x0010,
		BUILD_FROM_FLAT			= 0x0020,
		MERGE_VERTICES			= 0x0040,
		CULL_NULL_EDGES			= 0x0080,
		ELIMINATE_T_VERTICES	= 0x0100,
		MERGE_FACES				= 0x0200,
		CALC_PLANE_NORMALS		= 0x0400,
		CALC_BOUNDING_BOXES		= 0x0800,
		CALC_SEPARATOR_PLANES	= 0x1000,
		APPLY_TRANSFORMATIONS	= 0x2000,
	};

public:
	BspObjectList() { rep = new BspObjectListRep(); rep->ref_count = 1; }
	~BspObjectList() { if ( --rep->ref_count == 0 ) delete rep; }

	BspObjectList( const BspObjectList& copyobj );
	BspObjectList& operator =( const BspObjectList& copyobj );

	BspObject*		CreateNewObject() { return rep->CreateNewObject(); }
	BspObject*		InsertObject( BspObject *obj ) { return rep->InsertObject( obj ); }

	 
	 
	BoundingBox*	BuildBoundingBoxList() { return rep->BuildBoundingBoxList(); }

	int				CountListObjects() { return rep->CountListObjects(); }

	int				PrepareObjectBSPTree( ObjectBSPTree& objbsptree ) { return rep->PrepareObjectBSPTree( objbsptree ); }
	int				MergeObjectBSPTree( ObjectBSPTree& objbsptree ) { return rep->MergeObjectBSPTree( objbsptree ); }

	int				CollapseObjectList() { return rep->CollapseObjectList(); }

	 
	int				ProcessObjects( int flags ) { return rep->ProcessObjects( flags ); }

	 
	int				BspTreeAvailable() { return rep->BspTreeAvailable(); }
	 
	int				BSPTreeAvailable() { return rep->BSPTreeAvailable(); }
	 
	int				BSPTreeFlatAvailable() { return rep->BSPTreeFlatAvailable(); }

	 
	 
	 
	 
	 

	 
	BspObject*		getListHead() { return rep->getListHead(); }

private:
	BspObjectListRep *rep;
};

 
inline BspObjectList::BspObjectList( const BspObjectList& copyobj )
{
	rep = copyobj.rep;	 
	rep->ref_count++;	 
}

 
inline BspObjectList& BspObjectList::operator =( const BspObjectList& copyobj )
{
	if ( &copyobj != this ) {
		 
		if ( --rep->ref_count == 0 ) {
			delete rep;
		}
		rep = copyobj.rep;	 
		rep->ref_count++;	 
	}
	return *this;
}


} 




# 13 "../BspLib/InputData3D.h" 2

# 1 "../BspLib/IOData3D.h" 1
 
 
 
 
 
 




 





namespace BspLib  { 


 
 
class IOData3D : public virtual SystemIO {

public:

	 
	enum {
		DONT_CREATE_OBJECT	= 0x0000,	 
		UNKNOWN_FORMAT		= 0x0001,	 
		AOD_FORMAT_1_1		= 0x0002,	 
		VRML_FORMAT_1_0		= 0x0003,	 
		BSP_FORMAT_1_1		= 0x0004,	 
		_3DX_FORMAT_1_0		= 0x0005,	 
		 
	};

protected:

	 
	enum {
		NO_CHECKS		= 0x0000,		 
		CHECK_FILENAME	= 0x0001,		 
		ALL_CHECKS		= 0x0001
	};

public:
	IOData3D( BspObjectList objectlist, const String& filename, int checkflags = ALL_CHECKS );
	~IOData3D() { }

	BspObjectList	getObjectList() const { return m_objectlist; }
	String			getFileName() const { return m_filename; }
	void			setFileName( const String& filename ) { m_filename = filename; }

protected:
	BspObjectList	m_objectlist;	 
	String			m_filename;		 

protected:
	 
	static const char  AOD_SIGNATURE_1_1[];
	static const char VRML_SIGNATURE_1_0[];
	static const char  BSP_SIGNATURE_1_1[];
	static const char _3DX_SIGNATURE_1_0[];
	 

	 
	static const int TEXTLINE_MAX;
	static char	line[];
};


} 




# 14 "../BspLib/InputData3D.h" 2



namespace BspLib  { 


 
 
class InputData3D : public IOData3D {

public:
	 
	enum {
		CONVERT_COLINDXS_TO_RGB	= 0x0001,
		FILTER_SCALE_FACTORS	= 0x0002,
		FILTER_AXES_DIR_SWITCH	= 0x0004,
		FILTER_AXES_EXCHANGE	= 0x0008,
		DO_AXES_EXCHANGE		= FILTER_AXES_DIR_SWITCH | FILTER_AXES_EXCHANGE,
		FORCE_MAXIMUM_EXTENT	= 0x0010,
		ALLOW_N_GONS			= 0x0020,
	};

public:
	InputData3D( BspObjectList objectlist, const char *filename, int format = UNKNOWN_FORMAT );
	virtual ~InputData3D();

	virtual int		ParseObjectData();									 

	int				getObjectFormat() const { return m_objectformat; }	 
	InputData3D*	getRealObject() const { return m_data; }			 

	int				InputDataValid() const { return m_inputok; }		 

	 
	static int		EnableRGBConversion( int enable );
	static int		EnableScaleFactors( int enable );
	static int		EnableAxesChange( int enable );
	static int		EnableMaximumExtent( int enable, double extent );
	static int		EnableAllowNGons( int enable );

	 
	static int		getRGBConversionFlag() { return ( ( PostProcessingFlags & CONVERT_COLINDXS_TO_RGB ) == CONVERT_COLINDXS_TO_RGB ); }
	static int		getScaleFactorsFlag() { return ( ( PostProcessingFlags & FILTER_SCALE_FACTORS ) == FILTER_SCALE_FACTORS ); }
	static int		getAxesChangeFlag() { return ( ( PostProcessingFlags & DO_AXES_EXCHANGE ) == DO_AXES_EXCHANGE ); }
	static int		getEnforceExtentsFlag() { return ( ( PostProcessingFlags & FORCE_MAXIMUM_EXTENT ) == FORCE_MAXIMUM_EXTENT ); }
	static double	getMaxExtents() { return MaximumExtentToForce; }
	static int		getAllowNGonFlag() { return ( ( PostProcessingFlags & ALLOW_N_GONS ) == ALLOW_N_GONS ); }

private:
	int				ReadFileSignature();

protected:
	static dword	PostProcessingFlags;
	static double	MaximumExtentToForce;
	static const char parser_err_str[];

protected:
	int				m_objectformat;
	int				m_inputok;

private:
	InputData3D*	m_data;
};


} 




# 14 "../BspLib/VrmlFile.h" 2



namespace BspLib  { 


class BspObjectList;

 
 
class VrmlFile : public InputData3D {

	friend class BRep;

public:
	VrmlFile( BspObjectList objectlist, const char *filename );
	~VrmlFile();

	int				ParseObjectData();
	int				WriteOutputFile();

private:
	void			ApplySceneTransformations();
	void			EnforceSceneExtents( BoundingBox& unionbox );

private:
	BoundingBox*	m_bboxlist;
};


} 




# 7 "QvState.h" 2

 

class QvState {

  public:

     
    enum StackIndex {
	CameraIndex,
	Coordinate3Index,
	LightIndex,
	MaterialBindingIndex,
	MaterialIndex,
	NormalBindingIndex,
	NormalIndex,
	ShapeHintsIndex,
	Texture2Index,
	Texture2TransformationIndex,
	TextureCoordinate2Index,
	TransformationIndex,

	 
	NumStacks,
    };

    static const char *stackNames[NumStacks];	 

    int		depth;		 
    QvElement	**stacks;	 

 
	BspLib ::VrmlFile *vrmlfile_base;
	QvState( BspLib ::VrmlFile *base );
 

    QvState();
    ~QvState();

     
    void	addElement(StackIndex stackIndex, QvElement *elt);

     
    QvElement *	getTopElement(StackIndex stackIndex)
	{ return stacks[stackIndex]; }

     
    void	push();
    void	pop();

     
    void	popElement(StackIndex stackIndex);

     
    void	print();
};


# 3 "QvTraverse.cpp" 2






 
 
 
 
 
 
 

 
static int indent = 0;
static void
announce(const char *className)
{
    for (int i = 0; i < indent; i++)
	printf("\t");
    printf("Traversing a %s\n", className);
}









 
 
 
 
 

void
QvGroup::traverse(QvState *state)
{
    announce("QvGroup" ) ;
    indent++;
    for (int i = 0; i < getNumChildren(); i++)
	getChild(i)->traverse(state);
    indent--;
}

void
QvLevelOfDetail::traverse(QvState *state)
{
    announce("QvLevelOfDetail" ) ;
    indent++;

     
     
    if (getNumChildren() > 0)
	getChild(0)->traverse(state);

    indent--;
}

void
QvSeparator::traverse(QvState *state)
{
    announce("QvSeparator" ) ;
    state->push();
    indent++;
    for (int i = 0; i < getNumChildren(); i++)
	getChild(i)->traverse(state);
    indent--;
    state->pop();
}

void
QvSwitch::traverse(QvState *state)
{
    announce("QvSwitch" ) ;
    indent++;

    int which = whichChild.value;

    if (which == (-1) )
	;

    else if (which == (-3) )
	for (int i = 0; i < getNumChildren(); i++)
	    getChild(i)->traverse(state);

    else
	if (which < getNumChildren())
	    getChild(which)->traverse(state);

    indent--;
}

void
QvTransformSeparator::traverse(QvState *state)
{
    announce("QvTransformSeparator" ) ;

     
     
     
     

    QvElement *markerElt = new QvElement;
    markerElt->data = this;
    markerElt->type = QvElement::NoOpTransform;
    state->addElement(QvState::TransformationIndex, markerElt);

    indent++;
    for (int i = 0; i < getNumChildren(); i++)
	getChild(i)->traverse(state);
    indent--;

     
    while (state->getTopElement(QvState::TransformationIndex) != markerElt)
	state->popElement(QvState::TransformationIndex);
}

 
 
 
 
 


# 139 "QvTraverse.cpp"


# 150 "QvTraverse.cpp"


void QvCoordinate3::traverse(QvState *state)
{
    announce("QvCoordinate3" ) ;
    QvElement *elt = new QvElement;
    elt->data = this;
    state->addElement(QvState::Coordinate3Index, elt);

 
 



 

	printf( "---------------------------------------\n" );
}
 
 

void	 QvMaterial ::traverse(QvState *state)	{	announce("QvMaterial" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 			MaterialIndex , elt);	} 
void	 QvMaterialBinding ::traverse(QvState *state)	{	announce("QvMaterialBinding" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 		MaterialBindingIndex , elt);	} 
void	 QvNormal ::traverse(QvState *state)	{	announce("QvNormal" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 			NormalIndex , elt);	} 
void	 QvNormalBinding ::traverse(QvState *state)	{	announce("QvNormalBinding" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 		NormalBindingIndex , elt);	} 
void	 QvShapeHints ::traverse(QvState *state)	{	announce("QvShapeHints" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 		ShapeHintsIndex , elt);	} 
void	 QvTextureCoordinate2 ::traverse(QvState *state)	{	announce("QvTextureCoordinate2" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 	TextureCoordinate2Index , elt);	} 
void	 QvTexture2 ::traverse(QvState *state)	{	announce("QvTexture2" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 			Texture2Index , elt);	} 
void	 QvTexture2Transform ::traverse(QvState *state)	{	announce("QvTexture2Transform" ) ;	QvElement *elt = new QvElement;	elt->data = this;	state->addElement(QvState:: 	Texture2TransformationIndex , elt);	} 

void	 QvDirectionalLight ::traverse(QvState *state)	{	announce("QvDirectionalLight" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  DirectionalLight ;	state->addElement(QvState:: 	LightIndex , elt);	} 
void	 QvPointLight ::traverse(QvState *state)	{	announce("QvPointLight" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  PointLight ;	state->addElement(QvState:: 		LightIndex , elt);	} 
void	 QvSpotLight ::traverse(QvState *state)	{	announce("QvSpotLight" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  SpotLight ;	state->addElement(QvState:: 		LightIndex , elt);	} 

void	 QvOrthographicCamera ::traverse(QvState *state)	{	announce("QvOrthographicCamera" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  OrthographicCamera ;	state->addElement(QvState:: 	CameraIndex , elt);	} 
void	 QvPerspectiveCamera ::traverse(QvState *state)	{	announce("QvPerspectiveCamera" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  PerspectiveCamera ;	state->addElement(QvState:: 	CameraIndex , elt);	} 

void	 QvTransform ::traverse(QvState *state)	{	announce("QvTransform" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  Transform ;	state->addElement(QvState:: 	     TransformationIndex , elt);	} 
void	 QvRotation ::traverse(QvState *state)	{	announce("QvRotation" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  Rotation ;	state->addElement(QvState:: 	     TransformationIndex , elt);	} 
void	 QvMatrixTransform ::traverse(QvState *state)	{	announce("QvMatrixTransform" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  MatrixTransform ;	state->addElement(QvState::  TransformationIndex , elt);	} 
void	 QvTranslation ::traverse(QvState *state)	{	announce("QvTranslation" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  Translation ;	state->addElement(QvState::      TransformationIndex , elt);	} 
void	 QvScale ::traverse(QvState *state)	{	announce("QvScale" ) ;	QvElement *elt = new QvElement;	elt->data = this;	elt->type = QvElement::  Scale ;	state->addElement(QvState:: 	     TransformationIndex , elt);	} 

 
 
 
 
 

static void
printProperties(QvState *state)
{
    printf("--------------------------------------------------------------\n");
    state->print();
    printf("--------------------------------------------------------------\n");
}









 
 










 



 

 
 
 
 


# 1 "../BspLib/BRep.h" 1
 
 
 
 
 
 




 




# 1 "../BspLib/Transform2.h" 1
 
 
 
 
 
 




 




namespace BspLib  { 


 
 
class Transform2 {

	friend Transform2 operator *( const Transform2& trafo1, const Transform2& trafo2 );

public:
	Transform2();
	Transform2( const float trafo[3][3] );
	Transform2( const double trafo[3][3] );
	~Transform2() { }

	 
	Vector2		TransformVector2( const Vector2& vec ) const;

	double		Determinant();						 
	int			Inverse( Transform2& inverse );		 

	 
	Transform2&	Concat( const Transform2& cattrafo );
	Transform2&	ConcatR( const Transform2& cattrafo );

	 
	Transform2&	LoadIdentity();
	Transform2&	LoadRotation( double angle );
	Transform2&	LoadScale( double x, double y );
	Transform2&	LoadTranslation( double x, double y );

	 
	Transform2&	Rotate( double angle );
	Transform2&	Scale( double x, double y );
	Transform2&	Translate( double x, double y );

	 
	Transform2&	RotateR( double angle );
	Transform2&	ScaleR( double x, double y );
	Transform2&	TranslateR( double x, double y );

	 
	Vector2		FetchTranslation() const;
	Vector2		ExtractTranslation();

	 
	double *	LinMatrixAccess() { return (double *) m_matrix; }

private:
	 
	double	m_matrix[3][3];
};

 
inline Transform2& Transform2::LoadIdentity()
{
	memset( m_matrix, 0, sizeof( m_matrix ) );

	m_matrix[ 0 ][ 0 ] = 1.0;
	m_matrix[ 1 ][ 1 ] = 1.0;
	m_matrix[ 2 ][ 2 ] = 1.0;

	return *this;
}

 
inline Transform2::Transform2()
{
 
}

 
inline Transform2::Transform2( const double trafo[3][3] )
{
	 
	 
	 
	memcpy( m_matrix, trafo, sizeof( m_matrix ) );
}

 
inline Transform2::Transform2( const float trafo[3][3] )
{
	 
	 
	 
	for ( int i = 0; i < 9; i++ )
		((double *)m_matrix)[ i ] = (double) ((float *)trafo)[ i ];
}


} 




# 16 "../BspLib/BRep.h" 2



 








namespace BspLib  { 


 
 
class BRep : public virtual SystemIO {

public:
	BRep( QvState *state );
	~BRep() { }

	void			BuildFromSpherePrimitive( const QvSphere& spherenode );
	void			BuildFromConePrimitive( const QvCone& spherenode );
	void			BuildFromCubePrimitive( const QvCube& spherenode );
	void			BuildFromCylinderPrimitive( const QvCylinder& spherenode );
	void			BuildFromIndexedFaceSet( const QvIndexedFaceSet& faceset );

public:
	static int		getTriangulation() { return do_triangulation; }
	static void		setTriangulation( int tri ) { do_triangulation = tri; }

	static int		getTessellation() { return tessellation_slices; }
	static void		setTessellation( int tes ) { tessellation_slices = tes; }

	static int		getMaterialFlag() { return use_material_spec; }
	static void		setMaterialFlag( int mfl ) { use_material_spec = mfl; }

	static int		getMirrorTextureVFlag() { return mirror_v_axis; }
	static void		setMirrorTextureVFlag( int mtv ) { mirror_v_axis = mtv; }

private:
	float*			FetchCoordinate3State( int &num );
	float*			FetchTextureCoordinate2State( int &num );
	float*			FetchNormalState( int &num );
	void			FetchTransformationState( Transform3& trafo );
	void			FetchTextureTransformationState( Transform2& trafo );
	int				FetchMaterialState( Material& mat, int indx );
	void			FetchMaterialBindingState( int& binding );
	void			FetchNormalBindingState( int& binding );
	void			FetchShapeHintsState();
	Texture*		CheckTexture2State();
	void			CreateIndexedFace( Texture *texture, int curindex, int numtexindexs, long *texindexs, int v1, int v2, int v3 );
	void			PostProcessObject();

private:
	static int		shapehint_vertexOrdering;
	static int		shapehint_shapeType;
	static int		shapehint_faceType;
	static float	shapehint_creaseAngle;

	static int		use_material_spec;
	static int		mirror_v_axis;
	static int		do_triangulation;
	static int		tessellation_slices;

private:
	QvState*		m_state;
	BspObject*		m_baseobject;
};


} 




# 239 "QvTraverse.cpp" 2


void QvSphere::traverse(QvState *state)
{
	announce("QvSphere" ) ;
	BspLib ::BRep brep( state );
	brep.BuildFromSpherePrimitive( *this );
}

void QvCone::traverse(QvState *state)
{
	announce("QvCone" ) ;
	BspLib ::BRep brep( state );
	brep.BuildFromConePrimitive( *this );
}

void QvCube::traverse(QvState *state)
{
	announce("QvCube" ) ;
	BspLib ::BRep brep( state );
	brep.BuildFromCubePrimitive( *this );
}


void QvCylinder::traverse(QvState *state)
{
	announce("QvCylinder" ) ;
	BspLib ::BRep brep( state );
	brep.BuildFromCylinderPrimitive( *this );
}

void QvIndexedFaceSet::traverse(QvState *state)
{
	announce("QvIndexedFaceSet" ) ;
 




	 
	BspLib ::BRep brep( state );
	brep.BuildFromIndexedFaceSet( *this );
}
 

 
 

void	 QvIndexedLineSet ::traverse(QvState *state)	{	announce("QvIndexedLineSet" ) ;	printProperties(state);	} 
void	 QvPointSet ::traverse(QvState *state)	{	announce("QvPointSet" ) ;	printProperties(state);	} 



 
 
 
 
 

 
void	 QvWWWAnchor ::traverse(QvState *)	{	announce("QvWWWAnchor" ) ;	} 
void	 QvWWWInline ::traverse(QvState *)	{	announce("QvWWWInline" ) ;	} 

 
 
 
 
 

void	 QvInfo ::traverse(QvState *)	{	announce("QvInfo" ) ;	} 
void	  QvUnknownNode ::traverse(QvState *)	{	announce("QvUnknownNode" ) ;	} 

 






