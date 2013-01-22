//-----------------------------------------------------------------------------
//	BSPLIB MODULE: SystemIO.cpp
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

// bsplib header files
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


// define bsplib version string -----------------------------------------------
//
#define BSPLIB_VERSION		"1.48"


// set program's name which is an optional part of error messages -------------
//
void SystemIO::SetProgramName( const char *name )
{
	program_name = name;
}


// set callback for info message output function ------------------------------
//
void SystemIO::SetInfoMessageCallback( void (*callb)(const char*) )
{
	infomessage_callback = callb;
}


// set callback for error message output function -----------------------------
//
void SystemIO::SetErrorMessageCallback( void (*callb)(const char*) )
{
	errormessage_callback = callb;
}


// set callback for critical error handler ------------------------------------
//
void SystemIO::SetCriticalErrorCallback( int (*callb)(int) )
{
	criticalerror_callback = callb;
}


// display informational message (system dependent code encapsulated here) ----
//
void SystemIO::InfoMessage( const char *message )
{
	if ( infomessage_callback != NULL ) {
		// call user supplied handler function
		infomessage_callback( message );

	} else {

#if defined( _WINDOWS ) || defined( WIN32 )
		MessageBox( hwnd_main, message, "BspLib Info", MB_OK | MB_ICONINFORMATION );
#else
		printf( "%s", message );
		fflush( stdout );
#endif

	}
}


// display error message (system dependent code encapsulated here) ------------
//
void SystemIO::ErrorMessage( const char *message )
{
	if ( errormessage_callback != NULL ) {
		// call user supplied handler function
		errormessage_callback( message );

	} else {

#if defined( _WINDOWS ) || defined( WIN32 )
		MessageBox( hwnd_main, message, "BspLib Error", MB_OK | MB_ICONERROR );
#else
		fflush( stdout );
		fprintf( stderr, "%s: %s\n", program_name, message );
		fflush( stderr );
#endif

	}
}


// reaction to critical error -------------------------------------------------
//
void SystemIO::HandleCriticalError( int flags )
{
	int handled = 0;
	if ( criticalerror_callback != NULL ) {
		// give user a chance to handle critical error
		handled = criticalerror_callback( flags );
	}
	// if user failed to handle error exit from program
	if ( !handled || ( ( flags & CRITERR_ALLOW_RET ) == 0 ) )
		exit( EXIT_FAILURE );
}


// open file via FileAccess (resource acquisition is initialization) ----------
//
SystemIO::FileAccess::FileAccess( const char *fname, const char *mode, int flags )
	: SystemIO::FilePtr( fname, mode )
{
	filename = new char[ strlen( fname ) + 1 ];
	strcpy( filename, fname );

	status = ( fp != NULL ) ? SYSTEM_IO_OK : FILE_NOT_FOUND;

	if ( ( flags & CHECK_ERRORS ) && ( status != SYSTEM_IO_OK ) )
		IOError( status );
}


// close FileAccess (resource acquisition is initialization) ------------------
//
SystemIO::FileAccess::~FileAccess()
{
	delete filename;
}


// reopen file ----------------------------------------------------------------
//
int SystemIO::FileAccess::ReOpen( const char *fname, const char *mode, int flags )
{
	if ( status == FILE_CLOSED ) {
		status = ( fopen( fname, mode ) != NULL ) ? SYSTEM_IO_OK : FILE_NOT_FOUND;

		if ( ( flags & CHECK_ERRORS ) && ( status != SYSTEM_IO_OK ) )
			IOError( status );
		return status;
	} else {
		return FILE_ALREADY_OPEN;
	}
}


// close file -----------------------------------------------------------------
//
int SystemIO::FileAccess::Close()
{
	status = ( fclose( fp ) == 0 ) ? FILE_CLOSED : SYSTEM_IO_ERROR;
	return ( status == FILE_CLOSED );
}


// read from file -------------------------------------------------------------
//
int SystemIO::FileAccess::Read( void *buffer, size_t size, size_t count, int flags )
{
	if ( status == SYSTEM_IO_OK ) {
		if ( fread( buffer, size, count, fp ) == count )
			status = SYSTEM_IO_OK;
		else
			status = feof( fp ) ? END_OF_FILE : FILE_READ_ERROR;

		if ( ( flags & CHECK_ERRORS ) && ( status != SYSTEM_IO_OK ) )
			IOError( status );
	} else {
		if ( flags & CHECK_ERRORS ) IOError( status );
	}

	return status;
}


