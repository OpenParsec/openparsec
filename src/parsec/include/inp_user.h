/*
 * PARSEC HEADER: inp_user.h
 */

#ifndef _INP_USER_H_
#define _INP_USER_H_


// number selectable gun and missile types

#define NUMBER_OF_SELECTABLE_GUNTYPES			4
#define NUMBER_OF_SELECTABLE_MISSILETYPES		4


// external functions

void	INP_UserRotX( bams_t angle );
void	INP_UserRotY( bams_t angle );
void	INP_UserRotZ( bams_t angle );
void	INP_UserSlideX( geomv_t slideval );
void	INP_UserSlideY( geomv_t slideval );

void	INP_UserAcceleration( fixed_t c_speed );

void	INP_UserCycleGuns();
void	INP_UserCycleMissiles();
void	INP_UserCycleTargets();

void	INP_UserSelectFrontTarget();

void	INP_UserZeroSpeed();
void	INP_UserTrackTargetSpeed();

void	INP_UserResetActivityChecking();

void	INP_UserProcessInput();
void	INP_UserCheckFiring();

void	INP_UserActivateAfterBurner();
void	INP_UserDeactivateAfterBurner();

void	INP_UserFiredEMP();


#endif // _INP_USER_H_


