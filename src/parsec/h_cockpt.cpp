/*
 * PARSEC - Cockpit
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1999-2000
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1999-2000
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"
#include "od_class.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "net_defs.h"
#include "sys_defs.h"

// drawing subsystem
#include "d_bmap.h"
#include "d_font.h"
#include "d_iter.h"
#include "d_misc.h"

// mathematics header
#include "utl_math.h"

// model header
#include "utl_model.h"

// local module header
#include "h_cockpt.h"

// proprietary module headers
#include "con_arg.h"
#include "con_aux.h"
#include "con_com.h"
#include "con_main.h"
#include "con_std.h"
#include "e_callbk.h"
#include "e_color.h"
#include "e_draw.h"
#include "e_supp.h"
#include "h_drwhud.h"
#include "h_radar.h"
#include "obj_creg.h"
#include "obj_ctrl.h"
#include "obj_game.h"
#include "part_api.h"
#include "g_sfx.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char invalid_slot[] 			= "invalid slot number specified.";
static char invalid_texture[]		= "invalid texture specified.";


// ----------------------------------------------------------------------------
//
#define COCKPIT_VISIBLE 		( cp_fadepos == CP_ALPHA_HIGH )

#define INDICATOR_STRENGTH	150

static dword r_flash_ctr = 0;
static dword l_flash_ctr = 0;


// ship monitor geometry (based on 640x480 display) ---------------------------
//
#define LEFTMON_X	27.0f
#define LEFTMON_Y	24.0f
#define LEFTMON_W	102.0f
#define LEFTMON_H	65.0f

#define RIGHTMON_X	126.0f
#define RIGHTMON_Y	25.0f
#define RIGHTMON_W	101.0f
#define RIGHTMON_H	64.0f

#define TEXTURE_W	111
#define TEXTURE_H	64

#define LEFT_DAMAGE_X	0.4f
#define LEFT_DAMAGE_Y	0.0f


// radar lamps geometry (based on 640x480 display, offset from radar texture) -
//
#define LAMP_W				0.029f
#define LAMP_H	 			0.0167f

#define LAMP_ORIGIN_WC_X	0.01719f
#define LAMP_ORIGIN_WC_Y	0.1896f

#define LAMP_ORIGIN_ELITE_X	0.056f
#define LAMP_ORIGIN_ELITE_Y	0.176f

#define LAMP1_X				0.0f
#define LAMP2_X				0.04f
#define LAMP3_X				0.08f


// target tracking square -----------------------------------------------------
//
#define TARGET_SQUARE_OBJ_COL 	255
#define NORMAL_SQUARE_OBJ_COL 	87

extern int TrackingLeftBoundX;
extern int TrackingRightBoundX;
extern int TrackingLeftBoundY;
extern int TrackingRightBoundY;


// cockpit element types ------------------------------------------------------
//
enum {

	COCKPIT_TEXTURES_DAMAGE,
	COCKPIT_TEXTURES_WEAPONS,
	COCKPIT_TEXTURES_RADAR,
	COCKPIT_TEXTURES_ICONBAR_BACKGROUND,
	COCKPIT_TEXTURES_CROSSHAIR,

	COCKPIT_TEXTURES_NUM_TYPES		// must be last in list
};


// cockpit texture validity on type (all textures of type valid?) -------------
//
static int cockpit_tex_valid[ COCKPIT_TEXTURES_NUM_TYPES ];


// type spec for cockpit textures ---------------------------------------------
//
static int cockpit_tex_type[] = {

	COCKPIT_TEXTURES_DAMAGE,				// DAMAGE1
	COCKPIT_TEXTURES_DAMAGE,				// DAMAGE1_SLICE
	COCKPIT_TEXTURES_DAMAGE,				// DAMAGE2
	COCKPIT_TEXTURES_DAMAGE,				// DAMAGE2_SLICE
	COCKPIT_TEXTURES_WEAPONS,				// WEAPONS1
	COCKPIT_TEXTURES_WEAPONS,				// WEAPONS1_SLICE
	COCKPIT_TEXTURES_WEAPONS,				// WEAPONS2
	COCKPIT_TEXTURES_WEAPONS,				// WEAPONS2_WORM
	COCKPIT_TEXTURES_WEAPONS,				// WEAPONS2_ELITE
	COCKPIT_TEXTURES_RADAR,					// RADAR
	COCKPIT_TEXTURES_RADAR,					// RADAR_SLICE
	COCKPIT_TEXTURES_RADAR,					// RADAR_ELITE
	COCKPIT_TEXTURES_ICONBAR_BACKGROUND,	// ICONBAR1
	COCKPIT_TEXTURES_ICONBAR_BACKGROUND,	// ICONBAR2
	COCKPIT_TEXTURES_DAMAGE,				// ENERGYBAR
	COCKPIT_TEXTURES_DAMAGE,				// ENERGYBAR_EMPTY
	COCKPIT_TEXTURES_DAMAGE,				// SPEEDBAR
	COCKPIT_TEXTURES_DAMAGE,				// SPEEDBAR_EMPTY
	COCKPIT_TEXTURES_DAMAGE,				// DAMAGEBAR1
	COCKPIT_TEXTURES_DAMAGE,				// DAMAGEBAR1_EMPTY
	COCKPIT_TEXTURES_DAMAGE,				// DAMAGEBAR2
	COCKPIT_TEXTURES_DAMAGE,				// DAMAGEBAR2_EMPTY
	COCKPIT_TEXTURES_CROSSHAIR,				// CROSS
	COCKPIT_TEXTURES_CROSSHAIR,				// CROSS2
	COCKPIT_TEXTURES_RADAR,					// INCOMING
};


// texture position info ------------------------------------------------------
//
cockpitinfo_s cockpitinfo[] = {

	{    0.0,   0.0,   0.2,  0.266, 128,  128,  NULL, "cp_d1"  },	//offset from left & top
	{    0.2,   0.0,  0.05,  0.266,  32,  128,  NULL, "cp_d1s" },	//offset from left & top
	{    0.2,   0.0,   0.2,  0.266, 128,  128,  NULL, "cp_d2"  },	//offset from right & top
	{    0.3,   0.0,   0.1,  0.266,  64,  128,  NULL, "cp_d2s" },	//offset from right & top

	{    0.0, 0.265,   0.2,  0.266, 128,  128,  NULL, "cp_w1"  },	//offset from left & bottom
	{    0.2, 0.265,  0.05,  0.266,  32,  128,  NULL, "cp_w1s" },	//offset from left & bottom
	{    0.2, 0.265,   0.2,  0.266, 128,  128,  NULL, "cp_w2"  },	//offset from right & bottom

	{    0.4, 0.265,   0.2,  0.266, 128,  128,  NULL, "cp_w2w" },	//offset from right & bottom
	{    0.4, 0.265,   0.2,  0.266, 128,  128,  NULL, "cp_w2e" },	//offset from right & bottom

	{ 0.2591, 0.265,   0.4,  0.266, 256,  128,  NULL, "cp_r1"  },	//offset from x-center of radar (322) and bottom
	{ 0.2591, 0.333,   0.4,  0.066, 256,   32,  NULL, "cp_r1s" },	//offset from x-center of radar (322) and bottom

	{    0.8, 0.265,   0.4,  0.266, 256,  128,  NULL, "cp_r1e" },	//offset from right & bottom

	{  0.234,   0.0,   0.4,  0.266, 256,  128,  NULL, "cp_m1"  },
	{  0.634,   0.0,   0.1,  0.133,  64,   64,  NULL, "cp_m2"  },

	{ 0.2829, 0.029, 0.025,  0.266,  16,  128,  NULL, "cp_b1"  },	//offset from right
	{ 0.2829, 0.029, 0.025,  0.266,  16,  128,  NULL, "cp_b1e" },	//offset from right

	{ 0.2547, 0.029, 0.025,  0.266,  16,  128,  NULL, "cp_b2"  },	//offset from right
	{ 0.2547, 0.029, 0.025,  0.266,  16,  128,  NULL, "cp_b2e" },	//offset from right

	{ 0.0657, 0.196,   0.1,  0.035,  64,   16,  NULL, "cp_b3"  },	//offset from left
	{ 0.0657, 0.196,   0.1,  0.035,  64,   16,  NULL, "cp_b3e" },	//offset from left

	{ 0.1578, 0.202,   0.1,  0.035,  64,   16,  NULL, "cp_b4"  },	//offset from right
	{ 0.1578, 0.202,   0.1,  0.035,  64,   16,  NULL, "cp_b4e" },	//offset from right

	{ 	  -1,    -1,   0.2,  0.266, 128,  128,  NULL, "cross" 	 },
	{ 	  -1,    -1,   0.4,  0.266, 256,  128,  NULL, "cross2" 	 },
	{ 	  -1,    -1,   0.1,  0.067,  64,   32,  NULL, "incoming" },

	{ 0, 0, 0, 0, 0, 0, NULL, NULL }

};


// weapon state ---------------------------------------------------------------
//
#define MAX_WEAPON_SLOTS	12
#define MAX_WEAPON_LEVELS	4

struct weaponslot_s {

	int				slot_filled;
	int				available;
	int				blinking;
	int				curlevel;

	TextureMap*		unavail_texmap;
	TextureMap*		avail_texmap[ MAX_WEAPON_LEVELS ];
	TextureMap*		picture_texmap[ MAX_WEAPON_LEVELS ];
};

#define ICON_OWIDTH		64
#define ICON_OHEIGHT	64
#define ICON_WIDTH		0.028
#define ICON_HEIGHT		0.0375

static weaponslot_s weapon_list[ MAX_WEAPON_SLOTS ] = {

	{ FALSE, TRUE, 	FALSE, 0 },		// laser
	{ FALSE, FALSE,	FALSE, 0 },		// lightning cannon
	{ FALSE, FALSE,	FALSE, 0 },		// helix
	{ FALSE, FALSE, FALSE, 0 },		// photon cannon

	{ FALSE, FALSE, FALSE, 0 },		// dumb missiles
	{ FALSE, FALSE, FALSE, 0 },		// guided missiles
	{ FALSE, FALSE, FALSE, 0 },		// swarm missiles
	{ FALSE, FALSE, FALSE, 0 },		// mines

	{ FALSE, TRUE,  FALSE, 0 },		// emp
	{ FALSE, FALSE, FALSE, 0 },		// invisibility
	{ FALSE, FALSE, FALSE, 0 },		// invulnerability
	{ FALSE, FALSE, FALSE, 0 },		// afterburner
};

// FIXME: rather add a type field to weaponslot_s
// and extend the weaponreg command (to specify type)
#define LASER_SLOT					0
#define HELIX_SLOT					1
#define LIGHTNING_SLOT				2
#define PHOTON_SLOT					3

#define DUMB_SLOT					4
#define GUIDED_SLOT					5
#define SWARM_SLOT					6
#define MINE_SLOT					7

#define EMP_SLOT					8
#define INVIS_SLOT					9
#define INVUL_SLOT					10
#define ABURN_SLOT					11

#define SELECTED_WEAPON_COL 		255 //87


// this is needed by H_SUPP::MaintainMessages() -------------------------------
//
int	cockpit_messagearea_yoffs = 0;


// fading control variables ---------------------------------------------------
//
#define CP_ALPHA_LOW		100
#define CP_ALPHA_HIGH		255

#define CP_FADE_SPEED		10
#define CP_FADE_QUANTUM		10

static int			cp_fadepos 		= CP_ALPHA_HIGH;
static int			cp_fadetarget	= CP_ALPHA_HIGH;
static refframe_t	cp_lastref 		= REFFRAME_INVALID;


// cockpit scale factor -------------------------------------------------------
//
#define SCALE_FACTOR_MIN	0
#define SCALE_FACTOR_MAX	4

int cp_scale = 0;

float scale_tab[ SCALE_FACTOR_MAX + 1 ] = { 1.0f, 0.9f, 0.7f, 0.5f, 0.4f };

float Scaled_Screen_Width 	= 0;
float Scaled_Screen_Height 	= 0;


TextureMap *MonitorTextures[ MAX_SHIP_CLASSES ];


// fade cockpit in (called after the console has been closed) -----------------
//
void FadeInCockpit()
{
	if ( !AUX_DRAW_COCKPIT )
		return;

	cp_fadetarget = CP_ALPHA_HIGH;
}


// fade cockpit out (called when the console is opened) -----------------------
//
void FadeOutCockpit()
{
	if ( !AUX_DRAW_COCKPIT )
		return;

	cp_fadetarget = CP_ALPHA_LOW;
}


// draw energy display bar ----------------------------------------------------
//
void COCKPIT_DrawEnergyBar()
{
	texscreenrect_s rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	#define ENERGY_BAR_LEN	71

	int max_energy	= MyShip->MaxEnergy;
	int energy		= MyShip->CurEnergy;

	if ( energy > max_energy ) {
		energy = max_energy;
	}

	int bar_height = (int)( ( (float) energy / (float) max_energy)  * ENERGY_BAR_LEN);

	// draw empty bar
	rect.x = (int)(Screen_Width - cockpitinfo[ ENERGYBAR_EMPTY ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(cockpitinfo[ ENERGYBAR_EMPTY ].ypos * Scaled_Screen_Height);
	rect.w = cockpitinfo[ ENERGYBAR_EMPTY ].owidth;
	rect.h = cockpitinfo[ ENERGYBAR_EMPTY ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ ENERGYBAR_EMPTY ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ ENERGYBAR_EMPTY ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ ENERGYBAR_EMPTY ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	hprec_t y_offs = ( ( hprec_t ) ENERGY_BAR_LEN ) / ( hprec_t ) 480;
	hprec_t	height = ( ( hprec_t ) bar_height ) / ( hprec_t ) 480;

	rect.x = (int)(Screen_Width - cockpitinfo[ ENERGYBAR ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(( cockpitinfo[ ENERGYBAR ].ypos + y_offs ) * Scaled_Screen_Height);
	rect.w = cockpitinfo[ ENERGYBAR ].owidth;
	rect.h = - bar_height;
	rect.scaled_w = (int)(cockpitinfo[ ENERGYBAR ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(- height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = ENERGY_BAR_LEN;
	rect.texmap   = cockpitinfo[ ENERGYBAR ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );
}


// draw speed display bar -----------------------------------------------------
//
void COCKPIT_DrawSpeedBar()
{
	texscreenrect_s rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	#define SPEED_BAR_LEN	71

	int max_speed	= MyShip->MaxSpeed;
	int speed		= MyShip->CurSpeed;

	if ( speed > max_speed ) {
		speed = max_speed;
	}

	int bar_height = (int)(( (float) speed / (float) max_speed ) * SPEED_BAR_LEN);

	rect.x = (int)(Screen_Width - cockpitinfo[ SPEEDBAR_EMPTY ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(cockpitinfo[ SPEEDBAR_EMPTY ].ypos * Scaled_Screen_Height);
	rect.w = cockpitinfo[ SPEEDBAR_EMPTY ].owidth;
	rect.h = cockpitinfo[ SPEEDBAR_EMPTY ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ SPEEDBAR_EMPTY ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ SPEEDBAR_EMPTY ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ SPEEDBAR_EMPTY ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	hprec_t y_offs = ( ( hprec_t ) SPEED_BAR_LEN ) / ( hprec_t ) 480;
	hprec_t	height = ( ( hprec_t ) bar_height ) / ( hprec_t ) 480;

	rect.x = (int)(Screen_Width - cockpitinfo[ SPEEDBAR ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(( cockpitinfo[ SPEEDBAR ].ypos + y_offs ) * Scaled_Screen_Height);
	rect.w = cockpitinfo[ SPEEDBAR ].owidth;
	rect.h = - bar_height;
	rect.scaled_w = (int)(cockpitinfo[ SPEEDBAR ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(- height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = SPEED_BAR_LEN;
	rect.texmap   = cockpitinfo[ SPEEDBAR ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );
}


// draw remote player damage bar ----------------------------------------------
//
void COCKPIT_DrawRemoteDamageBar()
{
	texscreenrect_s rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	#define DAMAGE1_BAR_LEN	63

	ShipObject 	*targetpo = NULL;
	int 		damage = DAMAGE1_BAR_LEN;

	if ( TargetObjNumber != TARGETID_NO_TARGET ) {

		targetpo = (ShipObject *) FetchFirstShip();
		while ( ( targetpo != NULL ) && ( targetpo->HostObjNumber != TargetObjNumber ) )
			targetpo = (ShipObject *) targetpo->NextObj;

		if ( targetpo != NULL ) {

			damage = targetpo->CurDamage * DAMAGE1_BAR_LEN / targetpo->MaxDamage;
			if ( damage > DAMAGE1_BAR_LEN )
				damage = DAMAGE1_BAR_LEN;

		}
	}

	ASSERT( damage >= 0 );

	rect.x = (int)(cockpitinfo[ DAMAGEBAR1_EMPTY ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(cockpitinfo[ DAMAGEBAR1_EMPTY ].ypos * Scaled_Screen_Height);
	rect.w = cockpitinfo[ DAMAGEBAR1_EMPTY ].owidth;
	rect.h = cockpitinfo[ DAMAGEBAR1_EMPTY ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ DAMAGEBAR1_EMPTY ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGEBAR1_EMPTY ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGEBAR1_EMPTY ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	float	width = ( float ) ( DAMAGE1_BAR_LEN - damage ) / 640.0f;

	rect.x = (int)(cockpitinfo[ DAMAGEBAR1 ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(cockpitinfo[ DAMAGEBAR1 ].ypos * Scaled_Screen_Height);
	rect.w = DAMAGE1_BAR_LEN - damage;
	rect.h = cockpitinfo[ DAMAGEBAR1 ].oheight;
	rect.scaled_w = (int)(width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGEBAR1 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGEBAR1 ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );
}


// draw local player damage bar -----------------------------------------------
//
void COCKPIT_DrawLocalDamageBar()
{
	texscreenrect_s rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	#define DAMAGE2_BAR_LEN	64

	int	damage = MyShip->CurDamage * DAMAGE2_BAR_LEN / MyShip->MaxDamage;
	if ( damage > DAMAGE2_BAR_LEN )
		damage = DAMAGE2_BAR_LEN;

	ASSERT( damage >= 0 );

	rect.x = (int)(Screen_Width - cockpitinfo[ DAMAGEBAR2_EMPTY ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(cockpitinfo[ DAMAGEBAR2_EMPTY ].ypos * Scaled_Screen_Height);
	rect.w = cockpitinfo[ DAMAGEBAR2_EMPTY ].owidth;
	rect.h = cockpitinfo[ DAMAGEBAR2_EMPTY ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ DAMAGEBAR2_EMPTY ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGEBAR2_EMPTY ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGEBAR2_EMPTY ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	float	width = ( float ) ( DAMAGE2_BAR_LEN - damage ) / 640.0f;

	rect.x = (int)(Screen_Width - cockpitinfo[ DAMAGEBAR2 ].xpos * Scaled_Screen_Width + 1);
	rect.y = (int)(cockpitinfo[ DAMAGEBAR2 ].ypos * Scaled_Screen_Height);
	rect.w = DAMAGE2_BAR_LEN - damage;
	rect.h = cockpitinfo[ DAMAGEBAR2 ].oheight;
	rect.scaled_w = (int)(width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGEBAR2 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGEBAR2 ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );
}


// draw flashing of ship monitor during invulnerability -----------------------
//
void COCKPIT_DrawInvul( dword &ctr, int xpos, int ypos, int width, int height )
{
	ctr += 120 * CurScreenRefFrames;

	sincosval_s sincosv;
	GetSinCos( ctr, &sincosv );

	// draw yellow rectangle
	colscreenrect_s rect;

	rect.x		  = xpos;
	rect.y		  = ypos;
	rect.w		  = width;
	rect.h		  = height;
	rect.itertype = iter_rgba | iter_specularadd;
	rect.color.R  = INDICATOR_STRENGTH;
	rect.color.G  = (int)(INDICATOR_STRENGTH + 60 * GEOMV_TO_FLOAT( sincosv.sinval ));
	rect.color.B  = 0;
	rect.color.A  = ( 100 * cp_fadepos ) / CP_ALPHA_HIGH;

	DRAW_ColoredScreenRect( &rect );
}


// draw damage display portion of cockpit -------------------------------------
//
void COCKPIT_DrawDamage()
{
	texscreenrect_s	rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	// draw remote ship behind left ship monitor
	ShipObject 	*targetpo = NULL;
//	int			texture_index = -1;
	int			target_visible = 0;
	int			l_name_pos_x = 0;
	int			l_name_width = 0;
	int			r_name_pos_x = 0;
	int			r_name_width = 0;
	float 	x_offs;
	float 	y_offs;
	float 	width;
	float 	height;

	if ( TargetObjNumber != TARGETID_NO_TARGET ) {

		targetpo = (ShipObject *) FetchFirstShip();
		while ( ( targetpo != NULL ) && ( targetpo->HostObjNumber != TargetObjNumber ) )
			targetpo = (ShipObject *) targetpo->NextObj;

		if ( targetpo != NULL ) {

			target_visible = 1;

			x_offs = LEFTMON_X / 640.0f;
			y_offs = LEFTMON_Y / 480.0f;
			width  = LEFTMON_W / 640.0f;
			height = LEFTMON_H / 480.0f;

			int shipindex = ObjClassShipIndex[ targetpo->ObjectClass ];
			ASSERT( shipindex != SHIPINDEX_NO_SHIP );

			TextureMap *texture = ( shipindex != SHIPINDEX_NO_SHIP ) ?
				MonitorTextures[ shipindex ] : NULL;

			if ( texture != NULL ) {

				rect.x = r_name_pos_x = (int)(( cockpitinfo[ DAMAGE1 ].xpos + x_offs ) * Scaled_Screen_Width);
				rect.y = (int)(( cockpitinfo[ DAMAGE1 ].ypos + y_offs ) * Scaled_Screen_Height);
				rect.w = TEXTURE_W;
				rect.h = TEXTURE_H;
				rect.scaled_w = r_name_width = (int)(width * Scaled_Screen_Width);
				rect.scaled_h = (int)(height * Scaled_Screen_Height);
				rect.texofsx  = 0;
				rect.texofsy  = 0;
				rect.texmap   = texture;

				DRAW_TexturedScreenRect( &rect, NULL );

			} else {

				r_name_pos_x = (int)(( cockpitinfo[ DAMAGE1 ].xpos + x_offs ) * Scaled_Screen_Width);
				r_name_width = (int)(width * Scaled_Screen_Width);

			}

			// draw hit indicator
			if ( HitCurTarget > 0 ) {

				// draw blue rectangle
				colscreenrect_s rect;
				rect.x		  = (int)(x_offs * Scaled_Screen_Width);
				rect.y		  = (int)(y_offs * Scaled_Screen_Height);
				rect.w		  = (int)(width  * Scaled_Screen_Width);
				rect.h		  = (int)(height * Scaled_Screen_Height);
				rect.itertype = iter_rgba | iter_specularadd;
				rect.color.R  = 0;
				rect.color.G  = 0;
				rect.color.B  = INDICATOR_STRENGTH;
				rect.color.A  = ( 100 * cp_fadepos ) / CP_ALPHA_HIGH;

				DRAW_ColoredScreenRect( &rect );
			}

			// draw invulnerability shield indicator
			if ( PRT_ObjectHasAttachedClustersOfType( targetpo, SAT_MEGASHIELD_SPHERE ) ) {

				COCKPIT_DrawInvul( r_flash_ctr,
					(int)(x_offs * Scaled_Screen_Width), (int)(y_offs * Scaled_Screen_Height),
					(int)(width  * Scaled_Screen_Width), (int)(height * Scaled_Screen_Height) );
			}
		}
	}

	// draw left ship monitor
	rect.x = (int)(cockpitinfo[ DAMAGE1 ].xpos * Screen_Width);
	rect.y = (int)(cockpitinfo[ DAMAGE1 ].ypos * Screen_Height);
	rect.w = cockpitinfo[ DAMAGE1 ].owidth;
	rect.h = cockpitinfo[ DAMAGE1 ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ DAMAGE1 ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGE1 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGE1 ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	rect.x += rect.scaled_w;
	rect.y = (int)(cockpitinfo[ DAMAGE1_SLICE ].ypos * Screen_Height);
	rect.w = cockpitinfo[ DAMAGE1_SLICE ].owidth;
	rect.h = cockpitinfo[ DAMAGE1_SLICE ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ DAMAGE1_SLICE ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGE1_SLICE ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGE1_SLICE ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	// draw local ship behind right ship monitor
	int shipindex = ObjClassShipIndex[ MyShip->ObjectClass ];
	ASSERT( shipindex != SHIPINDEX_NO_SHIP );

	TextureMap *texture = ( shipindex != SHIPINDEX_NO_SHIP ) ?
		MonitorTextures[ shipindex ] : NULL;

	x_offs = RIGHTMON_X / 640.0f;
	y_offs = RIGHTMON_Y / 480.0f;
	width  = RIGHTMON_W / 640.0f;
	height = RIGHTMON_H / 480.0f;

	if ( texture != NULL ) {

		rect.x = l_name_pos_x = (int)(Screen_Width - ( LEFT_DAMAGE_X - x_offs ) * Scaled_Screen_Width + 1);
		rect.y = (int)(( LEFT_DAMAGE_Y + y_offs ) * Scaled_Screen_Height);
		rect.w = -TEXTURE_W;
		rect.h = TEXTURE_H;
		rect.scaled_w = l_name_width = (int)(width * Scaled_Screen_Width);
		rect.scaled_h 		     = (int)(height * Scaled_Screen_Height);
		rect.texofsx  = TEXTURE_W;
		rect.texofsy  = 0;
		rect.texmap   = texture;

		DRAW_TexturedScreenRect( &rect, NULL );

	} else {

		l_name_pos_x = (int)(Screen_Width - ( LEFT_DAMAGE_X - x_offs ) * Scaled_Screen_Width + 1);
		l_name_width = (int)(width * Scaled_Screen_Width);

	}

	// draw invulnerability shield indicator
	if ( PRT_ObjectHasAttachedClustersOfType( MyShip, SAT_MEGASHIELD_SPHERE ) ) {

		COCKPIT_DrawInvul( l_flash_ctr,
			(int)( Screen_Width - ( LEFT_DAMAGE_X - x_offs ) * Scaled_Screen_Width + 1 ),
			(int)( ( LEFT_DAMAGE_Y + y_offs ) * Scaled_Screen_Height ),
			(int)(width * Scaled_Screen_Width + 1), (int)(height * Scaled_Screen_Height + 1) );
	}

	// draw right ship monitor
	rect.x = (int)(( Screen_Width - cockpitinfo[ DAMAGE2_SLICE ].xpos * Scaled_Screen_Width ) + 2);
	rect.y = (int)(cockpitinfo[ DAMAGE2_SLICE ].ypos * Scaled_Screen_Height);
	rect.w = cockpitinfo[ DAMAGE2_SLICE ].owidth;
	rect.h = cockpitinfo[ DAMAGE2_SLICE ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ DAMAGE2_SLICE ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGE2_SLICE ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGE2_SLICE ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	rect.x += rect.scaled_w;
	rect.y = (int)(cockpitinfo[ DAMAGE2 ].ypos * Scaled_Screen_Height);
	rect.w = cockpitinfo[ DAMAGE2 ].owidth;
	rect.h = cockpitinfo[ DAMAGE2 ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ DAMAGE2 ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ DAMAGE2 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ DAMAGE2 ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	// draw indicator bars
	COCKPIT_DrawEnergyBar();

	COCKPIT_DrawSpeedBar();

	COCKPIT_DrawRemoteDamageBar();

	COCKPIT_DrawLocalDamageBar();

	// draw player names
	if ( COCKPIT_VISIBLE ) {

		D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
						CharsetInfo[ HUD_CHARSETNO ].geompointer,
						NULL,
						CharsetInfo[ HUD_CHARSETNO ].width,
						CharsetInfo[ HUD_CHARSETNO ].height );

		char *rem_name;
		char u_t_n[]   = "unknown";
		int  pl_name_x = l_name_pos_x + 4;
		int  pl_name_y = 0;
		int  pl_name_w = strlen( LocalPlayerName ) * CharsetInfo[ HUD_CHARSETNO ].width;
		if ( pl_name_w < l_name_width )
			pl_name_x += ( l_name_width - pl_name_w ) / 2;
		D_WriteString( LocalPlayerName, pl_name_x, pl_name_y );

		if ( target_visible ) {

			if ( ( targetpo->HostObjNumber & 0xffff ) == 0 )
				rem_name = NET_FetchPlayerName( GetObjectOwner( targetpo ) );
			else
				rem_name = u_t_n;
			pl_name_x = r_name_pos_x;
			pl_name_y = 0;
			pl_name_w = strlen( rem_name ) * CharsetInfo[ HUD_CHARSETNO ].width;
			if ( pl_name_w < r_name_width )
				pl_name_x += ( r_name_width - pl_name_w ) / 2;
			D_WriteString( rem_name, pl_name_x, pl_name_y );

		}
	}
}


// draw weapon display portion of cockpit -------------------------------------
//
void COCKPIT_DrawWeapons()
{
	texscreenrect_s rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	// draw left weapons display

	rect.x = (int)(cockpitinfo[ WEAPONS1 ].xpos * Scaled_Screen_Width);
	rect.y = (int)(Screen_Height - cockpitinfo[ WEAPONS1 ].ypos * Scaled_Screen_Height + 1);
	rect.w = cockpitinfo[ WEAPONS1 ].owidth;
	rect.h = cockpitinfo[ WEAPONS1 ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ WEAPONS1 ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ WEAPONS1 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;

	ASSERT( ( SelectedLaser >= 0 ) && ( SelectedLaser < 4 ) );
	if ( weapon_list[ SelectedLaser ].available ) {

		rect.texmap = weapon_list[ SelectedLaser ].picture_texmap[ weapon_list[ SelectedLaser ].curlevel ];
		if ( rect.texmap == NULL ) {
			rect.texmap = cockpitinfo[ WEAPONS1 ].texmap;
		}

	} else {

		rect.texmap = cockpitinfo[ WEAPONS1 ].texmap;
	}

	DRAW_TexturedScreenRect( &rect, NULL );

	rect.x = rect.scaled_w;
	rect.y = (int)(Screen_Height - cockpitinfo[ WEAPONS1_SLICE ].ypos * Scaled_Screen_Height + 1);
	rect.w = cockpitinfo[ WEAPONS1_SLICE ].owidth;
	rect.h = cockpitinfo[ WEAPONS1_SLICE ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ WEAPONS1_SLICE ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ WEAPONS1_SLICE ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ WEAPONS1_SLICE ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );


	// draw right weapons display

	if ( AUX_ENABLE_ELITE_RADAR && AUX_DRAW_COCKPIT_RADAR ) {

		rect.x = (int)(Screen_Width - cockpitinfo[ WEAPONS2_ELITE ].xpos * Scaled_Screen_Width + 1);
		rect.y = (int)(Screen_Height - cockpitinfo[ WEAPONS2_ELITE ].ypos * Scaled_Screen_Height + 1);
		rect.w = cockpitinfo[ WEAPONS2_ELITE ].owidth;
		rect.h = cockpitinfo[ WEAPONS2_ELITE ].oheight;
		rect.scaled_w = (int)(cockpitinfo[ WEAPONS2_ELITE ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ WEAPONS2_ELITE ].height * Scaled_Screen_Height);
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ WEAPONS2_ELITE ].texmap;

	} else {

		rect.x = (int)(Screen_Width - cockpitinfo[ WEAPONS2_WORM ].xpos * Scaled_Screen_Width + 1);
		rect.y = (int)(Screen_Height - cockpitinfo[ WEAPONS2_WORM ].ypos * Scaled_Screen_Height + 1);
		rect.w = cockpitinfo[ WEAPONS2_WORM ].owidth;
		rect.h = cockpitinfo[ WEAPONS2_WORM ].oheight;
		rect.scaled_w = (int)(cockpitinfo[ WEAPONS2_WORM ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ WEAPONS2_WORM ].height * Scaled_Screen_Height);
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ WEAPONS2_WORM ].texmap;

	}

	DRAW_TexturedScreenRect( &rect, NULL );

	rect.x += rect.scaled_w;
	rect.y = (int)(Screen_Height - cockpitinfo[ WEAPONS2 ].ypos * Scaled_Screen_Height + 1);
	rect.w = cockpitinfo[ WEAPONS2 ].owidth;
	rect.h = cockpitinfo[ WEAPONS2 ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ WEAPONS2 ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ WEAPONS2 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;

	//FIXME:
	// hack!
	int selected_missile = SelectedMissile + 4;
	if ( selected_missile == 6 )
		selected_missile = 7;
	else if ( selected_missile == 7 )
		selected_missile = 6;

	ASSERT( ( SelectedMissile >= 0 ) && ( SelectedMissile < 4 ) );
	if ( weapon_list[ selected_missile ].available ) {

		rect.texmap = weapon_list[ selected_missile ].picture_texmap[ weapon_list[ selected_missile ].curlevel ];
		if ( rect.texmap == NULL ) {
			rect.texmap = cockpitinfo[ WEAPONS2 ].texmap;
		}

	} else {

		rect.texmap = cockpitinfo[ WEAPONS2 ].texmap;
	}

	DRAW_TexturedScreenRect( &rect, NULL );

	if ( COCKPIT_VISIBLE ) {

		switch ( SelectedMissile ) {

			case 0:
				HUD_WriteAmmoString( MyShip->NumMissls, Screen_Width - 8*2 - 5, Screen_Height - 13 );
				break;

			case 1:
				HUD_WriteAmmoString( MyShip->NumHomMissls, Screen_Width - 8*2 - 5, Screen_Height - 13 );
				break;

			case 2:
				HUD_WriteAmmoString( MyShip->NumMines, Screen_Width - 8*2 - 5, Screen_Height - 13 );
				break;

			case 3:
				HUD_WriteAmmoString( MyShip->NumPartMissls, Screen_Width - 8*2 - 5, Screen_Height - 13 );
				break;
		}
	}
}


// ----------------------------------------------------------------------------
//
#define RADAR_X_CENTER				0.503125f

#define MINEDETECTION_RANGE1		FLOAT_TO_GEOMV( 400.0f )
#define MINEDETECTION_RANGE2		FLOAT_TO_GEOMV( 200.0f )
#define MINEDETECTION_RANGE3		FLOAT_TO_GEOMV( 100.0f )


#define BEEP_RATE					600

static int beep_count = 0;
static int beep_rate  = BEEP_RATE;


// draw mine detector lamps in radar area of cockpit --------------------------
//
void COCKPIT_DrawMineDetectorLamps()
{
	//NOTE:
	// only draw mine lamps if cockpit is fully opaque
	// otherwise the lamp quads can be seen through
	if ( cp_fadepos != CP_ALPHA_HIGH )
		return;

	geomv_t distance = MINEDETECTION_RANGE1;

	Mine1Obj *precnode = (Mine1Obj *) ExtraObjects;

	// search for closest mine object
	while ( precnode->NextObj != NULL ) {

		ASSERT( OBJECT_TYPE_EXTRA( precnode->NextObj ) );

		// get pointer to current extra
		Mine1Obj *curmine = (Mine1Obj *) precnode->NextObj;
		ASSERT( curmine != NULL );

		// continue in list if this is not a mine at all
		if ( curmine->ObjectType != MINE1TYPE ) {
			precnode = curmine;
			continue;
		}

		Vector3 minepos;
		Vector3 shippos;

		FetchTVector( curmine->ObjPosition, &minepos );
		FetchTVector( MyShip->ObjPosition, &shippos );

		Vector3 dirvect;

		dirvect.X = minepos.X - shippos.X;
		dirvect.Y = minepos.Y - shippos.Y;
		dirvect.Z = minepos.Z - shippos.Z;

		geomv_t tmpdist = VctLenX( &dirvect );

		if ( tmpdist < distance )
			distance = tmpdist;

		precnode = curmine;
	}

	float 	x_offs;
	float 	y_offs;
//	float 	width;
//	float 	height;
	colscreenrect_s rect;

	// draw first lamp if mine is close enough
	if ( distance < MINEDETECTION_RANGE1 ) {

		if ( ( beep_count += CurScreenRefFrames ) > beep_rate )
			beep_count = beep_rate - beep_count;

		if ( beep_count <= 0 ) {
			AUD_MineDetector();
		}

		if ( AUX_ENABLE_ELITE_RADAR ) {

			x_offs = cockpitinfo[ RADAR_ELITE ].xpos - LAMP_ORIGIN_ELITE_X + LAMP1_X;
			y_offs = cockpitinfo[ RADAR_ELITE ].ypos - LAMP_ORIGIN_ELITE_Y;

			rect.x = (int)(( Screen_Width - x_offs * Scaled_Screen_Width ) + 1);
			rect.y = (int)(Screen_Height - y_offs * Scaled_Screen_Height + 1);

		} else {


			x_offs = cockpitinfo[ RADAR ].xpos - LAMP_ORIGIN_WC_X + LAMP1_X;
			y_offs = cockpitinfo[ RADAR ].ypos - LAMP_ORIGIN_WC_Y;

			rect.x = (int)(( RADAR_X_CENTER * Screen_Width - x_offs * Scaled_Screen_Width ) + 1);
			rect.y = (int)(Screen_Height - y_offs * Scaled_Screen_Height + 1);
		}

		rect.w	      = (int)(LAMP_W * Scaled_Screen_Width);
		rect.h	      = (int)(LAMP_H * Scaled_Screen_Height);
		rect.itertype = iter_rgba | iter_specularadd;
		rect.color.R  = 0;
		rect.color.G  = 255;
		rect.color.B  = 0;
		rect.color.A  = 255;

		DRAW_ColoredScreenRect( &rect );

		// draw second lamp
		if ( distance < MINEDETECTION_RANGE2 ) {

			beep_rate = BEEP_RATE / 2;

			if ( AUX_ENABLE_ELITE_RADAR ) {

				x_offs -= LAMP2_X - LAMP1_X;
				rect.x = (int)(( Screen_Width - x_offs * Scaled_Screen_Width ) + 1);

			} else {


				x_offs -= LAMP2_X - LAMP1_X;
				rect.x = (int)(( RADAR_X_CENTER * Screen_Width - x_offs * Scaled_Screen_Width ) + 1);
			}

			rect.color.R  = 255;

			DRAW_ColoredScreenRect( &rect );

			// draw third lamp
			if ( distance < MINEDETECTION_RANGE3 ) {

				beep_rate = BEEP_RATE / 4;

				if ( AUX_ENABLE_ELITE_RADAR ) {

					x_offs -= LAMP3_X - LAMP2_X;
					rect.x = (int)(( Screen_Width - x_offs * Scaled_Screen_Width ) + 1);

				} else {


					x_offs -= LAMP3_X - LAMP2_X;
					rect.x = (int)(( RADAR_X_CENTER * Screen_Width - x_offs * Scaled_Screen_Width ) + 1);
				}

				rect.color.G  = 0;

				DRAW_ColoredScreenRect( &rect );

			} else {

				beep_rate = BEEP_RATE / 2;
			}

		} else {

			beep_rate = BEEP_RATE;
		}
	}
}


// ----------------------------------------------------------------------------
//
#define INCOMING_XOFS				16.0f
#define INCOMING_YOFS				51.0f

#define INCOMING_ELITE_XOFS			44.0f
#define INCOMING_ELITE_YOFS			45.0f


// draw radar display portion of cockpit --------------------------------------
//
void COCKPIT_DrawRadar()
{
	// do mine detection and draw warning lamps
	COCKPIT_DrawMineDetectorLamps();

	texscreenrect_s rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	if ( AUX_ENABLE_ELITE_RADAR ) {

		rect.x = (int)(( Screen_Width - cockpitinfo[ RADAR_ELITE ].xpos * Scaled_Screen_Width ) + 2);
		rect.y = (int)(Screen_Height - cockpitinfo[ RADAR_ELITE ].ypos * Scaled_Screen_Height + 1);
		rect.w = cockpitinfo[ RADAR_ELITE ].owidth;
		rect.h = cockpitinfo[ RADAR_ELITE ].oheight;
		rect.scaled_w = (int)(cockpitinfo[ RADAR_ELITE ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ RADAR_ELITE ].height * Scaled_Screen_Height);
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ RADAR_ELITE ].texmap;

		DRAW_TexturedScreenRect( &rect, NULL );

	} else {

		rect.x = (int)(( RADAR_X_CENTER * Screen_Width - cockpitinfo[ RADAR_SLICE ].xpos * Scaled_Screen_Width ) );
		rect.y = (int)(Screen_Height - cockpitinfo[ RADAR_SLICE ].ypos * Scaled_Screen_Height + 2);
		rect.w = cockpitinfo[ RADAR_SLICE ].owidth;
		rect.h = cockpitinfo[ RADAR_SLICE ].oheight;
		rect.scaled_w = (int)(cockpitinfo[ RADAR_SLICE ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ RADAR_SLICE ].height * Scaled_Screen_Height);
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ RADAR_SLICE ].texmap;

		DRAW_TexturedScreenRect( &rect, NULL );

		rect.x = (int)(( RADAR_X_CENTER * Screen_Width - cockpitinfo[ RADAR ].xpos * Scaled_Screen_Width ) );
		rect.y += rect.scaled_h;
		rect.w = cockpitinfo[ RADAR ].owidth;
		rect.h = cockpitinfo[ RADAR ].oheight;
		rect.scaled_w = (int)(cockpitinfo[ RADAR ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ RADAR ].height * Scaled_Screen_Height);
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ RADAR ].texmap;

		DRAW_TexturedScreenRect( &rect, NULL );
	}

	rect.itertype  = iter_texrgba | iter_specularadd;

	if ( IncomingMissile ) {

		if ( AUX_ENABLE_ELITE_RADAR ) {

			float x_offs = INCOMING_ELITE_XOFS / 640.0f;
			float y_offs = INCOMING_ELITE_YOFS / 480.0f;

			rect.x = (int)(Screen_Width - ( cockpitinfo[ RADAR_ELITE ].xpos - x_offs ) * Scaled_Screen_Width);
			rect.y = (int)(Screen_Height - ( cockpitinfo[ RADAR_ELITE ].ypos - y_offs ) * Scaled_Screen_Height + 1);

		} else {

			float x_offs = INCOMING_XOFS / 640.0f;
			float y_offs = INCOMING_YOFS / 480.0f;

			rect.x = (int)(( RADAR_X_CENTER * Screen_Width - ( cockpitinfo[ RADAR ].xpos - x_offs ) * Scaled_Screen_Width ) );
			rect.y = (int)(Screen_Height - ( cockpitinfo[ RADAR ].ypos - y_offs ) * Scaled_Screen_Height + 1);

		}

		rect.w = cockpitinfo[ INCOMING ].owidth;
		rect.h = cockpitinfo[ INCOMING ].oheight;
		rect.scaled_w = (int)(cockpitinfo[ INCOMING ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ INCOMING ].height * Scaled_Screen_Height);
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ INCOMING ].texmap;

		DRAW_TexturedScreenRect( &rect, NULL );
	}
}


// draw iconbar display background of cockpit ---------------------------------
//
void COCKPIT_DrawIconbarBackground()
{
	texscreenrect_s	rect;

	rect.itertype  = iter_texrgba | iter_premulblend;
	rect.alpha     = cp_fadepos;

	rect.x = (int)(cockpitinfo[ ICONBAR1 ].xpos * Scaled_Screen_Width);
	rect.y = (int)(cockpitinfo[ ICONBAR1 ].ypos * Screen_Height);
	rect.w = cockpitinfo[ ICONBAR1 ].owidth;
	rect.h = cockpitinfo[ ICONBAR1 ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ ICONBAR1 ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ ICONBAR1 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ ICONBAR1 ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	rect.x += rect.scaled_w;
	rect.y = (int)(cockpitinfo[ ICONBAR2 ].ypos * Screen_Height);
	rect.w = cockpitinfo[ ICONBAR2 ].owidth;
	rect.h = cockpitinfo[ ICONBAR2 ].oheight;
	rect.scaled_w = (int)(cockpitinfo[ ICONBAR2 ].width * Scaled_Screen_Width);
	rect.scaled_h = (int)(cockpitinfo[ ICONBAR2 ].height * Scaled_Screen_Height);
	rect.texofsx  = 0;
	rect.texofsy  = 0;
	rect.texmap   = cockpitinfo[ ICONBAR2 ].texmap;

	DRAW_TexturedScreenRect( &rect, NULL );

	cockpit_messagearea_yoffs = (int)(( 60.0f / 480.0f ) * Scaled_Screen_Height);
}


// draw iconbar display portion of cockpit ------------------------------------
//
void COCKPIT_DrawIconbar()
{
	TextureMap *texmap = NULL;

	texscreenrect_s	rect;
	rect.itertype = iter_texa | iter_alphablend;
	rect.alpha    = cp_fadepos;

	visual_t xlatcol = COLINDX_TO_VISUAL( SELECTED_WEAPON_COL );
	int selected_laser   = SelectedLaser;
	int selected_missile = SelectedMissile + 4;

	//FIXME:
	// hack!
	if ( selected_missile == 6 )
		selected_missile = 7;
	else if ( selected_missile == 7 )	
			selected_missile = 6;

	//FIXME:
	// just a quick hack!
	if ( MyShip->Specials & SPMASK_LASER_UPGRADE_2 ) {
		weapon_list[ LASER_SLOT ].curlevel = 2;
	} else if ( MyShip->Specials & SPMASK_LASER_UPGRADE_1 ) {
		weapon_list[ LASER_SLOT ].curlevel = 1;
	} else {
		weapon_list[ LASER_SLOT ].curlevel = 0;
	}

	if ( MyShip->Specials & SPMASK_EMP_UPGRADE_2 ) {
		weapon_list[ EMP_SLOT ].curlevel = 2;
	} else if ( MyShip->Specials & SPMASK_EMP_UPGRADE_1 ) {
		weapon_list[ EMP_SLOT ].curlevel = 1;
	} else {
		weapon_list[ EMP_SLOT ].curlevel = 0;
	}

	weapon_list[ LASER_SLOT ].available 	= MyShip->Weapons & WPMASK_CANNON_LASER;
	weapon_list[ HELIX_SLOT ].available 	= MyShip->Weapons & WPMASK_CANNON_HELIX;
	weapon_list[ LIGHTNING_SLOT ].available = MyShip->Weapons & WPMASK_CANNON_LIGHTNING;
	weapon_list[ PHOTON_SLOT ].available	= MyShip->Weapons & WPMASK_CANNON_PHOTON;

	weapon_list[ DUMB_SLOT ].available 		= MyShip->NumMissls;
	weapon_list[ GUIDED_SLOT ].available 	= MyShip->NumHomMissls;
	weapon_list[ MINE_SLOT ].available 		= MyShip->NumMines;
	weapon_list[ SWARM_SLOT ].available 	= MyShip->NumPartMissls;

	weapon_list[ EMP_SLOT ].available 		= MyShip->Weapons & WPMASK_DEVICE_EMP;
//	weapon_list[ INVUL_SLOT ].available 	= MyShip->Specials & SPMASK_INVULNERABILITY;
	weapon_list[ ABURN_SLOT ].available 	= MyShip->Specials & SPMASK_AFTERBURNER;
//	weapon_list[ DECOY_SLOT ].available 	= MyShip->Specials & SPMASK_DECOY;

	// check if invulnerability is active
	if ( PRT_ObjectHasAttachedClustersOfType( MyShip, SAT_MEGASHIELD_SPHERE ) ) {
		weapon_list[ INVUL_SLOT ].available = TRUE;
	} else {
		weapon_list[ INVUL_SLOT ].available = FALSE;
	}

	if ( AUX_DRAW_COCKPIT_ICONBAR_BACKGROUND ) {

		// draw icons with scaling to fit on iconbar background
		float x_offs = 52.0f / 640.0f;
		float y_offs = 19.0f / 480.0f;

		rect.x = (int)(( cockpitinfo[ ICONBAR1 ].xpos + x_offs ) * Scaled_Screen_Width);
		rect.y = (int)(( cockpitinfo[ ICONBAR1 ].ypos + y_offs ) * Scaled_Screen_Height);
		rect.w = ICON_OWIDTH;
		rect.h = ICON_OHEIGHT;
		rect.scaled_w = (int)(ICON_WIDTH * Scaled_Screen_Width);
		rect.scaled_h = (int)(ICON_HEIGHT * Scaled_Screen_Height);
		rect.texofsx  = 0;
		rect.texofsy  = 0;

		for ( int sid = 0; sid < MAX_WEAPON_SLOTS; sid++ ) {

			texmap = weapon_list[ sid ].available ?
				weapon_list[ sid ].avail_texmap[ weapon_list[ sid ].curlevel ] :
				weapon_list[ sid ].unavail_texmap;

			if ( weapon_list[ sid ].blinking ) {
				if ( ( SYSs_GetRefFrameCount() >> 8 ) & 1 )
					texmap = weapon_list[ sid ].unavail_texmap;
			}

			if ( texmap != NULL ) {
				rect.texmap = texmap;
				DRAW_TexturedScreenRect( &rect, NULL );
			}

			if ( COCKPIT_VISIBLE ) {

				if ( sid == selected_laser || sid == selected_missile ) {

					D_DrawHorzBar( rect.x, rect.y, xlatcol, rect.scaled_w );
					D_DrawVertBar( rect.x, rect.y, xlatcol, rect.scaled_h );

					D_DrawHorzBar( rect.x, rect.y + rect.scaled_h, xlatcol, rect.scaled_w );
					D_DrawVertBar( rect.x + rect.scaled_w, rect.y, xlatcol, rect.scaled_h );
				}

				// draw invulnerabilty lifetime bar below icon
				if ( sid == INVUL_SLOT && ( MyShip->Specials & SPMASK_INVULNERABILITY ) ) {

					// fetch shield strength
					float strength;
					basesphere_pcluster_s *cluster =
						SFX_FetchInvulnerabilityShield( MyShip, &strength );

					if ( cluster != NULL ) {
						D_DrawHorzBar( rect.x + 1, rect.y + rect.scaled_h, xlatcol,
							       max( 1, (int)(rect.scaled_w * strength - 2) ) );

						static int lastcountdown = 0;
						int secondsleft = cluster->lifetime / FRAME_MEASURE_TIMEBASE;

						if ( secondsleft == 5 )
							weapon_list[ sid ].blinking = TRUE;
						else if ( secondsleft == 0 || secondsleft > 5 )
							weapon_list[ sid ].blinking = FALSE;

						if ( ( secondsleft <= 5 ) && ( secondsleft != lastcountdown ) ) {
							AUD_Countdown( secondsleft );
							lastcountdown = secondsleft;
						}
					}
				}

				// draw afterburner energy bar below icon
				if ( sid == ABURN_SLOT && ( MyShip->Specials & SPMASK_AFTERBURNER ) ) {

					if ( MyShip->afterburner_energy > 0 ) {
						float energy = (float)MyShip->afterburner_energy / (float)AFTERBURNER_ENERGY;
						D_DrawHorzBar( rect.x + 1, rect.y + rect.scaled_h, xlatcol,
									   max( 1, (int)(rect.scaled_w * energy - 2) )  );
					}
				}
			}

			rect.x += rect.scaled_w;
		}

		cockpit_messagearea_yoffs = (int)(( 60.0f / 480.0f ) * Scaled_Screen_Height);

	} else {

		// draw icons slightly translucent
		rect.alpha = (int)(rect.alpha * 0.75f);

		// draw icons unscaled at max size
		float x_offs = 17.0f / 640.0f;
		float y_offs = 18.0f / 480.0f;

		float left_pos 	= cockpitinfo[ ICONBAR1 ].xpos * scale_tab[ cp_scale ];
		float right_pos 	= 1.0f - LEFT_DAMAGE_X * scale_tab[ cp_scale ];

		float width 		= right_pos - left_pos;
		float factor		= ( ( width - 0.02f ) / MAX_WEAPON_SLOTS ) / 0.027f;

		rect.x = (int)(( left_pos + 0.06f * scale_tab[ cp_scale ] ) * Screen_Width);
		rect.y = (int)(( cockpitinfo[ ICONBAR1 ].ypos + y_offs ) * Scaled_Screen_Height);
		rect.w = ICON_OWIDTH;
		rect.h = ICON_OHEIGHT;
		rect.scaled_w = (int)(ICON_WIDTH * Screen_Width * factor);
		rect.scaled_h = (int)(ICON_HEIGHT * (Screen_Width * 3.0f / 4.0f) * factor); // TODO: should scale by h instead of w
		rect.texofsx  = 0;
		rect.texofsy  = 0;

		for ( int sid = 0; sid < MAX_WEAPON_SLOTS; sid++ ) {

			texmap = weapon_list[ sid ].available ?
				weapon_list[ sid ].avail_texmap[ weapon_list[ sid ].curlevel ] :
				weapon_list[ sid ].unavail_texmap;

			if ( weapon_list[ sid ].blinking ) {
				if ( ( SYSs_GetRefFrameCount() >> 8 ) & 1 )
					texmap = weapon_list[ sid ].unavail_texmap;
			}

			if ( texmap != NULL ) {
				rect.texmap = texmap;
				DRAW_TexturedScreenRect( &rect, NULL );
			}

			if ( COCKPIT_VISIBLE ) {

				if ( sid == selected_laser || sid == selected_missile ) {

					D_DrawHorzBar( rect.x, rect.y, xlatcol, rect.scaled_w );
					D_DrawVertBar( rect.x, rect.y, xlatcol, rect.scaled_h );

					D_DrawHorzBar( rect.x, rect.y + rect.scaled_h, xlatcol, rect.scaled_w );
					D_DrawVertBar( rect.x + rect.scaled_w, rect.y, xlatcol, rect.scaled_h );
				}

				// draw invulnerabilty lifetime bar below icon
				if ( sid == INVUL_SLOT && ( MyShip->Specials & SPMASK_INVULNERABILITY ) ) {

					// fetch shield strength
					float strength;
					basesphere_pcluster_s *cluster =
						SFX_FetchInvulnerabilityShield( MyShip, &strength );

					if ( cluster != NULL ) {
						D_DrawHorzBar( rect.x + 1, rect.y + rect.scaled_h, xlatcol,
									   max( 1, (int)(rect.scaled_w * strength - 2) ) );

						static int lastcountdown = 0;
						int secondsleft = cluster->lifetime / FRAME_MEASURE_TIMEBASE;

						if ( secondsleft == 5 )
							weapon_list[ sid ].blinking = TRUE;
						else if ( secondsleft == 0 || secondsleft > 5 )
							weapon_list[ sid ].blinking = FALSE;

						if ( ( secondsleft <= 5 ) && ( secondsleft != lastcountdown ) ) {
							AUD_Countdown( secondsleft );
							lastcountdown = secondsleft;
						}
					}
				}

				// draw afterburner energy bar below icon
				if ( sid == ABURN_SLOT && ( MyShip->Specials & SPMASK_AFTERBURNER ) ) {

					if ( MyShip->afterburner_energy > 0 ) {
						float energy = (float)MyShip->afterburner_energy / (float)AFTERBURNER_ENERGY;
						D_DrawHorzBar( rect.x + 1, rect.y + rect.scaled_h, xlatcol,
									   max( 1, (int)(rect.scaled_w * energy - 2) )  );
					}
				}
			}

			rect.x += rect.scaled_w;
		}

		cockpit_messagearea_yoffs = rect.y + rect.scaled_h + 12;
	}
}


// draw crosshair -------------------------------------------------------------
//
void COCKPIT_DrawCrossHair()
{
	texscreenrect_s rect;

	rect.alpha = (int)((float)cp_fadepos * 0.7f);

	if ( AUX_DRAW_COCKPIT_CROSSHAIR == 1 ) {

		rect.itertype = iter_texrgba | iter_alphablend;
		rect.scaled_w = (int)(cockpitinfo[ CROSS ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ CROSS ].height * Scaled_Screen_Height);
		rect.x = (int)(( Screen_Width - rect.scaled_w ) / 2);
		rect.y = (int)(( Screen_Height - rect.scaled_h ) / 2);
		rect.w = cockpitinfo[ CROSS ].owidth;
		rect.h = cockpitinfo[ CROSS ].oheight;
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ CROSS ].texmap;

	} else if ( AUX_DRAW_COCKPIT_CROSSHAIR == 2 ) {

		rect.alpha = (int)(rect.alpha * 0.7f);

		rect.itertype = iter_texrgba | iter_specularadd;
		rect.scaled_w = (int)(cockpitinfo[ CROSS2 ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ CROSS2 ].height * Scaled_Screen_Height);
		rect.x = (int)(( Screen_Width - rect.scaled_w ) / 2);
		rect.y = (int)(( Screen_Height - rect.scaled_h ) / 2);
		rect.w = cockpitinfo[ CROSS2 ].owidth;
		rect.h = cockpitinfo[ CROSS2 ].oheight;
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ CROSS2 ].texmap;
	}

	DRAW_TexturedScreenRect( &rect, NULL );

	// draw target direction marker
	if ( AUX_DRAW_COCKPIT_CROSSHAIR == 2 && TargetObjNumber != TARGETID_NO_TARGET ) {

		// fetch target from ship list
		ShipObject *targetpo = (ShipObject *) FetchFirstShip();
		while ( ( targetpo != NULL ) && ( targetpo->HostObjNumber != TargetObjNumber ) )
			targetpo = (ShipObject *) targetpo->NextObj;

		if ( targetpo == NULL ) {
			return;
		}

		if ( TargetScreenX < (dword)( Screen_XOfs + Screen_XOfs/2 ) &&
			 TargetScreenX > (dword)( Screen_XOfs - Screen_XOfs/2 ) &&
			 TargetScreenY < (dword)( Screen_YOfs + Screen_YOfs/2 ) &&
			 TargetScreenY > (dword)( Screen_YOfs - Screen_YOfs/2 ) ) {
		
			 return;	 
		}

		// fetch ship's position
		float ox = GEOMV_TO_FLOAT( targetpo->CurrentXmatrx[ 0 ][ 3 ] );
		float oy = GEOMV_TO_FLOAT( targetpo->CurrentXmatrx[ 1 ][ 3 ] );
		float oz = GEOMV_TO_FLOAT( targetpo->CurrentXmatrx[ 2 ][ 3 ] );

		// square components
		hprec_t ox2 = ox * ox;
		hprec_t oy2 = oy * oy;

		// length of vector
		hprec_t len	= sqrt( ox2 + oy2 );
		if ( len < 1e-7 ) {
			return;
		}
		
		hprec_t targetangle;
		
		if ( ox < 0 ) {

			targetangle = asin( oy / len ) - HPREC_PI;

		} else {

			targetangle = asin( - oy / len );
		}

		// bring range to [-PI..0] and [0..+PI]
		if ( oz < 0 ) {
			targetangle = ( oy < 0 ) ? -HPREC_PI - targetangle : HPREC_PI - targetangle;
		}
		
		rastv_t xcenter = Screen_Width / 2;
		rastv_t ycenter = Screen_Height / 2;
		rastv_t xpos, xpos2; 
		rastv_t ypos, ypos2;
		
		sincosval_s sincosval;
		GetSinCos( RAD_TO_BAMS( targetangle ), &sincosval );
		
		xpos  = sincosval.cosval * rect.scaled_w * 0.3f;
		ypos  = sincosval.sinval * rect.scaled_h * 0.34f;
		xpos2 = sincosval.cosval * rect.scaled_w * 0.23f;
		ypos2 = sincosval.sinval * rect.scaled_h * 0.27f;

		// draw line from (xcenter+xpos2/ycenter-ypos2) to
		// (xcenter+xpos/ycenter-ypos)

		TextureMap *arrow = FetchTextureMap( "arrow" );
		
		if ( arrow == NULL ) {
			return;
		}
		
		int alpha = (int)((float)cp_fadepos * 0.7f);

		int width = 10;
		int height = 16;

		int xoffs = (int)(sincosval.sinval * width * scale_tab[ cp_scale ]);
		int yoffs = (int)(sincosval.cosval * width * scale_tab[ cp_scale ]);
		
		IterRectangle2 itrect;

		itrect.Vtxs[ 0 ].X = INT_TO_RASTV( xcenter + xpos - xoffs );
		itrect.Vtxs[ 0 ].Y = INT_TO_RASTV( ycenter - ypos - yoffs );
		itrect.Vtxs[ 0 ].Z = RASTV_1;
		itrect.Vtxs[ 0 ].W = GEOMV_1;
		itrect.Vtxs[ 0 ].U = INT_TO_GEOMV( 0 );
		itrect.Vtxs[ 0 ].V = INT_TO_GEOMV( 0 );
		itrect.Vtxs[ 0 ].R = alpha;
		itrect.Vtxs[ 0 ].G = 10;
		itrect.Vtxs[ 0 ].B = 10;
		itrect.Vtxs[ 0 ].A = alpha;

		itrect.Vtxs[ 1 ].X = INT_TO_RASTV( xcenter + xpos + xoffs );
		itrect.Vtxs[ 1 ].Y = INT_TO_RASTV( ycenter - ypos + yoffs );
		itrect.Vtxs[ 1 ].Z = RASTV_1;
		itrect.Vtxs[ 1 ].W = GEOMV_1;
		itrect.Vtxs[ 1 ].U = INT_TO_GEOMV( 31 );
		itrect.Vtxs[ 1 ].V = INT_TO_GEOMV( 0 );
		itrect.Vtxs[ 1 ].R = alpha;
		itrect.Vtxs[ 1 ].G = 10;
		itrect.Vtxs[ 1 ].B = 10;
		itrect.Vtxs[ 1 ].A = alpha;

		itrect.Vtxs[ 2 ].X = INT_TO_RASTV( xcenter + xpos2 + xoffs );
		itrect.Vtxs[ 2 ].Y = INT_TO_RASTV( ycenter - ypos2 + yoffs );
		itrect.Vtxs[ 2 ].Z = RASTV_1;
		itrect.Vtxs[ 2 ].W = GEOMV_1;
		itrect.Vtxs[ 2 ].U = INT_TO_GEOMV( 31 );
		itrect.Vtxs[ 2 ].V = INT_TO_GEOMV( 31 );
		itrect.Vtxs[ 2 ].R = alpha;
		itrect.Vtxs[ 2 ].G = 10;
		itrect.Vtxs[ 2 ].B = 10;
		itrect.Vtxs[ 2 ].A = alpha;

		itrect.Vtxs[ 3 ].X = INT_TO_RASTV( xcenter + xpos2 - xoffs );
		itrect.Vtxs[ 3 ].Y = INT_TO_RASTV( ycenter - ypos2 - yoffs );
		itrect.Vtxs[ 3 ].Z = RASTV_1;
		itrect.Vtxs[ 3 ].W = GEOMV_1;
		itrect.Vtxs[ 3 ].U = INT_TO_GEOMV( 0 );
		itrect.Vtxs[ 3 ].V = INT_TO_GEOMV( 31 );
		itrect.Vtxs[ 3 ].R = alpha;
		itrect.Vtxs[ 3 ].G = 10;
		itrect.Vtxs[ 3 ].B = 10;
		itrect.Vtxs[ 3 ].A = alpha;

		itrect.flags	 = ITERFLAG_NONE;
		itrect.itertype  = iter_texrgba | iter_specularadd;
		itrect.raststate = rast_texclamp;
		itrect.rastmask  = rast_nomask;
		itrect.texmap	 = arrow;

		D_DrawIterRectangle2( &itrect );
	}
}

#if 0
// draw crosshair -------------------------------------------------------------
//
void COCKPIT_DrawCrossHair()
{
	texscreenrect_s rect;

	rect.alpha = (float)cp_fadepos * 0.7f;

	if ( AUX_DRAW_COCKPIT_CROSSHAIR == 1 ) {

		rect.itertype = iter_texrgba | iter_alphablend;
		rect.scaled_w = (int)(cockpitinfo[ CROSS ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ CROSS ].height * Scaled_Screen_Height);
		rect.x = (int)(( Screen_Width - rect.scaled_w ) / 2);
		rect.y = (int)(( Screen_Height - rect.scaled_h ) / 2);
		rect.w = cockpitinfo[ CROSS ].owidth;
		rect.h = cockpitinfo[ CROSS ].oheight;
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ CROSS ].texmap;

	} else if ( AUX_DRAW_COCKPIT_CROSSHAIR == 2 ) {

		rect.alpha *= 0.7f;

		rect.itertype = iter_texrgba | iter_specularadd;
		rect.scaled_w = (int)(cockpitinfo[ CROSS2 ].width * Scaled_Screen_Width);
		rect.scaled_h = (int)(cockpitinfo[ CROSS2 ].height * Scaled_Screen_Height);
		rect.x = (int)(( Screen_Width - rect.scaled_w ) / 2);
		rect.y = (int)(( Screen_Height - rect.scaled_h ) / 2);
		rect.w = cockpitinfo[ CROSS2 ].owidth;
		rect.h = cockpitinfo[ CROSS2 ].oheight;
		rect.texofsx  = 0;
		rect.texofsy  = 0;
		rect.texmap   = cockpitinfo[ CROSS2 ].texmap;
	}

	DRAW_TexturedScreenRect( &rect, NULL );

	// draw target direction marker
	if ( AUX_DRAW_COCKPIT_CROSSHAIR == 2 && TargetObjNumber != TARGETID_NO_TARGET ) {

		// fetch target from ship list
		ShipObject *targetpo = (ShipObject *) FetchFirstShip();
		while ( ( targetpo != NULL ) && ( targetpo->HostObjNumber != TargetObjNumber ) )
			targetpo = (ShipObject *) targetpo->NextObj;

		if ( targetpo == NULL ) {
			return;
		}

		// fetch ship's position
		float ox = GEOMV_TO_FLOAT( targetpo->CurrentXmatrx[ 0 ][ 3 ] );
		float oy = GEOMV_TO_FLOAT( targetpo->CurrentXmatrx[ 1 ][ 3 ] );
		float oz = GEOMV_TO_FLOAT( targetpo->CurrentXmatrx[ 2 ][ 3 ] );

		// square components
		hprec_t ox2 = ox * ox;
		hprec_t oy2 = oy * oy;

		// length of vector
		hprec_t len	= sqrt( ox2 + oy2 );
		if ( len < 1e-7 ) {
			return;
		}
		
		hprec_t targetangle;
		
		if ( ox < 0 ) {

			targetangle = asin( oy / len ) - HPREC_PI;

		} else {

			targetangle = asin( - oy / len );
		}

		// bring range to [-PI..0] and [0..+PI]
		if ( oz < 0 ) {
			targetangle = ( oy < 0 ) ? -HPREC_PI - targetangle : HPREC_PI - targetangle;
		}
		
		rastv_t xcenter = Screen_Width / 2;
		rastv_t ycenter = Screen_Height / 2;
		rastv_t xpos, xpos2; 
		rastv_t ypos, ypos2;
		
		sincosval_s sincosval;
		GetSinCos( RAD_TO_BAMS( targetangle ), &sincosval );
		
		xpos  = sincosval.cosval * rect.scaled_w * 0.3;
		ypos  = sincosval.sinval * rect.scaled_h * 0.34;
		xpos2 = sincosval.cosval * rect.scaled_w * 0.23;
		ypos2 = sincosval.sinval * rect.scaled_h * 0.27;
		
		IterLine2 itline;
		itline.NumVerts  = 2;

		itline.flags	 = ITERFLAG_LS_ANTIALIASED;

		itline.itertype  = iter_rgba | iter_alphablend;
		itline.raststate = rast_default;
		itline.rastmask  = rast_nomask;

		itline.Vtxs[ 0 ].X 	   = xcenter + xpos2;
		itline.Vtxs[ 0 ].Y 	   = ycenter - ypos2;
		
		itline.Vtxs[ 0 ].R 	   = 255;
		itline.Vtxs[ 0 ].G 	   = 255;
		itline.Vtxs[ 0 ].B 	   = 255;
		itline.Vtxs[ 0 ].A 	   = (float)cp_fadepos * 0.7f;
		itline.Vtxs[ 0 ].flags = ITERVTXFLAG_NONE;

		itline.Vtxs[ 1 ].X 	   = xcenter + xpos;
		itline.Vtxs[ 1 ].Y 	   = ycenter - ypos;
		itline.Vtxs[ 1 ].R 	   = 255;
		itline.Vtxs[ 1 ].G 	   = 255;
		itline.Vtxs[ 1 ].B 	   = 255;
		itline.Vtxs[ 1 ].A 	   = itline.Vtxs[ 0 ].A;
		itline.Vtxs[ 1 ].flags = ITERVTXFLAG_NONE;

		Rectangle2 cliprect;
		cliprect.left   = 0;
		cliprect.right  = Screen_Width;
		cliprect.top    = 0;
		cliprect.bottom = Screen_Height;

		IterLine2 *clipline = CLIP_RectangleIterLine2( &itline, &cliprect );
		if ( clipline != NULL )
			D_DrawIterLine2( clipline );
	}
}

#endif

// draw bounding square around target ship ------------------------------------
//
void COCKPIT_DrawTargetBoundingSquare( int color )
{
	if ( TargetObjNumber == TARGETID_NO_TARGET )
		return;

	// fetch target from ship list
	ShipObject *targetpo = (ShipObject *) FetchFirstShip();
	while ( ( targetpo != NULL ) && ( targetpo->HostObjNumber != TargetObjNumber ) )
		targetpo = (ShipObject *) targetpo->NextObj;

	if ( targetpo == NULL ) {
		return;
	}

	geomv_t 	target_bsphere = targetpo->BoundingSphere;
	int			target_square;

	Vector3 	pos;
	Vector3		pos_t;

	pos.X = targetpo->ObjPosition[ 0 ][ 3 ];
	pos.Y = targetpo->ObjPosition[ 1 ][ 3 ];
	pos.Z = targetpo->ObjPosition[ 2 ][ 3 ];

	MtxVctMUL( ViewCamera, &pos, &pos_t );

	//FIXME:
	//NOTE:
	// this should not occur!! but it does...
	if ( pos_t.Z <= GEOMV_0 )
		return;

	ASSERT( target_bsphere > 0 );

	target_square = GEOMV_TO_COORD( GEOMV_DIV( target_bsphere, pos_t.Z ) );
	target_square = (int)(target_square * 0.7);

	int corner_len = target_square / 4;

	if ( corner_len < 2 )
		corner_len = 2;

	// calculate corner points of square
	int x1 		= TargetScreenX - target_square;
	int y1 		= TargetScreenY - target_square;
	int x2 		= TargetScreenX + target_square;
	int y2 		= TargetScreenY + target_square;

	// check if target seen at all
	if ( ( x1 >= Screen_Width ) || ( x2 < 0 ) || ( y1 >= Screen_Height ) || ( y2 < 0 ) )
		return;

	// coordinates for corner lines
	int x1_1 = x1;
	int x2_1 = x2;
	int y1_1 = y1;
	int y2_1 = y2;
	int x1_2 = x1 + corner_len - 1;
	int x2_2 = x2 - corner_len + 1;
	int y1_2 = y1 + corner_len - 1;
	int y2_2 = y2 - corner_len + 1;

	// clip corner-coordinates
	if ( x1_1 < 0 )
		x1_1 = 0;

	if ( y1_1 < 0 )
		y1_1 = 0;

	if ( x2_1 >= Screen_Width )
		x2_1 = Screen_Width - 1;

	if ( y2_1 >= Screen_Height )
		y2_1 = Screen_Height - 1;

	// clip corner-line-coordinates
	if ( x1_2 >= Screen_Width )
		x1_2 = Screen_Width - 1;

	if ( y1_2 >= Screen_Height )
		y1_2 = Screen_Height - 1;

	if ( x2_2 < 0 )
		x2_2 = 0;

	if ( y2_2 < 0 )
		y2_2 = 0;

	// draw all four corners if visible
	visual_t xlatcol = COLINDX_TO_VISUAL( color );

	if ( y1 >= 0 ) {
		if ( x1_2 >= 0 )
			D_DrawHorzBar( x1_1, y1_1, xlatcol, x1_2 - x1_1 + 1 );
		if ( x2_2 < Screen_Width )
			D_DrawHorzBar( x2_2, y1_1, xlatcol, x2_1 - x2_2 + 1 );
	}

	if ( x1 >= 0 ) {
		if ( y1_2 >= 0 )
			D_DrawVertBar( x1_1, y1_1, xlatcol, y1_2 - y1_1 + 1 );
		if ( y2_2 < Screen_Height )
			D_DrawVertBar( x1_1, y2_2, xlatcol, y2_1 - y2_2 + 1 );
	}

	if ( y2 < Screen_Height ) {
		if ( x1_2 >= 0 )
			D_DrawHorzBar( x1_1, y2_1, xlatcol, x1_2 - x1_1 + 1 );
		if ( x2_2 < Screen_Width )
			D_DrawHorzBar( x2_2, y2_1, xlatcol, x2_1 - x2_2 + 1 );
	}

	if ( x2 < Screen_Width ) {
		if ( y1_2 >= 0 )
			D_DrawVertBar( x2_1, y1_1, xlatcol, y1_2 - y1_1 + 1 );
		if ( y2_2 < Screen_Height )
			D_DrawVertBar( x2_1, y2_2, xlatcol, y2_1 - y2_2 + 1 );
	}

	// clip corner-coordinates
	if ( x1 < 0 )
		x1 = 0;

	if ( x2 >= Screen_Width )
		x2 = Screen_Width - 1;

	if ( y1 < 0 )
		y1 = 0;

	if ( y2 >= Screen_Height )
		y2 = Screen_Height - 1;

	// calculate distance to target object and print it beneath bounding square
	Vector3	from_pos;
	Vector3 to_pos;

	from_pos.X = MyShip->ObjPosition[ 0 ][ 3 ];
	from_pos.Y = MyShip->ObjPosition[ 1 ][ 3 ];
	from_pos.Z = MyShip->ObjPosition[ 2 ][ 3 ];

	to_pos.X = targetpo->ObjPosition[ 0 ][ 3 ];
	to_pos.Y = targetpo->ObjPosition[ 1 ][ 3 ];
	to_pos.Z = targetpo->ObjPosition[ 2 ][ 3 ];

	Vector3 dirvect;
	dirvect.X = from_pos.X - to_pos.X;
	dirvect.Y = from_pos.Y - to_pos.Y;
	dirvect.Z = from_pos.Z - to_pos.Z;

	int distance = GEOMV_TO_INT( VctLenX( &dirvect ) );

	sprintf( paste_str, "distance: %d", distance );

	int string_width  = strlen( paste_str ) * CharsetInfo[ HUD_CHARSETNO ].width;
	int string_height = CharsetInfo[ HUD_CHARSETNO ].height;

	int xpos = x1 + ( ( x2 - x1 - string_width ) / 2 );
	int ypos = y2 + 2;

	// check against screen boundaries
	if ( (   xpos                   >= 0 	         ) &&
		 (   ypos				    >= 0 	         ) &&
		 ( ( xpos + string_width  ) <= Screen_Width  ) &&
		 ( ( ypos + string_height ) <= Screen_Height ) ) {

		D_DrawTrRect( xpos - 1, ypos, string_width + 3, string_height + 3, TRTAB_PANELBACK );
		D_WriteTrString( paste_str, xpos + 1, ypos + 2, TRTAB_PANELTEXT );
	}

	// draw target player name above bounding square
	char *name;
	char unknown_str[] = "unknown";

	if ( ( targetpo->HostObjNumber & 0xffff ) == 0 )
		name = NET_FetchPlayerName( GetObjectOwner( targetpo ) );
	else
		name = unknown_str;

	string_width  = strlen( name ) * CharsetInfo[ HUD_CHARSETNO ].width;
	string_height = CharsetInfo[ HUD_CHARSETNO ].height;

	xpos = x1 + ( ( x2 - x1 - string_width ) / 2 );
	ypos = y1 - string_height - 4;

	// check against screen boundaries
	if ( (   xpos                   >= 0 	         ) &&
		 (   ypos				    >= 0 	         ) &&
		 ( ( xpos + string_width  ) <= Screen_Width  ) &&
		 ( ( ypos + string_height ) <= Screen_Height ) ) {

		D_DrawTrRect( xpos - 1, ypos, string_width + 3, string_height + 3, TRTAB_PANELBACK );
		D_WriteTrString( name, xpos + 1, ypos + 2, TRTAB_PANELTEXT );
	}

}


// draw target tracking -------------------------------------------------------
//
void COCKPIT_DrawTargetTracking()
{
	static int   trackingtarget = 0;
//	static dword targetx;
//	static dword targety;

	if ( MyShip->NumHomMissls == 0 && MyShip->NumPartMissls == 0 ) {
		trackingtarget = 0;
		TargetLocked   = 0;
	}

	if ( TargetVisible ) {

		if ( trackingtarget )
			TargetLocked = 1; //FIXME: nur jetzt: target gleich locken!

		if ( TargetLocked ) {

			COCKPIT_DrawTargetBoundingSquare( TARGET_SQUARE_OBJ_COL );

		} else if ( trackingtarget ) {

			//TODO: move crosshair slowly into lock-position

		} else {

			COCKPIT_DrawTargetBoundingSquare( NORMAL_SQUARE_OBJ_COL );

		}

		if ( ( SelectedMissile == 1 || SelectedMissile == 3 ) &&
			 ( TargetScreenX > (dword)TrackingLeftBoundX ) &&
			 ( TargetScreenX < (dword)TrackingRightBoundX ) &&
			 ( TargetScreenY > (dword)TrackingLeftBoundY ) &&
			 ( TargetScreenY < (dword)TrackingRightBoundY	) ) {

			if ( !TargetLocked )
				trackingtarget = 1;

		} else {
			TargetLocked   = 0;
			trackingtarget = 0;
		}

		TargetVisible = 0;

	} else {
		TargetLocked   = 0;
		trackingtarget = 0;
	}

}


// flag whether quicksay mode is currently active -----------------------------
//
static int quicksay_mode = FALSE;


// draw and handle quicksay chat buffer ---------------------------------------
//
void COCKPIT_DrawQuickSayBuffer()
{
	if ( !quicksay_mode ) {
		return;
	}

	// handle quicksay (one line) console input and drawing
	quicksay_mode = QuicksayConsole();
}


// fade the cockpit alpha to a specified target -------------------------------
//
INLINE
void FadeCockpitAlpha()
{
	if ( cp_fadepos == cp_fadetarget ) {
		cp_lastref = REFFRAME_INVALID;
		return;
	}

	if ( cp_fadepos < cp_fadetarget ) {

		// fade in
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( cp_lastref == REFFRAME_INVALID ) {
			cp_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - cp_lastref;
			for ( ; delta >= CP_FADE_SPEED; delta -= CP_FADE_SPEED ) {
				cp_fadepos += CP_FADE_QUANTUM;
				if ( cp_fadepos >= cp_fadetarget ) {
					cp_fadepos = cp_fadetarget;
					cp_lastref = REFFRAME_INVALID;
					break;
				}
				cp_lastref += CP_FADE_SPEED;
			}
		}

	} else {

		// fade out
		refframe_t refframecount = SYSs_GetRefFrameCount();
		if ( cp_lastref == REFFRAME_INVALID ) {
			cp_lastref = refframecount;
		} else {
			refframe_t delta = refframecount - cp_lastref;
			for ( ; delta >= CP_FADE_SPEED; delta -= CP_FADE_SPEED ) {
				cp_fadepos -= CP_FADE_QUANTUM;
				if ( cp_fadepos <= cp_fadetarget ) {
					cp_fadepos = cp_fadetarget;
					cp_lastref = REFFRAME_INVALID;
					break;
				}
				cp_lastref += CP_FADE_SPEED;
			}
		}
	}
}


// fetch textures for specified cockpit element -------------------------------
//
PRIVATE
void FetchElementTextures( int eltype )
{
	ASSERT( (dword)eltype < COCKPIT_TEXTURES_NUM_TYPES );
	cockpit_tex_valid[ eltype ] = TRUE;

	for ( int ctx = 0; cockpitinfo[ ctx ].texname; ctx++ ) {

		if ( cockpit_tex_type[ ctx ] != eltype )
			continue;

		TextureMap *texmap = FetchTextureMap( cockpitinfo[ ctx ].texname );
		cockpitinfo[ ctx ].texmap = texmap;

		if ( texmap == NULL ) {
			static refframe_t msgbase = 0;
			refframe_t absref = SYSs_GetRefFrameCount();
			if ( ( absref - msgbase ) > 1000 ) {
				msgbase = absref;
				MSGOUT( "cockpit texture (%s) not found.", cockpitinfo[ ctx ].texname );
			}
			cockpit_tex_valid[ eltype ] = FALSE;
		}
	}
}

#define FETCH_ELEMENT_TEXTURES(e) \
	ASSERT( (dword)COCKPIT_TEXTURES_##e < COCKPIT_TEXTURES_NUM_TYPES ); \
	if ( AUX_DRAW_COCKPIT_##e && !cockpit_tex_valid[ COCKPIT_TEXTURES_##e ] ) { \
		FetchElementTextures( COCKPIT_TEXTURES_##e ); \
	} else {}


// precalculate values depending on screen resolution -------------------------
//
int COCKPIT_CalcResValues( void* param )
{

//	Scaled_Screen_Width 	= Screen_Width * scale_tab[cp_scale];
	Scaled_Screen_Height 	= Screen_Height * scale_tab[cp_scale];
	Scaled_Screen_Width		= min(Screen_Width * scale_tab[cp_scale], Scaled_Screen_Height * 4.0f / 3.0f);

	return TRUE;

}


// draw cockpit (dispatcher for drawing cockpit elements) ---------------------
//
void COCKPIT_DrawDisplay()
{
	// never show the cockpit in floating menu, entry mode or object camera
	if ( InFloatingMenu || EntryMode || ObjCameraActive )
		return;

	if ( !AUX_DRAW_COCKPIT )
		return;

	FadeCockpitAlpha();

	// fetch texture sets for cockpit elements
	FETCH_ELEMENT_TEXTURES( DAMAGE );
	FETCH_ELEMENT_TEXTURES( WEAPONS );
	FETCH_ELEMENT_TEXTURES( RADAR );
	FETCH_ELEMENT_TEXTURES( ICONBAR_BACKGROUND );
	FETCH_ELEMENT_TEXTURES( CROSSHAIR );

	// draw target caret
	int targetvisible = TargetVisible;

	COCKPIT_DrawTargetTracking();

	#define DRAW_COCKPIT_ELEMENT(e) ( AUX_DRAW_COCKPIT_##e && cockpit_tex_valid[ COCKPIT_TEXTURES_##e ] )

	if ( DRAW_COCKPIT_ELEMENT( DAMAGE ) ) {
		COCKPIT_DrawDamage();
	}

	if ( DRAW_COCKPIT_ELEMENT( WEAPONS ) ) {
		COCKPIT_DrawWeapons();
	}

	if ( DRAW_COCKPIT_ELEMENT( RADAR ) ) {
		COCKPIT_DrawRadar();
	}

	if ( DRAW_COCKPIT_ELEMENT( ICONBAR_BACKGROUND ) ) {
		COCKPIT_DrawIconbarBackground();
	}

	if ( AUX_DRAW_COCKPIT_ICONBAR ) {
		COCKPIT_DrawIconbar();
	}

	if ( DRAW_COCKPIT_ELEMENT( CROSSHAIR ) ) {
		COCKPIT_DrawCrossHair();
	}

	// draw radar frame (and old crosshair)
	if ( !AUX_DRAW_COCKPIT_CROSSHAIR ) {
		HUD_DrawBitmaps();
	} else {
		HUD_DrawHUDRadarFrame();
	}

	// draw radar contents
	if ( COCKPIT_VISIBLE ) {
		HUD_DrawHUDRadar();
	}

	// tracking messages
	HUD_DrawTrackingText( targetvisible );

	// draw quick say (console line)
	// now called in HUD_DrawHUD
//	COCKPIT_DrawQuickSayBuffer();
}


// change scale factor of cockpit ---------------------------------------------
//
PRIVATE
int Cmd_COCKPIT_SCALE( char *paramstr )
{
	//NOTE:
	//CONCOM:
	// scale_command ::= 'cockpit.scale' [<factor>]

	ASSERT( paramstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( paramstr );

	int scale = cp_scale;
	char *scan;

	if ( (scan = QueryIntArgumentEx( paramstr, "%d", &scale )) ) {

		// determine if delta modification (++/--)
		int delta = 0;
		if ( ( scan[ 0 ] == '+' ) && ( scan[ 1 ] == '+' ) )
			delta = 1;
		else if ( ( scan[ 0 ] == '-' ) && ( scan[ 1 ] == '-' )  )
			delta = -1;
		if ( delta != 0 )
			scan += 2;

		if ( *scan != 0 ) {
			char *errpart;

			long sval = strtol( scan, &errpart, 10 );
			if ( *errpart == 0 ) {
				if ( delta != 0 )
					sval = scale + sval * delta;
				if ( sval >= SCALE_FACTOR_MIN && sval <= SCALE_FACTOR_MAX ) {
					cp_scale = sval;
					COCKPIT_CalcResValues( NULL );
				} else {

					//NOTE:
					// if delta modification was used, don't print a range_error
					// but just beep to give the user some feedback...

					if ( delta != 0 )
						AUD_Select2();
					else
						CON_AddLine( range_error );
				}
			} else {
				CON_AddLine( invalid_arg );
			}
		} else {
			CON_AddLine( invalid_arg );
		}
	}

	return TRUE;
}


// key table for weaponreg command --------------------------------------------
//
key_value_s weaponreg_key_value[] = {

	{ "slot",			NULL,	KEYVALFLAG_MANDATORY		},
	{ "icon_avail",		NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "icon_unavail",	NULL,	KEYVALFLAG_PARENTHESIZE		},
	{ "pic",			NULL,	KEYVALFLAG_PARENTHESIZE		},

	{ NULL,				NULL,	KEYVALFLAG_NONE				},
};

enum {

	KEY_WEAPONREG_SLOT,
	KEY_WEAPONREG_ICON_AVAIL,
	KEY_WEAPONREG_ICON_UNAVAIL,
	KEY_WEAPONREG_PIC
};


// console command for defining weapon icons ("weaponreg") --------------------
//
PRIVATE
int Cmd_WEAPONREG( char *classstr )
{
	//NOTE:
	//CONCOM:
	// weaponreg_command	::= 'weaponreg' <slot_spec> [<icon_avail_spec>]
	//							[<icon_unavail_spec>] [<picture_spec>]
	// slot_spec			::= 'slot' <slotnum>
	// icon_avail_spec		::= 'icon_avail' <texturename> | <texture_list>
	// icon_unavail_spec	::= 'icon_unavail' <texturename>
	// picture_spec			::= 'pic' <texturename> | <texture_list>
	// texture_list			::= '(' <texturename>+ ')'

	ASSERT( classstr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( classstr );

	// scan out all values to keys
	if ( !ScanKeyValuePairs( weaponreg_key_value, classstr ) )
		return TRUE;

	int slotnum = -1;

	if ( ScanKeyValueInt( &weaponreg_key_value[ KEY_WEAPONREG_SLOT ], (int*)&slotnum ) < 0 ) {
		CON_AddLine( invalid_slot );
		return TRUE;
	}

	if ( ( slotnum < 0 ) || ( slotnum >= MAX_WEAPON_SLOTS ) ) {
		CON_AddLine( invalid_slot );
		return TRUE;
	}

	// check for avail texture
	char *texturename = weaponreg_key_value[ KEY_WEAPONREG_ICON_AVAIL ].value;
	if ( texturename != NULL ) {

		// allow a list of textures to be specified
		char *leveltex = strtok( texturename, " " );
		for ( int curlevel = 0; leveltex != NULL; curlevel++ ) {

			if ( curlevel == MAX_WEAPON_LEVELS ) {
				CON_AddLine( "too many avail textures." );
				return TRUE;
			}

			leveltex = GetParenthesizedName( leveltex );
			TextureMap *texmap = FetchTextureMap( leveltex );
			if ( texmap != NULL ) {
				weapon_list[ slotnum ].slot_filled = TRUE;
				weapon_list[ slotnum ].avail_texmap[ curlevel ] = texmap;
			} else {
				CON_AddLine( invalid_texture );
				return TRUE;
			}

			leveltex = strtok( NULL, " " );
		}
	}

	// check for unavail texture
	texturename = weaponreg_key_value[ KEY_WEAPONREG_ICON_UNAVAIL ].value;
	if ( texturename != NULL ) {
		TextureMap *texmap = FetchTextureMap( texturename );
		if ( texmap != NULL ) {
			weapon_list[ slotnum ].slot_filled = TRUE;
			weapon_list[ slotnum ].unavail_texmap = texmap;
		} else {
			CON_AddLine( invalid_texture );
			return TRUE;
		}
	}

	// check for weapon picture texture
	texturename = weaponreg_key_value[ KEY_WEAPONREG_PIC ].value;
	if ( texturename != NULL ) {

		// allow a list of textures to be specified
		char *leveltex = strtok( texturename, " " );
		for ( int curlevel = 0; leveltex != NULL; curlevel++ ) {

			if ( curlevel == MAX_WEAPON_LEVELS ) {
				CON_AddLine( "too many picture textures." );
				return TRUE;
			}

			leveltex = GetParenthesizedName( leveltex );
			TextureMap *texmap = FetchTextureMap( leveltex );
			if ( texmap != NULL ) {
				weapon_list[ slotnum ].slot_filled = TRUE;
				weapon_list[ slotnum ].picture_texmap[ curlevel ] = texmap;
			} else {
				CON_AddLine( invalid_texture );
				return TRUE;
			}

			leveltex = strtok( NULL, " " );
		}
	}

	return TRUE;
}


// enable quicksay input (chat with closed console) ---------------------------
//
PRIVATE
int Cmd_QUICKSAY( char *argstr )
{
	//NOTE:
	//CONCOM:
	// quicksay_command	::= 'quicksay'

	ASSERT( argstr != NULL );
	HANDLE_COMMAND_DOMAIN( argstr );

	if ( quicksay_mode ) {

		quicksay_mode = FALSE;

	} else {
		
		quicksay_mode = TRUE;
		
		CON_DelLine();
		strcpy( con_lines[ con_bottom ], con_prompt );
		
		if (con_in_talk_mode) {
			cursor_x = 0;
		} else {
			strcpy( &con_lines[ con_bottom ][ PROMPT_SIZE ], CMSTR( CM_SAY ) );
			strcpy( &con_lines[ con_bottom ][ PROMPT_SIZE + CMLEN( CM_SAY ) ], " " );
			cursor_x = 4;
		}

	}

	// notify console code
	SetQuicksayConsole( quicksay_mode );

	return TRUE;
}


// register cockpit callbacks -------------------------------------------------
//
PRIVATE
void COCKPIT_RegisterCallbacks()
{
	// specify callback type and flags
	int callbacktype = CBTYPE_VIDMODE_CHANGED | CBFLAG_PERSISTENT;

	// register callback so that we get notified if video mode is changed
	CALLBACK_RegisterCallback( callbacktype, COCKPIT_CalcResValues, (void*) NULL );

	// calc the values for the first time
	COCKPIT_CalcResValues( NULL );
}


// module registration function -----------------------------------------------
//
REGISTER_MODULE( H_COCKPT )
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "cockpit.scale" command
	regcom.command	 = "cockpit.scale";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_COCKPIT_SCALE;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "weaponreg" command
	regcom.command	 = "weaponreg";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_WEAPONREG;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// register "quicksay" command
	regcom.command	 = "quicksay";
	regcom.numparams = 1;
	regcom.execute	 = Cmd_QUICKSAY;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );

	// initialize MonitorTextures array
	for ( int tid = 0; tid < MAX_SHIP_CLASSES; tid++ )
		MonitorTextures[ tid ] = NULL;

	COCKPIT_RegisterCallbacks();
}



