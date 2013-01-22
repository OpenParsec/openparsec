/*
 * PARSEC HEADER (ENGINE CORE)
 * Eventmanager Definitions V1.02
 *
 * Copyright (c) Clemens Beer 1999
 * All Rights Reserved.
 */

#ifndef _EVTDEFS_H_
#define _EVTDEFS_H_


// general event types ---------------------------------------------------------
//
#define EVT_TYPE_GENERAL				0x0000


// sound event types ----------------------------------------------------------
//
#define EVT_TYPE_SND_GENERAL				0x1000
#define EVT_TYPE_SND_GUN_SELECT				0x1001
#define EVT_TYPE_SND_MISSILE_SELECT			0x1002
#define EVT_TYPE_SND_COUNTDOWN				0x1003
#define EVT_TYPE_SND_SHIPDESTROYED			0x1004
#define EVT_TYPE_SND_THRUST_FADEOUT			0x1005
#define EVT_TYPE_SND_SLIDETHRUST_FADEOUT	0x1006
#define EVT_TYPE_SND_INCOMING_STOP			0x1007
#define EVT_TYPE_SND_LASERBEAM_STOP			0x1008
#define EVT_TYPE_SND_LIGHTNING_VOLUME   	0x1009
#define EVT_TYPE_SND_HELIX_VOLUME			0x100A
#define EVT_TYPE_SND_EXPLOSION_DELAY		0x100B
#define EVT_TYPE_SND_KILLSLEFT				0x100C
#define EVT_TYPE_SND_SWARM_VOLUME			0x100D
#define EVT_TYPE_SND_PHOTON_VOLUME			0x100E
#define EVT_TYPE_SND_TELEPORTER_VOLUME		0x100F


// background event types -----------------------------------------------------
//
#define EVT_TYPE_BKGN_GENERAL				0x1100
#define EVT_TYPE_BKGN_CDTRACK_END			0x1101
#define EVT_TYPE_BKGN_SAMPLE_END			0x1102
#define EVT_TYPE_BKGN_SILENCE_END			0x1104


#endif //_EVTDEFS_H_


