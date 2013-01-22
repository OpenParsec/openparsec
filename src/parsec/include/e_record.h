/*
 * PARSEC HEADER: e_record.h
 */

#ifndef _E_RECORD_H_
#define _E_RECORD_H_


// external functions


void	REC_InitMatrices();

void	REC_RecordActions();
void	REC_RecordFiring();
void	REC_RecordKeys();

int		REC_IsRecordingPackets();
void	REC_RecordRemotePacket( NetPacketExternal* ext_gamepacket );
void	REC_PlayRemotePacket( int session, int packetno );
size_t	REC_PlayRemotePacketBin( NetPacketExternal* ext_gamepacket );
size_t	REC_FetchRemotePacket( NetPacketExternal* ext_gamepacket, int session, int packetno );
void	REC_RecordServerMessage( char *message );

void	Save_StateInfo( FILE *fp );
void	Save_PrepRestore( FILE *fp );
void	Save_MyShipState( FILE *fp );
void	Save_RemoteState( FILE *fp );
void	Save_ObjListShips( FILE *fp );
void	Save_ObjListLasers( FILE *fp );
void	Save_ObjListMissls( FILE *fp );
void	Save_ObjListExtras( FILE *fp );
void	Save_ObjListCustom( FILE *fp );
void	Save_Camera( FILE *fp );
void	Save_PseudoStars( FILE *fp );

void	Record_LaserCreation( LaserObject *laserpo );
void	Record_MissileCreation( MissileObject *missilepo );
void	Record_TargetMissileCreation( TargetMissileObject *missilepo );
void	Record_MineCreation( MineObject *minepo );
void	Record_ExtraCreation( ExtraObject *extrapo );
void	Record_CustomCreation( CustomObject *custompo );
void	Record_EnergyFieldCreation( Vertex3& origin );
void	Record_SpreadFireFiring();
void	Record_LightningActivation();
void	Record_LightningDeactivation();
void	Record_HelixActivation();
void	Record_HelixDeactivation();
void	Record_PhotonActivation();
void	Record_PhotonDeactivation();
void	Record_EmpActivation();
void	Record_EmpDeactivation();
void	Record_EmpCreation();

void	Record_LocalPlayerState();
void	Record_Join();
void	Record_Unjoin( byte flag );

void	Record_MyShipStateWeapons();
void	Record_MyShipStateSpecials();
void	Record_MyShipStateCurDamage();
void	Record_MyShipStateCurEnergy();
void	Record_MyShipStateCurSpeed();
void	Record_MyShipStateNumMissls();
void	Record_MyShipStateNumHomMissls();
void	Record_MyShipStateNumPartMissls();
void	Record_MyShipStateNumMines();
void	Record_MyShipStateMaxDamage();
void	Record_MyShipStateMaxEnergy();
void	Record_MyShipStateMaxSpeed();
void	Record_MyShipStateMaxNumMissls();
void	Record_MyShipStateMaxNumHomMissls();
void	Record_MyShipStateMaxNumPartMissls();
void	Record_MyShipStateMaxNumMines();


#endif // _E_RECORD_H_


