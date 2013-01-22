/*
 * PARSEC - Subsystem Switching
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:42 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "inp_defs.h"
#include "vid_defs.h"

// subsystem linkage info
#include "linkinfo.h"

// mathematics header
#include "utl_math.h"

// local module header
#include "vid_plug.h"

// proprietary module headers
#include "con_ext.h"
#include "con_main.h"
#include "e_color.h"
#include "e_demo.h"
#include "e_loader.h"
#include "h_drwhud.h"
#include "h_supp.h"
#include "part_sys.h"
#include "sys_bind.h"
#include "vid_init.h"



// on-the-fly video subsystem switching ---------------------------------------
//
int VID_PlugInSubsys( const char *subsysname )
{
	ASSERT( subsysname != NULL );

	// ensure no replay in progress while switching
	if ( DEMO_ReplayActive() ) {
		DEMO_StopReplay();
	}

	// plug in new subsystems and perform switch
	if ( TextModeActive ) {

		// bind new subsystems
		SYS_BindDynamicFunctions();

		// init new video subsystem
		VID_InitSubsystem();

	} else {

		// switch back to text mode
		VIDs_RestoreDisplay();

		// init new video subsystem
		VID_InitSubsystem();

		// switch to new video mode
		VID_InitMode();

		// set up data depending on video mode
		VID_SetupDependentData();

		// init hud geometry
		InitHudDisplay();

		// init sizes of particles
		InitParticleSizes();

		// check console extents
		CheckConExtents();
	}

	// make sure mouse tracking works correctly
	INPs_MouseKillHandler();
	INPs_MouseInitHandler();

	return TRUE;
}


// change video mode on-the-fly in game mode ----------------------------------
//
int VID_HotChangeMode( int xres, int yres, bool fullscreen, int bpp )
{
	printf("HotChangeMode requested for xres: %d, yres: %d\n", xres, yres);
	
	int resindex = GetResolutionIndex(xres, yres);
	if (resindex < 0)
		return FALSE;
	
	// take init options override (from command line) into account
	if ( TextModeActive ) {
		
		// override only if forced by user
		if ( InitOp_FlipSynched != -1 ) {
			FlipSynched = InitOp_FlipSynched;
		}

		// exit if full init options override
		if (InitOp_Resolution.isValid() && InitOp_WindowedMode != -1)
			return TRUE;

		// override resolution, take fullscreen/windowed
		if (InitOp_Resolution.isValid()) {
			xres = Op_Resolution.width;
			yres = Op_Resolution.height;
			bpp  = VID_MapOptToBpp( Op_ColorDepth );
		}

		// override fullscreen/windowed, take resolution
		if ( InitOp_WindowedMode != -1 ) {
			fullscreen = !Op_WindowedMode;
		}
	}

	// set options corresponding to mode
	Op_Resolution.set(xres, yres);
	Op_ColorDepth	= VID_MapBppToOpt( bpp );
	Op_WindowedMode	= !fullscreen;

	// make sure this also works before VID_InitMode() has been
	// called for the first video mode init. in that case the
	// specified mode will be initialized later on.
	if ( TextModeActive )
		return TRUE;

	// switch to new video mode
	printf("Called from VID_HotChangeMode()\n");
	VID_ApplyOptions();

	return TRUE;
}



