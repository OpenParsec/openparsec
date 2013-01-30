/*
 * PARSEC HEADER: h_cockpt.h
 */

#ifndef _H_COCKPT_H_
#define _H_COCKPT_H_


// geometry of all cockpit elements -------------------------------------------
//
struct cockpitinfo_s {

	float		xpos;
	float		ypos;
	float		width;
	float		height;
	int			owidth;
	int			oheight;

	TextureMap*	texmap;
	const char*	texname;
};


// indices into cockpitinfo[] structure ---------------------------------------
//
enum {

	DAMAGE1,
	DAMAGE1_SLICE,
	DAMAGE2,
	DAMAGE2_SLICE,
	WEAPONS1,
	WEAPONS1_SLICE,
	WEAPONS2,
	WEAPONS2_WORM,
	WEAPONS2_ELITE,
	RADAR,
	RADAR_SLICE,
	RADAR_ELITE,
	ICONBAR1,
	ICONBAR2,
	ENERGYBAR,
	ENERGYBAR_EMPTY,
	SPEEDBAR,
	SPEEDBAR_EMPTY,
	DAMAGEBAR1,
	DAMAGEBAR1_EMPTY,
	DAMAGEBAR2,
	DAMAGEBAR2_EMPTY,
	CROSS,
	CROSS2,
	INCOMING
};


// external functions

void 	COCKPIT_DrawDisplay();
void	COCKPIT_DrawQuickSayBuffer();

void 	FadeInCockpit();
void 	FadeOutCockpit();


// external variables

extern cockpitinfo_s	cockpitinfo[];
extern int				cp_scale;
extern float			scale_tab[];
extern int				cockpit_messagearea_yoffs;
extern float 			Scaled_Screen_Width;
extern float 			Scaled_Screen_Height;


#endif // _H_COCKPT_H_


