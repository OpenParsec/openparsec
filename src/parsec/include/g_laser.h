/*
 * PARSEC HEADER: s_laser.h
 */

#ifndef _S_LASER_H_
#define _S_LASER_H_


// external functions

int		CreateLaserBeam( GenObject *ownerpo, dword targetobjno, dword *laserbeamobjno );
int		KillLaserBeam( dword laserbeamobjno );


#endif // _S_LASER_H_


