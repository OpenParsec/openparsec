/*
 * PARSEC HEADER: aud_game.h
 */

#ifndef _AUD_GAME_H_
#define _AUD_GAME_H_


// aud_game.c implements the following functions
// ---------------------------------------------
//	int		AUD_DamageRepaired();
//	int     AUD_EnergyBoosted();
//	int     AUD_ExtraCollected( int type );
//	int     AUD_Laser( GenObject *objectpo );
//	int     AUD_Missile( GenObject *objectpo );
//	int     AUD_Mine( GenObject *objectpo );
//	int     AUD_Locked();
//	int     AUD_LowEnergy();
//	int     AUD_MaxedOut( int type );
//	int     AUD_MineCollision();
//	int     AUD_PlayerKilled();
//	int     AUD_Select1();
//	int     AUD_Select2();
//	int     AUD_ShipDestroyed( GenObject *objectpo );
//	int     AUD_Tracking();
//	int     AUD_EnergyExtra();
//	int     AUD_Lightning( ShipObject *shippo );
//	int     AUD_LightningOff( ShipObject *shippo );
//	int     AUD_TrackingBeep();
//  int		AUD_ButtonSlided();
//  int		AUD_SpaceCraftViewerIn();
//  int		AUD_SpaceCraftViewerOut();
//  int		AUD_NewPlayerJoined();
//  int		AUD_SelectLaser( int sellaser );
//  int		AUD_SelectMissile( int selmissile );
//  int		AUD_SlideIn( int type );
//  int		AUD_SlideOut( int type );
//	int		AUD_Countdown( int num );
//	int		AUD_StartThrust();
//	int		AUD_IncomingMissile( int looptime );
//	int		AUD_BootOnBoardComputer();
//	int		AUD_StopOnBoardComputer();
//	int		AUD_StartLaserBeam();
//	int		AUD_StopLaserBeam();
//	int		AUD_EnemyShieldHit();
//	int		AUD_EnemyHullHit();
//	int		AUD_PlayerShieldHit();
//	int		AUD_PlayerHullHit();
//	int		AUD_MineDetector();
//	int 	AUD_LowShields();
//	int 	AUD_ActivateAfterBurner();
//	int 	AUD_DeactivateAfterBurner();
//	int 	AUD_SwarmMissiles( Vertex3 *origin, GenObject *dummyobj );
//	int 	AUD_SwarmMissilesOff( GenObject *dummypo );
//	int 	AUD_PhotonLoading( ShipObject *objectpo );
//	int 	AUD_PhotonLoadingOff( ShipObject *objectpo );
//	int 	AUD_PhotonFiring( GenObject* objectpo );
//	int 	AUD_KillsLeft( int killsleft );
//	int 	AUD_EmpBlast( ShipObject *shippo, int level );
//	int 	AUD_YouDidIt();
//	int 	AUD_YouHaveLost();
//	int		AUD_LiCommenting( int lcid );
//	int		AUD_LiBabbling( int lbid );
//	int		AUD_JimCommenting( int jcid );
//	int		AUD_JimBabbling( int jbid );
//	int     AUD_Helix( ShipObject *shippo );
//	int     AUD_HelixOff( ShipObject *shippo );
//  int		AUD_Teleporter( GenObject* teleporter );
//	int		AUD_TeleporterOff( GenObject* teleporter );
//	int 	AUD_SkillAmazing();
//	int 	AUD_SkillBrilliant();


// voice enumeration ----------------------------------------------------------
//
#define STD_VOICE					2
#define TRACKING_VOICE				3
#define LOCKED_VOICE				4
#define LOWENERGY_VOICE	   			5
#define EXPLOSION_VOICE				6
#define DESTROYED_VOICE				7
#define LASER1_VOICE				8
#define MISSILE1_VOICE				9
#define MISSILE2_VOICE				10
#define LIGHTNING_VOICE				11
#define TRACKBEEP_VOICE				12
#define BACKGROUND_VOICE			13
#define ENERGYBOOSTED_VOICE 		14
#define NEWPLAYERJOINED_VOICE		15
#define ENEMY_HIT_VOICE     		16
#define GUN_SELECT_VOICE			17
#define MISS_SELECT_VOICE   		18
#define THRUST_VOICE				19
#define SLIDETHRUST_VOICE			20
#define HELIX_VOICE					21
#define COUNTDOWN_VOICE				22
#define ANNOUNCE_VOICE				23
#define AFTERBURNER_VOICE			24
#define PHOTON_CANNON_VOICE			25
#define SWARM_MISSILE_VOICE			26
#define EMP_VOICE					27


