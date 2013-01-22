/*
 * PARSEC HEADER: con_aux.h
 */

#ifndef _CON_AUX_H_
#define _CON_AUX_H_


// size of aux arrays
#define MAX_AUX_ENABLING			512
#define MAX_AUX_DATA				128

//FIXME_SERVER: seperate aux flags for client and server


// auxenabling functionality (AuxEnabling[]) :
//
//  0:	Polygons do not fill the z-buffer
//  1:	Enable gouraud shading
//  2:	BSP: Extended sprod check
//  3:	if [2]: draw node itself; otherwise don't
//  4:	if [2]: draw back node, then front node (otherwise the other way around)
//  5:	if [2]: if ( |sprod| < eps ) skip entire subtree
//  6:	Disable ShipObjects
//  7:	Disable ExtraObjects
//  8:	Disable LaserObjects
//  9:	Disable MisslObjects
//  10: Show speed as number after change
//  11: Show energy as number after change
//  12: Don't show player names (panel)
//  13: Don't show missile stats (panel)
//  14: Show missile grafix panel
//  15: Use simple explosion
//  16: Don't show sent text in console
//	...well, the number of flags outpaced my motivation
//     to describe them here; see defines below.

#define AUX_POLYGONS_DONT_FILL_ZBUFFER			AuxEnabling[ 0 ]
#define AUX_ENABLE_GOURAUD_SHADING       		AuxEnabling[ 1 ]
#define AUX_BSP_DO_EXTENDED_SPROD_CHECK			AuxEnabling[ 2 ]
#define AUX_BSP_EXTCHECK_DRAW_NODE				AuxEnabling[ 3 ]
#define AUX_BSP_EXTCHECK_BACKNODE_FIRST 		AuxEnabling[ 4 ]
#define AUX_BSP_EXTCHECK_SKIPSUBTREE			AuxEnabling[ 5 ]
#define AUX_OBJCTRL_DISABLE_SHIPOBJECTS			AuxEnabling[ 6 ]
#define AUX_OBJCTRL_DISABLE_EXTRAOBJECTS		AuxEnabling[ 7 ]
#define AUX_OBJCTRL_DISABLE_LASEROBJECTS		AuxEnabling[ 8 ]
#define AUX_OBJCTRL_DISABLE_MISSLOBJECTS		AuxEnabling[ 9 ]
#define AUX_HUD_SHOW_CHANGED_SPEED_NUMBER		AuxEnabling[ 10 ]
#define AUX_HUD_SHOW_CHANGED_ENERGY_NUMBER		AuxEnabling[ 11 ]
#define AUX_HUD_PANEL_3_CONTROL					AuxEnabling[ 12 ]
#define AUX_HUD_MISSILE_BAR_NUMBERS				AuxEnabling[ 13 ]
#define AUX_HUD_PANEL_4_CONTROL					AuxEnabling[ 14 ]
#define AUX_USE_SIMPLE_EXPLOSION				AuxEnabling[ 15 ]
#define AUX_DISABLE_CONSOLE_SAY_OUTPUT			AuxEnabling[ 16 ]
#define AUX_HUD_DISABLE_USER_DISPLAYS			AuxEnabling[ 17 ]
#define AUX_DONT_DRAW_BLUEPRINTS				AuxEnabling[ 18 ]
#define AUX_CREATE_PROPULSION_FUMES 			AuxEnabling[ 19 ]
#define AUX_DISABLE_SERVER_SETTINGS_BUTTON		AuxEnabling[ 20 ]
#define AUX_DISABLE_SERVER_MENU_BUTTON			AuxEnabling[ 21 ]
#define AUX_DISABLE_ZBUFFER_CLEAR				AuxEnabling[ 22 ]
#define AUX_DISALLOW_USERCONTROLLED_MOTION		AuxEnabling[ 23 ]
#define AUX_DISABLE_SOUNDEFFECTS          		AuxEnabling[ 24 ]
#define AUX_DISABLE_CLEARDEMO_ON_REPSTOP		AuxEnabling[ 25 ]
#define AUX_DISPLAY_RATE_IN_VIEWERS				AuxEnabling[ 26 ]
#define AUX_AWAITRETRACE_ON_FRAMEFINISH 		AuxEnabling[ 27 ]
#define AUX_DISABLE_FIXED_STAR_RENDERING		AuxEnabling[ 28 ]
#define AUX_DISABLE_PSEUDO_STAR_RENDERING		AuxEnabling[ 29 ]
#define AUX_DISABLE_FLASHWHITE_ON_GAMEENTRY		AuxEnabling[ 30 ]
#define AUX_WAIT_FOR_KEYPRESS_AFTER_FADEIN		AuxEnabling[ 31 ]
#define AUX_DISABLE_REORTHO_CAMERA_IN_GAME		AuxEnabling[ 32 ]
#define AUX_DISABLE_REORTHO_IN_VIEWERS   		AuxEnabling[ 33 ]
#define AUX_DONT_CULL_PARTICLE_OBJECTS  		AuxEnabling[ 34 ]
#define AUX_DISABLE_PARTICLE_SYSTEM				AuxEnabling[ 35 ]
#define AUX_ENABLE_TEXTURE_OVERLOADING	 		AuxEnabling[ 36 ]
#define AUX_ENABLE_OBJECT_OVERLOADING	 		AuxEnabling[ 37 ]
#define AUX_ENABLE_BITMAP_OVERLOADING	 		AuxEnabling[ 38 ]
#define AUX_DISABLE_POLYGON_MIPMAPPING	 		AuxEnabling[ 39 ]
#define AUX_DISABLE_BITMAP_MIPMAPPING	 		AuxEnabling[ 40 ]
#define AUX_DISABLE_POLYGON_FILTERING	 		AuxEnabling[ 41 ]
#define AUX_DISABLE_BITMAP_FILTERING	 		AuxEnabling[ 42 ]
#define AUX_ENABLE_PANORAMIC_BACKGROUND			AuxEnabling[ 43 ]
#define AUX_DISABLE_PANORAMIC_LAYER_1			AuxEnabling[ 44 ]
#define AUX_DISABLE_PANORAMIC_LAYER_2			AuxEnabling[ 45 ]
#define AUX_DISABLE_PANORAMIC_LAYER_3			AuxEnabling[ 46 ]
#define AUX_DISABLE_TEXTURE_WRAPPING			AuxEnabling[ 47 ]
#define AUX_DISABLE_BUFFER_CLEAR				AuxEnabling[ 48 ]
#define AUX_SPREADFIRE_PARTICLES_EXTINFO		AuxEnabling[ 49 ]
#define AUX_LIGHTNING_PARTICLES_EXTINFO			AuxEnabling[ 50 ]
#define AUX_EXPLOSION_PARTICLES_EXTINFO			AuxEnabling[ 51 ]
#define AUX_ENERGYFIELD_PARTICLES_EXTINFO		AuxEnabling[ 52 ]
#define AUX_PROTSHIELD_PARTICLES_EXTINFO		AuxEnabling[ 53 ]
#define AUX_MEGASHIELD_PARTICLES_EXTINFO		AuxEnabling[ 54 ]
#define AUX_DONT_FLASH_SCREEN_ON_FLARE			AuxEnabling[ 55 ]
#define AUX_DONT_DRAW_FLARE_CIRCLES				AuxEnabling[ 56 ]
#define AUX_DISABLE_SCREENSHOT_MESSAGE			AuxEnabling[ 57 ]
#define AUX_DISABLE_FLOATLOGO_ROTATION			AuxEnabling[ 58 ]
#define AUX_VERBOSE_TEXTURE_MEM_MANAGER			AuxEnabling[ 59 ]
#define AUX_DISABLE_OBJECT_DEPTH_SORT			AuxEnabling[ 60 ]
#define AUX_ENABLE_CONSOLE_DEBUG_MESSAGES		AuxEnabling[ 61 ]
#define AUX_ENABLE_CONSOLE_MESSAGE_LOG			AuxEnabling[ 62 ]
#define AUX_RECORD_PACKETS_INTO_SESSION			AuxEnabling[ 63 ]
#define AUX_CREATE_EXTRAS_DURING_REPLAY			AuxEnabling[ 64 ]
#define AUX_DISABLE_ABSOLUTE_RECORDING			AuxEnabling[ 65 ]
#define AUX_RECORD_VERBOSE_PACKETS				AuxEnabling[ 66 ]
#define AUX_PERFORM_AUTOMATIC_CONNECT			AuxEnabling[ 67 ]
#define AUX_DISABLE_REPLAY_NET_SIMULATION		AuxEnabling[ 68 ]
#define AUX_DISABLE_LOCAL_STARGATE				AuxEnabling[ 69 ]
#define AUX_PARTICLE_BASE_EXPLOSION				AuxEnabling[ 70 ]
#define AUX_NO_DELAYED_JOIN_REPLAY				AuxEnabling[ 71 ]
#define AUX_HUD_ENABLE_PACKET_LOSS_METER		AuxEnabling[ 72 ]
#define AUX_PACKETDROP_TESTING					AuxEnabling[ 73 ]
#define AUX_DISABLE_TALK_ESCAPE_ON_TAB			AuxEnabling[ 74 ]
#define AUX_USE_BIG_FONT_IN_MESSAGE_AREA		AuxEnabling[ 75 ]
#define AUX_HUD_DISABLE_NONBASIC_DISPLAYS		AuxEnabling[ 76 ]
#define AUX_HUD_ENABLE_DEBUGINFO				AuxEnabling[ 77 ]
#define AUX_HUD_DEBUGINFO_MEMORY				AuxEnabling[ 78 ]
#define AUX_HUD_DEBUGINFO_OBJECTS				AuxEnabling[ 79 ]
#define AUX_HUD_DEBUGINFO_POSITION				AuxEnabling[ 80 ]
#define AUX_HUD_DEBUGINFO_PACKETS				AuxEnabling[ 81 ]
#define AUX_HUD_DEBUGINFO_GEOMETRY				AuxEnabling[ 82 ]
#define AUX_HUD_DEBUGINFO_RASTERIZATION			AuxEnabling[ 83 ]
#define AUX_ADVANCED_PLAYERINTERPOLATION		AuxEnabling[ 84 ]
#define AUX_SAVE_PERSISTENT_INTCOMMANDS			AuxEnabling[ 85 ]
#define AUX_SAVE_PERSISTENT_USRCOMMANDS			AuxEnabling[ 86 ]
#define AUX_SAVE_PERSISTENT_AUX_ARRAY			AuxEnabling[ 87 ]
#define AUX_DISABLE_RC_SCRIPT_SAVING			AuxEnabling[ 88 ]
#define AUX_FORCE_WORKDIR_TO_CURRENT			AuxEnabling[ 89 ]
#define AUX_FORCE_FILEPATH_TO_CURRENT			AuxEnabling[ 90 ]
#define AUX_DISABLE_DEAD_RECKONING				AuxEnabling[ 91 ]
#define AUX_REDIRECT_TALK_TO_IRC_SERVER			AuxEnabling[ 92 ]
#define AUX_HUD_DEBUGINFO_OVER_CONSOLE			AuxEnabling[ 93 ]
#define AUX_EXPLOSION_DRAW_SHOCKWAVE			AuxEnabling[ 94 ]
#define AUX_CMD_WRITE_ACTIVE_IN_TEXTMODE		AuxEnabling[ 95 ]
#define AUX_ENABLE_VOLUMETRIC_CLOUDS			AuxEnabling[ 96 ]
#define AUX_SMALL_PARTICLE_TRIANGLES			AuxEnabling[ 97 ]
#define AUX_OBJ_SORT_POLYS_ON_TEXTURE			AuxEnabling[ 98 ]
#define AUX_NO_DEFAULT_RASTERIZER_STATE			AuxEnabling[ 99 ]
#define AUX_ALLOW_ONLY_TEXTURE_LOADING			AuxEnabling[ 100 ]
#define AUX_CMD_WRITE_DISABLE_OUTPUT			AuxEnabling[ 101 ]
#define AUX_ATTACH_OBJECT_PARTICLES				AuxEnabling[ 102 ]
#define AUX_STOP_DEMO_REPLAY_ON_CONNECT			AuxEnabling[ 103 ]
#define AUX_DISABLE_DISCONNECT_BUTTON			AuxEnabling[ 104 ]
#define AUX_ENABLE_SAMPLE_OVERLOADING			AuxEnabling[ 105 ]
#define AUX_ENABLE_SHADER_OVERLOADING			AuxEnabling[ 106 ]
#define AUX_ATTACH_THRUST_OBJECTS				AuxEnabling[ 107 ]
#define AUX_DISABLE_GENOBJECT_PARTICLES			AuxEnabling[ 108 ]
#define AUX_AUTOSTART_BACKGROUNDPLAYER			AuxEnabling[ 109 ]
#define AUX_HUD_ADVANCED_KILLSTATS				AuxEnabling[ 110 ]
#define AUX_DISABLE_PACKAGE_SCRIPTS				AuxEnabling[ 111 ]
#define AUX_ENABLE_KILLED_SHIP_EXTRAS			AuxEnabling[ 112 ]
#define AUX_STOP_AT_END_OF_CD_TRACK				AuxEnabling[ 113 ]
#define AUX_DIRECT_CLUSTER_RENDERING			AuxEnabling[ 114 ]
#define AUX_ENABLE_ELITE_RADAR					AuxEnabling[ 115 ]
#define AUX_DRAW_MISSILES_ON_RADAR				AuxEnabling[ 116 ]
#define AUX_DRAW_COCKPIT						AuxEnabling[ 117 ]
#define AUX_DRAW_COCKPIT_DAMAGE					AuxEnabling[ 118 ]
#define AUX_DRAW_COCKPIT_WEAPONS				AuxEnabling[ 119 ]
#define AUX_DRAW_COCKPIT_RADAR					AuxEnabling[ 120 ]
#define AUX_DRAW_COCKPIT_ICONBAR				AuxEnabling[ 121 ]
#define AUX_OBJCTRL_DISABLE_CUSTOMOBJECTS		AuxEnabling[ 122 ]
#define AUX_DRAW_COCKPIT_CROSSHAIR				AuxEnabling[ 123 ]
#define AUX_ENABLE_CUSTOMOBJECT_HUDDISPLAY  	AuxEnabling[ 124 ]
#define AUX_DRAW_STARGATES_ON_RADAR				AuxEnabling[ 125 ]
#define AUX_ENABLE_COCKPIT_SWAYING				AuxEnabling[ 126 ]
#define AUX_ENABLE_COCKPIT_RATTLING				AuxEnabling[ 127 ]
#define AUX_ENABLE_SMOOTH_SHIP_CONTROL			AuxEnabling[ 128 ]
#define AUX_ENABLE_EXTENDED_TARGET_CARET		AuxEnabling[ 129 ]
#define AUX_SHOW_PING_IN_SERVERLIST				AuxEnabling[ 130 ]
#define AUX_DRAW_COCKPIT_ICONBAR_BACKGROUND		AuxEnabling[ 131 ]
#define AUX_ENABLE_MISSILE_PROPULSION_FUMES		AuxEnabling[ 132 ]
#define AUX_LASERBEAM_INSTEAD_OF_LASER			AuxEnabling[ 133 ]
#define AUX_ENABLE_FREE_CAMERA					AuxEnabling[ 134 ]
#define AUX_FLAGWORD_SCREENSHOT_HELPERS			AuxEnabling[ 135 ]
#define AUX_DISPLAY_REMOTE_JOIN_OBJECT_ID		AuxEnabling[ 136 ]
#define AUX_ENABLE_REFFRAME_TIMEPLOT			AuxEnabling[ 137 ]
#define AUX_DISABLE_NEARPLANE_TOUCH_CULLING		AuxEnabling[ 138 ]
#define AUX_DISABLE_MSGAREA_IF_CONSOLE_OPEN		AuxEnabling[ 139 ]
#define AUX_ENABLE_MISSILE_POLYGON_TRAILS		AuxEnabling[ 140 ]
#define AUX_ENABLE_FUNKY_AUDIO_COMMENTS			AuxEnabling[ 141 ]
#define AUX_DISABLE_PACKAGE_SCRIPT_LISTING		AuxEnabling[ 142 ]
#define AUX_DISABLE_DEMO_REPLAY_IN_MENU			AuxEnabling[ 143 ]
#define AUX_NO_DELAYED_CREATE_OBJECT_REPLAY		AuxEnabling[ 144 ]
#define AUX_DISABLE_SORTED_PLAYERLIST			AuxEnabling[ 145 ]
#define AUX_DISABLE_OBJECT_FREE_ON_CLEARDEMO	AuxEnabling[ 146 ]
#define AUX_ENABLE_DEMO_REPLAY_INFO_DISPLAY		AuxEnabling[ 147 ]
#define AUX_DISABLE_JOYSTICK_THROTTLE			AuxEnabling[ 148 ]
#define AUX_DISABLE_PACKAGE_DATA_FILES			AuxEnabling[ 149 ]
#define AUX_DONT_OVERLOAD_TEXTURE_GEOMETRY		AuxEnabling[ 150 ]
#define AUX_ENABLE_GAME_STATUS_WINDOW			AuxEnabling[ 151 ]
#define AUX_DISABLE_QUIT_BUTTON					AuxEnabling[ 152 ]
#define AUX_DISABLE_MESSAGE_AREA				AuxEnabling[ 153 ]
#define AUX_ENABLE_DEMO_REPLAY_USER_INPUT		AuxEnabling[ 154 ]
#define AUX_DISABLE_USER_GAMELOOP_EXIT			AuxEnabling[ 155 ]
#define AUX_DISABLE_FLOATING_MENU_DRAWING		AuxEnabling[ 156 ]
#define AUX_SAVE_AUX_ARRAY_ON_DEMO_REPLAY		AuxEnabling[ 157 ]
#define AUX_SAVE_AUX_ARRAY_ON_DEMO_RECORD		AuxEnabling[ 158 ]
#define AUX_SAVE_PLAYER_NAME_ON_DEMO_REPLAY		AuxEnabling[ 159 ]
#define AUX_KILL_LIMIT_FOR_GAME_END				AuxEnabling[ 160 ]
#define AUX_DISABLE_JOIN_GAME_BUTTON			AuxEnabling[ 161 ]
#define AUX_KILL_LIMIT_RESET_ON_GAME_END		AuxEnabling[ 162 ]
#define AUX_ENABLE_JOYSTICK_BINDING				AuxEnabling[ 163 ]
#define AUX_ENABLE_DEPTHCUED_RADAR_DOTS			AuxEnabling[ 164 ]
#define AUX_ENABLE_MOUSE_FOR_MENU_SCREENS		AuxEnabling[ 165 ]
#define AUX_CHEAT_DISABLE_AMMO_CHECKS			AuxEnabling[ 166 ]
#define AUX_CHEAT_DISABLE_ENERGY_CHECKS			AuxEnabling[ 167 ]
#define AUX_CHEAT_DISABLE_DEVICE_CHECKS			AuxEnabling[ 168 ]
#define AUX_DRAW_WIREFRAME						AuxEnabling[ 169 ]
#define AUX_DRAW_NORMALS						AuxEnabling[ 170 ]
#define AUX_ENABLE_LIGHTING_TYPES				AuxEnabling[ 171 ]
#define AUX_ENABLE_COLANIM_OVERLOADING			AuxEnabling[ 172 ]
#define AUX_ENABLE_TRILINEAR_FILTERING	 		AuxEnabling[ 173 ]
#define AUX_ENABLE_MOVIE_WRITING				AuxEnabling[ 174 ]
#define AUX_ENABLE_VOLUMETRIC_FOG				AuxEnabling[ 175 ]
#define AUX_ENABLE_MAX_DISPLAY_FREQ				AuxEnabling[ 176 ]
#define AUX_ENABLE_FOV_CONTROL					AuxEnabling[ 177 ]
#define AUX_ALLOW_CACHE_TEXTURE_EVICT			AuxEnabling[ 178 ]
#define AUX_DISABLE_CLEAR_TEXT_ON_CLEARDEMO		AuxEnabling[ 179 ]
#define AUX_DISABLE_CLASS_MAP_TABLE_RESET		AuxEnabling[ 180 ]
#define AUX_SAVE_REFFRAMEFREQ_ON_DEMO_REPLAY	AuxEnabling[ 181 ]
#define AUX_SAVE_LIGHTINGCONF_ON_DEMO_REPLAY	AuxEnabling[ 182 ]
#define AUX_ENABLE_MULTIPLE_TEXTURE_UNITS		AuxEnabling[ 183 ]
#define AUX_DISABLE_CAMFILT_RESET_ON_CLEARDEMO	AuxEnabling[ 184 ]
#define AUX_DISABLE_DYING_SHIP_MOVEMENT			AuxEnabling[ 185 ]
#define AUX_DISABLE_PANEL_DECORATIONS			AuxEnabling[ 186 ]
#define AUX_DISABLE_KILLS_LEFT_ANNOUNCEMENT		AuxEnabling[ 187 ]
#define AUX_DISABLE_AUTOMATIC_NEBULA_LIGHTING	AuxEnabling[ 188 ]
#define AUX_DISABLE_BACKGROUNDPLAYER_ITEM_FILL	AuxEnabling[ 189 ]
#define AUX_DRAW_PLANETS_ON_RADAR				AuxEnabling[ 190 ]
#define AUX_DRAW_SERVER_PING					AuxEnabling[ 191 ]
#define AUX_DISABLE_POINT_VISIBILITY_DETECTION	AuxEnabling[ 192 ]
#define AUX_ENABLE_TEXTURE_PRECACHING			AuxEnabling[ 193 ]
#define AUX_DISABLE_LEVEL_SYNC 					AuxEnabling[ 194 ]
#define AUX_DRAW_TELEPORTERS_ON_RADAR			AuxEnabling[ 195 ]
#define AUX_DISABLE_LEVEL_CONSOLE_MESSAGES		AuxEnabling[ 196 ]
#define AUX_NEBULA_FLAGS						AuxEnabling[ 197 ]
#define AUX_NETCODE_FLAGS						AuxEnabling[ 198 ]
#define AUX_ENABLE_BEEP_ON_RECV					AuxEnabling[ 199 ]
#define AUX_DEBUG_NETSTREAM_DUMP				AuxEnabling[ 200 ]
#define AUX_ANISOTROPIC_FILTERING				AuxEnabling[ 201 ]
#define AUX_MSAA								AuxEnabling[ 202 ]

