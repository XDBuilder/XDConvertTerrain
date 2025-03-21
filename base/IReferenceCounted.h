
#ifndef __I_IREFERENCE_COUNTED_H_INCLUDED__
#define __I_IREFERENCE_COUNTED_H_INCLUDED__

#include "assert.h"
#include "xdtypes.h"

class IReferenceCounted
{
	public:

		//! Constructor.
		IReferenceCounted()	: ReferenceCounter(1)	{ }

		//! Destructor.
		virtual ~IReferenceCounted() { }

		virtual void clear() { }

		void grab() const { ++ReferenceCounter; }

		bool drop() 
		{
//			__DEBUG_BREAK_IF(ReferenceCounter <= 0) // someone is doing bad reference counting.
			if(ReferenceCounter <= 0) return true;

			--ReferenceCounter;
			if (!ReferenceCounter)
			{
				clear();
				return true;
			}

			return false;
		}

		//! Returns the reference counter.
		s32 getReferenceCount() const
		{
			return ReferenceCounter;
		}


	private:

		mutable s32 ReferenceCounter;

};


#endif

