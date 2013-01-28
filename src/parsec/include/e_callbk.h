/*
 * PARSEC HEADER: e_callbk.h
 */

#ifndef _E_CALLBK_H_
#define _E_CALLBK_H_


// type for specification of callback type/attributes -------------------------
//
enum callbacktype_t {

	CBTYPE_DRAW_OBJECTS			= 0x0000,   // called before R_DrawWorld()
	CBTYPE_DRAW_PARTICLES		= 0x0001,   // called before R_DrawParticles()
	CBTYPE_DRAW_EFFECTS			= 0x0002,   // called before R_DrawLensFlare()

	CBTYPE_DRAW_PRE_WORLD		= 0x0003,   // called by R_DrawWorld() (begin)
	CBTYPE_DRAW_POST_WORLD		= 0x0004,   // called by R_DrawWorld() (end)
	CBTYPE_DRAW_PRE_PARTICLES	= 0x0005,   // called by R_DrawParticles() (begin)
	CBTYPE_DRAW_POST_PARTICLES	= 0x0006,   // called by R_DrawParticles() (end)

	CBTYPE_DRAW_CUSTOM_ITER		= 0x0007,	// drawing of custom iter objects

	CBTYPE_DRAW_OVERLAYS		= 0x0008,   // called at overlay drawing time

	CBTYPE_VIDMODE_CHANGED		= 0x0009,   // notify of video mode change

	CBTYPE_STREAM_ENDED			= 0x000A,   // notify of audiostream end

	CBTYPE_USER_INPUT			= 0x000B,   // called before GameLoop calls INP_UserProcessInput()

	CBTYPE_NUM_TYPES			= 12,		// ==> UPDATE THIS!! <==

	CBTYPE_MASK					= 0x00ff,	// mask for callback type (256 types possible)

	CBFLAG_REMOVE				= 0x0000,   // flag to remove the callback info from the list after calling
	CBFLAG_PERSISTENT			= 0x0100,	// callback info stays in list until removed
	CBFLAG_MASK					= 0x0f00	//
};


// callback function type -----------------------------------------------------
//
typedef int (*global_callback_fpt)( void *param );


// special callback id indicating registration failed -------------------------
//
#define CBID_FAILED		-1


// external functions ---------------------------------------------------------
//
int CALLBACK_RegisterCallback( int callbacktype, global_callback_fpt callback, void* param );
int CALLBACK_DeleteCallback(   int callbacktype, int callbackid );
int CALLBACK_DestroyCallback(  int callbacktype, void *param );
int CALLBACK_WalkCallbacks(    int callbacktype );


#endif // _E_CALLBK_H_