#define AUX_ARRAY_NUM_ENTRIES_USED				203	// UPDATE THIS!! <==

#if ( AUX_ARRAY_NUM_ENTRIES_USED > MAX_AUX_ENABLING )
	#error "MAX_AUX_ENABLING too small!"
#endif

// auxdata functionality (AuxData[]) :
//
//  0:	Set to least 1/z of all polygons drawn up to now (set by RenderPolygonEx() )
//  1:	Set to greatest 1/z
//  2:	Set to 1 if FillZBuffer() was called at least once
//  3:	sprod in BSP, if |sprod| < eps
//  4:	eps to use for BSP sprod check
//  5:	eps to use for BSP backface check
//  6:  pixelvalue in center of lens flare object
//
//  8:	number of particle clusters
//  9:	number of particle clusters currently visible
//  10: particle object creation code (trigger)
//  11: radius of particle sphere to create
//  12: animation type of particle sphere to create
//  13: last return code of a soundsystem call
//	14: next available location in texture memory
//	15: number of downloaded (resident) textures
//	16: number of resident textures that are nondiscardable
//	17: size of current tail bubble in texmem
//	18: size of current bubble in texmem
//	19: number of remote players
//	20: output format for screenshots
//	21: output flags for screenshots (subformat)
//	24: lifetime of messages in message area
//	25: number of lines (height) of message area

