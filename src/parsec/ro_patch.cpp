/*
 * PARSEC - Subsystem Patching Function
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001
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

// patching headers
#include "r_patch.h"

// local module header
#include "ro_patch.h"

// proprietary module headers
#include "ro_api.h"
#include "ro_supp.h"



// patch entire rendering subsystem -------------------------------------------
//
void R_PatchSubSystem()
{
	// invalidate on-board state

	RO_InvalidateRasterizerState();
	RO_InvalidateTextureMem();

	// NOTE:
	// this has to be done since state is not preserved when dynamically
	// switching VID/DRAW/REND subsystems.
}


// invalidate texture in cache ------------------------------------------------
//
void R_InvalidateCachedTexture( TextureMap *tmap )
{
	// NOTE:
	// if the supplied pointer is NULL the entire
	// texture cache will be flushed.

	if ( tmap != NULL ) {

		// invalidate only one texture
		RO_InvalidateCachedTexture( tmap );

	} else {

		// invalidate all textures
		RO_InvalidateTextureMem();
	}
}