// sound effects enumeration --------------------------------------------------
//
enum {

	SND_SELECT1,
	SND_SELECT2,
	SND_SHIPDESTROYED,
	SND_NEWPLAYER,
	SND_PLAYERLEFT,
	SND_LOWENERGY,
	SND_TRACKING,
	SND_TARGETLOCKED,
	SND_TRACKINGBEEP,
	SND_EXPLOSION,
	SND_LASER1,
	SND_MISSILE1,
	SND_MISSILE2,
	SND_ENERGYEXTRA,
	SND_DROPMINE,
	SND_LASER2,
	SND_LIGHTNING,
	SND_MINEHIT,
};


// sound effects names --------------------------------------------------------
//
#define SND_NAME_AFTERBURNER_COLLECTED		"afterb collected"
#define SND_NAME_BUTTONSLIDED				"buttonslided"
#define SND_NAME_DAMAGE_REPAIRED			"damage repaired"
#define SND_NAME_DAZZLING_COLLECTED			"dazzling collected"
#define SND_NAME_DROPMINE					"drop mine"
#define SND_NAME_DUMB_MISSILE_COLLECTED		"dmiss collected"
#define SND_NAME_ENERGYEXTRA				"get energy"
#define SND_NAME_EXPLOSION					"explosion"
#define SND_NAME_GUIDE_MISSILE_COLLECTED	"gmiss collected"
#define SND_NAME_INVISIBILITY_COLLECTED		"invis collected"
#define SND_NAME_INVUL_COLLECTED			"invul collected"
#define SND_NAME_LASER1						"laser level 1"
#define SND_NAME_LASER2						"laser level 2"
#define SND_NAME_LASER3						"laser level 3"
#define SND_NAME_LIGHTNING					"lightning beam"
#define SND_NAME_LOWENERGY					"low energy"
#define SND_NAME_MAXED_OUT					"maxed out"
#define SND_NAME_MINEHIT					"mine hit"
#define SND_NAME_MINE_COLLECTED				"mine collected"
#define SND_NAME_MISSILE1					"dumb missile"
#define SND_NAME_MISSILE2					"guided missile"
#define SND_NAME_HELIX						"helix"
#define SND_NAME_NEWPLAYER					"new player"
#define SND_NAME_OPTIONS_SLIDE_IN			"options in"
#define SND_NAME_OPTIONS_SLIDE_OUT			"options out"
#define SND_NAME_PLAYERLEFT					"player leaves"
#define SND_NAME_SCV_SLIDE_IN				"scv in"
#define SND_NAME_SCV_SLIDE_OUT				"scv out"
#define SND_NAME_SELECT1					"cursor movement"
#define SND_NAME_SELECT2					"option selected"
#define SND_NAME_SELECT_LASER1				"select laser1"
#define SND_NAME_SELECT_LASER2				"select laser2"
#define SND_NAME_SELECT_LASER3				"select laser3"
#define SND_NAME_SELECT_LASER4				"select laser4"
#define SND_NAME_SELECT_LASER5				"select laser5"
#define SND_NAME_SELECT_MISSILE1			"select missile1"
#define SND_NAME_SELECT_MISSILE2			"select missile2"
#define SND_NAME_SELECT_MISSILE3			"select missile3"
#define SND_NAME_SELECT_MISSILE4			"select missile4"
#define SND_NAME_SHIPDESTROYED				"Ship destroyed"
#define SND_NAME_TARGETLOCKED				"target locked"
#define SND_NAME_THIEF_COLLECTED			"thief collected"
#define SND_NAME_TRACKING					"tracking"
#define SND_NAME_TRACKINGBEEP				"tracker beep"
#define SND_NAME_COUNTDOWN1					"count1"
#define SND_NAME_COUNTDOWN2					"count2"
#define SND_NAME_COUNTDOWN3					"count3"
#define SND_NAME_COUNTDOWN4					"count4"
#define SND_NAME_COUNTDOWN5					"count5"
#define SND_NAME_SWITCH_GUN					"switch gun"
#define SND_NAME_SWITCH_MISSILE				"switch missile"
#define SND_NAME_THRUST						"thrust"
#define SND_NAME_SLIDETHRUST				"slidethrust"
#define SND_NAME_INCOMING					"incoming"
#define SND_NAME_BOOTING					"booting"
#define SND_NAME_LASERBEAM					"laserbeam"
#define SND_NAME_ENEMYSHIELD_HIT			"enemy hit"
#define SND_NAME_ENEMYHULL_HIT				"enemyhull hit"
#define SND_NAME_PLAYERSHIELD_HIT			"playershield hit"
#define SND_NAME_PLAYERHULL_HIT				"playerhull hit"
#define SND_NAME_LOW_SHIELDS				"low shields"
#define SND_NAME_ONE_KILL_LEFT				"one kill left"
#define SND_NAME_TWO_KILLS_LEFT				"two kills left"
#define SND_NAME_THREE_KILLS_LEFT			"two kills left"
#define SND_NAME_JIM_COMMENT_AFRAID_1		"jim afraid 1"
#define SND_NAME_JIM_COMMENT_AFRAID_2		"jim afraid 2"
#define SND_NAME_JIM_COMMENT_ANGRY_1		"jim angry 1"
#define SND_NAME_JIM_COMMENT_ANGRY_2		"jim angry 2"
#define SND_NAME_JIM_COMMENT_WEAPON_1		"jim weapon 1"
#define SND_NAME_JIM_COMMENT_WEAPON_2		"jim weapon 2"
#define SND_NAME_JIM_COMMENT_WEAPON_3		"jim weapon 3"
#define SND_NAME_JIM_COMMENT_WEAPON_4		"jim weapon 4"
#define SND_NAME_JIM_COMMENT_KILLED_1		"jim killed 1"
#define SND_NAME_JIM_COMMENT_KILLED_2		"jim killed 2"
#define SND_NAME_JIM_COMMENT_EAT_THIS_1		"jim eat this 1"
#define SND_NAME_JIM_COMMENT_EAT_THIS_2		"jim eat this 2"
#define SND_NAME_JIM_COMMENT_ENERGY			"jim energy"
#define SND_NAME_JIM_COMMENT_BORED_1		"jim bored 1"
#define SND_NAME_JIM_COMMENT_BORED_2		"jim bored 2"
#define SND_NAME_DECOY_COLLECTED			"decoy collected"
#define SND_NAME_LASER_UPGRADE_COLLECTED	"laser upgrade collected"
#define SND_NAME_PHOTON_CANNON_COLLECTED	"photon cannon collected"
#define SND_NAME_SWARM_MISSILES_COLLECTED	"swarm missiles collected"
#define SND_NAME_SKILL_AMAZING				"skill amazing"
#define SND_NAME_SKILL_BRILLIANT			"skill brilliant"
#define SND_NAME_AFTERBURNER_ACTIVATE		"afterburner activate"
#define SND_NAME_AFTERBURNER_DEACTIVATE		"afterburner deactivate"
#define SND_NAME_PHOTON_LOADING				"photon loading"
#define SND_NAME_PHOTON_FIRING				"photon firing"
#define SND_NAME_SWARM_MISSILES				"swarm missiles"
#define SND_NAME_YOU_DID_IT					"you did it"
#define SND_NAME_YOU_HAVE_LOST				"you have lost"
#define SND_NAME_MINE_DETECTOR				"mine detector"
#define SND_NAME_EMP_1						"emp1"
#define SND_NAME_EMP_2						"emp2"
#define SND_NAME_EMP_3						"emp3"
#define SND_NAME_TELEPORTER					"teleporter"

// distances for max. volume on distance based sound effects ------------------
//
#define MAX_VOLUME_DISTANCE_VISIBLE			4096
#define MAX_VOLUME_DISTANCE_TELEPORTER		500

#endif // _AUD_GAME_H_


