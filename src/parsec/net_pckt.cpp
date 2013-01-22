/*
 * PARSEC - Packet handling
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:30 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
//#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
//#include "aud_defs.h"
#include "net_defs.h"
//#include "sys_defs.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_pckt.h"

// proprietary module headers


// protocol type signature ----------------------------------------------------
//
const char net_game_signature[]					= "CG-Net!";	// internal 0.2
const char net_cryp_signature[]					= "gepsuas";	// PEER 0.1 and 0.2




