//-----------------------------------------------------------------------------
//	BSPLIB HEADER: Material.h
//
//  Copyright (c) 1997-1998 by Markus Hadwiger
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

// bsplib header files
#include "BspLibDefs.h"
#include "SystemIO.h"


BSPLIB_NAMESPACE_BEGIN


// class that describes OpenGL-style material properties ----------------------
//
class Material {

public:
	Material() { }
	~Material() { }

	ColorRGBA	getAmbientColor()	{ return ambientcolor; }
	ColorRGBA	getDiffuseColor()	{ return diffusecolor; }
	ColorRGBA	getSpecularColor()	{ return specularcolor; }
	ColorRGBA	getEmissiveColor()	{ return emissivecolor; }

	void		setAmbientColor( ColorRGBA col )  { ambientcolor = col; ambientcolor.A = 255; }
	void		setDiffuseColor( ColorRGBA col )  { diffusecolor = col; diffusecolor.A = 255; }
	void		setSpecularColor( ColorRGBA col ) { specularcolor = col; specularcolor.A = 255; }
	void		setEmissiveColor( ColorRGBA col ) { emissivecolor = col; emissivecolor.A = 255; }

	float		getShininess()		{ return shininess; }
	float		getTransparency()	{ return transparency; }

	void		setShininess( float s )		{ shininess = s; }
	void		setTransparency( float t )	{ transparency = t; }

private:
	ColorRGBA	ambientcolor;	// OpenGL-style material properties
	ColorRGBA	diffusecolor;
	ColorRGBA	specularcolor;
	ColorRGBA	emissivecolor;
	float		shininess;
	float		transparency;
};


BSPLIB_NAMESPACE_END


#endif // _MATERIAL_H_

