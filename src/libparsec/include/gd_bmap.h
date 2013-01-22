/*
 * PARSEC HEADER: gd_bmap.h
 */

#ifndef _GD_BMAP_H_
#define _GD_BMAP_H_


// ----------------------------------------------------------------------------
// BITMAP INDEXES                                                             -
// ----------------------------------------------------------------------------


// indexes of bitmaps in global bitmap table ----------------------------------
//
#define BM_CROSSHAIR		 	0
#define BM_RADAR			    1
#define BM_MSTAR1			    2
#define BM_MSTAR2			    3
#define BM_MSTAR3			    4
#define BM_MSTAR4			    5
#define BM_BIGPLANET1		    6
#define BM_SCLSTR1				7
#define BM_SCLSTR2			    8
#define BM_MEDPLANET1		    9
#define BM_MINPLANET1		    10
#define BM_TARGET			    11

#define BM_EXPLANIMBASE 	    12		// first frame of explosion animation
#define BM_NUMEXPLFRAMES	    12		// number of frames for explosion

#define BM_NEBULA			    24
#define BM_LOGO1			    25
#define BM_LOGO2			    26
#define BM_SHIP1_BLUEPRINT	    27
#define BM_SHIP2_BLUEPRINT	    28
#define BM_ROCKET1			    29
#define BM_ROCKET2			    30
#define BM_CROSSHAIR2		    31
#define BM_CROSSHAIR3		    32
#define BM_MINE				    33
#define BM_GUN1				    34
#define BM_GUN2				    35
#define BM_GUN3				    36
#define BM_OBJSLOGO			    37
#define BM_VIEWLOGO			    38
#define BM_LEFTARROW		    39
#define BM_RIGHTARROW   	    40
#define BM_SELECT			    41
#define BM_SUN				    42
#define BM_LIGHTNING2		    43
#define BM_LIGHTNING1		    44
#define BM_FIREBALL1		    45
#define BM_FIREBALL2		    46
#define BM_SHIELD1			    47
#define BM_SHIELD2			    48
#define BM_PROPFUMES1		    49
#define BM_PROPFUMES2		    50
#define BM_PROPFUMES3		    51
#define BM_FIREBALL3		    52
#define BM_LENSFLARE1			53
#define BM_LENSFLARE2			54
#define BM_LENSFLARE3			55
#define BM_LENSFLARE4			56

#define BM_CONTROLFILE_NUMBER	57		// number of bitmaps in control file


// names of default (control file-loaded) bitmaps -----------------------------
//
#define BM_NAME_CROSSHAIR		"crosshair"
#define BM_NAME_RADAR			"radar"
#define BM_NAME_MSTAR1			"mstar1"
#define BM_NAME_MSTAR2			"mstar2"
#define BM_NAME_MSTAR3			"mstar3"
#define BM_NAME_MSTAR4			"mstar4"
#define BM_NAME_BIGPLANET1		"bigplanet1"
#define BM_NAME_SCLSTR1			"sclstr1"
#define BM_NAME_SCLSTR2			"sclstr2"
#define BM_NAME_MEDPLANET1		"medplanet1"
#define BM_NAME_MINPLANET1		"minplanet1"
#define BM_NAME_TARGET			"target"
#define BM_NAME_EXPLANIM01		"explanim01"
#define BM_NAME_EXPLANIM02		"explanim02"
#define BM_NAME_EXPLANIM03		"explanim03"
#define BM_NAME_EXPLANIM04		"explanim04"
#define BM_NAME_EXPLANIM05		"explanim05"
#define BM_NAME_EXPLANIM06		"explanim06"
#define BM_NAME_EXPLANIM07		"explanim07"
#define BM_NAME_EXPLANIM08		"explanim08"
#define BM_NAME_EXPLANIM09		"explanim09"
#define BM_NAME_EXPLANIM10		"explanim10"
#define BM_NAME_EXPLANIM11		"explanim11"
#define BM_NAME_EXPLANIM12		"explanim12"
#define BM_NAME_NEBULA			"nebula"
#define BM_NAME_LOGO1			"logo1"
#define BM_NAME_LOGO2			"logo2"
#define BM_NAME_SHIP1_BLUEPRINT	"ship1_blueprint"
#define BM_NAME_SHIP2_BLUEPRINT	"ship2_blueprint"
#define BM_NAME_ROCKET1			"rocket1"
#define BM_NAME_ROCKET2			"rocket2"
#define BM_NAME_CROSSHAIR2		"crosshair2"
#define BM_NAME_CROSSHAIR3		"crosshair3"
#define BM_NAME_MINE			"mine"
#define BM_NAME_GUN1			"gun1"
#define BM_NAME_GUN2			"gun2"
#define BM_NAME_GUN3			"gun3"
#define BM_NAME_OBJSLOGO		"objslogo"
#define BM_NAME_VIEWLOGO		"viewlogo"
#define BM_NAME_LEFTARROW		"leftarrow"
#define BM_NAME_RIGHTARRORW		"rightarrow"
#define BM_NAME_SELECT			"select"
#define BM_NAME_SUN				"sun"
#define BM_NAME_LIGHTNING2		"lightning2"
#define BM_NAME_LIGHTNING1		"lightning1"
#define BM_NAME_FIREBALL1		"fireball1"
#define BM_NAME_FIREBALL2		"fireball2"
#define BM_NAME_SHIELD1			"shield1"
#define BM_NAME_SHIELD2			"shield2"
#define BM_NAME_PROPFUMES1		"propfumes1"
#define BM_NAME_PROPFUMES2		"propfumes2"
#define BM_NAME_PROPFUMES3		"propfumes3"
#define BM_NAME_FIREBALL3		"fireball3"
#define BM_NAME_LENSFLARE1		"lensflare1"
#define BM_NAME_LENSFLARE2		"lensflare2"
#define BM_NAME_LENSFLARE3		"lensflare3"
#define BM_NAME_LENSFLARE4		"lensflare4"


// names of non-default bitmaps (which have no predefined id) -----------------
//
#define BM_NAME_RADAR2			"radar2"


// explosion animation definitions --------------------------------------------
//
#define BM_EXPLVANISHFRAME		5		// the target vanishes in this frame
#define BM_EXPLNOTARGFRAME		3		// the target is lost in this frame
#define BM_EXPLPARTCLFRAME		3		// particle explosion starts in this frame

#define EXPL_REF_SPEED			64
#define MAX_EXPLOSION_COUNT		( BM_NUMEXPLFRAMES * EXPL_REF_SPEED - 1 )


#endif // _GD_BMAP_H_


