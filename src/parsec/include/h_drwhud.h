/*
 * PARSEC HEADER: h_drwhud.h
 */

#ifndef _H_DRWHUD_H_
#define _H_DRWHUD_H_


// external functions

void	InitHudDisplay();

void	DrawOnlineHelp();

void 	HUD_DrawTrackingText( int targetvisible );
void 	HUD_DrawBitmaps();
void 	HUD_WriteAmmoString( int ammo, int x, int y );

void	HUD_DrawHUD();


// external variables

extern int obj_cam_x;
extern int obj_cam_y;


#endif // _H_DRWHUD_H_


