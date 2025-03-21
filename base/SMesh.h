
#ifndef __S_MESH_H_INCLUDED__
#define __S_MESH_H_INCLUDED__

#include "../commonf.h"
#include "aabbox3d.h"
#include "s3dvertex.h"
#include "xdstring.h"
#include "SMaterial.h"
#include "IReferenceCounted.h"
#include "xarray.h"

class SMeshBuffer : public virtual IReferenceCounted 
{
	public:
		SMeshBuffer() {} // everything's default constructed
		virtual ~SMeshBuffer(){Vertices.clear(); Indices.clear();}

		u32 getVertexCount() const	{ return Vertices.size(); }
		u32 getIndexCount() const  { return Indices.size();	}
	
		//! recalculates the bounding box. should be called if the mesh changed.
		virtual void recalculateBoundingBox()
		{
			if (Vertices.size()==0)
				BoundingBox.reset(0,0,0);
			else
			{
				BoundingBox.reset(Vertices[0].p);
				for (u32 i=1; i<Vertices.size(); ++i)
					BoundingBox.addInternalPoint(Vertices[i].p);
			}
		}

		//! append the vertices and indices to the current buffer
		void append(const void* const vertices, u32 numVertices, const u16* const indices, u32 numIndices)
		{
			const u32 vertexCount = getVertexCount();

			u32 i;
			for (i=0; i<numVertices; ++i)
			{
				Vertices.push_back(reinterpret_cast<const S3DVertex*>(vertices)[i]);
				BoundingBox.addInternalPoint(reinterpret_cast<const S3DVertex*>(vertices)[i].p);
			}
			for (i=0; i<numIndices; ++i)
			{
				Indices.push_back(indices[i]+vertexCount);
			}
		}

		//! append the meshbuffer to the current buffer
		virtual void append(const SMeshBuffer* const other)
		{
			const u32 vertexCount = getVertexCount();

			u32 i;
			for (i=0; i<other->getVertexCount(); ++i)
			{
				Vertices.push_back(other->Vertices[i]);
			}

			for (i=0; i<other->getIndexCount(); ++i)
			{
				Indices.push_back(other->Indices[i]+vertexCount);
			}
			BoundingBox.addInternalBox(other->BoundingBox);
		}

		xstringc	m_texturename;
		SMaterial	Material;
		std::vector<S3DVertex> Vertices;
		std::vector<u16> Indices;
		aabbox3df BoundingBox;
};

// Simple implementation of the SMesh interface.
struct SMesh 
{
	SMesh(){}

	//! destructor
	virtual ~SMesh()
	{
		for (u32 i=0; i<MeshBuffers.size(); ++i)
			delete MeshBuffers[i];
	}

	//! returns amount of mesh buffers.
	virtual u32 getMeshBufferCount() const
	{
			return MeshBuffers.size();
	}

	//! returns pointer to a mesh buffer
	virtual SMeshBuffer* getMeshBuffer(u32 nr) const
	{
		return MeshBuffers[nr];
	}

	//! returns a meshbuffer which fits a material
	// reverse search
	virtual SMeshBuffer* getMeshBuffer( SMaterial & material) const
	{
		for (s32 i = (s32) MeshBuffers.size(); --i >= 0; )
		{
			if ( material == MeshBuffers[i]->Material)
				return MeshBuffers[i];
		}

		return 0;
	}

	//! returns an axis aligned bounding box
	virtual const aabbox3df& getBoundingBox() const
	{
		return BoundingBox;
	}

	//! set user axis aligned bounding box
	virtual void setBoundingBox( const aabbox3df& box)
	{
		BoundingBox = box;
	}

	//! recalculates the bounding box
	void recalculateBoundingBox()
	{
		if (MeshBuffers.size())
		{
			BoundingBox = MeshBuffers[0]->BoundingBox;
			for (u32 i=1; i<MeshBuffers.size(); ++i)
				BoundingBox.addInternalBox(MeshBuffers[i]->BoundingBox);
		}
		else
			BoundingBox.reset(0.0f, 0.0f, 0.0f);
	}

	//! adds a MeshBuffer
	void addMeshBuffer(SMeshBuffer* buf)
	{
		if (buf) MeshBuffers.push_back(buf);
	}

	xarray<SMeshBuffer*> MeshBuffers;
	aabbox3df BoundingBox;
};


/*
struct SMesh 
{
		SMesh()	{}

		virtual ~SMesh()
		{
			for (u32 i=0; i<MeshBuffers.size(); ++i)
				delete MeshBuffers[i];
			MeshBuffers.clear();
		}

		// returns amount of mesh buffers.
		virtual u32 getMeshBufferCount() const	{ return MeshBuffers.size();	}

		// returns pointer to a mesh buffer
		virtual SMeshBuffer* getMeshBuffer(u32 nr) const {	return MeshBuffers[nr];	}

		// recalculates the bounding box
		void recalculateBoundingBox()
		{
			if (MeshBuffers.size())
			{
				BoundingBox = MeshBuffers[0]->BoundingBox;
				for (u32 i=1; i<MeshBuffers.size(); ++i)
					BoundingBox.addInternalBox(MeshBuffers[i]->BoundingBox);
			}
			else
				BoundingBox.reset(0.0f, 0.0f, 0.0f);
		}

		// adds a MeshBuffer
		void addMeshBuffer(SMeshBuffer* buf) { 	if (buf) MeshBuffers.push_back(buf); }

		std::vector<SMeshBuffer*> MeshBuffers;
		aabbox3df BoundingBox;
};
*/


#endif

