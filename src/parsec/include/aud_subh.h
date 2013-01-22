/*
 * PARSEC HEADER: aud_subh.h
 */

#ifndef _AUD_SUBH_H_
#define _AUD_SUBH_H_


// ----------------------------------------------------------------------------
// AUDIO SUBSYSTEM (AUD) system-dependent function declarations               -
// ----------------------------------------------------------------------------


// sound effects playing functions

int		AUD_DamageRepaired();
int     AUD_EnergyBoosted();
int     AUD_ExtraCollected( int type );
int     AUD_Laser( GenObject *objectpo );
int     AUD_Missile( GenObject *objectpo );
int     AUD_Mine( GenObject *objectpo );
int     AUD_MineCollision();
int     AUD_Locked();
int     AUD_LowEnergy();
int     AUD_MaxedOut( int type );
int     AUD_PlayerKilled();
int     AUD_Select1();
int     AUD_Select2();
int     AUD_ShipDestroyed( GenObject *objectpo );
int     AUD_Tracking();
int     AUD_EnergyExtra();
int     AUD_Lightning( ShipObject *shippo );
int     AUD_LightningOff( ShipObject *shippo );
int     AUD_TrackingBeep();
int		AUD_ButtonSlided();
int		AUD_SpaceCraftViewerIn();
int		AUD_SpaceCraftViewerOut();
int		AUD_NewPlayerJoined();
int		AUD_SelectLaser( int sellaser );
int		AUD_SelectMissile( int selmissile );
int		AUD_SlideIn( int type );
int		AUD_SlideOut( int type );
int		AUD_Countdown( int num );
int		AUD_StartThrust( int type );
int		AUD_IncomingMissile( int looptime );
int		AUD_BootOnBoardComputer();
int		AUD_StopOnBoardComputer();
int		AUD_StartLaserBeam();
int		AUD_StopLaserBeam();
int		AUD_EnemyShieldHit();
int		AUD_EnemyHullHit();
int		AUD_PlayerShieldHit();
int		AUD_PlayerHullHit();
int		AUD_MineDetector();
int 	AUD_LowShields();
int 	AUD_ActivateAfterBurner();
int 	AUD_DeactivateAfterBurner();
int 	AUD_SwarmMissiles( Vertex3 *origin, GenObject *dummyobj );
int 	AUD_SwarmMissilesOff( GenObject *dummypo );
int 	AUD_PhotonLoading( ShipObject *objectpo );
int 	AUD_PhotonLoadingOff( ShipObject *objectpo );
int 	AUD_PhotonFiring( GenObject* objectpo );
int 	AUD_KillsLeft( int killsleft );
int 	AUD_EmpBlast( ShipObject *shippo, int level );
int 	AUD_YouDidIt();
int 	AUD_YouHaveLost();
int		AUD_LiCommenting( int lcid );
int		AUD_LiBabbling( int lbid );
int		AUD_JimCommenting( int jcid );
int		AUD_JimBabbling( int jbid );
int     AUD_Helix( ShipObject *shippo );
int     AUD_HelixOff( ShipObject *shippo );
int		AUD_Teleporter( GenObject* teleporter );
int		AUD_TeleporterOff( GenObject* teleporter );
int 	AUD_SkillAmazing();
int 	AUD_SkillBrilliant();


// audio api abstraction

int		AUDs_PlayVoice( int voice, int sound );
int 	AUDs_PlayVoiceBuffer( int voice, audiowave_t wave, SoundParams_s *params );
int 	AUDs_SetVoiceVolume( int voice, int volume );
int 	AUDs_LoadWaveFile( int num );
int     AUDs_FreeWaveFile( int num );
int     AUDs_CreateAudioVoice( int voice );
int     AUDs_CreateAudioData( audiowave_t wave );
int     AUDs_WriteAudioData( audiowave_t wave, dword offset, dword count );
int     AUDs_DestroyAudioData( audiowave_t wave );
int     AUDs_OpenVoices( int num );
int     AUDs_CloseVoices();
int     AUDs_SetVoicePanning( int voice, int panning );
int     AUDs_SetVoiceFrequency( int voice, int freq );
int     AUDs_GetVoiceStatus( int voice, int *stopped );
int     AUDs_StopVoice( int voice, int finishloop );
int     AUDs_UpdateAudio();

// cd audio functions

void	AUDs_CDInit();
void	AUDs_CDKill();
int		AUDs_CDOpen( short sCDDrive );
void	AUDs_CDClose();
void	AUDs_CDPlay( short sTrack, short from, short to );
int		AUDs_CDPause();
void	AUDs_CDStop();
int		AUDs_CDResume();
int		AUDs_CDEject();
int		AUDs_CDReadTrackInfo();
int		AUDs_CDHasMedia();
int		AUDs_CDCanPlay( short sTrack );
int		AUDs_CDSetVolume( int nVolume );
int		AUDs_CDGetVolume();
int		AUDs_CDList();
int		AUDs_CDGetTrackLength( short sTrack );

// speech functions

int		AUDs_SpeakString( char *speakstr );

// screen music control functions

int		AUDs_OpenIntroSound();
int		AUDs_CloseIntroSound();
int     AUDs_OpenMenuSound();
int     AUDs_CloseMenuSound();
int		AUDs_OpenCreditsSound();
int		AUDs_CloseCreditsSound();
int     AUDs_OpenGameSound();
int     AUDs_CloseGameSound();

// streaming functions

int		AUDs_PlayAudioStream( char *fname );
int		AUDs_StopAudioStream();

// sound effects memory management

void	AUDs_LoadAllSoundEffects();
void	AUDs_FreeAllSoundEffects();

// volume control

void	AUDs_MinMusicVolume();
void	AUDs_MaxMusicVolume();
void	AUDs_FadeMusicVolume();

// needs to be called regularly

void	AUDs_MaintainSound();

// init functions

int		AUDs_InitSoundSys();
int		AUDs_KillSoundSys();


#endif // _AUD_SUBH_H_


