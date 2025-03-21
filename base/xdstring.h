
#ifndef __XD_STRING_H_INCLUDED__
#define __XD_STRING_H_INCLUDED__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xdtypes.h"
#include "allocator.h"
#include "xdmath.h"

//	Very simple xstring class with some useful features.
/**	xstring<c8> and xstring<wchar_t> work both with unicode AND ascii,
so you can assign unicode to xstring<c8> and ascii to xstring<wchar_t>
(and the other way round) if your ever would want to.
Note that the conversation between both is not done using an encoding.

Known bugs:
Special characters like '?, '? and '? are ignored in the
methods make_upper, make_lower and equals_ignore_case.
*/
template <typename T, typename TAlloc = Allocator<T> >
class xstring
{
public:

	// Default constructor
	xstring() : array(0), allocated(1), used(1)
	{
		array = allocator.allocate(1); // new T[1];
		array[0] = 0x0;
	}



	// Constructor
	xstring(const xstring<T>& other) : array(0), allocated(0), used(0)
	{
		*this = other;
	}



	//! Constructs a xstring from a float
	xstring(const double number) : array(0), allocated(0), used(0)
	{
		c8 tmpbuf[255];
		snprintf(tmpbuf, 255, "%0.6f", number);
		*this = tmpbuf;
	}



	//! Constructs a xstring from an int
	xstring(int number) : array(0), allocated(0), used(0)
	{
		// store if negative and make positive

		bool negative = false;
		if (number < 0)
		{
			number *= -1;
			negative = true;
		}

		// temporary buffer for 16 numbers

		c8 tmpbuf[16];
		tmpbuf[15] = 0;
		u32 idx = 15;

		// special case '0'

		if (!number)
		{
			tmpbuf[14] = '0';
			*this = &tmpbuf[14];
			return;
		}

		// add numbers

		while(number && idx)
		{
			--idx;
			tmpbuf[idx] = (c8)('0' + (number % 10));
			number /= 10;
		}

		// add sign

		if (negative)
		{
			--idx;
			tmpbuf[idx] = '-';
		}

		*this = &tmpbuf[idx];
	}

	//! Constructs a xstring from an unsigned int
	xstring(unsigned int number)	: array(0), allocated(0), used(0)
	{
		// temporary buffer for 16 numbers

		c8 tmpbuf[16];
		tmpbuf[15] = 0;
		u32 idx = 15;

		// special case '0'

		if (!number)
		{
			tmpbuf[14] = '0';
			*this = &tmpbuf[14];
			return;
		}

		// add numbers
		while(number && idx)
		{
			--idx;
			tmpbuf[idx] = (c8)('0' + (number % 10));
			number /= 10;
		}

		*this = &tmpbuf[idx];
	}


	//! Constructor for copying a xstring from a pointer with a given length
	template <class B>
	xstring(const B* const c, u32 length) : array(0), allocated(0), used(0)
	{
		if (!c)
		{	// correctly init the xstring to an empty one
			*this="";
			return;
		}

		allocated = used = length+1;
		array = allocator.allocate(used); // new T[used];

		for (u32 l = 0; l<length; ++l)
			array[l] = (T)c[l];

		array[length] = 0;
	}



	//! Constructor for unicode and ascii strings
	template <class B>
	xstring(const B* const c) : array(0), allocated(0), used(0)
	{
		*this = c;
	}


	//! destructor
	~xstring()
	{
		allocator.deallocate(array); // delete [] array;
	}


	//! Assignment operator
	xstring<T>& operator=(const xstring<T>& other)
	{
		if (this == &other)
			return *this;

		allocator.deallocate(array); // delete [] array;
		allocated = used = other.size()+1;
		array = allocator.allocate(used); //new T[used];

		const T* p = other.c_str();
		for (u32 i=0; i<used; ++i, ++p)
			array[i] = *p;

		return *this;
	}


	//! Assignment operator for strings, ascii and unicode
	template <class B>
	xstring<T>& operator=(const B* const c)
	{
		if (!c)
		{
			if (!array)
			{
				array = allocator.allocate(1); //new T[1];
				allocated = 1;
			}
			used = 1;
			array[0] = 0x0;
			return *this;
		}

		if ((void*)c == (void*)array)
			return *this;

		u32 len = 0;
		const B* p = c;
		while(*p)
		{
			++len;
			++p;
		}

		// we'll take the old xstring for a while, because the new
		// xstring could be a part of the current xstring.
		T* oldArray = array;

		++len;
		allocated = used = len;
		array = allocator.allocate(used); //new T[used];

		for (u32 l = 0; l<len; ++l)
			array[l] = (T)c[l];

		allocator.deallocate(oldArray); // delete [] oldArray;
		return *this;
	}

