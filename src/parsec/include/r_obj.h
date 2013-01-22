/*
 * PARSEC HEADER: r_obj.h
 */

#ifndef _R_OBJ_H_
#define _R_OBJ_H_


// ----------------------------------------------------------------------------
// RENDERING SUBSYSTEM (R) OBJ group                                          -
// ----------------------------------------------------------------------------

void	R_RenderObject( GenObject *objectp );
void	R_ReCalcAndRenderObject( GenObject *objectp, const Camera camera );
void	R_DrawWorld( const Camera camera );
void	R_DirectObjectRendering( int flags );


#endif // _R_OBJ_H_