#define AUXDATA_LEAST_VERTEX_1_OVER_Z			AuxData[ 0 ]
#define AUXDATA_GREATEST_VERTEX_1_OVER_Z		AuxData[ 1 ]
#define AUXDATA_FILLZBUFFER_WAS_CALLED			AuxData[ 2 ]
#define AUXDATA_BSP_SCALARPRODUCT_VALUE 		AuxData[ 3 ]
#define AUXDATA_BSP_ERROR_EPS					AuxData[ 4 ]
#define AUXDATA_BSP_BACKFACE_TOLERANCE_EPS		AuxData[ 5 ]
#define AUXDATA_LENSFLARE_CENTER_PIXEL			AuxData[ 6 ]

#define AUXDATA_NUM_PARTICLE_CLUSTERS			AuxData[ 8 ]
#define AUXDATA_NUM_VISIBLE_PCLUSTERS			AuxData[ 9 ]
#define AUXDATA_PSPHERE_CREATION_CODE			AuxData[ 10 ]
#define AUXDATA_PSPHERE_RADIUS          		AuxData[ 11 ]
#define AUXDATA_PSPHERE_TYPE            		AuxData[ 12 ]

#define AUXDATA_TMM_NEXT_TEXMEM_LOCATION		AuxData[ 13 ]
#define AUXDATA_TMM_NUM_DOWNLOADED				AuxData[ 14 ]
#define AUXDATA_TMM_NUM_NONDISCARDABLE			AuxData[ 15 ]
#define AUXDATA_TMM_NUM_HASHTABLE_ENTRIES		AuxData[ 16 ]
#define AUXDATA_TMM_CUR_TAILBUBBLE_SIZE			AuxData[ 17 ]
#define AUXDATA_TMM_CUR_BUBBLE_SIZE				AuxData[ 18 ]
#define AUXDATA_TMM_MIPMAP_LOD_BIAS				AuxData[ 19 ]	// set

