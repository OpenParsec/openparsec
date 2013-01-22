/*
* PARSEC HEADER: e_world_trans.h
*/

#ifndef _E_WORLD_TRANS_H_
#define _E_WORLD_TRANS_H_

#ifdef PARSEC_SERVER

	// macros for migration purposes ------------------------------------------
	//
	#define PShipObjects		TheWorld->m_PShipObjects
	#define LaserObjects		TheWorld->m_LaserObjects
	#define MisslObjects		TheWorld->m_MisslObjects
	#define ExtraObjects		TheWorld->m_ExtraObjects
	#define CustmObjects		TheWorld->m_CustmObjects

	#define CreateObject		TheWorld->CreateObject
	#define FreeObjList			TheWorld->FreeObjList
	#define KillAllObjects		TheWorld->KillAllObjects
	#define FetchObject			TheWorld->FetchObject
	#define FetchFirstShip		TheWorld->FetchFirstShip
	#define FetchFirstLaser		TheWorld->FetchFirstLaser
	#define FetchFirstMissile	TheWorld->FetchFirstMissile
	#define FetchFirstExtra		TheWorld->FetchFirstExtra
	#define FetchFirstCustom	TheWorld->FetchFirstCustom
	#define KillClassInstances	TheWorld->KillClassInstances

	#define ObjClasses			TheWorld->ObjClasses

#endif // PARSEC_SERVER

#endif // _E_WORLD_TRANS_H_
