/*
 * PARSEC - Global Variables
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:44 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2001
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"

// subsystem headers
//#include "vid_defs.h"

// local module header
#include "e_global_sv.h"

// for definition of MAX_AUX_ENABLING and MAX_AUX_DATA
#include "con_aux_sv.h"

#include "net_limits.h"
#include "net_defs.h"
#include "MasterServer.h"

// counters for numbers of textures/objects/bitmaps ---------------------------
//
int 	NumLoadedObjects		= 0;


// info structures for loaded data --------------------------------------------
//
objectinfo_s	ObjectInfo[ MAX_DISTINCT_OBJCLASSES ];

// current working directory (for general use) --------------------------------
//
char	CurWorkDir[ PATH_MAX + 1 ] = ".";


// pointer to programname (used in error messages) ----------------------------
//
char*	sys_ProgramName;

// global color definitions ---------------------------------------------------
//
colrgba_s LightColorAmbient		= { 255, 255, 255, 255 };
colrgba_s LightColorDiffuse		= { 255, 255, 255, 255 };
colrgba_s LightColorSpecular	= { 255, 255, 255, 255 };


// global directional light source --------------------------------------------
//
Vector3	GlobalDirLight			= { GEOMV_0, GEOMV_0, GEOMV_1, 0 };

// flag if currently recording ------------------------------------------------
//
int		RecordingActive	    	= FALSE;


// file pointer to recording file ---------------------------------------------
//
FILE*	RecordingFp				= NULL;


// remote packet recording ----------------------------------------------------
//
int		RecordRemotePackets 	= FALSE;
int		RemoteRecSessionId  	= 0;
int		RemoteRecPacketId   	= 0;

// console variables ----------------------------------------------------------
//
int 	ConsoleSliding			= 0;
int 	ConsoleHeight			= 16;


// list of currently visible objects ------------------------------------------
//
GenObject*		VObjList		= NULL; // rebuilt each frame




// number of available object classes (object class table entries) ------------
//
int				NumObjClasses			= 0;



// auxiliary enabling flags ---------------------------------------------------
//
int				AuxEnabling[ MAX_SV_ARRAY_SIZE ];



// frame measurement counters -------------------------------------------------
//
refframe_t	RefFrameFrequency	= DEFAULT_REFFRAME_FREQUENCY;



// global class vars ----------------------------------------------------------
//
G_CollDet*			TheGameCollDet		= NULL;
G_ExtraManager*		TheGameExtraManager	= NULL;
G_Input*			TheGameInput		= NULL;
G_Main*				TheGame				= NULL;
NET_PacketDriver*	ThePacketDriver		= NULL;
NET_UDPDriver*		TheUDPDriver		= NULL;
E_DistManager*	    TheDistManager		= NULL;
E_World*			TheWorld			= NULL;
E_SimNetInput*	    TheSimNetInput		= NULL;
E_SimNetOutput*	    TheSimNetOutput		= NULL;
E_Simulator*		TheSimulator		= NULL;
E_ConnManager*		TheConnManager		= NULL;
E_GameServer*		TheServer			= NULL;
MasterServer*		TheMaster			= NULL;
E_PacketHandler*	ThePacketHandler	= NULL;


