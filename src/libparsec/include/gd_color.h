/*
 * PARSEC HEADER: gd_color.h
 */

#ifndef _GD_COLOR_H_
#define _GD_COLOR_H_


// ----------------------------------------------------------------------------
// COLOR TYPES                                                                -
// ----------------------------------------------------------------------------


// RGB color triplet (mainly used for vga compatible palette entries) ---------
//
struct colrgb_s {
	union {
		struct {
			byte R;
			byte G;
			byte B;
		};
		byte index[3];
	};
};


// RGBA color specification with alpha component (standard color type) --------
//
struct colrgba_s {
	union {
		struct {
			byte R;
			byte G;
			byte B;
			byte A;
		};
		byte index[4];
	};
};


// color type for the currently active framebuffer color format ---------------
//
typedef dword visual_t;


// helper macros --------------------------------------------------------------
//
#define SETRGBA( x, r,g,b,a ) { (x)->R = r; (x)->G = g; (x)->B = b; (x)->A = a; }
#define SETRGB ( x, r,g,b   ) { (x)->R = r; (x)->G = g; (x)->B = b;             }



#endif // _GD_COLOR_H_


