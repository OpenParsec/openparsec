/*
 * PARSEC HEADER (OBJECT)
 * Object Type Definitions V1.72
 *
 * Copyright (c) Markus Hadwiger 1996-2000
 * All Rights Reserved.
 */

#ifndef _OD_TYPES_H_
#define _OD_TYPES_H_


//NOTE:
// different object type means different data structure and handling.
// there may be many objects of the same type. these are distinguished
// by their object class. there may also be many objects of the same
// class (object class instances).


// numbers of available object types (predefined) -----------------------------
//
#define NUM_SHIP_TYPES			2

#define NUM_LASER_TYPES 		3
#define NUM_MISSILE_TYPES		5

#define NUM_PROJECTILE_TYPES	(NUM_LASER_TYPES+NUM_MISSILE_TYPES)
#define NUM_TARGETMISSILE_TYPES 2

#define NUM_EXTRA_TYPES 		4
#define NUM_MINE_TYPES			1

#define NUM_DISTINCT_OBJTYPES	(NUM_SHIP_TYPES+NUM_PROJECTILE_TYPES+NUM_EXTRA_TYPES)


// object type ids ------------------------------------------------------------
//
#define SHIP1TYPE				0x00000000
#define SHIP2TYPE				0x00000001
#define LASER1TYPE				0x80008102
#define LASER2TYPE				0x80008103
#define LASER3TYPE				0x80008104
#define MISSILE1TYPE			0x00000205
#define MISSILE2TYPE			0x00000206
#define MISSILE3TYPE			0x00000207
#define MISSILE4TYPE			0x00010208
#define MISSILE5TYPE			0x00020209
#define EXTRA1TYPE				0x0000030A
#define EXTRA2TYPE				0x0000030B
#define EXTRA3TYPE				0x0000030C
#define MINE1TYPE				0x0000030D

#define TYPENUMBERMASK			0x000000ff
#define TYPELISTMASK			0x00000f00
#define TYPEFLAGSMASK			0x0000f000
#define TYPECONTROLMASK 		0x000f0000

#define TYPE_ID_INVALID			0xffffffff
#define CLASS_ID_INVALID		0xffffffff

#define PSHIP_LIST_NO			0x00000000
#define LASER_LIST_NO			0x00000100
#define MISSL_LIST_NO			0x00000200
#define EXTRA_LIST_NO			0x00000300
#define CUSTM_LIST_NO			0x00000400

#define TYPEBACKFACEMASK		0x80008000	// no backfaceculling at all
#define TYPETWOSIDEDMASK		0x40004000	// some faces are two sided
#define TYPETRANSPARENTMASK 	0x20002000	// some faces are transparent

#define TYPEMISSILEISSTANDARD	0x00000000
#define TYPEMISSILEISHOMING 	0x00010000
#define TYPEMISSILEISSWARM	 	0x00020000


// type determination macros --------------------------------------------------
//
#define TYPEID_TYPE_SHIP(i)		( ( (i) & TYPELISTMASK ) == PSHIP_LIST_NO )
#define TYPEID_TYPE_LASER(i)	( ( (i) & TYPELISTMASK ) == LASER_LIST_NO )
#define TYPEID_TYPE_MISSILE(i)	( ( (i) & TYPELISTMASK ) == MISSL_LIST_NO )
#define TYPEID_TYPE_EXTRA(i)	( ( (i) & TYPELISTMASK ) == EXTRA_LIST_NO )
#define TYPEID_TYPE_CUSTOM(i)	( ( (i) & TYPELISTMASK ) == CUSTM_LIST_NO )

#define OBJECT_TYPE_SHIP(o)		( ( (o)->ObjectType & TYPELISTMASK ) == PSHIP_LIST_NO )
#define OBJECT_TYPE_LASER(o)	( ( (o)->ObjectType & TYPELISTMASK ) == LASER_LIST_NO )
#define OBJECT_TYPE_MISSILE(o)	( ( (o)->ObjectType & TYPELISTMASK ) == MISSL_LIST_NO )
#define OBJECT_TYPE_EXTRA(o)	( ( (o)->ObjectType & TYPELISTMASK ) == EXTRA_LIST_NO )
#define OBJECT_TYPE_CUSTOM(o)	( ( (o)->ObjectType & TYPELISTMASK ) == CUSTM_LIST_NO )


