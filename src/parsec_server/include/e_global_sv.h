/*
 * PARSEC HEADER: e_global_sv.h
 */

#ifndef _E_GLOBAL_SV_H_
#define _E_GLOBAL_SV_H_


// include data info tables -------------------------------

#include "gd_tabs.h"

// global externals (ENGINE CORE) -------------------------

extern int				NumLoadedObjects;

extern objectinfo_s		ObjectInfo[];

extern char				CurWorkDir[];


extern char*			sys_ProgramName;

extern colrgba_s		LightColorAmbient;
extern colrgba_s		LightColorDiffuse;
extern colrgba_s		LightColorSpecular;

extern Vector3			GlobalDirLight;

extern char*			sys_ProgramName;

#define VISFRAME_NEVER	0xFFFFFFFF

extern int				RecordingActive;
extern FILE*			RecordingFp;
extern int				RecordRemotePackets;
extern int				RemoteRecSessionId;
extern int				RemoteRecPacketId;

extern int				ConsoleSliding;
extern int				ConsoleHeight;

extern GenObject*		VObjList;
extern GenObject*		ObjClasses[];
extern int		 		NumObjClasses;

extern dword			NextObjNumber;

extern int				AuxEnabling[];
extern int				AuxData[];

extern refframe_t		RefFrameFrequency;



// forward decls --------------------------------------------------------------
//
class G_CollDet;
class G_ExtraManager;
class G_Input;
class G_Main;
class NET_PacketDriver;
class NET_UDPDriver;
class E_DistManager;
class E_SimNetInput;
class E_SimNetOutput;
class E_Simulator;
class E_World;
class E_ConnManager;
class E_GameServer;
class MasterServer;
class E_PacketHandler;


// global variables, these are set in SV_MAIN::main() and in E_GameServer::Init()
//
extern G_CollDet*			TheGameCollDet;
extern G_ExtraManager*		TheGameExtraManager;
extern G_Input*			TheGameInput;
extern G_Main*				TheGame;
extern NET_PacketDriver*	ThePacketDriver;
extern NET_UDPDriver*		TheUDPDriver;
extern E_DistManager*		TheDistManager;
extern E_World*			TheWorld;
extern E_SimNetInput*		TheSimNetInput;
extern E_SimNetOutput*	TheSimNetOutput;
extern E_Simulator*		TheSimulator;
extern E_ConnManager*		TheConnManager;
extern E_GameServer*		TheServer;
extern MasterServer*		TheMaster;
extern E_PacketHandler*	ThePacketHandler;



#endif // _E_GLOBAL_SV_H_


