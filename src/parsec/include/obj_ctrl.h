/*
 * PARSEC HEADER: obj_ctrl.h
 */

#ifndef _OBJ_CTRL_H_
#define _OBJ_CTRL_H_

#ifdef PARSEC_SERVER

	#undef CreateObject		
	#undef FreeObjList			
	#undef KillAllObjects		
	#undef FetchObject			
	#undef FetchFirstShip		
	#undef FetchFirstLaser		
	#undef FetchFirstMissile	
	#undef FetchFirstExtra		
	#undef FetchFirstCustom	
	#undef KillClassInstances

#endif // PARSEC_SERVER


// external functions

void			InitObjCtrl();

void			FreeObjectMem( GenObject *objectpo );
int 			FreeObjList( GenObject *listpo );
int 			FreeObjects();

GenObject*		CreateObject( int objclass, const Xmatrx startmatrx );
GenObject*		SummonObject( int objclass, const Xmatrx startmatrx );
GenObject*		SummonObjectFromType( int objtype, const Xmatrx startmatrx );
CustomObject*	CreateVirtualObject( dword objtypeid );

GenObject*		FetchSpecificObject( dword objno, GenObject *listpo );
GenObject*		FetchSpecificHostObject( dword hostobjno, GenObject *listpo );
GenObject*		FetchObject( dword objno );
GenObject*		FetchHostObject( dword hostobjno );

int 			KillSpecificObject( dword objno, GenObject *listpo );
int 			KillSpecificHostObject( dword hostobjno, GenObject *listpo );
int 			KillObject( dword objno );
int				KillHostObject( dword hostobjno );
int 			KillClassInstances( int objclass );

void			OBJ_CorrectObjectInstance( GenObject *dstobj, GenObject *srcobj );
void			OBJ_SwitchObjectLod( GenObject *obj, dword lod );
void			OBJ_AutoSelectObjectLod( GenObject *obj );

void			ScanActiveObjects( const Camera camera );


// return pointer to first ship in ship-objects list --------------------------
//
inline ShipObject *FetchShipListHead()
{
	// fetch head of ship-objects list
	ShipObject *head = (ShipObject *) PShipObjects->NextObj;

	return head;
}


// return pointer to first "real" ship in ship-objects list -------------------
//
inline ShipObject *FetchFirstShip()
{
	// fetch head of ship-objects list
	ShipObject *head = (ShipObject *) PShipObjects->NextObj;

#ifdef PARSEC_CLIENT

	// if local ship is head of list return next object
	if ( head == MyShip ) {
		head = (ShipObject *) head->NextObj;
	}

#endif // PARSEC_CLIENT

	return head;
}


// return pointer to first laser in laser-objects list ------------------------
//
inline LaserObject *FetchFirstLaser()
{
	// fetch head of laser-objects list
	LaserObject *head = (LaserObject *) LaserObjects->NextObj;

	return head;
}


// return pointer to first missile in missile-objects list --------------------
//
inline MissileObject *FetchFirstMissile()
{
	// fetch head of missile-objects list
	MissileObject *head = (MissileObject *) MisslObjects->NextObj;

	return head;
}


// return pointer to first extra in extra-objects list ------------------------
//
inline ExtraObject *FetchFirstExtra()
{
	// fetch head of extra-objects list
	ExtraObject *head = (ExtraObject *) ExtraObjects->NextObj;

	return head;
}


// return pointer to first custom object in custom-objects list ---------------
//
inline CustomObject *FetchFirstCustom()
{
	// fetch head of custom-objects list
	CustomObject *head = (CustomObject *) CustmObjects->NextObj;

	return head;
}


#endif // _OBJ_CTRL_H_


