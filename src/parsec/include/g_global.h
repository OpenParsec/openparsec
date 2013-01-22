/*
 * PARSEC HEADER: g_global.h
 */

#ifndef _G_GLOBAL_H_
#define _G_GLOBAL_H_


// global externals (GAME CODE) ---------------------------

extern int				FireDisable;
extern int				FireRepeat;
extern int				CurGun;
extern int				MissileDisable;
extern int				CurLauncher;

extern int				InFloatingMenu;
extern int				InStarMap;
extern int				InGameLoop;
extern int				ExitGameLoop;

extern dword			CurStreamReplayPos;

extern int				Op_DetailLevel;
extern int				Op_PacketSendFreq;
extern int				Op_FixFrameRate;
extern int				Op_SoundEffects;
extern int				Op_Music;
extern int				Op_Joystick;
extern int				Op_Mouse;

extern int				HelpActive;

extern dword			TargetObjNumber;
extern int				TargetVisible;
extern int				TargetLocked;
extern int				TargetRemId;
extern dword			TargetScreenX;
extern dword			TargetScreenY;

extern int				MaxNumShots;
extern int				NumShots;
extern int				MaxNumMissiles;
extern int				NumMissiles;

extern int				SelectedLaser;
extern int				SelectedMissile;

extern int				HitCurTarget;
extern int				IncomingMissile;


// game control data

extern int				EnergyExtraBoost;
extern int              RepairExtraBoost;

extern int              DumbPackNumMissls;
extern int              HomPackNumMissls;
extern int 				SwarmPackNumMissls;
extern int              ProxPackNumMines;

extern int              ExtraProbability;
extern int              MaxExtraArea;
extern int              MinExtraDist;

extern int              ProbHelixCannon;
extern int              ProbLightningDevice;
extern int				ProbPhotonCannon;

extern int              ProbProximityMine;

extern int              ProbMissilePack;
extern int              ProbDumbMissPack;
extern int              ProbHomMissPack;
extern int				ProbSwarmMissPack;

extern int              ProbRepairExtra;
extern int              ProbAfterburner;
extern int				ProbHoloDecoy;
extern int              ProbInvisibility;
extern int              ProbInvulnerability;
extern int              ProbEnergyField;

extern int              ProbLaserUpgrade;
extern int              ProbLaserUpgrade1;
extern int              ProbLaserUpgrade2;

extern int              ProbEmpUpgrade1;
extern int              ProbEmpUpgrade2;

extern int              MegaShieldStrength;


// fixed/pseudo star structures

// star number of sun
#define SUN_STAR_NO 52

struct fixedstar_s {

	Vertex3	location;
	dword	type;
};

typedef Vertex3			pseudostar_s;

extern pXmatrx			PseudoStarMovement;
extern pseudostar_s		PseudoStars[];
extern int				NumPseudoStars;
extern int				MaxPseudoStars;

extern fixedstar_s		FixedStars[];
extern int				NumFixedStars;


#endif // _G_GLOBAL_H_


