/*
 * PARSEC - BSP Tree Operations
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:43 $
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
#include <math.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "utl_bsp.h"



// static tree traversal variables --------------------------------------------
//
static Vertex3*		bsp_line_v0;
static Vertex3*		bsp_line_v1;
static CullBSPNode*	bsp_tree;
static dword		bsp_collider;
static geomv_t		bsp_coll_t;


// traversal function to find line segment <-> bsp node collision -------------
//
PRIVATE
int BSP_FindCollider( dword nodeid, geomv_t t0, geomv_t t1 )
{
	ASSERT( nodeid != 0 );

	//NOTE:
	// nodeid==0: tree NULL
	// nodeid==1: tree root

	//NOTE:
	// baseseg: [ (t=0) v0 --------------------- v1 (t=1) ]
	// subseg:  [ (d0)        t0 --------- t1        (d1) ]
	// seg0:    [             t0 --- t                    ]
	// seg1:    [                    t --- t1             ]

	static CullBSPNode *node;
	node = &bsp_tree[ nodeid ];

	// signed distances of baseseg vertices to current plane
	static geomv_t d0;
	static geomv_t d1;
	d0 = PLANE_DOT( &node->plane, bsp_line_v0 ) - PLANE_OFFSET( &node->plane );
	d1 = PLANE_DOT( &node->plane, bsp_line_v1 ) - PLANE_OFFSET( &node->plane );

	static	dword seg0treeid;
			dword seg1treeid;
	int seg0solid;

	// determine halfspace (subtree) of v0
	if ( GEOMV_GEZERO( d0 ) ) {
		seg0solid	= FALSE;				// leaf would be empty
		seg0treeid	= node->subtrees[ 1 ];	// fronttree
		seg1treeid	= node->subtrees[ 0 ];	// backtree
	} else {
		seg0solid	= TRUE;					// leaf would be solid
		seg0treeid	= node->subtrees[ 0 ];	// backtree
		seg1treeid	= node->subtrees[ 1 ];	// fronttree
	}

	// special case if baseseg not straddling
	if ( ( DW32( d0 ) ^ DW32( d1 ) ) < 0x80000000 ) {

pushseg:
		// classify if seg in leaf
		if ( seg0treeid == 0 )
			return seg0solid;

		// push down without clipping (tail rec)
		return BSP_FindCollider( seg0treeid, t0, t1 );
	}

	// drop distance signs
	ABS_GEOMV( d0 );
	ABS_GEOMV( d1 );

	// calc projected length
	static geomv_t seglen;
	seglen = d0 + d1;
	ASSERT( seglen >= 0 );

	// simply push down if baseseg is on
	if ( seglen <= FLOAT_TO_GEOMV( 0.00001 ) ) {
		goto pushseg;
	}

	// calc intersection parameter using baseseg
	geomv_t t = GEOMV_DIV( d0, seglen );

	// subseg not straddling: case 1
	if ( t <= t0 ) {

		// invert classification (subseg in v1's halfspace)
		seg0solid	= !seg0solid;
		seg0treeid	= seg1treeid;

		goto pushseg;
	}

	// subseg not straddling: case 2
	if ( t >= t1 ) {
		goto pushseg;
	}

	// check seg0 first
	if ( seg0treeid != 0 ) {

		// push seg0 into its tree if not leaf
		if ( BSP_FindCollider( seg0treeid, t0, t ) )
			return TRUE;

	} else if ( seg0solid ) {

		// collision if seg0 in solid leaf
		return TRUE;
	}

	// this node becomes potential collider (ray passes through its plane)
	bsp_collider = nodeid;
	bsp_coll_t   = t;

	// check if seg1's tree is a leaf
	if ( seg1treeid == 0 ) {

		// classification of seg1 is inverse of seg0's
		return !seg0solid;
	}

	// push seg1 into its tree
	return BSP_FindCollider( seg1treeid, t, t1 );
}


// find collider (tree node) for specified line segment -----------------------
//
int BSP_FindColliderLine( CullBSPNode *tree, Vertex3 *v0, Vertex3 *v1, dword *colnode, geomv_t *colt )
{
	ASSERT( tree != NULL );
	ASSERT( v0 != NULL );
	ASSERT( v1 != NULL );

	// set root pointer and lineseg
	bsp_tree	= tree;
	bsp_line_v0	= v0;
	bsp_line_v1	= v1;
	bsp_coll_t	= GEOMV_0;

	// no potential collider until first boundary pierced
	bsp_collider = 0;

	// clip lineseg into bsp tree
	int collided = BSP_FindCollider( 1, GEOMV_0, GEOMV_1 );

	// set collider (node id) and t (parameter of collision)
	if ( colnode != NULL )
		*colnode = bsp_collider;
	if ( colt != NULL )
		*colt = bsp_coll_t;

	return collided;
}



