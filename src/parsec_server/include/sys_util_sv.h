/*
* PARSEC HEADER: sys_util_sv.h
*/

#ifndef _SYS_UTIL_SV_H_
#define _SYS_UTIL_SV_H_

// ----------------------------------------------------------------------------
// SERVER SYSTEM SUBSYSTEM (SYS) UTILITY FUNCTIONS
// ----------------------------------------------------------------------------

void SYSs_InstallSignalHandlers();
void SYSs_CleanUp();

void SYS_HexDump( char* data, int len );

#endif //_SYS_UTIL_SV_H_

