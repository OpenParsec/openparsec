/*
 * PARSEC HEADER: e_loader.h
 */

#ifndef _E_LOADER_H_
#define _E_LOADER_H_


// external functions

void	LoadData( char *cfn, int dispinfo );
void	ReloadResData( char *cfn );
void	ReloadDetailData( char *cfn );
void	FreeLoaderDataBuffers();
void	ShowFilesLoaded( int flag );


#endif // _E_LOADER_H_