	//! Add operator for other strings
	xstring<T> operator+(const xstring<T>& other) const
	{
		xstring<T> str(*this);
		str.append(other);

		return str;
	}

	//! Add operator for strings, ascii and unicode
	template <class B>
	xstring<T> operator+(const B* const c) const
	{
		xstring<T> str(*this);
		str.append(c);

		return str;
	}



	//! Direct access operator
	T& operator [](const u32 index) const
	{
		__DEBUG_BREAK_IF(index>=used) // bad index

		return array[index];
	}


	//! Comparison operator
	bool operator ==(const T* const str) const
	{
		if (!str)
			return false;

		u32 i;
		for(i=0; array[i] && str[i]; ++i)
			if (array[i] != str[i])
				return false;

		return !array[i] && !str[i];
	}

	//! Comparison operator
	bool operator ==(const xstring<T>& other) const
	{
		for(u32 i=0; array[i] && other.array[i]; ++i)
			if (array[i] != other.array[i])
				return false;

		return used == other.used;
	}


	//! Is smaller operator
	bool operator <(const xstring<T>& other) const
	{
		for(u32 i=0; array[i] && other.array[i]; ++i)
		{
			s32 diff = array[i] - other.array[i];
			if ( diff )
				return diff < 0;
/*
			if (array[i] != other.array[i])
				return (array[i] < other.array[i]);
*/
		}

		return used < other.used;
	}

	//! Equals not operator
	bool operator !=(const T* const str) const
	{
		return !(*this == str);
	}

	//! Equals not operator
	bool operator !=(const xstring<T>& other) const
	{
		return !(*this == other);
	}

	// Returns length of xstring
	// \return Returns length of the xstring in characters. 
	u32 size() const
	{
		return used-1;
	}

	// Returns character xstring
	// return Returns pointer to C-style zero terminated xstring. 
	const T* c_str() const	{	return array;	}

	// Makes the xstring lower case.
	void make_lower()
	{
		for (u32 i=0; i<used; ++i)
			array[i] = ansi_lower ( array[i] );
	}

	// Makes the xstring upper case.
	void make_upper()
	{
		const T a = (T)'a';
		const T z = (T)'z';
		const T diff = (T)'A' - a;

		for (u32 i=0; i<used; ++i)
		{
			if (array[i]>=a && array[i]<=z)
				array[i] += diff;
		}
	}

	// Compares the xstring ignoring case.
	// \param other: Other xstring to compare.
	// \return Returns true if the xstring are equal ignoring case. 
	bool equals_ignore_case(const xstring<T>& other) const
	{
		for(u32 i=0; array[i] && other[i]; ++i)
			if (ansi_lower(array[i]) != ansi_lower(other[i]))
				return false;

		return used == other.used;
	}

	// Compares the xstring ignoring case.
	// \param other: Other xstring to compare.
	// \return Returns true if the xstring is smaller ignoring case. 
	bool lower_ignore_case(const xstring<T>& other) const
	{
		for(u32 i=0; array[i] && other.array[i]; ++i)
		{
			s32 diff = (s32) ansi_lower ( array[i] ) - (s32) ansi_lower ( other.array[i] );
			if ( diff )
				return diff < 0;
		}

		return used < other.used;
	}