// owner id for local player --------------------------------------------------
//
#define OWNER_LOCAL_PLAYER		-1


// special target identifiers -------------------------------------------------
//
#define TARGETID_NO_TARGET		((dword)-1)


// generic ship object --------------------------------------------------------
//
struct ShipObject : GenObject {

	int 		CurDamage;
	int 		MaxDamage;

	// fractional part: 65536 == 1.0
	dword		CurDamageFrac;

	int 		CurShield;
	int 		MaxShield;

	int 		CurEnergy;
	int 		MaxEnergy;

	// fractional part: 65536 == 1.0
	dword		CurEnergyFrac;

	int 		CurFuel;
	int 		MaxFuel;

	fixed_t		CurSpeed;
	fixed_t		MaxSpeed;

	//NOTE:
	// speeds are always fixed_t since they are too
	// often used as such to become geomv_t now.

	Vertex3 	DirectionVec;

	int 		BounceCount;
	Vertex3 	BounceVec;

	// counter for animation of explosion
	int 		ExplosionCount;

	// used by particle system to start explosion
	int			DelayExplosion;

	// weapons availability state
	dword		Weapons;

	// weapons active state (same masks as Weapons)
	dword		WeaponsActive;

	// special device states (invulnerability, etc.)
	dword		Specials;

	// absorption factor of mega shield
	int			MegaShieldAbsorption;

	// downcounter for invisibility duration
	int			InvisibilityCount;

	int 		NumMissls;
	int 		MaxNumMissls;

	int 		NumHomMissls;
	int 		MaxNumHomMissls;

	int 		NumPartMissls;
	int 		MaxNumPartMissls;

	int 		NumMines;
	int 		MaxNumMines;

	// ship steering
	bams_t		YawPerRefFrame;
	bams_t		PitchPerRefFrame;
	bams_t		RollPerRefFrame;

	geomv_t		XSlidePerRefFrame;
	geomv_t		YSlidePerRefFrame;

	int			SpeedIncPerRefFrame;
	int			SpeedDecPerRefFrame;

	// firing control
	int			FireRepeatDelay;
	int			FireDisableDelay;
	int			MissileDisableDelay;

	// object camera control
	geomv_t		ObjCamMinDistance;
	geomv_t		ObjCamMaxDistance;
	geomv_t		ObjCamStartDist;
	bams_t		ObjCamStartPitch;
	bams_t		ObjCamStartYaw;
	bams_t		ObjCamStartRoll;

	// weapon info
	dword		Laser1_Class[ 4 ][ 4 ];
	geomv_t		Laser1_X[ 4 ][ 4 ];
	geomv_t		Laser1_Y[ 4 ][ 4 ];
	geomv_t		Laser1_Z[ 4 ][ 4 ];

	dword		Missile1_Class[ 4 ];
	geomv_t		Missile1_X[ 4 ];
	geomv_t		Missile1_Y[ 4 ];
	geomv_t		Missile1_Z[ 4 ];

	dword		Missile2_Class[ 4 ];
	geomv_t		Missile2_X[ 4 ];
	geomv_t		Missile2_Y[ 4 ];
	geomv_t		Missile2_Z[ 4 ];

	geomv_t		Mine1_X;
	geomv_t		Mine1_Y;
	geomv_t		Mine1_Z;

	fixed_t		SpreadSpeed;
	int         SpreadLifeTime;

	geomv_t		Spread_X[ 4 ];
	geomv_t		Spread_Y;
	geomv_t		Spread_Z;

	fixed_t		HelixSpeed;
	int         HelixLifeTime;
	bams_t		HelixCurBams;
	refframe_t	helix_refframes_delta;

	geomv_t		Helix_X;
	geomv_t		Helix_Y;
	geomv_t		Helix_Z;

    fixed_t     PhotonSpeed;
    int         PhotonLifeTime;

	refframe_t	EmpRefframesDelta;

    geomv_t     Beam_X[ 4 ];
	geomv_t		Beam_Y;
	geomv_t		Beam_Z;

