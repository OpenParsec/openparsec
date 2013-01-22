/*
 * PARSEC HEADER: e_mouse.h
 */

#ifndef _E_MOUSE_H_
#define _E_MOUSE_H_


// mouse area definition in reference size

struct mouse_area_s {

	int xpos;
	int ypos;
	int width;
	int height;
};


// mouse area definition for scaled coordinates

struct mouse_area_scaled_s {

	int xleft;
	int xright;
	int ytop;
	int ybottom;
};


// external functions

void	MouseCalcScaledAreas( mouse_area_s *src, mouse_area_scaled_s *dst, int numareas );
int		DrawMouseCursor();


// determine whether a position is within the supplied area -------------------
//
inline
int MouseOverArea( mouse_area_scaled_s *area, int mousex, int mousey )
{
	ASSERT( area != NULL );

	return ( ( mousex >= area->xleft ) && ( mousex < area->xright  ) &&
			 ( mousey >= area->ytop  ) && ( mousey < area->ybottom ) );
}


#endif // _E_MOUSE_H_


