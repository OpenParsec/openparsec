/*
 * PARSEC HEADER: isdl_joy.h
 */

#ifndef _ISDL_JOY_H_
#define _ISDL_JOY_H_


// external functions ---------------------------------------------------------
//
void	ISDL_JoyInitHandler();
void	ISDL_JoyKillHandler();
void	ISDL_JoyCollect();


// device/platform dependend constraints --------------------------------------
//
#define JOY_DEVICE_MIN				-32767
#define JOY_DEVICE_MAX			 	 32767
#define JOY_DEVICE_RANGE		 	 65535

#define JOY_X_DIV				 	 1024 	//JOY_DEVICE_RANGE / JOY_X_RANGE;
#define JOY_Y_DIV					 1024	//JOY_DEVICE_RANGE / JOY_Y_RANGE;
#define JOY_THROTTLE_DIV			 256 	//JOY_DEVICE_RANGE / JOY_THROTTLE_RANGE
#define JOY_THROTTLE_OFF			 128	//JOY_THROTTLE_RANGE / 2
#define JOY_RUDDER_DIV				 1024 	//JOY_DEVICE_RANGE / JOY_RUDDER_RANGE

#endif // _ISDL_JOY_H_


