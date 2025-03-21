// BigFile.cpp: implementation of the CBigFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BigFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CBigFile, CFile)

CBigFile::CBigFile()
{

}

CBigFile::~CBigFile()
{

}

LONG CBigFile::Seek(LONG dOff, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != hFileNull);
	ASSERT(nFrom == begin || nFrom == end || nFrom == current);
	ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);
	LARGE_INTEGER li;
	li.QuadPart = dOff;

	li.LowPart  = ::SetFilePointer((HANDLE)m_hFile, li.LowPart, &li.HighPart, (DWORD)nFrom);
	DWORD dw = GetLastError();
	if ((li.LowPart == (DWORD)-1) && (dw != NO_ERROR))
	{
		CFileException::ThrowOsError((LONG)dw);
	}

	return (LONG)li.QuadPart;

}
