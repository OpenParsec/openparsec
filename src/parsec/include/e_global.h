/*
 * PARSEC HEADER: e_global.h
 */

#ifndef _E_GLOBAL_H_
#define _E_GLOBAL_H_


// include data info tables -------------------------------

#include "gd_tabs.h"


// global externals (ENGINE CORE) -------------------------

extern int				NumLoadedObjects;
extern int				NumLoadedTextures;
extern int				NumLoadedTexfonts;
extern int				NumLoadedBitmaps;
extern int				NumLoadedCharsets;
extern int				NumLoadedSamples;
extern int				NumLoadedSongs;
extern int				NumLoadedPalettes;

extern objectinfo_s		ObjectInfo[];
extern textureinfo_s	TextureInfo[];
extern texfontinfo_s	TexfontInfo[];
extern bitmapinfo_s		BitmapInfo[];
extern charsetinfo_s	CharsetInfo[];
extern sampleinfo_s		SampleInfo[];
extern songinfo_s		SongInfo[];

extern char				CurWorkDir[];

extern char				ForcedPlayerName[];

extern int				ParticleSysEnabled;
extern int				ZBufferEnabled;
extern int				ParticlesTranslucent;

extern char*			sys_ProgramName;
extern char*			TextureMem;
extern char*			BitmapMem;
extern char*			SampleMem;
extern char*			PaletteMem;
extern visual_t			IndexToVisualTab[];
extern colrgba_s		IndexToRGBATab[];

extern colrgba_s		PanelBackColor;
extern colrgba_s		PanelTextColor;
extern colrgba_s		FlareBaseColor;

extern colrgba_s		LightColorAmbient;
extern colrgba_s		LightColorDiffuse;
extern colrgba_s		LightColorSpecular;

extern Vector3			GlobalDirLight;

extern int     			NumRenderedPolygons;

extern refframe_t 		CurScreenRefFrames;
extern refframe_t		ScreenFrameBase;
extern refframe_t		CurPacketRefFrames;
extern refframe_t		PacketFrameBase;
extern refframe_t		RefFrameFrequency;

#define VISFRAME_NEVER	0xFFFFFFFF
#define VISFRAME_START	1
extern dword			CurVisibleFrame;

extern int				ShowFrameRate;

extern int				FixedFrameRate;
extern refframe_t		FixedFrameRateVal;

extern refframe_t		MaxRefFrameVal;

extern int				RecordingActive;
extern FILE*			RecordingFp;
extern int				RecordRemotePackets;
extern int				RemoteRecSessionId;
extern int				RemoteRecPacketId;

extern int				SoundAvailable;
extern int				SoundDisabled;

extern int				SkipCalibrationCode;
extern int				JoystickDisabled;
extern int				DirectNetPlay;
extern int				PrintVidModeList;
extern int				PlayDemo;
extern int				FloatingMenu;
extern int				EnableLogWindow;

extern int				QueryJoystick;
extern int				UserInputDisabled;

extern int				DoLensFlare;
extern int				FlareIntensity;

extern int				ConsoleSliding;
extern int				ConsoleHeight;

extern bams_t 			CurYaw;
extern bams_t 			CurPitch;
extern bams_t 			CurRoll;
extern geomv_t			CurSlideHorz;
extern geomv_t			CurSlideVert;

extern bams_t 			RecYaw;
extern bams_t 			RecPitch;
extern bams_t 			RecRoll;
extern geomv_t			RecSlideHorz;
extern geomv_t			RecSlideVert;

extern bams_t			LastPitch;
extern bams_t			LastYaw;
extern bams_t			LastRoll;
extern geomv_t			LastSlideHorz;
extern geomv_t			LastSlideVert;
extern fixed_t			LastSpeed;
extern int				IdleDuration;

extern int				CurActionWait;
extern bams_t			AutomaticPitch;
extern bams_t			AutomaticYaw;
extern bams_t			AutomaticRoll;
extern geomv_t			AutomaticSlideHorz;
extern geomv_t			AutomaticSlideVert;
extern fixed_t			AutomaticMovement;

extern int				ReplayObjCamActive;

extern bams_t 			AbsYaw;
extern bams_t 			AbsPitch;
extern bams_t 			AbsRoll;

extern Camera			ViewCamera;
extern Camera			ObjectCamera;
extern Camera			ShipViewCamera;

extern int				ObjCameraActive;

extern int				LocalShipClass;
extern ShipObject*		MyShip;
extern size_t			MyShipMaxInstanceSize;

extern ShipObject*		PShipObjects;
extern LaserObject*		LaserObjects;
extern MissileObject*	MisslObjects;
extern ExtraObject*		ExtraObjects;
extern CustomObject*	CustmObjects;
extern GenObject*		VObjList;
extern GenObject*		ObjClasses[];
extern int		 		NumObjClasses;

extern dword			NextObjNumber;

extern int				CurrentNumExtras;
extern int				CurrentNumPrtExtras;
extern int				MaxNumExtras;

extern int				CameraMoved;

extern float			SetScreenBlue;
extern float			SetScreenWhite;
extern int				SetScreenFade;
extern int				SetScreenFadeSpeed;
extern colrgba_s		SetScreenFadeColor;

extern int				AuxEnabling[];
extern int				AuxData[];


#endif // _E_GLOBAL_H_


