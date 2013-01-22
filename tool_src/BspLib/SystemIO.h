//-----------------------------------------------------------------------------
//	BSPLIB HEADER: SystemIO.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _SYSTEMIO_H_
#define _SYSTEMIO_H_

// bsplib header files
#include "BspLibDefs.h"

// windows header files
#if defined( _WINDOWS ) || defined( WIN32 )
#include "windows.h"
#endif


BSPLIB_NAMESPACE_BEGIN


// system specific I/O class; virtual base of all I/O capable classes ---------
//
class SystemIO {

public:

	// possible status codes
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

	// error action flags for critical error handling
	enum {
		CRITERR_ALLOW_RET	= 0x0001,
	};

	// error action flags for file access
	enum {
		CHECK_ERRORS		= 0x0001,
	};

public:
	static void InfoMessage( const char *message );
	static void ErrorMessage( const char *message );
	static void HandleCriticalError( int flags = 0 );
	static void SetProgramName( const char *name );
	static void SetInfoMessageCallback( void (*callb)(const char*) );
	static void SetErrorMessageCallback( void (*callb)(const char*) );
	static void SetCriticalErrorCallback( int (*callb)(int) );

#if defined( _WINDOWS ) || defined( WIN32 )
	static void	SetMainWindowHandle( HWND hwnd ) { hwnd_main = hwnd; }
#endif

public:
	static const char	version_str[];

private:
	static const char	*program_name;
	static void			(*infomessage_callback)(const char*);
	static void			(*errormessage_callback)(const char*);
	static int			(*criticalerror_callback)(int);

#if defined( _WINDOWS ) || defined( WIN32 )
	static HWND			hwnd_main;
#endif


protected:


// file ptr class supporting "resource acquisition is initialization" ----
class FilePtr {

public:
	FilePtr( const char *fname, const char *mode ) { fp = fopen( fname, mode ); }
	~FilePtr() { if ( fp ) fclose( fp ); }

	operator FILE*() { return fp; }

protected:
	FILE *fp;

}; // class SystemIO::FilePtr


// augmented FilePtr -----------------------------------------------------
class FileAccess : public FilePtr {

public:
	FileAccess( const char *fname, const char *mode, int flags = CHECK_ERRORS );
	~FileAccess();

	int ReOpen( const char *fname, const char *mode, int flags = CHECK_ERRORS );
	int	Close();

	int Read( void *buffer, size_t size, size_t count, int flags = 0 );
	int Write( void *buffer, size_t size, size_t count, int flags = 0 );

	char *ReadLine( char *line, int maxlen, int flags = 0 );
	const char *WriteLine( const char *line, int flags = 0 );

	int Status() { return status; }

	char *getFileName() { return filename; }

private:
	void IOError( int errorcode );

private:
	char *filename;
	int  status;

}; // class SystemIO::FileAccess


// string scratch pad ----------------------------------------------------
class StrScratch {

public:
	StrScratch() { line = new char[ 1024 ]; }
	~StrScratch() { delete line; }

	operator char*() { return line; }

private:
	char *line;

}; // class SystemIO::StrScratch


}; // class SystemIO


// quick and dirty string class -----------------------------------------------
//
class String {

public:
	String() { data = NULL; }
	String( const char *src );
	~String() { delete data; }

	String( const String& copyobj );
	String& operator =( const String& copyobj );

	operator char*() { return data; }

	int IsNULL() const { return ( data == NULL ); }
	int IsEmpty() const { return data ? ( *data == '\0' ) : TRUE; }

	int getLength() const { return data ? strlen( data ) : 0; }

private:
	char *data;
};


BSPLIB_NAMESPACE_END


#endif // _SYSTEMIO_H_

