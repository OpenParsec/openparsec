/*
 * PARSEC HEADER: g_camera.h
 */

#ifndef _G_CAMERA_H_
#define _G_CAMERA_H_


// external functions

void			CAMERA_InitShipOrigin();
void			CAMERA_MakeFixedStarCam( Camera dstcam );

void			CAMERA_GetViewCamera();

void			CAMERA_ResetFilter();
void			CAMERA_BeginModify();
void			CAMERA_EndModify();

void			CAMERA_BeginFrameView();
void			CAMERA_EndFrameView();


// external variables

extern int		pseudo_framecam_is_id;
extern Xmatrx	pseudo_framecam;


#endif // _G_CAMERA_H_


