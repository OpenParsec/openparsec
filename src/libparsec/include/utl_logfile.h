/*
 * PARSEC HEADER: utl_logfile.h
 */

#ifndef _UTL_LOGFILE_H_
#define _UTL_LOGFILE_H_

// max. length of temporary buffer --------------------------------------------
//
#define MAX_LOG_BUFFER_LEN		4096

// class for logging information ----------------------------------------------
//
class UTL_LogFile
{
protected:
	char		m_szBuffer[ MAX_LOG_BUFFER_LEN + 1 ];
	size_t		m_nBufferLen;
	FILE*		m_pFile;
	refframe_t	m_LastFlushRefframe;

protected:

	// add an entry to the logfile
	void _AddEntry( const char* szEntry, size_t len2 );

public:
	
	// standard ctor
	UTL_LogFile( const char* szFilename = NULL );

	// standard dtor
	~UTL_LogFile();

	// open the logfile
	int Open( const char* szFilename );

	// printf like adding to the logfile
	int printf( const char *format, ... );

	// flush the buffer to the disk file
	void Flush();
};


// external functions ---------------------------------------------------------
//

// flush all registerd logfiles
void UTL_FlushRegisteredLogfiles();


#endif // !_UTL_LOGFILE_H_
