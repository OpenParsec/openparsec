/*
 * PARSEC - Init Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:27 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
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

// local module header
#include "inp_init.h"



// input subsystem initialization ---------------------------------------------
//
void INP_InitSubsystem()
{
	// general input subsystem init
	INPs_InitGeneral();

	// init proprietary keyboard code
	INPs_KeybInitHandler();

	// init joystick code
	INPs_JoyInitHandler();

	// init mouse code
	INPs_MouseInitHandler();
}


// input subsystem de-initialization ------------------------------------------
//
void INP_KillSubsystem()
{
	// kill mouse code
	INPs_MouseKillHandler();

	// kill joystick code
	INPs_JoyKillHandler();

	// kill keyboard code
	INPs_KeybKillHandler();

	// kill input handling in general
	INPs_KillGeneral();
}



