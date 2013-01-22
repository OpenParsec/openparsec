/*
 * PARSEC HEADER: inp_subh.h
 */

#ifndef _INP_SUBH_H_
#define _INP_SUBH_H_


// ----------------------------------------------------------------------------
// INPUT SUBSYSTEM (INP) system-dependent function declarations               -
// ----------------------------------------------------------------------------

void  INPs_InitGeneral();
void  INPs_KillGeneral();

void  INPs_KeybInitHandler();
void  INPs_KeybKillHandler();

void  INPs_JoyInitHandler();
void  INPs_JoyKillHandler();

void  INPs_MouseInitHandler();
void  INPs_MouseKillHandler();
int   INPs_MouseSetState( mousestate_s *state );
int   INPs_MouseGetState( mousestate_s *state );


int			INPs_ActivateGun();
int			INPs_ActivateMissile();

void		INPs_Collect();
void		INPs_UserProcessAuxInput();

#endif // _INP_SUBH_H_


