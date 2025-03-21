#ifndef __S_3D_VERTEX_H_INCLUDED__
#define __S_3D_VERTEX_H_INCLUDED__

#include "vector3d.h"
#include "vector2d.h"
#include "scolor.h"

enum E_VERTEX_TYPE // Enumeration for all vertex types there are.
{
	EVT_STANDARD = 0,// Standard vertex type (Vertex with normal texture coord)	
	EVT_COLORS,	// Vertex with Color texture coord
	EVT_TCOLORS,// Vertex with w Color texture coord(화면 좌표 render to Texture)
	EVT_2TCOORDS,// Vertex with two texture coordinates. Usually used for geometry with lightmaps or other special materials.
	EVT_TANGENTS, // Vertex with a tangent and binormal vector. Usually used for tangent space normal mapping.
	EVT_TERRAIN
};

struct S3DVertex //기본적인 법선벡터와 텍스쳐 좌표
{
	S3DVertex() {};
	S3DVertex(f32 x, f32 y, f32 z, f32 nx, f32 ny, f32 nz, f32 tu, f32 tv)
		: p(x,y,z), n(nx,ny,nz), tuv(tu,tv) {}
	S3DVertex(const vector3df& pos, const vector3df& normal, const vector2df& tcoords)
		: p(pos), n(normal), tuv(tcoords) {}

	vector3df	p;		// Position
	vector3df	n;		// Normal vector
	vector2df	tuv;	// Texture coordinates

	bool operator == (const S3DVertex& other) const	{
		return (p == other.p && n == other.n && tuv == other.tuv);
	}

	bool operator != (const S3DVertex& other) const	{
		return (p != other.p || n != other.n ||	tuv != other.tuv);
	}
	S3DVertex& operator=(const S3DVertex& other){ 
		p = other.p; n = other.n; tuv = other.tuv; return *this; 
	}
};

struct S3DLVertex//색감 좌표를 포함하는 버텍스
{
	S3DLVertex() {};

	S3DLVertex(f32 x, f32 y, f32 z, SColor c, f32 tu, f32 tv)
		: p(x,y,z), color(c), tuv(tu,tv) {}

	S3DLVertex(const vector3df& pos, SColor color, const vector2df& tcoords)
		: p(pos), color(color), tuv(tcoords) {}

	vector3df	p;		// Position
	SColor		color;	// Color
	vector2df	tuv;	// Texture coordinates

	bool operator == (const S3DLVertex& other) const	{
		return (p == other.p && color == other.color && tuv == other.tuv);
	}

	bool operator != (const S3DLVertex& other) const	{
		return (p != other.p || color != other.color || tuv != other.tuv);
	}
	S3DLVertex& operator=(const S3DLVertex& other){ 
		p = other.p; color = other.color; tuv = other.tuv; return *this; 
	}

};

struct S3DTLVertex//2d화면상에 그리기 위한 점
{
	S3DTLVertex() {};

	S3DTLVertex(f32 x, f32 y, f32 z, f32 wcom, SColor c, f32 tu, f32 tv)
		: p(x,y,z), w(wcom), color(c), tuv(tu,tv) {}

	S3DTLVertex(const vector3df& pos, f32 wcom, SColor color, const vector2df& tcoords)
		: p(pos), w(wcom),color(color), tuv(tcoords) {}

	vector3df	p;		// Position
	f32			w;		// w-component
	SColor		color;	// Color
	vector2df	tuv;	// Texture coordinates

	bool operator == (const S3DTLVertex& other) const	{
		return (p == other.p && w==other.w && color == other.color && tuv == other.tuv);
	}

	bool operator != (const S3DTLVertex& other) const	{
		return (p != other.p || w!=other.w || color != other.color || tuv != other.tuv);
	}
};

// Vertex with two texture coordinates.
// Usually used for geometry with lightmaps or other special materials.
struct S3DVertex2TCoords
{
	S3DVertex2TCoords() {};

	S3DVertex2TCoords(f32 x, f32 y, f32 z, SColor c, f32 tu, f32 tv, f32 tu2, f32 tv2)
		: Pos(x,y,z), Color(c), TCoords(tu,tv), TCoords2(tu2,tv2) {}

