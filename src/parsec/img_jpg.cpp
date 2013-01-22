/*
 * PARSEC - JPEG Format Functions
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:35 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998-2001
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

// local module header
#include "img_jpg.h"

// proprietary module headers
#include "con_aux.h"
#include "e_color.h"
#include "e_supp.h"
#include "img_api.h"
#include "img_conv.h"
#include "img_load.h"
#include "img_supp.h"
#include "sys_file.h"


// save buffer in jpeg format -------------------------------------------------
//
int JPG_SaveBuffer( const char *filename, char *buff, int width, int height )
{
	ASSERT( filename != NULL );
	ASSERT( buff != NULL );
	ASSERT( width > 0 );
	ASSERT( height > 0 );

	return FALSE;
}

