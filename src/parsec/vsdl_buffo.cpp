/*
 * PARSEC - Frame Buffer Functions (OPENGL)
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   1999
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
#include "config.h"

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
#include "vid_defs.h"

// local module header
#include "vsdl_buffo.h"

// opengl headers
#include "r_gl.h"

// proprietary module headers
#include "con_aux.h"
#include "ro_api.h"
#include "ro_supp.h"
#include "vsdl_ogl.h"


// clear entire rendering buffer ----------------------------------------------
//
void VIDs_ClearRenderBuffer()
{
	glClearColor( 0, 0, 0, 0 );
	glClearDepth( 0.0 );

	//NOTE:
	// when clearing the depth buffer, write access has
	// to be enabled which is disabled on function entry.
	// interestingly enough, on TNT drivers this doesn't
	// matter, since the mask is completely ignored by
	// the driver when clearing the buffer. however, this
	// is not the case on a Rage128, for instance.

	GLbitfield clearmask = 0;

	if (!AUX_DISABLE_BUFFER_CLEAR) {
		clearmask |= GL_COLOR_BUFFER_BIT;
	}

	if (!AUX_DISABLE_ZBUFFER_CLEAR) {
		clearmask |= GL_DEPTH_BUFFER_BIT;
		RO_DepthMask(GL_TRUE);
	}

	if (clearmask != 0) {
		glClear(clearmask);
	}

	if (!AUX_DISABLE_ZBUFFER_CLEAR) {
		RO_DepthMask(GL_FALSE);
	}
}


// commit render buffer (make backbuffer visible) -----------------------------
//
void VIDs_CommitRenderBuffer()
{
	// swap buffers
	VSDL_CommitOGLBuff();
}


// create/deinitialize screenshot buffer --------------------------------------
//
char *VIDs_ScreenshotBuffer( int create, int *size )
{
	//NOTE:
	// this function must be called twice in order to
	// make a screenshot.
	// first, to get a pointer to the screenshot buffer
	// and its size (create==TRUE). like:
	// shotbuff = VIDs_ScreenshotBuffer( TRUE, &bufsiz );
	// second, to deallocate the buffer (create==FALSE).
	// like: VIDs_ScreenshotBuffer( FALSE, NULL );

	// temporary screenshot buffer
	static char *buffer = NULL;

	// validate parameters with respect to buffer allocation
	ASSERT( (  create && ( buffer == NULL ) && ( size != NULL ) ) ||
			( !create && ( buffer != NULL ) && ( size == NULL ) ) );

	if ( create ) {

		// alloc screenshot buffer
		*size  = Screen_Width * Screen_Height * 3;
		buffer = (char *) ALLOCMEM( *size );
		if ( buffer == NULL ) {
			return NULL;
		}

		// alloc temp readpixels buffer
		char *readbuffer = NULL;
		readbuffer = (char *) ALLOCMEM( *size );
		if ( readbuffer == NULL ) {
			return NULL;
		}

		// assert that screenwidth is even
		ASSERT( ( Screen_Width & 0x01 ) == 0 );

		// read RGB data from the backbuffer
		glReadPixels( 0, 0, Screen_Width, Screen_Height, GL_RGB, GL_UNSIGNED_BYTE, readbuffer );

		size_t scanlinelen = (Screen_Width * 3);

		char *readpo  = readbuffer + ((Screen_Height-1) * scanlinelen);
		char *writepo = buffer;

		// flip the image upside down
		for ( int i = 0; i < Screen_Height; i++ ) {

			memcpy( writepo, readpo, scanlinelen );

			writepo += scanlinelen;
			readpo  -= scanlinelen;
		}

		FREEMEM( readbuffer );

	} else if ( buffer != NULL ) {

		// free screenshot buffer
		FREEMEM( buffer );
		buffer = NULL;
	}

	return buffer;
}

