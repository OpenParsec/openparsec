/*
 * PARSEC HEADER: obj_xtra.h
 */

#ifndef _OBJ_XTRA_H_
#define _OBJ_XTRA_H_


// external functions

void	OBJ_FillExtraMemberVars( ExtraObject *extrapo );
void	OBJ_KillExtra( ExtraObject *precnode, int collected );
void	OBJ_AnimateExtra( ExtraObject *extrapo );
void	OBJ_DoExtraPlacement();

void 	OBJ_CreateShipExtras( ShipObject *shippo );


#endif // _OBJ_XTRA_H_


