
#ifndef ___ALLOCATOR_H_INCLUDED__
#define ___ALLOCATOR_H_INCLUDED__

#include <new>
#include <memory.h>
#include "xdtypes.h"

#ifdef DEBUG_CLIENTBLOCK
#undef DEBUG_CLIENTBLOCK
#define DEBUG_CLIENTBLOCK new
#endif

//!	Very simple allocator implementation, containers using it are able to
//! be used it across dll boundaries
template<typename T>
class Allocator
{
public:

	//! destructor
	virtual ~Allocator() {}

	//! allocate memory for an array of objects
	T* allocate(size_t cnt)
	{
		return (T*)internal_new(cnt* sizeof(T));
	}

	//! deallocate memory for an array of objects
	void deallocate(T* ptr)
	{
		internal_delete(ptr);
	}

	//! construct an element
	void construct(T* ptr, const T&e)
	{
		new ((void*)ptr) T(e);
	}

	//! destruct an element
	void destruct(T* ptr)
	{
		ptr->~T();
	}

protected:

	virtual void* internal_new(size_t cnt)
	{
		return operator new(cnt);
	}

	virtual void internal_delete(void* ptr)
	{
		operator delete(ptr);
	}

};


//! fast allocator, only to be used in containers inside the same memory heap.
/** Containers using it are NOT able to be used it across dll boundaries. Use this
when using in an internal class or function or when compiled into a static lib */
template<typename T>
class AllocatorFast
{
public:

	//! allocate memory for an array of objects
	T* allocate(size_t cnt)
	{
		return (T*)operator new(cnt* sizeof(T));
	}

	//! deallocate memory for an array of objects
	void deallocate(T* ptr)
	{
		operator delete(ptr);
	}

	//! construct an element
	void construct(T* ptr, const T&e)
	{
		new ((void*)ptr) T(e);
	}

	//! destruct an element
	void destruct(T* ptr)
	{
		ptr->~T();
	}
};



#ifdef DEBUG_CLIENTBLOCK
#undef DEBUG_CLIENTBLOCK
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

#endif

