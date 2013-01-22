/*
 * PARSEC HEADER: sys_swap.h
 */

#ifndef _SYS_SWAP_H_
#define _SYS_SWAP_H_


// external functions

void	SYS_SwapPackageHeader( packageheader_s *p );
void	SYS_SwapPFileInfo( pfileinfodisk_s *p );

void	SYS_SwapBdtHeader( BdtHeader *b );
void	SYS_SwapFntHeader( FntHeader *f );
void	SYS_SwapTexHeader( TexHeader *t );
void	SYS_SwapDemHeader( DemHeader *d );
void	SYS_SwapPfgHeader( PfgHeader *p );
void	SYS_SwapPfgTable( int fixsize, dword *geomtab, size_t tabsize );

void	SYS_SwapGenObject( GenObject *g );


#endif // _SYS_SWAP_H_


