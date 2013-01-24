/*
 * PARSEC HEADER: r_gl.h
 */

#ifndef _R_GL_H_
#define _R_GL_H_


// include OpenGL headers -----------------------------------------------------

#include <glew/glew.h>



// define basic vertex types --------------------------------------------------

struct GLVertex2 {

	GLfloat	x;		// viewspace x
	GLfloat	y;		// viewspace y

	GLfloat	s;		// texture s
	GLfloat	t;		// texture t

	GLubyte	r;		// red component
	GLubyte	g;		// green component
	GLubyte	b;		// blue component
	GLubyte	a;		// alpha component
};

struct GLVertex3 {

	GLfloat	x;		// viewspace x
	GLfloat	y;		// viewspace y
	GLfloat	z;		// viewspace z

	GLubyte	r;		// red component
	GLubyte	g;		// green component
	GLubyte	b;		// blue component
	GLubyte	a;		// alpha component

	GLfloat	s;		// texture s
	GLfloat	t;		// texture t
	GLfloat	p;		// volume coordinate (r)
	GLfloat	q;		// homogeneous component (w)
};

struct GLVertex3m {

	GLfloat	x;		// viewspace x
	GLfloat	y;		// viewspace y
	GLfloat	z;		// viewspace z

	GLubyte	r;		// red component
	GLubyte	g;		// green component
	GLubyte	b;		// blue component
	GLubyte	a;		// alpha component

	GLfloat	s0;		// texture s0
	GLfloat	t0;		// texture t0
	GLfloat	s1;		// texture s1
	GLfloat	t1;		// texture t1
};

struct GLVertex4 {

	GLfloat	x;		// viewspace x
	GLfloat	y;		// viewspace y
	GLfloat	z;		// viewspace z
	GLfloat	w;		// viewspace w

	GLfloat	s;		// texture s
	GLfloat	t;		// texture t
	GLfloat	p;		// volume coordinate (r)
	GLfloat	q;		// homogeneous component (w)
};

struct GLVertex4m {

	GLfloat	x;		// viewspace x
	GLfloat	y;		// viewspace y
	GLfloat	z;		// viewspace z
	GLfloat	w;		// viewspace w

	GLfloat	s0;		// texture s0
	GLfloat	t0;		// texture t0
	GLfloat	s1;		// texture s1
	GLfloat	t1;		// texture t1
};


#endif // _R_GL_H_

