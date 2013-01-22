/*
 * PARSEC - Stars Drawing
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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
#include "vid_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_iter.h"
#include "d_misc.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "g_stars.h"

// proprietary module headers
#include "e_color.h"
#include "g_camera.h"
#include "obj_game.h"


// flags
#define USE_FIXED_STARS_INIT_TAB
//#define SAVE_FIXED_STARS_INIT_TAB
#define USE_ITER_PSEUDO_STARS



// constants ------------------------------------------------------------------
//
#define SUN_STAR_NO				52 //63			// star number of sun
#define FIXED_STAR_COLOR		(7*16+12)


// star clipping z values -----------------------------------------------------
//
static geomv_t near_clip_z = FLOAT_TO_GEOMV( 10.0 );
static geomv_t far_clip_z  = FLOAT_TO_GEOMV( 800.0 );


// init table for fixed star positions and types ------------------------------
//
fixedstar_s fixed_stars_init_tab[ MAX_FIXED_STARS ]

#ifdef USE_FIXED_STARS_INIT_TAB
= {
	{ { FLOAT_TO_GEOMV(470.295013), FLOAT_TO_GEOMV(358.982574), FLOAT_TO_GEOMV(-538.473877) }, 5 },
	{ { FLOAT_TO_GEOMV(-530.865173), FLOAT_TO_GEOMV(150.155624), FLOAT_TO_GEOMV(-579.340576) }, 0 },
	{ { FLOAT_TO_GEOMV(-409.642212), FLOAT_TO_GEOMV(-588.860718), FLOAT_TO_GEOMV(354.169830) }, 6 },
	{ { FLOAT_TO_GEOMV(-500.419434), FLOAT_TO_GEOMV(-529.289795), FLOAT_TO_GEOMV(-330.806122) }, 0 },
	{ { FLOAT_TO_GEOMV(-519.114014), FLOAT_TO_GEOMV(-400.387177), FLOAT_TO_GEOMV(-458.487549) }, 4 },
	{ { FLOAT_TO_GEOMV(558.433716), FLOAT_TO_GEOMV(572.813965), FLOAT_TO_GEOMV(-5.991778) }, 9 },
	{ { FLOAT_TO_GEOMV(-161.033676), FLOAT_TO_GEOMV(-540.007629), FLOAT_TO_GEOMV(-567.855591) }, 2 },
	{ { FLOAT_TO_GEOMV(679.621399), FLOAT_TO_GEOMV(419.870819), FLOAT_TO_GEOMV(42.698727) }, 0 },
	{ { FLOAT_TO_GEOMV(-156.703949), FLOAT_TO_GEOMV(-588.378967), FLOAT_TO_GEOMV(518.897034) }, 0 },
	{ { FLOAT_TO_GEOMV(671.223206), FLOAT_TO_GEOMV(404.319489), FLOAT_TO_GEOMV(161.199265) }, 3 },
	{ { FLOAT_TO_GEOMV(191.492874), FLOAT_TO_GEOMV(-466.763885), FLOAT_TO_GEOMV(-620.855835) }, 3 },
	{ { FLOAT_TO_GEOMV(641.521118), FLOAT_TO_GEOMV(-129.128448), FLOAT_TO_GEOMV(-460.191833) }, 10 },
	{ { FLOAT_TO_GEOMV(134.000717), FLOAT_TO_GEOMV(721.874817), FLOAT_TO_GEOMV(-317.711365) }, 4 },
	{ { FLOAT_TO_GEOMV(-80.673073), FLOAT_TO_GEOMV(436.498932), FLOAT_TO_GEOMV(665.552856) }, 5 },
	{ { FLOAT_TO_GEOMV(-320.688080), FLOAT_TO_GEOMV(-711.269714), FLOAT_TO_GEOMV(-176.789581) }, 0 },
	{ { FLOAT_TO_GEOMV(-30.840954), FLOAT_TO_GEOMV(709.341980), FLOAT_TO_GEOMV(368.622833) }, 5 },
	{ { FLOAT_TO_GEOMV(350.584167), FLOAT_TO_GEOMV(-497.004639), FLOAT_TO_GEOMV(-519.689514) }, 4 },
	{ { FLOAT_TO_GEOMV(403.195740), FLOAT_TO_GEOMV(-606.356384), FLOAT_TO_GEOMV(331.308136) }, 0 },
	{ { FLOAT_TO_GEOMV(750.887634), FLOAT_TO_GEOMV(50.479843), FLOAT_TO_GEOMV(-271.329163) }, 4 },
	{ { FLOAT_TO_GEOMV(-515.707825), FLOAT_TO_GEOMV(436.471436), FLOAT_TO_GEOMV(-428.413513) }, 0 },
	{ { FLOAT_TO_GEOMV(-220.513138), FLOAT_TO_GEOMV(-671.371216), FLOAT_TO_GEOMV(-375.012787) }, 3 },
	{ { FLOAT_TO_GEOMV(299.884338), FLOAT_TO_GEOMV(552.479187), FLOAT_TO_GEOMV(-494.809143) }, 0 },
	{ { FLOAT_TO_GEOMV(-226.605698), FLOAT_TO_GEOMV(-606.584778), FLOAT_TO_GEOMV(-469.792328) }, 7 },
	{ { FLOAT_TO_GEOMV(674.359436), FLOAT_TO_GEOMV(424.051331), FLOAT_TO_GEOMV(-73.620026) }, 0 },
	{ { FLOAT_TO_GEOMV(20.439421), FLOAT_TO_GEOMV(-527.045044), FLOAT_TO_GEOMV(-601.502930) }, 4 },
	{ { FLOAT_TO_GEOMV(-531.221252), FLOAT_TO_GEOMV(-586.482544), FLOAT_TO_GEOMV(-117.653030) }, 4 },
	{ { FLOAT_TO_GEOMV(194.851013), FLOAT_TO_GEOMV(748.513062), FLOAT_TO_GEOMV(204.355942) }, 3 },
	{ { FLOAT_TO_GEOMV(-405.742279), FLOAT_TO_GEOMV(-681.814331), FLOAT_TO_GEOMV(-102.481300) }, 4 },
	{ { FLOAT_TO_GEOMV(-68.533798), FLOAT_TO_GEOMV(-167.527054), FLOAT_TO_GEOMV(-779.254639) }, 3 },
	{ { FLOAT_TO_GEOMV(470.247803), FLOAT_TO_GEOMV(-549.886536), FLOAT_TO_GEOMV(-341.308899) }, 0 },
	{ { FLOAT_TO_GEOMV(553.913696), FLOAT_TO_GEOMV(-410.576385), FLOAT_TO_GEOMV(-405.717468) }, 0 },
	{ { FLOAT_TO_GEOMV(640.738586), FLOAT_TO_GEOMV(-415.496002), FLOAT_TO_GEOMV(-238.363495) }, 0 },
	{ { FLOAT_TO_GEOMV(485.016602), FLOAT_TO_GEOMV(461.107330), FLOAT_TO_GEOMV(438.336578) }, 0 },
	{ { FLOAT_TO_GEOMV(129.355270), FLOAT_TO_GEOMV(-587.978516), FLOAT_TO_GEOMV(-526.828735) }, 3 },
	{ { FLOAT_TO_GEOMV(-720.304321), FLOAT_TO_GEOMV(-227.464523), FLOAT_TO_GEOMV(263.479736) }, 4 },
	{ { FLOAT_TO_GEOMV(-362.319855), FLOAT_TO_GEOMV(-78.765190), FLOAT_TO_GEOMV(708.886719) }, 0 },
	{ { FLOAT_TO_GEOMV(-730.826294), FLOAT_TO_GEOMV(-74.104767), FLOAT_TO_GEOMV(-316.861755) }, 0 },
	{ { FLOAT_TO_GEOMV(143.809753), FLOAT_TO_GEOMV(-678.915771), FLOAT_TO_GEOMV(397.985107) }, 0 },
	{ { FLOAT_TO_GEOMV(-522.056396), FLOAT_TO_GEOMV(67.853485), FLOAT_TO_GEOMV(-602.372803) }, 0 },
	{ { FLOAT_TO_GEOMV(600.333252), FLOAT_TO_GEOMV(508.950592), FLOAT_TO_GEOMV(143.419998) }, 3 },
	{ { FLOAT_TO_GEOMV(678.904663), FLOAT_TO_GEOMV(-396.453094), FLOAT_TO_GEOMV(-148.031845) }, 2 },
	{ { FLOAT_TO_GEOMV(41.491188), FLOAT_TO_GEOMV(336.716949), FLOAT_TO_GEOMV(-724.499939) }, 0 },
	{ { FLOAT_TO_GEOMV(46.577557), FLOAT_TO_GEOMV(631.384644), FLOAT_TO_GEOMV(489.064331) }, 2 },
	{ { FLOAT_TO_GEOMV(-437.191986), FLOAT_TO_GEOMV(-217.633026), FLOAT_TO_GEOMV(-633.639526) }, 2 },
	{ { FLOAT_TO_GEOMV(486.767700), FLOAT_TO_GEOMV(-216.341202), FLOAT_TO_GEOMV(-596.869934) }, 0 },
	{ { FLOAT_TO_GEOMV(-670.834900), FLOAT_TO_GEOMV(433.025696), FLOAT_TO_GEOMV(49.691475) }, 24 },
	{ { FLOAT_TO_GEOMV(-278.446259), FLOAT_TO_GEOMV(-693.066956), FLOAT_TO_GEOMV(286.576080) }, 0 },
	{ { FLOAT_TO_GEOMV(343.682953), FLOAT_TO_GEOMV(357.430298), FLOAT_TO_GEOMV(-627.794250) }, 4 },
	{ { FLOAT_TO_GEOMV(-472.287018), FLOAT_TO_GEOMV(-33.302288), FLOAT_TO_GEOMV(644.853455) }, 0 },
	{ { FLOAT_TO_GEOMV(-159.171249), FLOAT_TO_GEOMV(514.245544), FLOAT_TO_GEOMV(591.790527) }, 0 },
	{ { FLOAT_TO_GEOMV(337.887085), FLOAT_TO_GEOMV(-382.827728), FLOAT_TO_GEOMV(615.853271) }, 0 },
	{ { FLOAT_TO_GEOMV(-7.476309), FLOAT_TO_GEOMV(0.000000), FLOAT_TO_GEOMV(799.965088) }, 0 },
	{ { FLOAT_TO_GEOMV(772.895325), FLOAT_TO_GEOMV(-147.708878), FLOAT_TO_GEOMV(144.273788) }, 42 },
	{ { FLOAT_TO_GEOMV(46.655708), FLOAT_TO_GEOMV(-418.396332), FLOAT_TO_GEOMV(680.270325) }, 3 },
	{ { FLOAT_TO_GEOMV(467.555939), FLOAT_TO_GEOMV(-575.544739), FLOAT_TO_GEOMV(-300.232635) }, 0 },
	{ { FLOAT_TO_GEOMV(-496.358612), FLOAT_TO_GEOMV(-301.839691), FLOAT_TO_GEOMV(550.018982) }, 0 },
	{ { FLOAT_TO_GEOMV(-481.559998), FLOAT_TO_GEOMV(414.283234), FLOAT_TO_GEOMV(-486.281158) }, 3 },
	{ { FLOAT_TO_GEOMV(-484.269714), FLOAT_TO_GEOMV(-480.669189), FLOAT_TO_GEOMV(417.660126) }, 3 },
	{ { FLOAT_TO_GEOMV(-183.531372), FLOAT_TO_GEOMV(771.244202), FLOAT_TO_GEOMV(-107.231812) }, 4 },
	{ { FLOAT_TO_GEOMV(215.779373), FLOAT_TO_GEOMV(179.412064), FLOAT_TO_GEOMV(-749.166565) }, 2 },
	{ { FLOAT_TO_GEOMV(466.668640), FLOAT_TO_GEOMV(562.395569), FLOAT_TO_GEOMV(-325.471466) }, 5 },
	{ { FLOAT_TO_GEOMV(405.082092), FLOAT_TO_GEOMV(-480.518311), FLOAT_TO_GEOMV(494.985504) }, 0 },
	{ { FLOAT_TO_GEOMV(586.878296), FLOAT_TO_GEOMV(-184.447464), FLOAT_TO_GEOMV(-511.422516) }, 0 },
	{ { FLOAT_TO_GEOMV(46.208626), FLOAT_TO_GEOMV(728.903809), FLOAT_TO_GEOMV(-326.441559) }, 0 },
	{ { FLOAT_TO_GEOMV(660.017883), FLOAT_TO_GEOMV(-440.948120), FLOAT_TO_GEOMV(99.704834) }, 5 },
	{ { FLOAT_TO_GEOMV(-484.196533), FLOAT_TO_GEOMV(-634.115112), FLOAT_TO_GEOMV(58.751881) }, 4 },
	{ { FLOAT_TO_GEOMV(-126.043976), FLOAT_TO_GEOMV(-623.217468), FLOAT_TO_GEOMV(485.502716) }, 0 },
	{ { FLOAT_TO_GEOMV(272.788818), FLOAT_TO_GEOMV(-752.053162), FLOAT_TO_GEOMV(-1.507121) }, 5 },
	{ { FLOAT_TO_GEOMV(-485.906403), FLOAT_TO_GEOMV(558.962219), FLOAT_TO_GEOMV(-302.417267) }, 0 },
	{ { FLOAT_TO_GEOMV(306.976440), FLOAT_TO_GEOMV(-738.662048), FLOAT_TO_GEOMV(11.991267) }, 2 },
	{ { FLOAT_TO_GEOMV(-477.403046), FLOAT_TO_GEOMV(482.550262), FLOAT_TO_GEOMV(423.357422) }, 2 },
	{ { FLOAT_TO_GEOMV(-155.223495), FLOAT_TO_GEOMV(-634.753235), FLOAT_TO_GEOMV(461.512726) }, 5 },
	{ { FLOAT_TO_GEOMV(311.193970), FLOAT_TO_GEOMV(-224.514374), FLOAT_TO_GEOMV(701.962646) }, 4 },
	{ { FLOAT_TO_GEOMV(27.945837), FLOAT_TO_GEOMV(-580.652405), FLOAT_TO_GEOMV(549.601440) }, 3 },
	{ { FLOAT_TO_GEOMV(29.694841), FLOAT_TO_GEOMV(796.361633), FLOAT_TO_GEOMV(-70.187805) }, 2 },
	{ { FLOAT_TO_GEOMV(299.724457), FLOAT_TO_GEOMV(454.382294), FLOAT_TO_GEOMV(586.261047) }, 0 },
	{ { FLOAT_TO_GEOMV(423.694000), FLOAT_TO_GEOMV(-228.660248), FLOAT_TO_GEOMV(638.903625) }, 3 },
	{ { FLOAT_TO_GEOMV(-447.458588), FLOAT_TO_GEOMV(-283.671844), FLOAT_TO_GEOMV(-599.425659) }, 8 },
	{ { FLOAT_TO_GEOMV(-296.315765), FLOAT_TO_GEOMV(-485.404419), FLOAT_TO_GEOMV(562.654053) }, 3 },
	{ { FLOAT_TO_GEOMV(590.387268), FLOAT_TO_GEOMV(-176.253403), FLOAT_TO_GEOMV(510.272095) }, 0 },
	{ { FLOAT_TO_GEOMV(53.837811), FLOAT_TO_GEOMV(-643.361816), FLOAT_TO_GEOMV(-472.426788) }, 5 },
	{ { FLOAT_TO_GEOMV(-628.672363), FLOAT_TO_GEOMV(426.370453), FLOAT_TO_GEOMV(250.956787) }, 5 },
	{ { FLOAT_TO_GEOMV(632.829346), FLOAT_TO_GEOMV(460.354828), FLOAT_TO_GEOMV(-166.133560) }, 2 },
	{ { FLOAT_TO_GEOMV(-669.068726), FLOAT_TO_GEOMV(317.926300), FLOAT_TO_GEOMV(302.109070) }, 4 },
	{ { FLOAT_TO_GEOMV(479.151947), FLOAT_TO_GEOMV(35.405315), FLOAT_TO_GEOMV(-639.656006) }, 0 },
	{ { FLOAT_TO_GEOMV(586.194275), FLOAT_TO_GEOMV(-436.409607), FLOAT_TO_GEOMV(-325.458008) }, 0 },
	{ { FLOAT_TO_GEOMV(-70.186890), FLOAT_TO_GEOMV(792.373108), FLOAT_TO_GEOMV(84.963081) }, 0 },
	{ { FLOAT_TO_GEOMV(703.276367), FLOAT_TO_GEOMV(-263.540405), FLOAT_TO_GEOMV(275.587952) }, 0 },
	{ { FLOAT_TO_GEOMV(-567.907898), FLOAT_TO_GEOMV(453.050110), FLOAT_TO_GEOMV(-335.001831) }, 2 },
	{ { FLOAT_TO_GEOMV(397.815338), FLOAT_TO_GEOMV(448.088715), FLOAT_TO_GEOMV(530.056152) }, 4 },
	{ { FLOAT_TO_GEOMV(551.638672), FLOAT_TO_GEOMV(209.125305), FLOAT_TO_GEOMV(540.334595) }, 0 },
	{ { FLOAT_TO_GEOMV(-48.199692), FLOAT_TO_GEOMV(-271.123260), FLOAT_TO_GEOMV(-751.111877) }, 3 },
	{ { FLOAT_TO_GEOMV(211.834045), FLOAT_TO_GEOMV(-572.303040), FLOAT_TO_GEOMV(-517.296387) }, 0 },
	{ { FLOAT_TO_GEOMV(-541.188110), FLOAT_TO_GEOMV(494.128265), FLOAT_TO_GEOMV(320.862488) }, 0 },
	{ { FLOAT_TO_GEOMV(556.163818), FLOAT_TO_GEOMV(385.838654), FLOAT_TO_GEOMV(426.392273) }, 0 },
	{ { FLOAT_TO_GEOMV(230.868805), FLOAT_TO_GEOMV(-716.810364), FLOAT_TO_GEOMV(269.967560) }, 0 },
	{ { FLOAT_TO_GEOMV(-147.968475), FLOAT_TO_GEOMV(706.066956), FLOAT_TO_GEOMV(-345.795898) }, 0 },
	{ { FLOAT_TO_GEOMV(-571.498291), FLOAT_TO_GEOMV(-196.823059), FLOAT_TO_GEOMV(-524.071045) }, 3 },
	{ { FLOAT_TO_GEOMV(433.007050), FLOAT_TO_GEOMV(611.393799), FLOAT_TO_GEOMV(280.539795) }, 0 },
	{ { FLOAT_TO_GEOMV(-529.411926), FLOAT_TO_GEOMV(297.886383), FLOAT_TO_GEOMV(520.563843) }, 0 },
	{ { FLOAT_TO_GEOMV(51.372635), FLOAT_TO_GEOMV(-251.358963), FLOAT_TO_GEOMV(-757.746338) }, 5 },
	{ { FLOAT_TO_GEOMV(-333.648987), FLOAT_TO_GEOMV(-492.289642), FLOAT_TO_GEOMV(-535.097412) }, 0 },
	{ { FLOAT_TO_GEOMV(266.786224), FLOAT_TO_GEOMV(-323.192474), FLOAT_TO_GEOMV(-681.448242) }, 3 },
	{ { FLOAT_TO_GEOMV(543.926880), FLOAT_TO_GEOMV(-586.091736), FLOAT_TO_GEOMV(-25.298925) }, 5 },
	{ { FLOAT_TO_GEOMV(-335.923157), FLOAT_TO_GEOMV(-489.208466), FLOAT_TO_GEOMV(-536.498596) }, 0 },
	{ { FLOAT_TO_GEOMV(760.183960), FLOAT_TO_GEOMV(170.240753), FLOAT_TO_GEOMV(182.039627) }, 3 },
	{ { FLOAT_TO_GEOMV(-241.866882), FLOAT_TO_GEOMV(717.735046), FLOAT_TO_GEOMV(-257.598053) }, 5 },
	{ { FLOAT_TO_GEOMV(-562.366577), FLOAT_TO_GEOMV(-386.477448), FLOAT_TO_GEOMV(417.587097) }, 0 },
	{ { FLOAT_TO_GEOMV(354.775177), FLOAT_TO_GEOMV(-714.898743), FLOAT_TO_GEOMV(55.266483) }, 4 },
	{ { FLOAT_TO_GEOMV(113.737640), FLOAT_TO_GEOMV(-35.649109), FLOAT_TO_GEOMV(791.070740) }, 0 },
	{ { FLOAT_TO_GEOMV(618.435242), FLOAT_TO_GEOMV(-255.405716), FLOAT_TO_GEOMV(-438.526794) }, 3 },
	{ { FLOAT_TO_GEOMV(739.628479), FLOAT_TO_GEOMV(298.446564), FLOAT_TO_GEOMV(-62.284504) }, 0 },
	{ { FLOAT_TO_GEOMV(-543.831604), FLOAT_TO_GEOMV(473.792694), FLOAT_TO_GEOMV(346.074677) }, 3 },
	{ { FLOAT_TO_GEOMV(40.596317), FLOAT_TO_GEOMV(230.758011), FLOAT_TO_GEOMV(-764.920044) }, 2 },
	{ { FLOAT_TO_GEOMV(-138.996796), FLOAT_TO_GEOMV(-692.258545), FLOAT_TO_GEOMV(376.108978) }, 5 },
	{ { FLOAT_TO_GEOMV(-46.234795), FLOAT_TO_GEOMV(549.950684), FLOAT_TO_GEOMV(579.151611) }, 5 },
	{ { FLOAT_TO_GEOMV(702.487610), FLOAT_TO_GEOMV(-314.312134), FLOAT_TO_GEOMV(218.446930) }, 0 },
	{ { FLOAT_TO_GEOMV(623.936951), FLOAT_TO_GEOMV(485.852570), FLOAT_TO_GEOMV(121.036957) }, 4 },
	{ { FLOAT_TO_GEOMV(417.943207), FLOAT_TO_GEOMV(-663.052734), FLOAT_TO_GEOMV(160.263931) }, 3 },
	{ { FLOAT_TO_GEOMV(-125.345833), FLOAT_TO_GEOMV(-242.222351), FLOAT_TO_GEOMV(-752.074951) }, 0 },
}
#endif
;


// init fixed star positions --------------------------------------------------
//
void InitFixedStars()
{
	ASSERT( NumFixedStars == 0 );

#ifdef USE_FIXED_STARS_INIT_TAB

	ASSERT( CALC_NUM_ARRAY_ENTRIES( fixed_stars_init_tab ) == MAX_FIXED_STARS );

	for ( ; NumFixedStars < MAX_FIXED_STARS; NumFixedStars++ ) {

		FixedStars[ NumFixedStars ].location.X = fixed_stars_init_tab[ NumFixedStars ].location.X;
		FixedStars[ NumFixedStars ].location.Y = fixed_stars_init_tab[ NumFixedStars ].location.Y;
		FixedStars[ NumFixedStars ].location.Z = fixed_stars_init_tab[ NumFixedStars ].location.Z;
		FixedStars[ NumFixedStars ].type	   = fixed_stars_init_tab[ NumFixedStars ].type;
	}

#else

	#ifdef SAVE_FIXED_STARS_INIT_TAB
		FILE *fp = fopen( "fixstars.txt", "w" );
	#endif

	float	radius = 800.0f;

	for ( ; NumFixedStars < MAX_FIXED_STARS; NumFixedStars++ ) {

		float x = ( RAND() % 1000 ) - 500;
		float y = ( RAND() % 1000 ) - 500;
		float z = ( RAND() % 1000 ) - 500;
		float r = sqrt( x*x + y*y + z*z );
		x = x / r * radius;
		y = y / r * radius;
		z = z / r * radius;
		int t = RAND() % 53;

		FixedStars[ NumFixedStars ].location.X = FLOAT_TO_GEOMV( x );
		FixedStars[ NumFixedStars ].location.Y = FLOAT_TO_GEOMV( y );
		FixedStars[ NumFixedStars ].location.Z = FLOAT_TO_GEOMV( z );

		dword type;
		switch ( NumFixedStars ) {

			case SUN_STAR_NO:
				type = BM_SUN;
				break;

			case 45:
				type = BM_NEBULA;
				break;

			case 2:
				type = BM_BIGPLANET1;
				break;

			case 5:
				type = BM_MEDPLANET1;
				break;

			case 11:
				type = BM_MINPLANET1;
				break;

			case 22:
				type = BM_SCLSTR1;
				break;

			case 77:
				type = BM_SCLSTR2;
				break;

			default:

				type = 0;

				if ( ( t >= 1 ) && ( t <= 5 ) )
					type = BM_MSTAR1;
				else if ( ( t >= 6 ) && ( t <= 14 ) )
					type = BM_MSTAR2;
				else if ( ( t >= 15 ) && ( t <= 22 ) )
					type = BM_MSTAR3;
				else if ( ( t >= 23 ) && ( t <= 27 ) )
					type = BM_MSTAR4;

		}

		FixedStars[ NumFixedStars ].type = type;

		#ifdef SAVE_FIXED_STARS_INIT_TAB
			if ( fp != NULL )
				fprintf( fp, "\t{ { FLOAT_TO_GEOMV(%f), FLOAT_TO_GEOMV(%f), FLOAT_TO_GEOMV(%f) }, %d },\n",
					GEOMV_TO_FLOAT( FixedStars[ NumFixedStars ].location.X ),
					GEOMV_TO_FLOAT( FixedStars[ NumFixedStars ].location.Y ),
					GEOMV_TO_FLOAT( FixedStars[ NumFixedStars ].location.Z ),
					FixedStars[ NumFixedStars ].type );
		#endif
	}

	#ifdef SAVE_FIXED_STARS_INIT_TAB
		if ( fp != NULL )
			fclose( fp );
	#endif

#endif

}


// draw fixedstars in background (dots, bitmaps, rotating bitmaps ) -----------
//
void DrawFixedStars()
{
	visual_t scol = COLINDX_TO_VISUAL( FIXED_STAR_COLOR );

	Camera fixedstarcam;
	CAMERA_MakeFixedStarCam( fixedstarcam );

	for ( int i = 0; i < NumFixedStars; i++ ) {

		Vertex3 tempvert;
		MtxVctMUL( fixedstarcam, &FixedStars[ i ].location, &tempvert );

		if ( tempvert.Z > near_clip_z ) {

			SPoint loc;
			PROJECT_TO_SCREEN( tempvert, loc );

			if ( FixedStars[ i ].type == 0 ) {

				D_DrawSquare( loc.X, loc.Y, scol, Star_Siz );

			} else {
				
				int j = FixedStars[ i ].type;
				if ( ( loc.X > -BitmapInfo[ j ].width/2  ) &&
					 ( loc.X < Screen_Width+BitmapInfo[ j ].width/2 ) &&
					 ( loc.Y > -BitmapInfo[ j ].height/2 ) &&
					 ( loc.Y < Screen_Height+BitmapInfo[ j ].height/2 ) ) {

					D_PutClipBitmap( BitmapInfo[ j ].bitmappointer,
								   BitmapInfo[ j ].width,
								   BitmapInfo[ j ].height,
								   loc.X - BitmapInfo[ j ].width/2,
								   loc.Y - BitmapInfo[ j ].height/2 );
				}
			}
		}
	}
}


// calculate random position of pseudo star -----------------------------------
//
PRIVATE
void CalcPseudoStarPosition( int no )
{
	ASSERT( (dword)no < MAX_PSEUDO_STARS );

	int rndx = ( RAND() & 0xff ) - 128;
	int rndy = ( RAND() & 0xff ) - 128;

	// leave out small area in the center
	if ( ( rndx > -30 ) && ( rndx < 30 ) ) {
		if ( ( rndy > -30 ) && ( rndy < 30 ) ) {

			long ax = rndx;
			long ay = rndy;
			ABS32( ax );
			ABS32( ay );

			if ( ax > ay ) {
				if ( rndx < 0 ) {
					rndx = -30;
				} else {
					rndx = 30;
				}
			} else {
				if ( rndy < 0 ) {
					rndy = -30;
				} else {
					rndy = 30;
				}
			}
		}
	}

	PseudoStars[ no ].X = INT_TO_GEOMV( rndx );
	PseudoStars[ no ].Y = INT_TO_GEOMV( rndy );

	int rndz = ( RAND() % 700 ) + 100;
	PseudoStars[ no ].Z = INT_TO_GEOMV( rndz );
}


// init pseudo star positions -------------------------------------------------
//
void InitPseudoStars()
{
	while ( NumPseudoStars < MaxPseudoStars ) {

		CalcPseudoStarPosition( NumPseudoStars );
		NumPseudoStars++;
	}
}


// draw single pseudo star ----------------------------------------------------
//
int DrawPseudoStar_( int x, int y, geomv_t z )
{
	#define STAR_DIST_1	FLOAT_TO_GEOMV( 592.0 )	// far
	#define STAR_DIST_2	FLOAT_TO_GEOMV( 368.0 )
	#define STAR_DIST_3	FLOAT_TO_GEOMV( 320.0 )
	#define STAR_DIST_4	FLOAT_TO_GEOMV( 256.0 )
	#define STAR_DIST_5	FLOAT_TO_GEOMV( 128.0 )	// near

	int col;
	int siz;

	if ( z > STAR_DIST_1 ) {
		col = 64;
		siz = 1;
	} else if ( z > STAR_DIST_2 ) {
		col = 56;
		siz = 1;
	} else if ( z > STAR_DIST_3 ) {
		col = 48;
		siz = Star_Siz - 2;
	} else if ( z > STAR_DIST_4 ) {
		col = 40;
		siz = Star_Siz - 1;
	} else if ( z > STAR_DIST_5 ) {
		col = 32;
		siz = Star_Siz;
	} else {
		col = 24;
		siz = Star_Siz;
	}

	if ( siz < 1 )
		siz = 1;

	return D_DrawSquare( x, y, COLINDX_TO_VISUAL( col ), siz );
}


// temp storage for pseudo star drawing ---------------------------------------
//
#define MAX_PSEUDO_STAR_SIZES	40

static int			num_scheduled_pseudo_stars[ MAX_PSEUDO_STAR_SIZES ];
static IterPoint2*	scheduled_pseudo_stars[ MAX_PSEUDO_STAR_SIZES ];

float pseudo_stars_size_tab[] = { 1.4f, 2.0f, 2.8f, 3.4f };


// determine gray level of pseudo star according to distance ------------------
//
INLINE
int GetPseudoStarGrayLevel( geomv_t z )
{
	#define STAR_GRAY_MIN 100.0f
	#define STAR_GRAY_MAX 230.0f

	byte graylvl;

	if ( z > STAR_DIST_1 ) {
		graylvl = (int)STAR_GRAY_MIN;
	} else if ( z < STAR_DIST_5 ) {
		graylvl = (int)STAR_GRAY_MAX;
	} else {
		float zrange = GEOMV_TO_FLOAT( STAR_DIST_1 - STAR_DIST_5 );
		float zramp  = GEOMV_TO_FLOAT( z - STAR_DIST_5 ) / zrange;
		int grayramp   = (int)(STAR_GRAY_MAX - ( STAR_GRAY_MAX - STAR_GRAY_MIN ) * zramp);
		if ( grayramp < 0 )
			grayramp = 0;
		if ( grayramp > 255 )
			grayramp = 255;
		graylvl = grayramp;
	}

	return graylvl;
}


// determine discrete size of pseudo star according to distance ---------------
//
INLINE
int GetPseudoStarSize( geomv_t z )
{
	int starsize = Star_Siz;

	if ( z > STAR_DIST_1 ) {
		starsize = 1;
	} else if ( z > STAR_DIST_2 ) {
		starsize = 1;
	} else if ( z > STAR_DIST_3 ) {
		starsize = Star_Siz - 2;
	} else if ( z > STAR_DIST_4 ) {
		starsize = Star_Siz - 1;
	} else if ( z > STAR_DIST_5 ) {
		starsize = Star_Siz - 1;
	}

	if ( starsize < 1 ) {
		starsize = 1;
	}

	ASSERT( starsize >= 1 );
	ASSERT( starsize <= MAX_PSEUDO_STAR_SIZES );

	return starsize;
}


// schedule single pseudo star ------------------------------------------------
//
PRIVATE
int SchedulePseudoStar( int x, int y, geomv_t z )
{
	// determine smooth gray level
	byte graylvl = GetPseudoStarGrayLevel( z );

	// determine discrete size
	int starsize = GetPseudoStarSize( z );

	// schedule point for drawing later on
	ASSERT( scheduled_pseudo_stars[ starsize - 1 ] != NULL );
	IterVertex2 *vtx = &scheduled_pseudo_stars[ starsize - 1 ]->Vtxs[
		num_scheduled_pseudo_stars[ starsize - 1 ]++ ];

	vtx->X     = INT_TO_RASTV( x );
	vtx->Y     = INT_TO_RASTV( y );
	vtx->Z     = RASTV_0;
	vtx->R     = graylvl;
	vtx->G     = graylvl;
	vtx->B     = graylvl;
	vtx->A     = 255;
	vtx->flags = ITERVTXFLAG_NONE;

	return TRUE;
}


// draw single pseudo star as line --------------------------------------------
//
PRIVATE
int DrawPseudoStarLine( int x1, int y1, int x2, int y2, geomv_t z )
{
	// determine smooth gray level
	byte graylvl = GetPseudoStarGrayLevel( z );

	// determine discrete size
	int starsize = GetPseudoStarSize( z );

	IterLine2 itline;
	itline.NumVerts  = 2;
	itline.flags	 = ITERFLAG_LS_ANTIALIASED;
	itline.itertype  = iter_rgba | iter_alphablend;
	itline.raststate = rast_default;
	itline.rastmask  = rast_nomask;

	itline.Vtxs[ 0 ].X 	   = INT_TO_RASTV( x1 );
	itline.Vtxs[ 0 ].Y 	   = INT_TO_RASTV( y1 );
	itline.Vtxs[ 0 ].Z	   = RASTV_0;
	itline.Vtxs[ 0 ].R 	   = graylvl;
	itline.Vtxs[ 0 ].G 	   = graylvl;
	itline.Vtxs[ 0 ].B 	   = graylvl;
	itline.Vtxs[ 0 ].A 	   = 255;
	itline.Vtxs[ 0 ].flags = ITERVTXFLAG_NONE;

	itline.Vtxs[ 1 ].X 	   = INT_TO_RASTV( x2 );
	itline.Vtxs[ 1 ].Y 	   = INT_TO_RASTV( y2 );
	itline.Vtxs[ 1 ].Z	   = RASTV_0;
	itline.Vtxs[ 1 ].R 	   = graylvl;
	itline.Vtxs[ 1 ].G 	   = graylvl;
	itline.Vtxs[ 1 ].B 	   = graylvl;
	itline.Vtxs[ 1 ].A 	   = 255;
	itline.Vtxs[ 1 ].flags = ITERVTXFLAG_NONE;

	Rectangle2 cliprect;

	cliprect.left   = 0;
	cliprect.right  = Screen_Width;
	cliprect.top    = 0;
	cliprect.bottom = Screen_Height;

	IterLine2 *clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
	if ( clipline != NULL )
		D_DrawIterLine2( clipline );

	return TRUE;
}


// draw pseudostars in background (moving random stars) -----------------------
//
void DrawPseudoStars()
{
	if ( NumPseudoStars < 1 ) {
		return;
	}

#ifdef USE_ITER_PSEUDO_STARS

	// use lines from previous position to current position instead
	// of points if the afterburner is currently active
	int uselines = MyShip->afterburner_active;

	// alloc point array if we are using points
	IterPoint2 *itpmem = NULL;
	if ( !uselines ) {

		// alloc temp point storage
		size_t itpsiz = (size_t)&((IterPoint2*)0)->Vtxs[ NumPseudoStars ];
		itpmem = (IterPoint2 *) ALLOCMEM( itpsiz * MAX_PSEUDO_STAR_SIZES );
		if ( itpmem == NULL )
			OUTOFMEM( 0 );

		// init point arrays for all possible sizes
		for ( int csiz = 0; csiz < MAX_PSEUDO_STAR_SIZES; csiz++ ) {

			num_scheduled_pseudo_stars[ csiz ] = 0;
			scheduled_pseudo_stars[ csiz ] = (IterPoint2 *) ( (char*)itpmem + itpsiz * csiz );

			IterPoint2 *itpoint = scheduled_pseudo_stars[ csiz ];

			itpoint->flags		= ITERFLAG_PS_ANTIALIASED;
			itpoint->itertype	= iter_rgba | iter_alphablend;
			itpoint->raststate	= rast_chromakeyoff;
			itpoint->rastmask	= rast_mask_zbuffer | rast_mask_texclamp |
								  rast_mask_mipmap  | rast_mask_texfilter;
			itpoint->pointsize	= pseudo_stars_size_tab[ csiz ];
		}
	}

#endif

	// process all pseudo stars
	for ( int psno = 0; psno < NumPseudoStars; psno++ ) {

		// calc new position
		Vertex3 tempvert;
		MtxVctMUL( PseudoStarMovement, &PseudoStars[ psno ], &tempvert );

		Vertex3 *pvert;
		Vertex3 framevert;

		SPoint prvloc;
		if ( uselines ) {

			// fetch old position
			Vertex3 oldvert;
			oldvert.X = PseudoStars[ psno ].X;
			oldvert.Y = PseudoStars[ psno ].Y;
			oldvert.Z = PseudoStars[ psno ].Z;

			if ( pseudo_framecam_is_id ) {
				pvert = &oldvert;
			} else {
				// framecam
				MtxVctMUL( pseudo_framecam, &oldvert, &framevert );
				pvert = &framevert;
			}

			PROJECT_TO_SCREEN( *pvert, prvloc );
		}

		// overwrite old position
		PseudoStars[ psno ].X = tempvert.X;
		PseudoStars[ psno ].Y = tempvert.Y;
		PseudoStars[ psno ].Z = tempvert.Z;

		if ( pseudo_framecam_is_id ) {
			pvert = &tempvert;
		} else {
			// framecam
			MtxVctMUL( pseudo_framecam, &tempvert, &framevert );
			pvert = &framevert;
		}

		int starvisible = FALSE;
		if ( ( pvert->Z > near_clip_z ) && ( pvert->Z < far_clip_z ) ) {

			SPoint curloc;
			PROJECT_TO_SCREEN( *pvert, curloc );

			if ( ( curloc.X >= 0 ) && ( curloc.X < Screen_Width ) &&
				 ( curloc.Y >= 0 ) && ( curloc.Y < Screen_Height ) ) {

#ifdef USE_ITER_PSEUDO_STARS
				if ( uselines ) {
					if ( ( curloc.X == prvloc.X ) && ( curloc.Y == prvloc.Y ) ) {
						prvloc.X++;
						prvloc.Y++;
					}
					starvisible = DrawPseudoStarLine( curloc.X, curloc.Y, prvloc.X, prvloc.Y, tempvert.Z );
				} else {
					starvisible = SchedulePseudoStar( curloc.X, curloc.Y, tempvert.Z );
				}
#else
				starvisible = DrawPseudoStar_( curloc.X, curloc.Y, tempvert.Z );
#endif
			}
		}

		// calculate new position if clipped
		if ( !starvisible ) {
			CalcPseudoStarPosition( psno );
		}
	}

#ifdef USE_ITER_PSEUDO_STARS

	if ( !uselines ) {

		// draw scheduled point sets
		for ( int csiz = 0; csiz < MAX_PSEUDO_STAR_SIZES; csiz++ ) {

			IterPoint2 *itpoint = scheduled_pseudo_stars[ csiz ];
			ASSERT( itpoint != NULL );

			// draw sets of each size with a single call
			if ( num_scheduled_pseudo_stars[ csiz ] > 0 ) {
				itpoint->NumVerts = num_scheduled_pseudo_stars[ csiz ];
				D_DrawIterPoint2( itpoint );
			}
		}
	}

	if ( itpmem != NULL ) {
		FREEMEM( itpmem );
		itpmem = NULL;
	}

#endif

	// reset matrix
	MakeIdMatrx( PseudoStarMovement );
}



