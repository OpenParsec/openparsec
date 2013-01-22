/*
 * PARSEC HEADER: vid_supp.h
 */

#ifndef _VID_SUPP_H_
#define _VID_SUPP_H_


// external functions
int		VID_GetFOV();
int		VID_SetFOV( int fov2set );
void	VID_SetViewParameters( int mode );
void	VID_SetViewingVolume();
void	VID_RealizeFOV();

#endif // _VID_SUPP_H_


