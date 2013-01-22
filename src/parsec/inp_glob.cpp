/*
 * PARSEC - Global Variables
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:27 $
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_props.h"

// subsystem headers
#include "inp_defs.h"

// local module header
#include "inp_glob.h"



// default joystick sensitivity (timing) --------------------------------------
//
#define JOY_YAW_CORR_REFFRAME		( 0.5f / SHIP_YAW_PER_REFFRAME )
#define JOY_PITCH_CORR_REFFRAME		( 0.5f / SHIP_PITCH_PER_REFFRAME )
#define JOY_ROLL_CORR_REFFRAME		( 1.0f / SHIP_ROLL_PER_REFFRAME )
#define JOY_ACC_CORR_REFFRAME		( 2.0f / SHIP_SPEED_INC_PER_REFFRAME )


// current joystick sensitivity (timing) --------------------------------------
//
float joy_yaw_corr_refframe		= JOY_YAW_CORR_REFFRAME;
float joy_pitch_corr_refframe		= JOY_PITCH_CORR_REFFRAME;
float joy_roll_corr_refframe		= JOY_ROLL_CORR_REFFRAME;
float joy_acc_corr_refframe		= JOY_ACC_CORR_REFFRAME;


// mouse config flags ---------------------------------------------------------
//
int inp_mouse_invert_yaxis 			= FALSE;
int inp_mouse_sensitivity  			= 100;
int inp_mouse_drift        			= 10;


