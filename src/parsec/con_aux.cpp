/*
 * PARSEC - Verbose Aux Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
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
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// rendering subsystem
#include "r_patch.h"
#include "r_supp.h"

// subsystem headers
#include "net_defs.h"

// local module header
#include "con_aux.h"

// proprietary module headers
#include "con_int.h"
#include "e_supp.h"
#include "net_rmev.h"


// flags
#define REGISTER_VERBOSE_AUX_COMMANDS
//#define EXCLUDE_SPECIAL_AUX_FLAGS
#define REGISTER_VERBOSE_AUX_DATA
//#define EXCLUDE_SPECIAL_AUX_DATA


//NOTE:
// this module is only loosely related to the CON_AUX.H
// header. that is, it may be omitted from linking
// entirely, if there should be no verbose aux commands.
// apart from this, project functionality will not be
// reduced. CON_AUX.H must always be part of the project.



// perform remote event syncing for certain aux flags -------------------------
//
#define DEF_RMEV_STATESYNC( f, k, v ) \
PUBLIC \
void RESync##f() \
{ \
	if ( !NET_RmEvAllowed( RE_STATESYNC ) ) \
		return; \
	NET_RmEvStateSync( (k), (v) ); \
}


DEF_RMEV_STATESYNC( NebulaId,	RMEVSTATE_NEBULAID,		AUXDATA_BACKGROUND_NEBULA_ID	)
DEF_RMEV_STATESYNC( Amazing,	RMEVSTATE_AMAZING,		AUXDATA_SKILL_AMAZING_TIME		)
DEF_RMEV_STATESYNC( Brilliant,	RMEVSTATE_BRILLIANT,	AUXDATA_SKILL_BRILLIANT_TIME	)
DEF_RMEV_STATESYNC( KillLimit,	RMEVSTATE_KILLLIMIT,	AUX_KILL_LIMIT_FOR_GAME_END		)


// flush texture cache after lod bias has been altered ------------------------
//
PRIVATE
void FlushTexCache()
{
	if ( !TextModeActive ) {

		// flush entire texture cache
		R_InvalidateCachedTexture( NULL );

		// make sure texture data that might have been freed
		// by the texture cache is available once again
		ReloadFreedTextureBitmaps();

		// precache textures
		R_PrecacheTextures();
	}
}


// for registration of aux flags as integer variable commands -----------------
//
PRIVATE
int_command_s verbose_aux_commands[] = {

	{ 0x80,	"aux_polygons_dont_fill_zbuffer",			0, 1,	&AUX_POLYGONS_DONT_FILL_ZBUFFER,			NULL,				NULL },
	{ 0x80,	"aux_enable_gouraud_shading",				0, 1,	&AUX_ENABLE_GOURAUD_SHADING,				NULL,				NULL },
	{ 0x80,	"aux_bsp_do_extended_sprod_check",			0, 1,	&AUX_BSP_DO_EXTENDED_SPROD_CHECK,			NULL,				NULL },
	{ 0x80,	"aux_bsp_extcheck_draw_node",				0, 1,	&AUX_BSP_EXTCHECK_DRAW_NODE,				NULL,				NULL },
	{ 0x80,	"aux_bsp_extcheck_backnode_first",			0, 1,	&AUX_BSP_EXTCHECK_BACKNODE_FIRST,			NULL,				NULL },
	{ 0x80,	"aux_bsp_extcheck_skipsubtree",				0, 1,	&AUX_BSP_EXTCHECK_SKIPSUBTREE,				NULL,				NULL },
	{ 0x00,	"aux_objctrl_disable_shipobjects",			0, 1,	&AUX_OBJCTRL_DISABLE_SHIPOBJECTS,			NULL,				NULL },
	{ 0x00,	"aux_objctrl_disable_extraobjects",			0, 1,	&AUX_OBJCTRL_DISABLE_EXTRAOBJECTS,			NULL,				NULL },
	{ 0x00,	"aux_objctrl_disable_laserobjects",			0, 1,	&AUX_OBJCTRL_DISABLE_LASEROBJECTS,			NULL,				NULL },
	{ 0x00,	"aux_objctrl_disable_misslobjects",			0, 1,	&AUX_OBJCTRL_DISABLE_MISSLOBJECTS,			NULL,				NULL },
	{ 0x00,	"aux_hud_show_changed_speed_number",		0, 1,	&AUX_HUD_SHOW_CHANGED_SPEED_NUMBER,			NULL,				NULL },
	{ 0x00,	"aux_hud_show_changed_energy_number",		0, 1,	&AUX_HUD_SHOW_CHANGED_ENERGY_NUMBER,		NULL,				NULL },
	{ 0x00,	"aux_hud_panel_3_control",					0, 2,	&AUX_HUD_PANEL_3_CONTROL,					NULL,				NULL },
	{ 0x00,	"aux_hud_missile_bar_numbers",				0, 1,	&AUX_HUD_MISSILE_BAR_NUMBERS,				NULL,				NULL },
	{ 0x00,	"aux_hud_panel_4_control",					0, 1,	&AUX_HUD_PANEL_4_CONTROL,					NULL,				NULL },
	{ 0x80,	"aux_use_simple_explosion",					0, 1,	&AUX_USE_SIMPLE_EXPLOSION,					NULL,				NULL },
	{ 0x00,	"aux_disable_console_say_output",			0, 1,	&AUX_DISABLE_CONSOLE_SAY_OUTPUT,			NULL,				NULL },
	{ 0x00,	"aux_hud_disable_user_displays",			0, 1,	&AUX_HUD_DISABLE_USER_DISPLAYS,				NULL,				NULL },
	{ 0x00,	"aux_dont_draw_blueprints",					0, 1,	&AUX_DONT_DRAW_BLUEPRINTS,					NULL,				NULL },
	{ 0x80,	"aux_create_propulsion_fumes",				0, 1,	&AUX_CREATE_PROPULSION_FUMES,				NULL,				NULL },
	{ 0x80,	"aux_disable_server_settings_button",		0, 1,	&AUX_DISABLE_SERVER_SETTINGS_BUTTON,		NULL,				NULL },
	{ 0x80,	"aux_disable_server_menu_button",			0, 1,	&AUX_DISABLE_SERVER_MENU_BUTTON,			NULL,				NULL },
	{ 0x80,	"aux_disable_zbuffer_clear",				0, 1,	&AUX_DISABLE_ZBUFFER_CLEAR,					NULL,				NULL },
	{ 0x80,	"aux_disallow_usercontrolled_motion",		0, 1,	&AUX_DISALLOW_USERCONTROLLED_MOTION,		NULL,				NULL },
	{ 0x00,	"aux_disable_soundeffects",					0, 1,	&AUX_DISABLE_SOUNDEFFECTS,					NULL,				NULL },
	{ 0x80,	"aux_disable_cleardemo_on_repstop",			0, 1,	&AUX_DISABLE_CLEARDEMO_ON_REPSTOP,			NULL,				NULL },
	{ 0x80,	"aux_display_rate_in_viewers",				0, 1,	&AUX_DISPLAY_RATE_IN_VIEWERS,				NULL,				NULL },
	{ 0x00,	"aux_awaitretrace_on_framefinish",			0, 1,	&AUX_AWAITRETRACE_ON_FRAMEFINISH,			NULL,				NULL },
	{ 0x00,	"aux_disable_fixed_star_rendering",			0, 1,	&AUX_DISABLE_FIXED_STAR_RENDERING,			NULL,				NULL },
	{ 0x00,	"aux_disable_pseudo_star_rendering",		0, 1,	&AUX_DISABLE_PSEUDO_STAR_RENDERING,			NULL,				NULL },
	{ 0x80,	"aux_disable_flashwhite_on_gameentrY",		0, 1,	&AUX_DISABLE_FLASHWHITE_ON_GAMEENTRY,		NULL,				NULL },
	{ 0x80,	"aux_wait_for_keypress_after_fadein",		0, 1,	&AUX_WAIT_FOR_KEYPRESS_AFTER_FADEIN,		NULL,				NULL },
	{ 0x80,	"aux_disable_reortho_camera_in_game",		0, 1,	&AUX_DISABLE_REORTHO_CAMERA_IN_GAME,		NULL,				NULL },
	{ 0x80,	"aux_disable_reortho_in_viewers",			0, 1,	&AUX_DISABLE_REORTHO_IN_VIEWERS,			NULL,				NULL },
	{ 0x80,	"aux_dont_cull_particle_objects",			0, 1,	&AUX_DONT_CULL_PARTICLE_OBJECTS,			NULL,				NULL },
	{ 0x00,	"aux_disable_particle_system",				0, 1,	&AUX_DISABLE_PARTICLE_SYSTEM,				NULL,				NULL },
	{ 0x00,	"aux_enable_texture_overloading",			0, 1,	&AUX_ENABLE_TEXTURE_OVERLOADING,			NULL,				NULL },
	{ 0x00,	"aux_enable_object_overloading",			0, 1,	&AUX_ENABLE_OBJECT_OVERLOADING,				NULL,				NULL },
	{ 0x00,	"aux_enable_bitmap_overloading",			0, 1,	&AUX_ENABLE_BITMAP_OVERLOADING,				NULL,				NULL },
	{ 0x80,	"aux_disable_polygon_mipmapping",			0, 1,	&AUX_DISABLE_POLYGON_MIPMAPPING,			NULL,				NULL },
	{ 0x80,	"aux_disable_bitmap_mipmapping",			0, 1,	&AUX_DISABLE_BITMAP_MIPMAPPING,				NULL,				NULL },
	{ 0x80,	"aux_disable_polygon_filtering",			0, 1,	&AUX_DISABLE_POLYGON_FILTERING,				NULL,				NULL },
	{ 0x80,	"aux_disable_bitmap_filtering",				0, 1,	&AUX_DISABLE_BITMAP_FILTERING,				NULL,				NULL },
	{ 0x00,	"aux_enable_panoramic_background",			0, 1,	&AUX_ENABLE_PANORAMIC_BACKGROUND,			NULL,				NULL },
	{ 0x00,	"aux_disable_panoramic_layer_1",			0, 1,	&AUX_DISABLE_PANORAMIC_LAYER_1,				NULL,				NULL },
	{ 0x00,	"aux_disable_panoramic_layer_2",			0, 1,	&AUX_DISABLE_PANORAMIC_LAYER_2,				NULL,				NULL },
	{ 0x00,	"aux_disable_panoramic_layer_3",			0, 1,	&AUX_DISABLE_PANORAMIC_LAYER_3,				NULL,				NULL },
	{ 0x80,	"aux_disable_texture_wrapping",				0, 1,	&AUX_DISABLE_TEXTURE_WRAPPING,				NULL,				NULL },
	{ 0x80,	"aux_disable_buffer_clear",					0, 1,	&AUX_DISABLE_BUFFER_CLEAR,					NULL,				NULL },
	{ 0x80,	"aux_spreadfire_particles_extinfo",			0, 1,	&AUX_SPREADFIRE_PARTICLES_EXTINFO,			NULL,				NULL },
	{ 0x80,	"aux_lightning_particles_extinfo",			0, 1,	&AUX_LIGHTNING_PARTICLES_EXTINFO,			NULL,				NULL },
	{ 0x80,	"aux_explosion_particles_extinfo",			0, 1,	&AUX_EXPLOSION_PARTICLES_EXTINFO,			NULL,				NULL },
	{ 0x80,	"aux_energyfield_particles_extinfo",		0, 1,	&AUX_ENERGYFIELD_PARTICLES_EXTINFO,			NULL,				NULL },
	{ 0x80,	"aux_protshield_particles_extinfo",			0, 1,	&AUX_PROTSHIELD_PARTICLES_EXTINFO,			NULL,				NULL },
	{ 0x80,	"aux_megashield_particles_extinfo",			0, 1,	&AUX_MEGASHIELD_PARTICLES_EXTINFO,			NULL,				NULL },
	{ 0x00,	"aux_dont_flash_screen_on_flare",			0, 1,	&AUX_DONT_FLASH_SCREEN_ON_FLARE,			NULL,				NULL },
	{ 0x00,	"aux_dont_draw_flare_circles",				0, 1,	&AUX_DONT_DRAW_FLARE_CIRCLES,				NULL,				NULL },
	{ 0x00,	"aux_disable_screenshot_message",			0, 1,	&AUX_DISABLE_SCREENSHOT_MESSAGE,			NULL,				NULL },
	{ 0x80,	"aux_disable_floatlogo_rotation",			0, 1,	&AUX_DISABLE_FLOATLOGO_ROTATION,			NULL,				NULL },
	{ 0x80,	"aux_verbose_texture_mem_manager",			0, 1,	&AUX_VERBOSE_TEXTURE_MEM_MANAGER,			NULL,				NULL },
	{ 0x80,	"aux_disable_object_depth_sort",			0, 1,	&AUX_DISABLE_OBJECT_DEPTH_SORT,				NULL,				NULL },
	{ 0x80,	"aux_enable_console_debug_messages",		0, 1,	&AUX_ENABLE_CONSOLE_DEBUG_MESSAGES,			NULL,				NULL },
	{ 0x80,	"aux_enable_console_message_log",			0, 15,	&AUX_ENABLE_CONSOLE_MESSAGE_LOG,			NULL,				NULL },
	{ 0x80,	"aux_record_packets_into_session",			0, 1,	&AUX_RECORD_PACKETS_INTO_SESSION,			NULL,				NULL },
	{ 0x80,	"aux_create_extras_during_replay",			0, 1,	&AUX_CREATE_EXTRAS_DURING_REPLAY,			NULL,				NULL },
	{ 0x80,	"aux_disable_absolute_recording",			0, 1,	&AUX_DISABLE_ABSOLUTE_RECORDING,			NULL,				NULL },
	{ 0x80,	"aux_record_verbose_packets",				0, 1,	&AUX_RECORD_VERBOSE_PACKETS,				NULL,				NULL },
	{ 0x00,	"aux_perform_automatic_connect",			0, 1,	&AUX_PERFORM_AUTOMATIC_CONNECT,				NULL,				NULL },
	{ 0x80,	"aux_disable_replay_net_simulation",		0, 1,	&AUX_DISABLE_REPLAY_NET_SIMULATION,			NULL,				NULL },
	{ 0x00,	"aux_disable_local_stargate",				0, 1,	&AUX_DISABLE_LOCAL_STARGATE,				NULL,				NULL },
	{ 0x80,	"aux_particle_base_explosion",				0, 1,	&AUX_PARTICLE_BASE_EXPLOSION,				NULL,				NULL },
	{ 0x80,	"aux_no_delayed_join_replay",				0, 1,	&AUX_NO_DELAYED_JOIN_REPLAY,				NULL,				NULL },
	{ 0x00,	"aux_hud_enable_packet_loss_meter",			0, 1,	&AUX_HUD_ENABLE_PACKET_LOSS_METER,			NULL,				NULL },
	{ 0x80,	"aux_packetdrop_testing",					0, 100,	&AUX_PACKETDROP_TESTING,					NULL,				NULL },
	{ 0x00,	"aux_disable_talk_escape_on_tab",			0, 1,	&AUX_DISABLE_TALK_ESCAPE_ON_TAB,			NULL,				NULL },
	{ 0x00,	"aux_use_big_font_in_message_area",			0, 1,	&AUX_USE_BIG_FONT_IN_MESSAGE_AREA,			NULL,				NULL },
	{ 0x00,	"aux_hud_disable_nonbasic_displays",		0, 1,	&AUX_HUD_DISABLE_NONBASIC_DISPLAYS,			NULL,				NULL },
	{ 0x80,	"aux_hud_enable_debuginfo",					0, 1,	&AUX_HUD_ENABLE_DEBUGINFO,					NULL,				NULL },
	{ 0x80,	"aux_hud_debuginfo_memory",					0, 1,	&AUX_HUD_DEBUGINFO_MEMORY,					NULL,				NULL },
	{ 0x80,	"aux_hud_debuginfo_objects",				0, 1,	&AUX_HUD_DEBUGINFO_OBJECTS,					NULL,				NULL },
	{ 0x80,	"aux_hud_debuginfo_position",				0, 1,	&AUX_HUD_DEBUGINFO_POSITION,				NULL,				NULL },
	{ 0x80,	"aux_hud_debuginfo_packets",				0, 1,	&AUX_HUD_DEBUGINFO_PACKETS,					NULL,				NULL },
	{ 0x80,	"aux_hud_debuginfo_geometry",				0, 1,	&AUX_HUD_DEBUGINFO_GEOMETRY,				NULL,				NULL },
	{ 0x80,	"aux_hud_debuginfo_rasterization",			0, 1,	&AUX_HUD_DEBUGINFO_RASTERIZATION,			NULL,				NULL },
	{ 0x80,	"aux_advanced_playerinterpolation",			0, 3,	&AUX_ADVANCED_PLAYERINTERPOLATION,			NULL,				NULL },
	{ 0x80,	"aux_save_persistent_intcommands",			0, 1,	&AUX_SAVE_PERSISTENT_INTCOMMANDS,			NULL,				NULL },
	{ 0x80,	"aux_save_persistent_usrcommands",			0, 1,	&AUX_SAVE_PERSISTENT_USRCOMMANDS,			NULL,				NULL },
	{ 0x80,	"aux_save_persistent_aux_array",			0, 1,	&AUX_SAVE_PERSISTENT_AUX_ARRAY,				NULL,				NULL },
	{ 0x00,	"aux_disable_rc_script_saving",				0, 1,	&AUX_DISABLE_RC_SCRIPT_SAVING,				NULL,				NULL },
	{ 0x00,	"aux_force_workdir_to_current",				0, 1,	&AUX_FORCE_WORKDIR_TO_CURRENT,				NULL,				NULL },
	{ 0x00,	"aux_force_filepath_to_current",			0, 1,	&AUX_FORCE_FILEPATH_TO_CURRENT,				NULL,				NULL },
	{ 0x80,	"aux_disable_dead_reckoning",				0, 1,	&AUX_DISABLE_DEAD_RECKONING,				NULL,				NULL },
	{ 0x00,	"aux_redirect_talk_to_irc_server",			0, 1,	&AUX_REDIRECT_TALK_TO_IRC_SERVER,			NULL,				NULL },
	{ 0x80,	"aux_hud_debuginfo_over_console",			0, 1,	&AUX_HUD_DEBUGINFO_OVER_CONSOLE,			NULL,				NULL },
	{ 0x80,	"aux_explosion_draw_shockwave",				0, 3,	&AUX_EXPLOSION_DRAW_SHOCKWAVE,				NULL,				NULL },
	{ 0x80,	"aux_cmd_write_active_in_textmode",			0, 1,	&AUX_CMD_WRITE_ACTIVE_IN_TEXTMODE,			NULL,				NULL },
	{ 0x80,	"aux_enable_volumetric_clouds",				0, 1,	&AUX_ENABLE_VOLUMETRIC_CLOUDS,				NULL,				NULL },
	{ 0x80,	"aux_small_particle_triangles",				0, 256,	&AUX_SMALL_PARTICLE_TRIANGLES,				NULL,				NULL },
	{ 0x80,	"aux_obj_sort_polys_on_texture",			0, 1,	&AUX_OBJ_SORT_POLYS_ON_TEXTURE,				NULL,				NULL },
	{ 0x80,	"aux_no_default_rasterizer_state",			0, 1,	&AUX_NO_DEFAULT_RASTERIZER_STATE,			NULL,				NULL },
	{ 0x80,	"aux_allow_only_texture_loading",			0, 1,	&AUX_ALLOW_ONLY_TEXTURE_LOADING,			NULL,				NULL },
	{ 0x80,	"aux_cmd_write_disable_output",				0, 1,	&AUX_CMD_WRITE_DISABLE_OUTPUT,				NULL,				NULL },
	{ 0x80,	"aux_attach_object_particles",				0, 1,	&AUX_ATTACH_OBJECT_PARTICLES,				NULL,				NULL },
	{ 0x80,	"aux_stop_demo_replay_on_connect",			0, 1,	&AUX_STOP_DEMO_REPLAY_ON_CONNECT,			NULL,				NULL },
	{ 0x80,	"aux_disable_disconnect_button",			0, 1,	&AUX_DISABLE_DISCONNECT_BUTTON,				NULL,				NULL },
	{ 0x00,	"aux_enable_sample_overloading",			0, 1,	&AUX_ENABLE_SAMPLE_OVERLOADING,				NULL,				NULL },
	{ 0x80,	"aux_enable_shader_overloading",			0, 1,	&AUX_ENABLE_SHADER_OVERLOADING,				NULL,				NULL },
	{ 0x80,	"aux_attach_thrust_objects",				0, 1,	&AUX_ATTACH_THRUST_OBJECTS,					NULL,				NULL },
	{ 0x80,	"aux_disable_genobject_particles",			0, 1,	&AUX_DISABLE_GENOBJECT_PARTICLES,			NULL,				NULL },
	{ 0x80,	"aux_autostart_backgroundplayer",			0, 1,	&AUX_AUTOSTART_BACKGROUNDPLAYER,			NULL,				NULL },
	{ 0x80,	"aux_hud_advanced_killstats",				0, 1,	&AUX_HUD_ADVANCED_KILLSTATS,				NULL,				NULL },
	{ 0x80,	"aux_disable_package_scripts",				0, 1,	&AUX_DISABLE_PACKAGE_SCRIPTS,				NULL,				NULL },
	{ 0x80,	"aux_enable_killed_ship_extras",			0, 1,	&AUX_ENABLE_KILLED_SHIP_EXTRAS,				NULL,				NULL },
	{ 0x80,	"aux_stop_at_end_of_cd_track",				0, 1,	&AUX_STOP_AT_END_OF_CD_TRACK,				NULL,				NULL },
	{ 0x80,	"aux_direct_cluster_rendering",				0, 1,	&AUX_DIRECT_CLUSTER_RENDERING,				NULL,				NULL },
	{ 0x00,	"aux_enable_elite_radar",					0, 1,	&AUX_ENABLE_ELITE_RADAR,					NULL,				NULL },
	{ 0x80,	"aux_draw_missiles_on_radar",				0, 1,	&AUX_DRAW_MISSILES_ON_RADAR,				NULL,				NULL },
	{ 0x00,	"aux_draw_cockpit",							0, 1,	&AUX_DRAW_COCKPIT,							NULL,				NULL },
	{ 0x00,	"aux_draw_cockpit_damage",					0, 1,	&AUX_DRAW_COCKPIT_DAMAGE,					NULL,				NULL },
	{ 0x00,	"aux_draw_cockpit_weapons",					0, 1,	&AUX_DRAW_COCKPIT_WEAPONS,					NULL,				NULL },
	{ 0x00,	"aux_draw_cockpit_radar",					0, 1,	&AUX_DRAW_COCKPIT_RADAR,					NULL,				NULL },
	{ 0x00,	"aux_draw_cockpit_iconbar",					0, 1,	&AUX_DRAW_COCKPIT_ICONBAR,					NULL,				NULL },
	{ 0x80,	"aux_objctrl_disable_customobjects",		0, 1,	&AUX_OBJCTRL_DISABLE_CUSTOMOBJECTS,			NULL,				NULL },
	{ 0x80,	"aux_draw_cockpit_crosshair",				0, 2,	&AUX_DRAW_COCKPIT_CROSSHAIR,				NULL,				NULL },
	{ 0x80,	"aux_enable_customobject_huddisplay",		0, 1,	&AUX_ENABLE_CUSTOMOBJECT_HUDDISPLAY,		NULL,				NULL },
	{ 0x80,	"aux_draw_stargates_on_radar",				0, 1,	&AUX_DRAW_STARGATES_ON_RADAR,				NULL,				NULL },
	{ 0x80,	"aux_enable_cockpit_swaying",				0, 2,	&AUX_ENABLE_COCKPIT_SWAYING,				NULL,				NULL },
	{ 0x80,	"aux_enable_cockpit_rattling",				0, 1,	&AUX_ENABLE_COCKPIT_RATTLING,				NULL,				NULL },
	{ 0x80,	"aux_enable_smooth_ship_control",			0, 1,	&AUX_ENABLE_SMOOTH_SHIP_CONTROL,			NULL,				NULL },
	{ 0x80,	"aux_enable_extended_target_caret",			0, 1,	&AUX_ENABLE_EXTENDED_TARGET_CARET,			NULL,				NULL },
	{ 0x00,	"aux_show_ping_in_serverlist",				0, 1,	&AUX_SHOW_PING_IN_SERVERLIST,				NULL,				NULL },
	{ 0x00,	"aux_draw_cockpit_iconbar_background",		0, 1,	&AUX_DRAW_COCKPIT_ICONBAR_BACKGROUND,		NULL,				NULL },
	{ 0x80,	"aux_enable_missile_propulsion_fumes",		0, 3,	&AUX_ENABLE_MISSILE_PROPULSION_FUMES,		NULL,				NULL },
	{ 0x80,	"aux_laserbeam_instead_of_laser",			0, 1,	&AUX_LASERBEAM_INSTEAD_OF_LASER,			NULL,				NULL },
	{ 0x80,	"aux_enable_free_camera",					0, 1,	&AUX_ENABLE_FREE_CAMERA,					NULL,				NULL },
	{ 0x80,	"aux_flagword_screenshot_helpers",			0, 255,	&AUX_FLAGWORD_SCREENSHOT_HELPERS,			NULL,				NULL },
	{ 0x80,	"aux_display_remote_join_object_id",		0, 1,	&AUX_DISPLAY_REMOTE_JOIN_OBJECT_ID,			NULL,				NULL },
	{ 0x00,	"aux_enable_refframe_timeplot",				0, 1,	&AUX_ENABLE_REFFRAME_TIMEPLOT,				NULL,				NULL },
	{ 0x80,	"aux_disable_nearplane_touch_culling",		0, 1,	&AUX_DISABLE_NEARPLANE_TOUCH_CULLING,		NULL,				NULL },
	{ 0x00,	"aux_disable_msgarea_if_console_open",		0, 1,	&AUX_DISABLE_MSGAREA_IF_CONSOLE_OPEN,		NULL,				NULL },
	{ 0x80,	"aux_enable_missile_polygon_trails",		0, 3,	&AUX_ENABLE_MISSILE_POLYGON_TRAILS,			NULL,				NULL },
	{ 0x00,	"aux_enable_funky_audio_comments",			0, 1,	&AUX_ENABLE_FUNKY_AUDIO_COMMENTS,			NULL,				NULL },
	{ 0x00,	"aux_disable_package_script_listing",		0, 1,	&AUX_DISABLE_PACKAGE_SCRIPT_LISTING,		NULL,				NULL },
	{ 0x00,	"aux_disable_demo_replay_in_menu",			0, 1,	&AUX_DISABLE_DEMO_REPLAY_IN_MENU,			NULL,				NULL },
	{ 0x80,	"aux_no_delayed_create_object_replay",		0, 1,	&AUX_NO_DELAYED_CREATE_OBJECT_REPLAY,		NULL,				NULL },
	{ 0x80,	"aux_disable_sorted_playerlist",			0, 1,	&AUX_DISABLE_SORTED_PLAYERLIST,				NULL,				NULL },
	{ 0x80,	"aux_disable_object_free_on_cleardemo",		0, 1,	&AUX_DISABLE_OBJECT_FREE_ON_CLEARDEMO,		NULL,				NULL },
	{ 0x80,	"aux_enable_demo_replay_info_display",		0, 1,	&AUX_ENABLE_DEMO_REPLAY_INFO_DISPLAY,		NULL,				NULL },
	{ 0x80,	"aux_disable_joystick_throttle",			0, 1,	&AUX_DISABLE_JOYSTICK_THROTTLE,				NULL,				NULL },
	{ 0x80,	"aux_disable_package_data_files",			0, 1,	&AUX_DISABLE_PACKAGE_DATA_FILES,			NULL,				NULL },
	{ 0x80,	"aux_dont_overload_texture_geometry",		0, 1,	&AUX_DONT_OVERLOAD_TEXTURE_GEOMETRY,		NULL,				NULL },
	{ 0x80,	"aux_enable_game_status_window",			0, 1,	&AUX_ENABLE_GAME_STATUS_WINDOW,				NULL,				NULL },
	{ 0x80,	"aux_disable_quit_button",					0, 1,	&AUX_DISABLE_QUIT_BUTTON,					NULL,				NULL },
	{ 0x80,	"aux_disable_message_area",					0, 1,	&AUX_DISABLE_MESSAGE_AREA,					NULL,				NULL },
	{ 0x80,	"aux_enable_demo_replay_user_input",		0, 1,	&AUX_ENABLE_DEMO_REPLAY_USER_INPUT,			NULL,				NULL },
	{ 0x80,	"aux_disable_user_gameloop_exit",			0, 1,	&AUX_DISABLE_USER_GAMELOOP_EXIT,			NULL,				NULL },
	{ 0x80,	"aux_disable_floating_menu_drawing",		0, 1,	&AUX_DISABLE_FLOATING_MENU_DRAWING,			NULL,				NULL },
	{ 0x80,	"aux_save_aux_array_on_demo_replay",		0, 1,	&AUX_SAVE_AUX_ARRAY_ON_DEMO_REPLAY,			NULL,				NULL },
	{ 0x80,	"aux_save_aux_array_on_demo_record",		0, 1,	&AUX_SAVE_AUX_ARRAY_ON_DEMO_RECORD,			NULL,				NULL },
	{ 0x80,	"aux_save_player_name_on_demo_replay",		0, 1,	&AUX_SAVE_PLAYER_NAME_ON_DEMO_REPLAY,		NULL,				NULL },
	{ 0x80,	"aux_kill_limit_for_game_end",				0, 99,	&AUX_KILL_LIMIT_FOR_GAME_END,				RESyncKillLimit,	NULL },
	{ 0x80,	"aux_disable_join_game_button",				0, 1,	&AUX_DISABLE_JOIN_GAME_BUTTON,				NULL,				NULL },
	{ 0x80,	"aux_kill_limit_reset_on_game_end",			0, 1,	&AUX_KILL_LIMIT_RESET_ON_GAME_END,			NULL,				NULL },
	{ 0x80,	"aux_enable_joystick_binding",				0, 1,	&AUX_ENABLE_JOYSTICK_BINDING,				NULL,				NULL },
	{ 0x80,	"aux_enable_depthcued_radar_dots",			0, 1,	&AUX_ENABLE_DEPTHCUED_RADAR_DOTS,			NULL,				NULL },
	{ 0x80,	"aux_enable_mouse_for_menu_screens",		0, 1,	&AUX_ENABLE_MOUSE_FOR_MENU_SCREENS,			NULL,				NULL },
#ifdef ENABLE_CHEAT_COMMANDS
	{ 0x80,	"aux_cheat_disable_ammo_checks",			0, 1,	&AUX_CHEAT_DISABLE_AMMO_CHECKS,				NULL,				NULL },
	{ 0x80,	"aux_cheat_disable_energy_checks",			0, 1,	&AUX_CHEAT_DISABLE_ENERGY_CHECKS,			NULL,				NULL },
	{ 0x80,	"aux_cheat_disable_device_checks",			0, 1,	&AUX_CHEAT_DISABLE_DEVICE_CHECKS,			NULL,				NULL },
#endif // ENABLE_CHEAT_COMMANDS
	{ 0x80,	"aux_draw_wireframe",						0, 2,	&AUX_DRAW_WIREFRAME,						NULL,				NULL },
	{ 0x80,	"aux_draw_normals",							0, 1,	&AUX_DRAW_NORMALS,							NULL,				NULL },
	{ 0x80,	"aux_enable_lighting_types",				0, 0xf,	&AUX_ENABLE_LIGHTING_TYPES,					NULL,				NULL },
	{ 0x80,	"aux_enable_colanim_overloading",			0, 1,	&AUX_ENABLE_COLANIM_OVERLOADING,			NULL,				NULL },
	{ 0x00,	"aux_enable_trilinear_filtering",			0, 1,	&AUX_ENABLE_TRILINEAR_FILTERING,			NULL,				NULL,	1 },
	{ 0x00,	"aux_enable_movie_writing",					0, 1,	&AUX_ENABLE_MOVIE_WRITING,					NULL,				NULL },
	{ 0x00,	"aux_enable_volumetric_fog",				0, 1,	&AUX_ENABLE_VOLUMETRIC_FOG,					NULL,				NULL },
	{ 0x00,	"aux_enable_max_display_freq",				0, 1,	&AUX_ENABLE_MAX_DISPLAY_FREQ,				NULL,				NULL },
	{ 0x00,	"aux_enable_fov_control",					0, 1,	&AUX_ENABLE_FOV_CONTROL,					NULL,				NULL,	1 },
	{ 0x00,	"aux_allow_cache_texture_evict",			0, 1,	&AUX_ALLOW_CACHE_TEXTURE_EVICT,				NULL,				NULL },
	{ 0x80,	"aux_disable_clear_text_on_cleardemo",		0, 1,	&AUX_DISABLE_CLEAR_TEXT_ON_CLEARDEMO,		NULL,				NULL },
	{ 0x80,	"aux_disable_class_map_table_reset",		0, 1,	&AUX_DISABLE_CLASS_MAP_TABLE_RESET,			NULL,				NULL },
	{ 0x80,	"aux_save_refframefreq_on_demo_replay",		0, 1,	&AUX_SAVE_REFFRAMEFREQ_ON_DEMO_REPLAY,		NULL,				NULL },
	{ 0x80,	"aux_save_lightingconf_on_demo_replay",		0, 1,	&AUX_SAVE_LIGHTINGCONF_ON_DEMO_REPLAY,		NULL,				NULL },
	{ 0x80,	"aux_enable_multiple_texture_units",		0, 1,	&AUX_ENABLE_MULTIPLE_TEXTURE_UNITS,			NULL,				NULL },
	{ 0x80,	"aux_disable_camfilt_reset_on_cleardemo",	0, 1,	&AUX_DISABLE_CAMFILT_RESET_ON_CLEARDEMO,	NULL,				NULL },
	{ 0x80,	"aux_disable_dying_ship_movement",			0, 1,	&AUX_DISABLE_DYING_SHIP_MOVEMENT,			NULL,				NULL },
	{ 0x80,	"aux_disable_panel_decorations",			0, 1,	&AUX_DISABLE_PANEL_DECORATIONS,				NULL,				NULL },
	{ 0x80,	"aux_disable_kills_left_announcement",		0, 1,	&AUX_DISABLE_KILLS_LEFT_ANNOUNCEMENT,		NULL,				NULL },
	{ 0x80,	"aux_disable_automatic_nebula_lighting",	0, 1,	&AUX_DISABLE_AUTOMATIC_NEBULA_LIGHTING,		NULL,				NULL },
	{ 0x80,	"aux_disable_backgroundplayer_item_fill",	0, 1,	&AUX_DISABLE_BACKGROUNDPLAYER_ITEM_FILL,	NULL,				NULL },
	{ 0x80,	"aux_draw_planets_on_radar",				0, 1,	&AUX_DRAW_PLANETS_ON_RADAR,					NULL,				NULL },
	{ 0x80,	"aux_draw_server_ping",						0, 1,	&AUX_DRAW_SERVER_PING,						NULL,				NULL },
	{ 0x80,	"aux_disable_point_visibility_detection",	0, 1,	&AUX_DISABLE_POINT_VISIBILITY_DETECTION,	NULL,				NULL },
	{ 0x80,	"aux_enable_texture_precaching",			0, 1,	&AUX_ENABLE_TEXTURE_PRECACHING,				NULL,				NULL },
	{ 0x80,	"aux_disable_level_sync",					0, 1,	&AUX_DISABLE_LEVEL_SYNC ,					NULL,				NULL },
	{ 0x80,	"aux_draw_teleporters_on_radar",			0, 1,	&AUX_DRAW_TELEPORTERS_ON_RADAR,				NULL,				NULL },
	{ 0x80,	"aux_disable_level_console_messages",		0, 1,	&AUX_DISABLE_LEVEL_CONSOLE_MESSAGES,		NULL,				NULL },
	{ 0x80,	"aux_nebula_flags",							0, 16,	&AUX_NEBULA_FLAGS,							NULL,				NULL },
	{ 0x80,	"aux_netcode_flags",						0, 64,	&AUX_NETCODE_FLAGS,							NULL,				NULL },
	{ 0x80,	"aux_enable_beep_on_recv",					0, 1,	&AUX_ENABLE_BEEP_ON_RECV,					NULL,				NULL },
	{ 0x80,	"aux_debug_netstream_dump",					0, 15,	&AUX_DEBUG_NETSTREAM_DUMP,					NULL,				NULL },
	{ 0x80, "aux_anisotropic_filtering",				0, 16,  &AUX_ANISOTROPIC_FILTERING,					NULL,				NULL },
	{ 0x80, "aux_msaa",									0, 8,   &AUX_MSAA,									NULL,				NULL },
};

#define NUM_VERBOSE_AUX_COMMANDS	CALC_NUM_ARRAY_ENTRIES( verbose_aux_commands )


// for registration of aux data as integer variable commands ------------------
//
PRIVATE
int_command_s verbose_aux_data[] = {

	{ 0x80,	"auxdata_least_vertex_1_over_z",			0, 0,	&AUXDATA_LEAST_VERTEX_1_OVER_Z,				NULL,				NULL },
	{ 0x80,	"auxdata_greatest_vertex_1_over_z",			0, 0,	&AUXDATA_GREATEST_VERTEX_1_OVER_Z,			NULL,				NULL },
	{ 0x80,	"auxdata_fillzbuffer_was_called",			0, 0,	&AUXDATA_FILLZBUFFER_WAS_CALLED,			NULL,				NULL },
	{ 0x80,	"auxdata_bsp_scalarproduct_value",			0, 0,	&AUXDATA_BSP_SCALARPRODUCT_VALUE,			NULL,				NULL },
	{ 0x80,	"auxdata_bsp_error_eps",					0, 0,	&AUXDATA_BSP_ERROR_EPS,						NULL,				NULL },
	{ 0x80,	"auxdata_bsp_backface_tolerance_eps",		0, 0,	&AUXDATA_BSP_BACKFACE_TOLERANCE_EPS,		NULL,				NULL },
	{ 0x80,	"auxdata_lensflare_center_pixel",			0, 0,	&AUXDATA_LENSFLARE_CENTER_PIXEL,			NULL,				NULL },
	{ 0x00,	"auxdata_num_particle_clusters",			0, 0,	&AUXDATA_NUM_PARTICLE_CLUSTERS,				NULL,				NULL },
	{ 0x00,	"auxdata_num_visible_pclusters",			0, 0,	&AUXDATA_NUM_VISIBLE_PCLUSTERS,				NULL,				NULL },
	{ 0x00,	"auxdata_psphere_creation_code",			0, 0,	&AUXDATA_PSPHERE_CREATION_CODE,				NULL,				NULL },
	{ 0x00,	"auxdata_psphere_radius",					0, 0,	&AUXDATA_PSPHERE_RADIUS,					NULL,				NULL },
	{ 0x00,	"auxdata_psphere_type",						0, 0,	&AUXDATA_PSPHERE_TYPE,						NULL,				NULL },
	{ 0x00,	"auxdata_tmm_next_texmem_location",			0, 0,	&AUXDATA_TMM_NEXT_TEXMEM_LOCATION,			NULL,				NULL },
	{ 0x00,	"auxdata_tmm_num_downloaded",				0, 0,	&AUXDATA_TMM_NUM_DOWNLOADED,				NULL,				NULL },
	{ 0x00,	"auxdata_tmm_num_nondiscardable",			0, 0,	&AUXDATA_TMM_NUM_NONDISCARDABLE,			NULL,				NULL },
	{ 0x00,	"auxdata_tmm_num_hashtable_entries",		0, 0,	&AUXDATA_TMM_NUM_HASHTABLE_ENTRIES,			NULL,				NULL },
	{ 0x00,	"auxdata_tmm_cur_tailbubble_size",			0, 0,	&AUXDATA_TMM_CUR_TAILBUBBLE_SIZE,			NULL,				NULL },
	{ 0x00,	"auxdata_tmm_cur_bubble_size",				0, 0,	&AUXDATA_TMM_CUR_BUBBLE_SIZE,				NULL,				NULL },
	{ 0x00,	"auxdata_tmm_mipmap_lod_bias",				0, 8,	&AUXDATA_TMM_MIPMAP_LOD_BIAS,				FlushTexCache,		NULL },
	{ 0x00,	"auxdata_screenshot_format",				0, 2,	&AUXDATA_SCREENSHOT_FORMAT,					NULL,				NULL },
	{ 0x00,	"auxdata_screenshot_subformat",				0, 1,	&AUXDATA_SCREENSHOT_SUBFORMAT,				NULL,				NULL },
	{ 0x00,	"auxdata_movie_format",						0, 2,	&AUXDATA_MOVIE_FORMAT,						NULL,				NULL },
	{ 0x00,	"auxdata_movie_subformat",					0, 1,	&AUXDATA_MOVIE_SUBFORMAT,					NULL,				NULL },
	{ 0x00,	"auxdata_message_lifetime",					100, 10000,	&AUXDATA_MESSAGE_LIFETIME,				NULL,				NULL },
	{ 0x00,	"auxdata_message_area_size",				0, MAX_SCREENMESSAGES, &AUXDATA_MESSAGE_AREA_SIZE,	NULL,				NULL },
	{ 0x80,	"auxdata_mouse_sensitivity",				1, 100,	&AUXDATA_MOUSE_SENSITIVITY,					NULL,				NULL },
	{ 0x80,	"auxdata_playerlerp_transition_time",		0, 2000,&AUXDATA_PLAYERLERP_TRANSITION_TIME,		NULL,				NULL },
	{ 0x00,	"auxdata_lod_discrete_geometry_bias",		0, 5,	&AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS,		NULL,				NULL },
	{ 0x00,	"auxdata_smooth_ship_control_factor",		0, 1000,&AUXDATA_SMOOTH_SHIP_CONTROL_FACTOR,		NULL,				NULL },
	{ 0x00,	"auxdata_background_nebula_id",				0, 6,	&AUXDATA_BACKGROUND_NEBULA_ID,				RESyncNebulaId,		NULL },
	{ 0x00,	"auxdata_skill_amazing_time",				0, 6000,&AUXDATA_SKILL_AMAZING_TIME,				RESyncAmazing,		NULL },
	{ 0x00,	"auxdata_skill_brilliant_time",				0, 6000,&AUXDATA_SKILL_BRILLIANT_TIME,				RESyncBrilliant,	NULL },
	{ 0x00,	"auxdata_last_soundsys_returncode",			0, 0,	&AUXDATA_LAST_SOUNDSYS_RETURNCODE,			NULL,				NULL },
	{ 0x00,	"auxdata_numremplayers_copy",				0, 0,	&AUXDATA_NUMREMPLAYERS_COPY,				NULL,				NULL },

};

#define NUM_VERBOSE_AUX_DATA		CALC_NUM_ARRAY_ENTRIES( verbose_aux_data )


// module registration function -----------------------------------------------
//
REGISTER_MODULE( CON_AUX )
{
	ASSERT( NUM_VERBOSE_AUX_COMMANDS <= AUX_ARRAY_NUM_ENTRIES_USED );
	ASSERT( NUM_VERBOSE_AUX_DATA <= AUXDATA_ARRAY_NUM_ENTRIES_USED );

#ifdef REGISTER_VERBOSE_AUX_COMMANDS

	// register aux flags
	for ( unsigned int caux = 0; caux < NUM_VERBOSE_AUX_COMMANDS; caux++ ) {

		//NOTE:
		// bit 7 of the persistence field is used to denote
		// special aux flags that may be excluded from registration.

		#ifdef EXCLUDE_SPECIAL_AUX_FLAGS

			if ( verbose_aux_commands[ caux ].persistence & 0x80 )
				continue;

		#endif

		// destructive but doesn't matter here
		verbose_aux_commands[ caux ].persistence &= ~0x80;

		CON_RegisterIntCommand( verbose_aux_commands + caux );
	}

#endif // REGISTER_VERBOSE_AUX_COMMANDS

#ifdef REGISTER_VERBOSE_AUX_DATA

	// register aux data
	for ( unsigned int cauxd = 0; cauxd < NUM_VERBOSE_AUX_DATA; cauxd++ ) {

		//NOTE:
		// bit 7 of the persistence field is used to denote
		// special aux data that may be excluded from registration.

		#ifdef EXCLUDE_SPECIAL_AUX_DATA

			if ( verbose_aux_data[ cauxd ].persistence & 0x80 )
				continue;

		#endif

		// destructive but doesn't matter here
		verbose_aux_data[ cauxd ].persistence &= ~0x80;

		CON_RegisterIntCommand( verbose_aux_data + cauxd );
	}

#endif // REGISTER_VERBOSE_AUX_DATA

}



