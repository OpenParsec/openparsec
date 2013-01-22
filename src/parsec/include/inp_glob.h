/*
 * PARSEC HEADER: inp_glob.h
 */

#ifndef _INP_GLOB_H_
#define _INP_GLOB_H_


// ----------------------------------------------------------------------------
// INPUT SUBSYSTEM (INP) related global declarations and definitions          -
// ----------------------------------------------------------------------------

// current joystick sensitivity (timing)

extern float	joy_yaw_corr_refframe;
extern float	joy_pitch_corr_refframe;
extern float	joy_roll_corr_refframe;
extern float	joy_acc_corr_refframe;

extern int 		inp_mouse_invert_yaxis;
extern int 		inp_mouse_sensitivity;
extern int 		inp_mouse_drift;

#endif // _INP_GLOB_H_


