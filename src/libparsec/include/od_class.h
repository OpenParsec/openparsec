/*
 * PARSEC HEADER (OBJECT)
 * Class Codes V1.20
 *
 * Copyright (c) Markus Hadwiger 1996-2000
 * All Rights Reserved.
 */

#ifndef _OD_CLASS_H_
#define _OD_CLASS_H_


//NOTE:
//
// this lists the class ids of all object classes available.
// (actually: indexes into ObjClasses[] to retrieve class of certain id.)
//
// this *MUST* be consistent with the following information in other places:
//
// - list in control file (#objects); to ensure correct data<->class relation.
// - list in CON_INFO.C (ClassProperties[]); this provides textual class info.
// - version of remote player; class ids are part of some remote messages!
//

enum enum_class_ids {

    SHIP_CLASS_1,				// 0
    LASER0_CLASS_1,				// 1
    LASER0_CLASS_2,				// 2
    DUMB_CLASS_1,				// 3
    GUIDE_CLASS_1,				// 4
    SWARM_CLASS_1,				// 5
    ENERGY_EXTRA_CLASS,			// 6
    DUMB_PACK_CLASS,			// 7
    SHIP_CLASS_2,				// 8
    SHIP_CLASS_3,				// 9
    GUIDE_PACK_CLASS,			// 10
    HELIX_DEVICE_CLASS,			// 11
    LIGHTNING_DEVICE_CLASS,		// 12
    MINE_PACK_CLASS,			// 13
    MINE_CLASS_1,				// 14
    LASER1_CLASS_1,				// 15
    LASER2_CLASS_1,				// 16
    REPAIR_EXTRA_CLASS,			// 17
    AFTERBURNER_DEVICE_CLASS,	// 18
    SWARM_PACK_CLASS,			// 19
    INVISIBILITY_CLASS,			// 20
    PHOTON_DEVICE_CLASS,		// 21
    DECOY_DEVICE_CLASS,			// 22
	LASERUPGRADE1_CLASS,		// 23
    LASERUPGRADE2_CLASS,		// 24
	INVULNERABILITY_CLASS,		// 25
//	EMPUPGRADE1_CLASS,			// 26
//	EMPUPGRADE2_CLASS,			// 27

	NUM_ALTERABLE_CLASSES		// must be last in list!!
};


// indexes into OBJ_CREG::ExtraClasses[] for extras ---------------------------
//
enum {

	EXTRAINDX_ENERGY_EXTRA,			// extra classindex 00: energy boost
	EXTRAINDX_DUMB_PACK,			// extra classindex 01: missile pack (dumb missiles)
	EXTRAINDX_GUIDE_PACK,			// extra classindex 02: missile pack (guided missiles)
	EXTRAINDX_HELIX_DEVICE,			// extra classindex 03: dazzling laser/helix cannon
	EXTRAINDX_LIGHTNING_DEVICE,		// extra classindex 04: thief laser/lightning device
	EXTRAINDX_MINE_PACK,			// extra classindex 05: mine pack (proximity mines)
	EXTRAINDX_REPAIR_EXTRA,			// extra classindex 06: repair damage
	EXTRAINDX_AFTERBURNER_DEVICE,	// extra classindex 07: afterburner
	EXTRAINDX_SWARM_PACK,			// extra classindex 08: missile pack (swarm missiles)
	EXTRAINDX_INVISIBILITY,			// extra classindex 09: invisibility (not used yet)
	EXTRAINDX_PHOTON_DEVICE,		// extra classindex 10: photon cannon
	EXTRAINDX_DECOY_DEVICE,			// extra classindex 11: decoy device
	EXTRAINDX_LASERUPGRADE1,		// extra classindex 12: laser upgrade 1
	EXTRAINDX_LASERUPGRADE2,		// extra classindex 13: laser upgrade 2
	EXTRAINDX_INVULNERABILITY,		// extra classindex 14: invulnerability
	EXTRAINDX_MINE,					// extra classindex 15: invulnerability
	EXTRAINDX_EMPUPGRADE1,			// extra classindex 16: emp upgrade 1
	EXTRAINDX_EMPUPGRADE2,			// extra classindex 17: emp upgrade 2
};


// identifiers of collectable devices -----------------------------------------
//
enum {

	UNKNOWN_DEVICE,					// 0
	HELIX_DEVICE,					// 1
	LIGHTNING_DEVICE,				// 2
	AFTERBURNER_DEVICE,				// 3
	INVISIBILITY_DEVICE,			// 4
	PHOTON_DEVICE,					// 5
	DECOY_DEVICE,					// 6
	INVULNERABILITY_DEVICE,			// 7
	LASER_UPGRADE_1_DEVICE,			// 8
	LASER_UPGRADE_2_DEVICE,			// 9
	EMP_UPGRADE_1_DEVICE,			// 10
	EMP_UPGRADE_2_DEVICE,			// 11
};


#endif // _OD_CLASS_H_


