/*
 * PARSEC HEADER: vid_glob.h
 */

#ifndef _VID_GLOB_H_
#define _VID_GLOB_H_


// ----------------------------------------------------------------------------
// VIDEO SUBSYSTEM (VID) related global declarations and definitions          -
// ----------------------------------------------------------------------------

// C++ STL vector for the resolution list
#include <vector>

//NOTE:
// the definition of hiresmodeinfo_s would belong into VID_DEFS.H
// but is defined here in order to make the array extdef possible.

struct resinfo_s {
	word width, height;
	
	resinfo_s();
	
	resinfo_s(word w, word h);
	
	// set to a new resolution
	void set(word w, word h);
	
	// has been set to a real resolution
	bool isValid() const;
	
	// sorting and comparing
	bool operator < (const resinfo_s & res) const;	
	bool operator == (const resinfo_s & res) const;
	bool operator != (const resinfo_s & res) const;
};

int GetResolutionIndex(word xres, word yres);


extern int				MaxScreenBPP;

extern resinfo_s		GameScreenRes;
extern int				GameScreenBPP;
extern int				GameScreenWindowed;

extern int				CurDataColorBits;
extern int				CurDataDetail;

extern resinfo_s		Op_Resolution;
extern int				Op_ColorDepth;
extern int				Op_WindowedMode;

extern resinfo_s		InitOp_Resolution;
extern int				InitOp_ColorDepth;
extern int				InitOp_WindowedMode;
extern int				InitOp_FlipSynched;

extern float			GammaCorrection;

extern int				TextModeActive;

extern int				FlipSynched;

extern int				Screen_Width;
extern int				Screen_Height;
extern int				Screen_XOfs;
extern int				Screen_YOfs;
extern int				Screen_BytesPerPixel;
extern int				D_Value;
extern int				Star_Siz;
extern int				RObj_Siz;

extern geomv_t			Near_View_Plane;
extern geomv_t			Far_View_Plane;
extern geomv_t			Criterion_X;
extern geomv_t			Criterion_Y;
extern Plane3			View_Volume[ 6 ];
extern CullPlane3		Cull_Volume[ 6 ];
extern Plane3			World_ViewVolume[ 6 ];
extern CullPlane3		World_CullVolume[ 6 ];
extern Plane3			Object_ViewVolume[ 6 ];
extern CullPlane3		Object_CullVolume[ 6 ];

extern std::vector<resinfo_s> Resolutions;

extern int				VidInfo_NumTextureUnits;
extern int				VidInfo_MaxTextureSize;
extern int				VidInfo_MaxTextureLod;
extern int				VidInfo_UseIterForDemoTexts;
extern int*				VidInfo_SupportedTexFormats;


// determine if video mode is available using mode info table
#define VID_MODE_AVAILABLE(x)			( x >= 0 && x < Resolutions.size() )

// determine current color depth
#define VID_CUR_COLBITS					( GameScreenBPP )



#endif // _VID_GLOB_H_


