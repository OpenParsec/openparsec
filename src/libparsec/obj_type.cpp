/*
 * PARSEC - Object Type Management
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:40 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"
#include "od_props.h"

// global externals
#include "globals.h"

// local module header
#include "obj_type.h"

// proprietary module headers
#include "obj_cust.h"
#ifdef PARSEC_SERVER
	#include "g_main_sv.h"
#else // !PARSEC_SERVER
	#include "obj_game.h"
#endif // !PARSEC_SERVER



// flags
#define ALLOW_ONLY_KNOWN_TYPES



// object type information tables  --------------------------------------------
//

// type init structure for ShipObject
struct ShipObj_TI {

	int 	MaxDamage;
	int 	MaxShield;
	fixed_t	MaxSpeed;
	int 	MaxEnergy;
	int 	MaxFuel;
	int 	Weapons;
	int 	Specials;
	int 	MaxNumMissls;
	int 	MaxNumHomMissls;
	int 	MaxNumPartMissls;
	int 	MaxNumMines;
	bams_t	YawPerRefFrame;
	bams_t	PitchPerRefFrame;
	bams_t	RollPerRefFrame;
	geomv_t	XSlidePerRefFrame;
	geomv_t	YSlidePerRefFrame;
	int		SpeedIncPerRefFrame;
	int		SpeedDecPerRefFrame;
	int		FireRepeatDelay;
	int		FireDisableDelay;
	int		MissileDisableDelay;
	geomv_t	ObjCamMinDistance;
	geomv_t	ObjCamMaxDistance;
	geomv_t	ObjCamStartDist;
	bams_t	ObjCamStartPitch;
	bams_t	ObjCamStartYaw;
	bams_t	ObjCamStartRoll;

	dword	Laser1_Class[ 4 ][ 4 ];
	geomv_t	Laser1_X[ 4 ][ 4 ];
	geomv_t	Laser1_Y[ 4 ][ 4 ];
	geomv_t	Laser1_Z[ 4 ][ 4 ];

	dword	Missile1_Class[ 4 ];
	geomv_t	Missile1_X[ 4 ];
	geomv_t	Missile1_Y[ 4 ];
	geomv_t	Missile1_Z[ 4 ];

	dword	Missile2_Class[ 4 ];
	geomv_t	Missile2_X[ 4 ];
	geomv_t	Missile2_Y[ 4 ];
	geomv_t	Missile2_Z[ 4 ];

	geomv_t	Mine1_X;
	geomv_t	Mine1_Y;
	geomv_t	Mine1_Z;

	geomv_t	Spread_X[ 4 ];
	geomv_t	Spread_Y;
	geomv_t	Spread_Z;

	geomv_t	Helix_X;
	geomv_t	Helix_Y;
	geomv_t	Helix_Z;

	geomv_t	Beam_X[ 4 ];
	geomv_t	Beam_Y;
	geomv_t	Beam_Z;

	geomv_t	Fume_X[ 4 ];
	geomv_t	Fume_Y;
	geomv_t	Fume_Z;

};

// type init structure for ProjectileObject
struct ProjectileObj_TI {

	int 	LifeTimeCount;
	fixed_t	Speed;
	dword	HitPoints;

};

// type init structure for TargetMissileObject
struct TargetMissObj_TI {

	dword	Latency;
	bams_t	MaxRotation;

};

// type init structure for ExtraObject
struct ExtraObj_TI {

	int 	LifeTimeCount;
	bams_t	SelfRotX;
	bams_t	SelfRotY;
	bams_t	SelfRotZ;

};

// type init structure for MineObject
struct MineObj_TI {

	int 	HitPoints;
	int 	LifeTimeCount;
	bams_t	SelfRotX;
	bams_t	SelfRotY;
	bams_t	SelfRotZ;

};


// type property init tables

ShipObj_TI ShipObj_TIs[ NUM_SHIP_TYPES ] = {

	{
	SHIP1_MAXDAMAGE, SHIP1_MAXSHIELD, SHIP1_MAX_SPEED, SHIP1_MAX_ENERGY,
	SHIP1_MAX_FUEL, SHIP1_WEAPONS, 0,
	SHIP1_NUM_MISSLS, SHIP1_NUM_HOMMISSLS, SHIP1_NUM_SWARMMISSLS, SHIP1_NUM_MINES,
	SHIP_YAW_PER_REFFRAME, SHIP_PITCH_PER_REFFRAME, SHIP_ROLL_PER_REFFRAME,
	SHIP_SLIDE_PER_REFFRAME, SHIP_SLIDE_PER_REFFRAME, SHIP_SPEED_INC_PER_REFFRAME,
	SHIP_SPEED_DEC_PER_REFFRAME, SHIP_FIRE_REPEAT_DELAY, SHIP_FIRE_DISABLE_DELAY,
	SHIP_MISSILE_DISABLE_DELAY,
	SHIP1_OBJCAM_MINDISTANCE, SHIP1_OBJCAM_MAXDISTANCE, SHIP1_OBJCAM_STARTDIST,
	SHIP1_OBJCAM_STARTPITCH, SHIP1_OBJCAM_STARTYAW, SHIP1_OBJCAM_STARTROLL,
	{ { LASER0_CLASS_1, LASER0_CLASS_2, LASER0_CLASS_2, LASER0_CLASS_1 },
	  { LASER1_CLASS_1, LASER1_CLASS_1, LASER1_CLASS_1, LASER1_CLASS_1 },
	  { LASER2_CLASS_1, LASER2_CLASS_1, LASER2_CLASS_1, LASER2_CLASS_1 },
	  { 0, 0, 0, 0 } },
	{ { LASER1_START_X_1, LASER1_START_X_2, LASER1_START_X_3, LASER1_START_X_4 },
	  { LASER1_START_X_1, LASER1_START_X_2, LASER1_START_X_3, LASER1_START_X_4 },
	  { LASER1_START_X_1, LASER1_START_X_2, LASER1_START_X_3, LASER1_START_X_4 },
	  { LASER1_START_X_1, LASER1_START_X_2, LASER1_START_X_3, LASER1_START_X_4 } },
	{ { LASER1_START_Y, LASER1_START_Y, LASER1_START_Y, LASER1_START_Y },
	  { LASER1_START_Y, LASER1_START_Y, LASER1_START_Y, LASER1_START_Y },
	  { LASER1_START_Y, LASER1_START_Y, LASER1_START_Y, LASER1_START_Y },
	  { LASER1_START_Y, LASER1_START_Y, LASER1_START_Y, LASER1_START_Y } },
	{ { LASER1_START_Z, LASER1_START_Z, LASER1_START_Z, LASER1_START_Z },
	  { LASER1_START_Z, LASER1_START_Z, LASER1_START_Z, LASER1_START_Z },
	  { LASER1_START_Z, LASER1_START_Z, LASER1_START_Z, LASER1_START_Z },
	  { LASER1_START_Z, LASER1_START_Z, LASER1_START_Z, LASER1_START_Z } },
	DUMB_CLASS_1, DUMB_CLASS_1, DUMB_CLASS_1, DUMB_CLASS_1,
	MISSILE1_START_X_1, MISSILE1_START_X_2, MISSILE1_START_X_3, MISSILE1_START_X_4,
	MISSILE1_START_Y, MISSILE1_START_Y, MISSILE1_START_Y, MISSILE1_START_Y,
	MISSILE1_START_Z, MISSILE1_START_Z, MISSILE1_START_Z, MISSILE1_START_Z,
	GUIDE_CLASS_1, GUIDE_CLASS_1, GUIDE_CLASS_1, GUIDE_CLASS_1,
	HOMMISS1_START_X_1, HOMMISS1_START_X_2, HOMMISS1_START_X_3,
	HOMMISS1_START_X_4, HOMMISS1_START_Y, HOMMISS1_START_Z,
	MINE1_START_X, MINE1_START_Y, MINE1_START_Z,
	SPREAD_START_X_1, SPREAD_START_X_2, SPREAD_START_X_3,
	SPREAD_START_X_4, SPREAD_START_Y, SPREAD_START_Z,
	HELIX_START_X, HELIX_START_Y, HELIX_START_Z,
	BEAM_START_X_1, BEAM_START_X_2, BEAM_START_X_3,
	BEAM_START_X_4,	BEAM_START_Y, BEAM_START_Z,
	PROP_FUME_START_X1, PROP_FUME_START_X2, PROP_FUME_START_X3,
	PROP_FUME_START_X4, PROP_FUME_START_Y, PROP_FUME_START_Z,
	},

	{
	SHIP2_MAXDAMAGE, SHIP2_MAXSHIELD, SHIP2_MAX_SPEED, SHIP2_MAX_ENERGY,
	SHIP2_MAX_FUEL, SHIP2_WEAPONS, 0,
	SHIP2_NUM_MISSLS, SHIP2_NUM_HOMMISSLS, SHIP2_NUM_SWARMMISSLS, SHIP2_NUM_MINES,
	SHIP_YAW_PER_REFFRAME, SHIP_PITCH_PER_REFFRAME, SHIP_ROLL_PER_REFFRAME,
	SHIP_SLIDE_PER_REFFRAME, SHIP_SLIDE_PER_REFFRAME, SHIP_SPEED_INC_PER_REFFRAME,
	SHIP_SPEED_DEC_PER_REFFRAME, SHIP_FIRE_REPEAT_DELAY, SHIP_FIRE_DISABLE_DELAY,
	SHIP_MISSILE_DISABLE_DELAY,
	SHIP2_OBJCAM_MINDISTANCE, SHIP2_OBJCAM_MAXDISTANCE, SHIP2_OBJCAM_STARTDIST,
	SHIP2_OBJCAM_STARTPITCH, SHIP2_OBJCAM_STARTYAW, SHIP2_OBJCAM_STARTROLL,
	{ { LASER0_CLASS_1, LASER0_CLASS_2, LASER0_CLASS_2, LASER0_CLASS_1 },
	  { LASER1_CLASS_1, LASER1_CLASS_1, LASER1_CLASS_1, LASER1_CLASS_1 },
	  { LASER2_CLASS_1, LASER2_CLASS_1, LASER2_CLASS_1, LASER2_CLASS_1 },
	  { 0, 0, 0, 0 } },
	{ { _2_LASER1_START_X_1, _2_LASER1_START_X_2, _2_LASER1_START_X_3, _2_LASER1_START_X_4 },
	  { _2_LASER1_START_X_1, _2_LASER1_START_X_2, _2_LASER1_START_X_3, _2_LASER1_START_X_4 },
	  { _2_LASER1_START_X_1, _2_LASER1_START_X_2, _2_LASER1_START_X_3, _2_LASER1_START_X_4 },
	  { _2_LASER1_START_X_1, _2_LASER1_START_X_2, _2_LASER1_START_X_3, _2_LASER1_START_X_4 } },
	{ { _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y },
	  { _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y },
	  { _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y },
	  { _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y, _2_LASER1_START_Y } },
	{ { _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z },
	  { _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z },
	  { _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z },
	  { _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z, _2_LASER1_START_Z } },
	DUMB_CLASS_1, DUMB_CLASS_1, DUMB_CLASS_1, DUMB_CLASS_1,
	_2_MISSILE1_START_X_1, _2_MISSILE1_START_X_2, _2_MISSILE1_START_X_3, _2_MISSILE1_START_X_4,
	_2_MISSILE1_START_Y, _2_MISSILE1_START_Y, _2_MISSILE1_START_Y, _2_MISSILE1_START_Y,
	_2_MISSILE1_START_Z, _2_MISSILE1_START_Z, _2_MISSILE1_START_Z, _2_MISSILE1_START_Z,
	GUIDE_CLASS_1, GUIDE_CLASS_1, GUIDE_CLASS_1, GUIDE_CLASS_1,
	HOMMISS1_START_X_1, HOMMISS1_START_X_2, HOMMISS1_START_X_3,
	HOMMISS1_START_X_4, HOMMISS1_START_Y, HOMMISS1_START_Z,
	_2_MINE1_START_X, _2_MINE1_START_Y, _2_MINE1_START_Z,
	_2_SPREAD_START_X_1, _2_SPREAD_START_X_2, _2_SPREAD_START_X_3,
	_2_SPREAD_START_X_4,_2_SPREAD_START_Y, _2_SPREAD_START_Z,
	_2_HELIX_START_X, _2_HELIX_START_Y, _2_HELIX_START_Z,
	_2_BEAM_START_X_1, _2_BEAM_START_X_2, _2_BEAM_START_X_3,
	_2_BEAM_START_X_4,	_2_BEAM_START_Y, _2_BEAM_START_Z,
	_2_PROP_FUME_START_X1, _2_PROP_FUME_START_X2, _2_PROP_FUME_START_X3,
	_2_PROP_FUME_START_X4, _2_PROP_FUME_START_Y, _2_PROP_FUME_START_Z,
	}

};

ProjectileObj_TI ProjectileObj_TIs[ NUM_PROJECTILE_TYPES ] = {

	{ LASER1_LIFETIME, LASER1_SPEED, LASER1_HITPOINTS },
	{ LASER2_LIFETIME, LASER2_SPEED, LASER2_INTENSITY },
	{ LASER3_LIFETIME, LASER3_SPEED, LASER3_THEFTDAMAGE },
	{ MISSILE1_LIFETIME, MISSILE1_SPEED, MISSILE1_HITPOINTS },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ HOMMISS1_LIFETIME, HOMMISS1_SPEED, HOMMISS1_HITPOINTS },
	{ 0, 0, 0 }

};

TargetMissObj_TI TargetMissObj_TIs[ NUM_TARGETMISSILE_TYPES ] = {

	{ HOMMISS1_LATENCY, HOMMISS1_MAX_ROT },
	{ 0, 0 }

};

ExtraObj_TI ExtraObj_TIs[ NUM_EXTRA_TYPES - NUM_MINE_TYPES ] = {

	{ ENERGYEXTRA_LIFETIME,
	  ENERGYEXTRA_SELFROTX, ENERGYEXTRA_SELFROTY, ENERGYEXTRA_SELFROTZ },
	{ MISSILEEXTRA_LIFETIME,
	  MISSILEEXTRA_SELFROTX, MISSILEEXTRA_SELFROTY, MISSILEEXTRA_SELFROTZ },
	{ DEVICEEXTRA_LIFETIME,
	  DEVICEEXTRA_SELFROTX, DEVICEEXTRA_SELFROTY, DEVICEEXTRA_SELFROTZ }

};

MineObj_TI MineObj_TIs[ NUM_MINE_TYPES ] = {

	{ MINE1_HITPOINTS, MINE1_LIFETIME,
	  MINE1_SELFROTX, MINE1_SELFROTY, MINE1_SELFROTZ },

};

int LaserEnergyNeeds[ NUM_LASER_TYPES ] = {

	LASER1_ENERGY, LASER2_ENERGY, LASER3_ENERGY

};


// names of standard object types ---------------------------------------------
//
PUBLIC
const char *objtype_name[ NUM_DISTINCT_OBJTYPES ] = {

	"*Ship_1*",
	"*Ship_2*",
	"*Laser_1*",
	"*Laser_2*",
	"*Laser_3*",
	"*Missile_1*",
	"*Missile_2*",
	"*Missile_3*",
	"*Missile_4*",
	"*Missile_5*",
	"*Extra_1*",
	"*Extra_2*",
	"*Extra_3*",
	"*Mine_1*",
};


// ids of standard object types -----------------------------------------------
//
PUBLIC
dword objtype_id[ NUM_DISTINCT_OBJTYPES ] = {

	SHIP1TYPE,
	SHIP2TYPE,
	LASER1TYPE,
	LASER2TYPE,
	LASER3TYPE,
	MISSILE1TYPE,
	MISSILE2TYPE,
	MISSILE3TYPE,
	MISSILE4TYPE,
	MISSILE5TYPE,
	EXTRA1TYPE,
	EXTRA2TYPE,
	EXTRA3TYPE,
	MINE1TYPE,
};


// structure sizes of standard object types -----------------------------------
//
PRIVATE
size_t objtype_size[ NUM_DISTINCT_OBJTYPES ] = {

	sizeof( Ship1Obj ),
	sizeof( Ship2Obj ),
	sizeof( Laser1Obj ),
	sizeof( Laser2Obj ),
	sizeof( Laser3Obj ),
	sizeof( Missile1Obj ),
	sizeof( Missile2Obj ),
	sizeof( Missile3Obj ),
	sizeof( Missile4Obj ),
	sizeof( Missile5Obj ),
	sizeof( Extra1Obj ),
	sizeof( Extra2Obj ),
	sizeof( Extra3Obj ),
	sizeof( Mine1Obj ),
};


// translate type string to type id -------------------------------------------
//
dword OBJ_FetchTypeIdFromName( const char* typestr )
{
	ASSERT( typestr != NULL );

	//NOTE:
	// if the type name cannot be resolved
	// TYPE_ID_INVALID will be returned.

	// scan basic type string table
	int tid = 0;
	for ( tid = 0; tid < NUM_DISTINCT_OBJTYPES; tid++ ) {

		const char *tname = objtype_name[ tid ];
		size_t slen = strlen( tname );
		if ( ( tname[ 0 ] == '*' ) && ( tname[ slen - 1 ] == '*' ) ) {

			++tname;
			const char *scan = NULL;
			for ( scan = typestr; *tname != '*'; tname++, scan++ )
				if ( tolower( *tname ) != tolower( *scan ) )
					break;
			if ( ( *tname == '*' ) && ( *scan == '\0' ) )
				break;

		} else if ( stricmp( tname, typestr ) == 0 ) {

			break;
		}
	}

	if ( tid < NUM_DISTINCT_OBJTYPES ) {

		// found basic type
		return objtype_id[ tid ];

	} else {

		// try custom type
		return OBJ_FetchCustomTypeId( typestr );
	}
}


// fetch instance size of specified type --------------------------------------
//
size_t OBJ_FetchTypeSize( dword objtypeid )
{
	dword typebase = objtypeid & TYPENUMBERMASK;

	if ( typebase < NUM_DISTINCT_OBJTYPES ) {

		// simply use table
		return objtype_size[ typebase ];

	} else {

		// scan custom types
		return OBJ_FetchCustomTypeSize( objtypeid );
	}
}


// init default values for object type (basic initialization) -----------------
//
void OBJ_InitDefaultTypeFields( GenObject *classpo )
{
	ASSERT( classpo != NULL );

	switch ( classpo->ObjectType ) {

		case SHIP1TYPE:
		case SHIP2TYPE:
			{

			ShipObject *shippo = (ShipObject*) classpo;
			int tno = ( classpo->ObjectType & TYPENUMBERMASK ) -
					  ( SHIP1TYPE & TYPENUMBERMASK );

			shippo->MaxDamage			= ShipObj_TIs[ tno ].MaxDamage;
			shippo->CurShield			= ShipObj_TIs[ tno ].MaxShield;
			shippo->MaxShield			= ShipObj_TIs[ tno ].MaxShield;
			shippo->MaxSpeed			= ShipObj_TIs[ tno ].MaxSpeed;
			shippo->CurEnergy			= ShipObj_TIs[ tno ].MaxEnergy;
			shippo->MaxEnergy			= ShipObj_TIs[ tno ].MaxEnergy;
			shippo->CurFuel 			= ShipObj_TIs[ tno ].MaxFuel;
			shippo->MaxFuel 			= ShipObj_TIs[ tno ].MaxFuel;
			shippo->Weapons 			= ShipObj_TIs[ tno ].Weapons;
			shippo->Specials			= ShipObj_TIs[ tno ].Specials;
			shippo->NumMissls			= ShipObj_TIs[ tno ].MaxNumMissls;
			shippo->NumHomMissls		= ShipObj_TIs[ tno ].MaxNumHomMissls;
			shippo->NumPartMissls		= ShipObj_TIs[ tno ].MaxNumPartMissls;
			shippo->NumMines			= ShipObj_TIs[ tno ].MaxNumMines;
			shippo->MaxNumMissls		= ShipObj_TIs[ tno ].MaxNumMissls;
			shippo->MaxNumHomMissls 	= ShipObj_TIs[ tno ].MaxNumHomMissls;
			shippo->MaxNumPartMissls	= ShipObj_TIs[ tno ].MaxNumPartMissls;
			shippo->MaxNumMines 		= ShipObj_TIs[ tno ].MaxNumMines;
			shippo->YawPerRefFrame		= ShipObj_TIs[ tno ].YawPerRefFrame;
			shippo->PitchPerRefFrame	= ShipObj_TIs[ tno ].PitchPerRefFrame;
			shippo->RollPerRefFrame		= ShipObj_TIs[ tno ].RollPerRefFrame;
			shippo->XSlidePerRefFrame	= ShipObj_TIs[ tno ].XSlidePerRefFrame;
			shippo->YSlidePerRefFrame	= ShipObj_TIs[ tno ].YSlidePerRefFrame;
			shippo->SpeedIncPerRefFrame	= ShipObj_TIs[ tno ].SpeedIncPerRefFrame;
			shippo->SpeedDecPerRefFrame	= ShipObj_TIs[ tno ].SpeedDecPerRefFrame;
			shippo->FireRepeatDelay		= ShipObj_TIs[ tno ].FireRepeatDelay;
			shippo->FireDisableDelay	= ShipObj_TIs[ tno ].FireDisableDelay;
			shippo->MissileDisableDelay	= ShipObj_TIs[ tno ].MissileDisableDelay;
			shippo->ObjCamMinDistance	= ShipObj_TIs[ tno ].ObjCamMinDistance;
			shippo->ObjCamMaxDistance	= ShipObj_TIs[ tno ].ObjCamMaxDistance;
			shippo->ObjCamStartDist		= ShipObj_TIs[ tno ].ObjCamStartDist;
			shippo->ObjCamStartPitch	= ShipObj_TIs[ tno ].ObjCamStartPitch;
			shippo->ObjCamStartYaw		= ShipObj_TIs[ tno ].ObjCamStartYaw;
			shippo->ObjCamStartRoll		= ShipObj_TIs[ tno ].ObjCamStartRoll;

			shippo->Laser1_Class[ 0 ][ 0 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 0 ][ 0 ];
			shippo->Laser1_Class[ 0 ][ 1 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 0 ][ 1 ];
			shippo->Laser1_Class[ 0 ][ 2 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 0 ][ 2 ];
			shippo->Laser1_Class[ 0 ][ 3 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 0 ][ 3 ];

			shippo->Laser1_Class[ 1 ][ 0 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 1 ][ 0 ];
			shippo->Laser1_Class[ 1 ][ 1 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 1 ][ 1 ];
			shippo->Laser1_Class[ 1 ][ 2 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 1 ][ 2 ];
			shippo->Laser1_Class[ 1 ][ 3 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 1 ][ 3 ];

			shippo->Laser1_Class[ 2 ][ 0 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 2 ][ 0 ];
			shippo->Laser1_Class[ 2 ][ 1 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 2 ][ 1 ];
			shippo->Laser1_Class[ 2 ][ 2 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 2 ][ 2 ];
			shippo->Laser1_Class[ 2 ][ 3 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 2 ][ 3 ];

			shippo->Laser1_Class[ 3 ][ 0 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 3 ][ 0 ];
			shippo->Laser1_Class[ 3 ][ 1 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 3 ][ 1 ];
			shippo->Laser1_Class[ 3 ][ 2 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 3 ][ 2 ];
			shippo->Laser1_Class[ 3 ][ 3 ]	= ShipObj_TIs[ tno ].Laser1_Class[ 3 ][ 3 ];

			shippo->Laser1_X[ 0 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_X[ 0 ][ 0 ];
			shippo->Laser1_X[ 0 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_X[ 0 ][ 1 ];
			shippo->Laser1_X[ 0 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_X[ 0 ][ 2 ];
			shippo->Laser1_X[ 0 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_X[ 0 ][ 3 ];

			shippo->Laser1_X[ 1 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_X[ 1 ][ 0 ];
			shippo->Laser1_X[ 1 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_X[ 1 ][ 1 ];
			shippo->Laser1_X[ 1 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_X[ 1 ][ 2 ];
			shippo->Laser1_X[ 1 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_X[ 1 ][ 3 ];

			shippo->Laser1_X[ 2 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_X[ 2 ][ 0 ];
			shippo->Laser1_X[ 2 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_X[ 2 ][ 1 ];
			shippo->Laser1_X[ 2 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_X[ 2 ][ 2 ];
			shippo->Laser1_X[ 2 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_X[ 2 ][ 3 ];

			shippo->Laser1_X[ 3 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_X[ 3 ][ 0 ];
			shippo->Laser1_X[ 3 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_X[ 3 ][ 1 ];
			shippo->Laser1_X[ 3 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_X[ 3 ][ 2 ];
			shippo->Laser1_X[ 3 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_X[ 3 ][ 3 ];

			shippo->Laser1_Y[ 0 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 0 ][ 0 ];
			shippo->Laser1_Y[ 0 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 0 ][ 1 ];
			shippo->Laser1_Y[ 0 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 0 ][ 2 ];
			shippo->Laser1_Y[ 0 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 0 ][ 3 ];

			shippo->Laser1_Y[ 1 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 1 ][ 0 ];
			shippo->Laser1_Y[ 1 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 1 ][ 1 ];
			shippo->Laser1_Y[ 1 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 1 ][ 2 ];
			shippo->Laser1_Y[ 1 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 1 ][ 3 ];

			shippo->Laser1_Y[ 2 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 2 ][ 0 ];
			shippo->Laser1_Y[ 2 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 2 ][ 1 ];
			shippo->Laser1_Y[ 2 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 2 ][ 2 ];
			shippo->Laser1_Y[ 2 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 2 ][ 3 ];

			shippo->Laser1_Y[ 3 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 3 ][ 0 ];
			shippo->Laser1_Y[ 3 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 3 ][ 1 ];
			shippo->Laser1_Y[ 3 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 3 ][ 2 ];
			shippo->Laser1_Y[ 3 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Y[ 3 ][ 3 ];

			shippo->Laser1_Z[ 0 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 0 ][ 0 ];
			shippo->Laser1_Z[ 0 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 0 ][ 1 ];
			shippo->Laser1_Z[ 0 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 0 ][ 2 ];
			shippo->Laser1_Z[ 0 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 0 ][ 3 ];

			shippo->Laser1_Z[ 1 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 1 ][ 0 ];
			shippo->Laser1_Z[ 1 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 1 ][ 1 ];
			shippo->Laser1_Z[ 1 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 1 ][ 2 ];
			shippo->Laser1_Z[ 1 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 1 ][ 3 ];

			shippo->Laser1_Z[ 2 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 2 ][ 0 ];
			shippo->Laser1_Z[ 2 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 2 ][ 1 ];
			shippo->Laser1_Z[ 2 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 2 ][ 2 ];
			shippo->Laser1_Z[ 2 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 2 ][ 3 ];

			shippo->Laser1_Z[ 3 ][ 0 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 3 ][ 0 ];
			shippo->Laser1_Z[ 3 ][ 1 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 3 ][ 1 ];
			shippo->Laser1_Z[ 3 ][ 2 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 3 ][ 2 ];
			shippo->Laser1_Z[ 3 ][ 3 ]		= ShipObj_TIs[ tno ].Laser1_Z[ 3 ][ 3 ];

			shippo->Missile1_Class[ 0 ] = ShipObj_TIs[ tno ].Missile1_Class[ 0 ];
			shippo->Missile1_Class[ 1 ] = ShipObj_TIs[ tno ].Missile1_Class[ 1 ];
			shippo->Missile1_Class[ 2 ]	= ShipObj_TIs[ tno ].Missile1_Class[ 2 ];
			shippo->Missile1_Class[ 3 ]	= ShipObj_TIs[ tno ].Missile1_Class[ 3 ];
			shippo->Missile1_X[ 0 ]		= ShipObj_TIs[ tno ].Missile1_X[ 0 ];
			shippo->Missile1_X[ 1 ] 	= ShipObj_TIs[ tno ].Missile1_X[ 1 ];
			shippo->Missile1_X[ 2 ] 	= ShipObj_TIs[ tno ].Missile1_X[ 2 ];
			shippo->Missile1_X[ 3 ] 	= ShipObj_TIs[ tno ].Missile1_X[ 3 ];
			shippo->Missile1_Y[ 0 ]		= ShipObj_TIs[ tno ].Missile1_Y[ 0 ];
			shippo->Missile1_Y[ 1 ]		= ShipObj_TIs[ tno ].Missile1_Y[ 1 ];
			shippo->Missile1_Y[ 2 ]		= ShipObj_TIs[ tno ].Missile1_Y[ 2 ];
			shippo->Missile1_Y[ 3 ]		= ShipObj_TIs[ tno ].Missile1_Y[ 3 ];
			shippo->Missile1_Z[ 0 ]		= ShipObj_TIs[ tno ].Missile1_Z[ 0 ];
			shippo->Missile1_Z[ 1 ]		= ShipObj_TIs[ tno ].Missile1_Z[ 1 ];
			shippo->Missile1_Z[ 2 ]		= ShipObj_TIs[ tno ].Missile1_Z[ 2 ];
			shippo->Missile1_Z[ 3 ]		= ShipObj_TIs[ tno ].Missile1_Z[ 3 ];

			shippo->Missile2_Class[ 0 ] = ShipObj_TIs[ tno ].Missile2_Class[ 0 ];
			shippo->Missile2_Class[ 1 ] = ShipObj_TIs[ tno ].Missile2_Class[ 1 ];
			shippo->Missile2_Class[ 2 ]	= ShipObj_TIs[ tno ].Missile2_Class[ 2 ];
			shippo->Missile2_Class[ 3 ]	= ShipObj_TIs[ tno ].Missile2_Class[ 3 ];
			shippo->Missile2_X[ 0 ]		= ShipObj_TIs[ tno ].Missile2_X[ 0 ];
			shippo->Missile2_X[ 1 ] 	= ShipObj_TIs[ tno ].Missile2_X[ 1 ];
			shippo->Missile2_X[ 2 ] 	= ShipObj_TIs[ tno ].Missile2_X[ 2 ];
			shippo->Missile2_X[ 3 ] 	= ShipObj_TIs[ tno ].Missile2_X[ 3 ];
			shippo->Missile2_Y[ 0 ]		= ShipObj_TIs[ tno ].Missile2_Y[ 0 ];
			shippo->Missile2_Y[ 1 ]		= ShipObj_TIs[ tno ].Missile2_Y[ 1 ];
			shippo->Missile2_Y[ 2 ]		= ShipObj_TIs[ tno ].Missile2_Y[ 2 ];
			shippo->Missile2_Y[ 3 ]		= ShipObj_TIs[ tno ].Missile2_Y[ 3 ];
			shippo->Missile2_Z[ 0 ]		= ShipObj_TIs[ tno ].Missile2_Z[ 0 ];
			shippo->Missile2_Z[ 1 ]		= ShipObj_TIs[ tno ].Missile2_Z[ 1 ];
			shippo->Missile2_Z[ 2 ]		= ShipObj_TIs[ tno ].Missile2_Z[ 2 ];
			shippo->Missile2_Z[ 3 ]		= ShipObj_TIs[ tno ].Missile2_Z[ 3 ];

			shippo->Mine1_X 			= ShipObj_TIs[ tno ].Mine1_X;
			shippo->Mine1_Y 			= ShipObj_TIs[ tno ].Mine1_Y;
			shippo->Mine1_Z 			= ShipObj_TIs[ tno ].Mine1_Z;

			shippo->SpreadSpeed			= SPREADFIRE_SPEED;
			shippo->SpreadLifeTime		= SPREADFIRE_LIFETIME;

			shippo->Spread_X[ 0 ]		= ShipObj_TIs[ tno ].Spread_X[ 0 ];
			shippo->Spread_X[ 1 ] 		= ShipObj_TIs[ tno ].Spread_X[ 1 ];
			shippo->Spread_X[ 2 ] 		= ShipObj_TIs[ tno ].Spread_X[ 2 ];
			shippo->Spread_X[ 3 ] 		= ShipObj_TIs[ tno ].Spread_X[ 3 ];
			shippo->Spread_Y			= ShipObj_TIs[ tno ].Spread_Y;
			shippo->Spread_Z			= ShipObj_TIs[ tno ].Spread_Z;

			shippo->HelixSpeed			= HELIX_SPEED;
			shippo->HelixLifeTime		= HELIX_LIFETIME;

			shippo->Helix_X				= ShipObj_TIs[ tno ].Helix_X;
			shippo->Helix_Y				= ShipObj_TIs[ tno ].Helix_Y;
			shippo->Helix_Z				= ShipObj_TIs[ tno ].Helix_Z;

            shippo->PhotonSpeed         = PHOTON_SPEED;
            shippo->PhotonLifeTime      = PHOTON_LIFETIME;

			shippo->Beam_X[ 0 ]			= ShipObj_TIs[ tno ].Beam_X[ 0 ];
			shippo->Beam_X[ 1 ] 		= ShipObj_TIs[ tno ].Beam_X[ 1 ];
			shippo->Beam_X[ 2 ] 		= ShipObj_TIs[ tno ].Beam_X[ 2 ];
			shippo->Beam_X[ 3 ] 		= ShipObj_TIs[ tno ].Beam_X[ 3 ];
			shippo->Beam_Y				= ShipObj_TIs[ tno ].Beam_Y;
			shippo->Beam_Z				= ShipObj_TIs[ tno ].Beam_Z;

			shippo->FumeFreq			= PROP_FUME_FREQUENCY;
			shippo->FumeSpeed   		= PROP_FUME_SPEED;
			shippo->FumeLifeTime		= PROP_FUME_LIFETIME;
			shippo->FumeCount			= 0;

			shippo->Fume_X[ 0 ]			= ShipObj_TIs[ tno ].Fume_X[ 0 ];
			shippo->Fume_X[ 1 ] 		= ShipObj_TIs[ tno ].Fume_X[ 1 ];
			shippo->Fume_X[ 2 ] 		= ShipObj_TIs[ tno ].Fume_X[ 2 ];
			shippo->Fume_X[ 3 ] 		= ShipObj_TIs[ tno ].Fume_X[ 3 ];
			shippo->Fume_Y				= ShipObj_TIs[ tno ].Fume_Y;
			shippo->Fume_Z				= ShipObj_TIs[ tno ].Fume_Z;

			shippo->Orbit				= NULL;

			shippo->afterburner_previous_speed	= 0;
			shippo->afterburner_active 			= FALSE;
			shippo->afterburner_energy			= AFTERBURNER_ENERGY;

			}
			break;

		case LASER1TYPE:
		case LASER2TYPE:
		case LASER3TYPE:
			{

			LaserObject *laserpo = (LaserObject*) classpo;
			int tno = ( classpo->ObjectType & TYPENUMBERMASK ) -
					  ( LASER1TYPE & TYPENUMBERMASK );

			laserpo->LifeTimeCount = ProjectileObj_TIs[ tno ].LifeTimeCount;
			laserpo->Speed		   = ProjectileObj_TIs[ tno ].Speed;
			laserpo->HitPoints	   = ProjectileObj_TIs[ tno ].HitPoints;
			laserpo->EnergyNeeded  = LaserEnergyNeeds[ tno ];

			}
			break;

		case MISSILE4TYPE:
			{

			TargetMissileObject *targetmisspo = (TargetMissileObject*) classpo;
			int tno = ( classpo->ObjectType & TYPENUMBERMASK ) -
					  ( MISSILE4TYPE & TYPENUMBERMASK );

			targetmisspo->Latency	  = TargetMissObj_TIs[ tno ].Latency;
			targetmisspo->MaxRotation = TargetMissObj_TIs[ tno ].MaxRotation;

			}
			/* FALLTHROUGH */

		case MISSILE1TYPE:
		case MISSILE2TYPE:
		case MISSILE3TYPE:
			{

			ProjectileObject *projectilepo = (ProjectileObject*) classpo;
			int tno = ( classpo->ObjectType & TYPENUMBERMASK ) -
					  ( LASER1TYPE & TYPENUMBERMASK );

			projectilepo->LifeTimeCount = ProjectileObj_TIs[ tno ].LifeTimeCount;
			projectilepo->Speed 		= ProjectileObj_TIs[ tno ].Speed;
			projectilepo->HitPoints 	= ProjectileObj_TIs[ tno ].HitPoints;

			}
			break;

		case MINE1TYPE:
			{

			MineObject *minepo = (MineObject*) classpo;
			int tno = ( classpo->ObjectType & TYPENUMBERMASK ) -
					  ( MINE1TYPE & TYPENUMBERMASK );

			minepo->LifeTimeCount = MineObj_TIs[ tno ].LifeTimeCount;
			minepo->SelfRotX	  = MineObj_TIs[ tno ].SelfRotX;
			minepo->SelfRotY	  = MineObj_TIs[ tno ].SelfRotY;
			minepo->SelfRotZ	  = MineObj_TIs[ tno ].SelfRotZ;
			minepo->HitPoints	  = MineObj_TIs[ tno ].HitPoints;
			minepo->Owner		  = OWNER_LOCAL_PLAYER;

			}
			break;

		case EXTRA1TYPE:
		case EXTRA2TYPE:
		case EXTRA3TYPE:
			{

				ExtraObject *extrapo = (ExtraObject*) classpo;
				int tno = ( classpo->ObjectType & TYPENUMBERMASK ) - ( EXTRA1TYPE & TYPENUMBERMASK );

				extrapo->LifeTimeCount	= ExtraObj_TIs[ tno ].LifeTimeCount;
				extrapo->SelfRotX		= ExtraObj_TIs[ tno ].SelfRotX;
				extrapo->SelfRotY		= ExtraObj_TIs[ tno ].SelfRotY;
				extrapo->SelfRotZ		= ExtraObj_TIs[ tno ].SelfRotZ;
				extrapo->VisibleFrame_Reset_Frames = 0;

			}
			break;

#ifdef ALLOW_ONLY_KNOWN_TYPES

		default:
			PERROR( "unknown object type number: %d.", classpo->ObjectType );
#endif

	}
}



