
#ifndef __S_PARTICLE_H_INCLUDED__
#define __S_PARTICLE_H_INCLUDED__

#include "vector3d.h"
#include "SColor.h"

	//! Struct for holding particle data
	struct SParticle
	{
		//! Position of the particle
		vector3df pos;

		//! Direction and speed of the particle
		vector3df vector;

		//! Start life time of the particle
		u32 startTime;

		//! End life time of the particle
		u32 endTime;

		//! Current color of the particle
		SColor color;

		//! Original color of the particle. That's the color
		//! of the particle it had when it was emitted.
		SColor startColor;

		//! Original direction and speed of the particle, 
		//! the direction and speed the particle had when
		//! it was emitted.
		vector3df startVector;
	};

#endif