	int			FumeFreq;
	fixed_t		FumeSpeed;
	int			FumeLifeTime;
	int			FumeCount;

	geomv_t		Fume_X[ 4 ];
	geomv_t		Fume_Y;
	geomv_t		Fume_Z;

	GenObject*	Orbit;

	// afterburner info per ship
	int			afterburner_previous_speed;
	int			afterburner_active;
	refframe_t	afterburner_energy;
};

// object type "Ship_1" (Objecttype #00)
struct Ship1Obj : ShipObject {

};

// object type "Ship_2" (Objecttype #01)
struct Ship2Obj : ShipObject {

};


// generic projectile object --------------------------------------------------
//
struct ProjectileObject : GenObject {

	int 		LifeTime;
	int 		LifeTimeCount;
	Vertex3 	DirectionVec;
	fixed_t		Speed;
	dword		HitPoints;
	int 		Owner;		// OWNER_LOCAL_PLAYER means local player is owner
	Vertex3		PrevPosition;

};


// generic laser object -------------------------------------------------------
//
struct LaserObject : ProjectileObject {

	int 		EnergyNeeded;

};

// object type "Laser_1" (Objecttype #02)
struct Laser1Obj : LaserObject {

};

// object type "Laser_2" (Objecttype #03)
struct Laser2Obj : LaserObject {

};

// object type "Laser_3" (Objecttype #04)
struct Laser3Obj : LaserObject {

};


// generic missile object -----------------------------------------------------
//
struct MissileObject : ProjectileObject {

};


// object type "Missile_1" (Objecttype #05)
struct Missile1Obj : MissileObject {

};

// object type "Missile_2" (Objecttype #06)
struct Missile2Obj : MissileObject {

};

// object type "Missile_3" (Objecttype #07)
struct Missile3Obj : MissileObject {

};


// generic missile object with targeting --------------------------------------
//
struct TargetMissileObject : MissileObject {

	dword		TargetObjNumber;
	GenObject*	TargetObjPointer;
	dword		Latency;
	bams_t		MaxRotation;

};

// object type "Missile_4" (Objecttype #08)
struct Missile4Obj : TargetMissileObject {

};

// object type "Missile_5" (Objecttype #09)
struct Missile5Obj : TargetMissileObject {

};


// generic extra object -------------------------------------------------------
//
struct ExtraObject : GenObject {

	int 		LifeTime;
	int 		LifeTimeCount;
	bams_t		SelfRotX;
	bams_t		SelfRotY;
	bams_t		SelfRotZ;
	int			DriftTimeout;
	Vector3		DriftVec;
	refframe_t	VisibleFrame_Reset_Frames;		

};

// object type "Extra_1" (Objecttype #0A)
struct Extra1Obj : ExtraObject {

	int 		EnergyBoost;

};

// object type "Extra_2" (Objecttype #0B)
struct Extra2Obj : ExtraObject {

	int 		MissileType;
	int 		NumMissiles;

};

// object type "Extra_3" (Objecttype #0C)
struct Extra3Obj : ExtraObject {

	int 		DeviceType;
	int 		DeviceSpecials1;
	int 		DeviceSpecials2;
	int 		DeviceSpecials3;
	int 		DeviceSpecials4;

};


// generic mine object --------------------------------------------------------
//
struct MineObject : ExtraObject {

	int 		HitPoints;
	int 		Owner;		// OWNER_LOCAL_PLAYER means local player is owner

};

// object type "Mine_1" (Objecttype #0D) [counts as extra object!]
struct Mine1Obj : MineObject {

};


// custom (application specific object) ---------------------------------------
//
struct CustomObject : GenObject {

	void		(*callback_instant)( CustomObject *base );
	void		(*callback_destroy)( CustomObject *base );

	int			(*callback_animate)( CustomObject *base );
	int			(*callback_collide)( CustomObject *base );

	void		(*callback_notify)( CustomObject *base, GenObject *genobj, int event );
	int			(*callback_persist)( CustomObject* base, int ToStream, void* relist );
};


// planet object --------------------------------------------------------------
//
struct PlanetObject : CustomObject {

};


#endif // _OD_TYPES_H_


