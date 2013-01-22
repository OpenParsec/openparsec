/*
 * PARSEC HEADER: obj_creg.h
 */

#ifndef _OBJ_CREG_H_
#define _OBJ_CREG_H_


// maximum number of ship classes
#define MAX_SHIP_CLASSES			16

// maximum number of extra classes
#define MAX_EXTRA_CLASSES			64

// class is not a ship
#define SHIPINDEX_NO_SHIP			-1

// class is not an extra
#define EXTRAINDEX_NO_EXTRA			-1


// function pointer type for class registration callback
typedef void (*classreg_fpt)( int numparams, int *params );


// external variables

extern int 		NumShipClasses;
extern dword	ShipClasses[];
extern int		ObjClassShipIndex[];

extern int 		NumExtraClasses;
extern dword	ExtraClasses[];
extern int		ObjClassExtraIndex[];


// external functions

int			OBJ_RegisterClassRegistration( classreg_fpt regfunc );
int			OBJ_RegisterShipClass( dword classid );
int			OBJ_RegisterExtraClass( dword classid );


#endif // _OBJ_CREG_H_


