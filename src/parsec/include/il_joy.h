/*
 * PARSEC HEADER: il_joy.h
 */

#ifndef _IL_JOY_H_
#define _IL_JOY_H_


// external functions ---------------------------------------------------------
//
void	IL_JoyInitHandler();
void	IL_JoyKillHandler();
void	IL_JoyCollect();


// device/platform dependend constraints --------------------------------------
//
#define JOY_DEVICE_MIN				-32767
#define JOY_DEVICE_MAX			 	 32767
#define JOY_DEVICE_RANGE		 	 65535


#endif // _IL_JOY_H_


