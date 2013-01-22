/*
 * PARSEC - Valid ASCII Characters
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:40 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1997-1998
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
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// local module header
#include "con_vald.h"


// table indicating if an ASCII character is valid in console -----------------
//
int ConsoleASCIIValid[ CON_ASCII_TABLE_LEN ] = {

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	1,													// ' '
	1,													// '!'
	1,													// '"'
	1,													// '#'
	1,													// '$'
	1,													// '%'
	0,													// '&'
	1,													// '''
	1,													// '('
	1,													// ')'
	1,													// '*'
	1,													// '+'
	1,													// ','
	1,													// '-'
	1,													// '.'
	1,													// '/'

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,						// '0'..'9'

	1,													// ':'
	1,													// ';'
	1,													// '<'
	1,													// '='
	1,													// '>'
	1,													// '?'
	1,													// '@'

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 'A'..'Z'

	1,													// '['
	0,													// '\'
	1,													// ']'
	0,													// '^'
	1,													// '_'
	0,													// '`'

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 'a'..'z'

	0,													// '{'
	1,													// '|'
	0,													// '}'
	0,													// '~'
	0

};



