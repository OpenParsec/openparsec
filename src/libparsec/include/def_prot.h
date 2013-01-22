/*
 * PARSEC HEADER: def_prot.h
 */

int		PFX( Connect,		() );
int		PFX( Disconnect,	() );
int		PFX( Join,			() );
int		PFX( Unjoin,		( byte flag ) );
int		PFX( UpdateName,	() );
void	PFX( MaintainNet,	() );

// protocol api - game functions

size_t	PFX( RmEvList_GetMaxSize, () );
void	PFX( UpdateKillStats, ( RE_KillStats* killstats ) );

// protocol api - packet handling functions 

size_t	PFX( HandleOutPacket,				( const NetPacket*			gamepacket,		NetPacketExternal*	ext_gamepacket ) );
size_t	PFX( HandleOutPacket_DEMO,			( const NetPacket*			gamepacket,		NetPacketExternal*	ext_gamepacket ) );
int		PFX( HandleInPacket,				( const NetPacketExternal*	ext_gamepacket,	const int ext_pktlen, NetPacket* gamepacket ) );
int		PFX( HandleInPacket_DEMO,			( const NetPacketExternal*	ext_gamepacket,						  NetPacket* gamepacket, size_t* psize_external ) );
size_t	PFX( NetPacketExternal_DEMO_GetSize,( const NetPacketExternal*  ext_gamepacket ) ); 
void    PFX( StdGameHeader,					( byte command,	NetPacket* pIntPkt ) );
void	PFX( WritePacketInfo,				( FILE *fp, NetPacketExternal* ext_gamepacket ) );