#define AUXDATA_SCREENSHOT_FORMAT				AuxData[ 20 ]	// set
#define AUXDATA_SCREENSHOT_SUBFORMAT			AuxData[ 21 ]	// set

#define AUXDATA_MOVIE_FORMAT					AuxData[ 22 ]	// set
#define AUXDATA_MOVIE_SUBFORMAT					AuxData[ 23 ]	// set

#define AUXDATA_MESSAGE_LIFETIME				AuxData[ 24 ]	// set
#define AUXDATA_MESSAGE_AREA_SIZE				AuxData[ 25 ]	// set

#define AUXDATA_MOUSE_SENSITIVITY				AuxData[ 26 ]	// set

#define AUXDATA_PLAYERLERP_TRANSITION_TIME		AuxData[ 27 ]	// set

#define AUXDATA_LOD_DISCRETE_GEOMETRY_BIAS		AuxData[ 28 ]	// set

#define AUXDATA_SMOOTH_SHIP_CONTROL_FACTOR		AuxData[ 29 ]	// set

#define AUXDATA_BACKGROUND_NEBULA_ID			AuxData[ 30 ]	// set

#define AUXDATA_SKILL_AMAZING_TIME				AuxData[ 31 ]	// set
#define AUXDATA_SKILL_BRILLIANT_TIME			AuxData[ 32 ]	// set

#define AUXDATA_LAST_SOUNDSYS_RETURNCODE		AuxData[ 33 ]
#define AUXDATA_NUMREMPLAYERS_COPY				AuxData[ 34 ]

#define AUXDATA_ARRAY_NUM_ENTRIES_USED			35	// UPDATE THIS!! <==

#if ( AUXDATA_ARRAY_NUM_ENTRIES_USED > MAX_AUX_DATA )
	#error "MAX_AUX_DATA too small!"
#endif


#endif // _CON_AUX_H_