	S3DVertex2TCoords(const vector3df& pos, SColor color,
		const vector2df& tcoords, const vector2df& tcoords2)
		: Pos(pos), Color(color), TCoords(tcoords), TCoords2(tcoords2) {}

	S3DVertex2TCoords(const vector3df& pos, const vector3df& normal, const SColor& color,
		const vector2df& tcoords, const vector2df& tcoords2)
		: Pos(pos), Normal(normal), Color(color), TCoords(tcoords), TCoords2(tcoords2) {}

	S3DVertex2TCoords(f32 x, f32 y, f32 z, f32 nx, f32 ny, f32 nz, SColor c, f32 tu, f32 tv)
		: Pos(x,y,z), Normal(nx,ny,nz), Color(c), TCoords(tu,tv), TCoords2(tu,tv) {}

	S3DVertex2TCoords(const vector3df& pos, const vector3df& normal,
		SColor color, const vector2df& tcoords)
		: Pos(pos), Normal(normal), Color(color), TCoords(tcoords), TCoords2(tcoords) {}

	
	vector3df	Pos;		// Position
	vector3df	Normal;		// Normal
	SColor		Color;		// Color
	vector2df	TCoords;	// First set of texture coordinates
	vector2df	TCoords2;	// Second set of texture coordinates

	bool operator == (const S3DVertex2TCoords& other) const
	{
		return (Pos == other.Pos && Normal == other.Normal &&
			Color == other.Color && TCoords == other.TCoords &&
			TCoords2 == other.TCoords2);
	}

	bool operator != (const S3DVertex2TCoords& other) const
	{
		return (Pos != other.Pos || Normal != other.Normal ||
			Color != other.Color || TCoords != other.TCoords ||
			TCoords2 != other.TCoords2);
	}

};


// Vertex with a tangent and binormal vector.
// Usually used for tangent space normal mapping.
struct S3DVertexTangents
{
	S3DVertexTangents() { };
	S3DVertexTangents(f32 x, f32 y, f32 z)	: Pos(x,y,z) { }

	S3DVertexTangents(const vector3df& pos,	const vector2df& tcoords, SColor c)
			: Pos(pos), Color(c), TCoords(tcoords) { }

	
	vector3df	Pos;		// Position
	vector3df	Normal;		// Normal vector
	SColor		Color;		// Color
	vector2df	TCoords;	// Texture coordinates
	vector3df	Tangent;	// Tangent vector along the x-axis of the texture
	vector3df	Binormal;	// Binormal vector (tangent x normal)

	bool operator == (const S3DVertexTangents& other) const
	{
		return (Pos == other.Pos && Normal == other.Normal &&
			Color == other.Color && TCoords == other.TCoords &&
			Tangent == other.Tangent && Binormal == other.Binormal);
	}

	bool operator != (const S3DVertexTangents& other) const
	{
		return (Pos != other.Pos || Normal != other.Normal ||
			Color != other.Color || TCoords != other.TCoords ||
			Tangent != other.Tangent || Binormal != other.Binormal);
	}

};

struct S3DVertexTerrain
{
	S3DVertexTerrain() {};

	S3DVertexTerrain(f32 x, f32 y, f32 z,  f32 tu, f32 tv, f32 tu2, f32 tv2)
		: p(x,y,z), tuv(tu,tv), tuv2(tu2,tv2) {}

	S3DVertexTerrain(const vector3df& pos,const vector2df& tcoords, const vector2df& tcoords2)
		: p(pos), tuv(tcoords), tuv2(tcoords2) {}

	S3DVertexTerrain(f32 x, f32 y, f32 z,  f32 tu, f32 tv)
		: p(x,y,z), tuv(tu,tv), tuv2(tu,tv) {}

	S3DVertexTerrain(const vector3df& pos, const vector2df& tcoords)
		: p(pos), tuv(tcoords), tuv2(tcoords) {}

	
	vector3df	p;		// Position
//	SColor		color;
	vector2df	tuv;	// First set of texture coordinates
	vector2df	tuv2;	// Second set of texture coordinates

	bool operator == (const S3DVertexTerrain& other) const
	{
		return (p == other.p && tuv == other.tuv && tuv2 == other.tuv2);
	}

	bool operator != (const S3DVertexTerrain& other) const
	{
		return (p != other.p ||  tuv != other.tuv || tuv2 != other.tuv2);
	}

};


#endif