// write to file --------------------------------------------------------------
//
int SystemIO::FileAccess::Write( void *buffer, size_t size, size_t count, int flags )
{
	if ( status == SYSTEM_IO_OK ) {
		status = ( fwrite( buffer, size, count, fp ) == count ) ? SYSTEM_IO_OK : FILE_WRITE_ERROR;

		if ( ( flags & CHECK_ERRORS ) && ( status != SYSTEM_IO_OK ) )
			IOError( status );
	} else {
		if ( flags & CHECK_ERRORS ) IOError( status );
	}

	return status;
}


// read text line from file ---------------------------------------------------
//
char *SystemIO::FileAccess::ReadLine( char *line, int maxlen, int flags )
{
	if ( status == SYSTEM_IO_OK ) {
		if ( fgets( line, maxlen, fp ) != NULL )
			status = SYSTEM_IO_OK;
		else
			status = feof( fp ) ? END_OF_FILE : FILE_READ_ERROR;

		if ( ( flags & CHECK_ERRORS ) && ( status != SYSTEM_IO_OK ) )
			IOError( status );
		return ( status == SYSTEM_IO_OK ) ? line : NULL;
	} else {
		if ( flags & CHECK_ERRORS ) IOError( status );
		return NULL;
	}
}


// write text line to file ----------------------------------------------------
//
const char *SystemIO::FileAccess::WriteLine( const char *line, int flags )
{
	if ( status == SYSTEM_IO_OK ) {
		status = ( fprintf( fp, "%s", line ) != -1 ) ? SYSTEM_IO_OK : FILE_WRITE_ERROR;

		if ( ( flags & CHECK_ERRORS ) && ( status != SYSTEM_IO_OK ) )
			IOError( status );
		return ( status == SYSTEM_IO_OK ) ? line : NULL;
	} else {
		if ( flags & CHECK_ERRORS ) IOError( status );
		return NULL;
	}
}


// I/O error handler ----------------------------------------------------------
//
void SystemIO::FileAccess::IOError( int errorcode )
{
	{
	StrScratch errtext;
	sprintf( errtext, "%s: %s", filename, strerror( errno ) );
	ErrorMessage( errtext );
	}
	HandleCriticalError();
}


// construct String with init string ------------------------------------------
//
String::String( const char *src )
{
	if ( src != NULL ) {
		data = new char[ strlen( src ) + 1 ];
		strcpy( data, src );
	} else {
		data = NULL;
	}
}


// copy constructor for String ------------------------------------------------
//
String::String( const String& copyobj )
{
	if ( copyobj.data != NULL ) {
		data = new char[ strlen( copyobj.data ) + 1 ];
		strcpy( data, copyobj.data );
	} else {
		data = NULL;
	}
}


// assignment operator for String ---------------------------------------------
//
String& String::operator =( const String& copyobj )
{
	if ( &copyobj != this ) {
		delete data;
		if ( copyobj.data != NULL ) {
			data = new char[ strlen( copyobj.data ) + 1 ];
			strcpy( data, copyobj.data );
		} else {
			data = NULL;
		}
	}
	return *this;
}


// pointer to global string containing the program's name ---------------------
//
const char *SystemIO::program_name				= NULL;


// pointers to user supplied callback functions -------------------------------
//
void (*SystemIO::infomessage_callback)(const char*)	= NULL;
void (*SystemIO::errormessage_callback)(const char*)	= NULL;
int  (*SystemIO::criticalerror_callback)(int)	= NULL;


// handle of main window (used only when system is Win32) ---------------------
//
#if defined( _WINDOWS ) || defined( WIN32 )
HWND SystemIO::hwnd_main						= NULL;
#endif


// bsplib version string ------------------------------------------------------
//
const char SystemIO::version_str[]				= BSPLIB_VERSION;


BSPLIB_NAMESPACE_END

//-----------------------------------------------------------------------------
