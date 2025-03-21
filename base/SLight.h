#ifndef __S_LIGHT_H_INCLUDED__
#define __S_LIGHT_H_INCLUDED__

#include "scolor.h"

enum E_LIGHT_TYPE // Enumeration for different types of lights
{
	ELT_POINT,
	ELT_DIRECTIONAL
};

// Names for light types
const c8* const LightTypeNames[] = {	"Point",	"Directional",	0  };

// structure for holding data describing a dynamic point light.
// ambient light and point lights are the only light supported by the xd engine.
struct SLight
{
	SLight() : AmbientColor(0.0f,0.0f,0.0f), DiffuseColor(1.0f, 1.0f, 1.0f), 
		SpecularColor(1.0f,1.0f,1.0f), Position(0.0f, 0.0f, 0.0f), Radius(100.0f),
		CastShadows(true), Type(ELT_POINT)
		 {};
	
	SColorf AmbientColor; // Ambient color emitted by the light
	SColorf DiffuseColor; // Diffuse color emitted by the light.
	SColorf SpecularColor;// Specular color emitted by the light. 
	vector3df Position;	// Position of the light. If Type is ELT_DIRECTIONAL, this is the direction vector the light is coming from.
	f32 Radius;			// Radius of light. Everything within this radius be be lighted.
	bool CastShadows;	// Does the light cast shadows?
	E_LIGHT_TYPE Type; // Type of the light. Default: ELT_POINT
};

#endif