	// compares the first n characters of the strings
	bool equalsn(const xstring<T>& other, int len) const
	{
		u32 i;
		for(i=0; array[i] && other[i] && i < len; ++i)
			if (array[i] != other[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same length
		return (i == len) || (used == other.used);
	}

	// compares the first n characters of the strings
	bool equalsn(const T* const str, int len) const
	{
		if (!str)
			return false;
		u32 i;
		for(i=0; array[i] && str[i] && i < len; ++i)
			if (array[i] != str[i])
				return false;

		// if one (or both) of the strings was smaller then they
		// are only equal if they have the same length
		return (i == len) || (array[i] == 0 && str[i] == 0);
	}


	// Appends a character to this xstring
	// \param character: Character to append. 
	void append(T character)
	{
		if (used + 1 > allocated)
			reallocate(used + 1);

		++used;

		array[used-2] = character;
		array[used-1] = 0;
	}

	//! Appends a char xstring to this xstring
	/** \param other: Char xstring to append. */
	void append(const T* const other)
	{
		if (!other)
			return;

		u32 len = 0;
		const T* p = other;
		while(*p)
		{
			++len;
			++p;
		}

		if (used + len > allocated)
			reallocate(used + len);

		--used;
		++len;

		for (u32 l=0; l<len; ++l)
			array[l+used] = *(other+l);

		used += len;
	}


	//! Appends a xstring to this xstring
	/** \param other: String to append. */
	void append(const xstring<T>& other)
	{
		--used;
		u32 len = other.size()+1;

		if (used + len > allocated)
			reallocate(used + len);

		for (u32 l=0; l<len; ++l)
			array[used+l] = other[l];

		used += len;
	}


	//! Appends a xstring of the length l to this xstring.
	/** \param other: other String to append to this xstring.
	 \param length: How much characters of the other xstring to add to this one. */
	void append(const xstring<T>& other, u32 length)
	{
		if (other.size() < length)
		{
			append(other);
			return;
		}

		if (used + length > allocated)
			reallocate(used + length);

		--used;

		for (u32 l=0; l<length; ++l)
			array[l+used] = other[l];
		used += length;

		// ensure proper termination
		array[used]=0;
		++used;
	}


	//! Reserves some memory.
	/** \param count: Amount of characters to reserve. */
	void reserve(u32 count)
	{
		if (count < allocated)
			return;

		reallocate(count);
	}


	//! finds first occurrence of character in xstring
	/** \param c: Character to search for.
	\return Returns position where the character has been found,
	or -1 if not found. */
	s32 findFirst(T c) const
	{
		for (u32 i=0; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}

	//! finds first occurrence of a character of a list in xstring
	/** \param c: List of strings to find. For example if the method
	should find the first occurrence of 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Ususally,
	this should be strlen(ofParameter1)
	\return Returns position where one of the character has been found,
	or -1 if not found. */
	s32 findFirstChar(const T* const c, u32 count) const
	{
		if (!c)
			return -1;

		for (u32 i=0; i<used; ++i)
			for (u32 j=0; j<count; ++j)
				if (array[i] == c[j])
					return i;

		return -1;
	}


	//! Finds first position of a character not in a given list.
	/** \param c: List of characters not to find. For example if the method
	 should find the first occurrence of a character not 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Ususally,
	this should be strlen(ofParameter1)
	\return Returns position where the character has been found,
	or -1 if not found. */
	template <class B>
	s32 findFirstCharNotInList(const B* const c, u32 count) const
	{
		for (u32 i=0; i<used-1; ++i)
		{
			u32 j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	//! Finds last position of a character not in a given list.
	/** \param c: List of characters not to find. For example if the method
	 should find the first occurrence of a character not 'a' or 'b', this parameter should be "ab".
	\param count: Amount of characters in the list. Ususally,
	this should be strlen(ofParameter1)
	\return Returns position where the character has been found,
	or -1 if not found. */
	template <class B>
	s32 findLastCharNotInList(const B* const c, u32 count) const
	{
		for (s32 i=(s32)(used-2); i>=0; --i)
		{
			u32 j;
			for (j=0; j<count; ++j)
				if (array[i] == c[j])
					break;

			if (j==count)
				return i;
		}

		return -1;
	}

	// finds next occurrence of character in xstring
	/** \param c: Character to search for.
	\param startPos: Position in xstring to start searching.
	\return Returns position where the character has been found,
	or -1 if not found. */
	s32 findNext(T c, u32 startPos) const
	{
		for (u32 i=startPos; i<used; ++i)
			if (array[i] == c)
				return i;

		return -1;
	}


	// finds last occurrence of character in xstring
	// \param c: Character to search for.
	// \param start: start to search reverse ( default = -1, on end )
	// \return Returns position where the character has been found, or -1 if not found.
	s32 findLast(T c, s32 start = -1) const
	{	
		s32 a=0;
		if(start < 0)  a = (s32)((used) - 1);
		else a=start;

		s32 b = (s32)((used) - 1);

		start = min_ (max_(a,0), b); //<==clamp(a,0,b);
		for (s32 i=start; i>=0; --i)
			if (array[i] == c)
				return i;

		return -1;
	}

	// finds another xstring in this xstring
	// \param str: Another xstring
	// \return Returns positions where the xstring has been found, or -1 if not found.
	template <class B>
	s32 find(const B* const str) const
	{
		if (str && *str)
		{
			u32 len = 0;

			while (str[len])
				++len;

			if (len > used-1)
				return -1;

			for (u32 i=0; i<used-len; ++i)
			{
				u32 j=0;

				while(str[j] && array[i+j] == str[j])
					++j;

				if (!str[j])
					return i;
			}
		}

		return -1;
	}


	// Returns a substring
	xstring<T> subString(u32 begin, s32 length) const
	{
		if ((length+begin) > size())
			length = size()-begin;
		if (length <= 0)
			return xstring<T>("");

		xstring<T> o;
		o.reserve(length+1);

		for (s32 i=0; i<length; ++i)
			o.array[i] = array[i+begin];

		o.array[length] = 0;
		o.used = o.allocated;

		return o;
	}

	xstring<T> getSepString(T c,int i) // soo 20090407
	{
		u32 start=0;
		u32 length=0;
		u32 count=0;
		for(u32 k=0;k< used; k++){
			if ( array[k]==c ) {
				if(count==i) break;
				start=k+1;
				count++;
				length=0;
			}else 
				length++;
		}
		return subString(start,length);
	}


	xstring<T>& operator += (T c)
	{
		append(c);
		return *this;
	}


	xstring<T>& operator += (const T* const c)
	{
		append(c);
		return *this;
	}


	xstring<T>& operator += (const xstring<T>& other)
	{
		append(other);
		return *this;
	}


	xstring<T>& operator += (const int i)
	{
		append(xstring<T>(i));
		return *this;
	}


	xstring<T>& operator += (const unsigned int i)
	{
		append(xstring<T>(i));
		return *this;
	}


	xstring<T>& operator += (const long i)
	{
		append(xstring<T>(i));
		return *this;
	}


	xstring<T>& operator += (const unsigned long& i)
	{
		append(xstring<T>(i));
		return *this;
	}


	xstring<T>& operator += (const double i)
	{
		append(xstring<T>(i));
		return *this;
	}


	xstring<T>& operator += (const float i)
	{
		append(xstring<T>(i));
		return *this;
	}


	//! replaces all characters of a special type with another one
	void replace(T toReplace, T replaceWith)
	{
		for (u32 i=0; i<used; ++i)
			if (array[i] == toReplace)
				array[i] = replaceWith;
	}

	// trims the xstring. Removes whitespace from begin and end of the xstring. 
	void trim()
	{
		const c8 whitespace[] = " \t\n\r";
		const u32 whitespacecount = 4;

		// find start and end of real xstring without whitespace
		s32 begin = findFirstCharNotInList(whitespace, whitespacecount);
		if (begin == -1)
		{
			*this="";
			return;
		}

		s32 end = findLastCharNotInList(whitespace, whitespacecount);

		*this = subString(begin, (end +1) - begin);
	}


	// Erases a character from the xstring. May be slow, because all elements
	// following after the erased element have to be copied. 
	// \param index: Index of element to be erased.
	void erase(u32 index)
	{
		__DEBUG_BREAK_IF(index>=used) // access violation

		for (u32 i=index+1; i<used; ++i)
			array[i-1] = array[i];

		--used;
	}



private:
/*
	T toLower(const T& t) const
	{
		if (t>=(T)'A' && t<=(T)'Z')
			return t + ((T)'a' - (T)'A');
		else
			return t;
	}
*/
	// Returns a character converted to lower case
	inline T ansi_lower ( u32 x ) const
	{
		return x >= 'A' && x <= 'Z' ? (T) x + 0x20 : (T) x;
	}


	// Reallocate the array, make it bigger or smaller
	void reallocate(u32 new_size)
	{
		T* old_array = array;

		array = allocator.allocate(new_size); //new T[new_size];
		allocated = new_size;

		u32 amount = used < new_size ? used : new_size;
		for (u32 i=0; i<amount; ++i)
			array[i] = old_array[i];

		if (allocated < used)
			used = allocated;

		allocator.deallocate(old_array); // delete [] old_array;
	}

	//--- member variables
	T* array;
	u32 allocated;
	u32 used;
	TAlloc allocator;
};


// Typedef for character strings
typedef xstring<c8> xstringc;

// Typedef for wide character strings
typedef xstring<wchar_t> xstringw;


//inline s32 isdigit(s32 c) { return c >= '0' && c <= '9'; }
//inline s32 isspace(s32 c) { return	c ==  ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';	}
//inline s32 isupper(s32 c) { return c >= 'A' && c <= 'Z'; }


#endif

