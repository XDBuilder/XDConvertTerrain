#include "memfile.h"
#include <string.h>
#include <stdlib.h>

#include <algorithm> 
using namespace std;
// Hoon(추가_140213) 메모리 누수 찾기
// jsyun tell 변수 도입 및 기존 방식 수정
//용량 세팅..
int getNextPowerOfTwo(int value) 
{
	int i = 1;
	while (i < value) {
		i *= 2;
	}
	return i;
}

MemoryFile::MemoryFile( int size ) : m_buffer(0), m_capacity(size), m_position(0), m_size(0) 
{
	m_capacity = getNextPowerOfTwo(size);
	m_size = size;
	m_buffer = new u8[m_capacity];
	
	m_position = 0;
	m_tell=0;
}

MemoryFile::~MemoryFile()
{ 	
	if(m_buffer) {
		delete [] m_buffer;
		m_buffer=0;
	}
}

void MemoryFile::clearBuffer()
{
	if(m_buffer) {
		delete [] m_buffer;
		m_buffer=0;
	}
	m_buffer = new u8[m_capacity];
	m_position = 0;
	m_tell=0;
}
void MemoryFile::clearBufferFrom(int pos)
{
	if(m_buffer) {
		memset(m_buffer+pos,0,m_size-pos);
	}
	m_tell=pos;
	m_position=pos;
}
int MemoryFile::read(void* buffer, int size) 
{
	int real_read = min((m_tell - m_position), size);
	memcpy(buffer, m_buffer + m_position, real_read);
	m_position += real_read;
	return real_read;
}

int MemoryFile::write(const void* buffer, int size) 
{
	ensureSize(m_position + size);
	memcpy(m_buffer + m_position, buffer, size);
	m_position += size;
	if(m_tell < m_position+size) m_tell = m_position+size;
	
	return size;
}

bool MemoryFile::seek(int position, int mode) 
{
	int real_pos;
	switch (mode) 
	{
		case ESM_BEGIN:   real_pos = position;              break;
		case ESM_CURRENT: real_pos = m_position + position; break;
		case ESM_END:     real_pos = m_tell - position;     break; //m_size
		default:      return false;
	}
	
	if (real_pos < 0 || real_pos > m_tell) 
	{
		m_position = 0;
		return false;
	} else {
		m_position = real_pos;
		return true;
	}
}


void MemoryFile::ensureSize(int min_size) 
{
	bool realloc_needed = false;
	while (m_capacity < min_size) {
		m_capacity *= 2;
		realloc_needed = true;
	}
	
	if (realloc_needed) 
	{
		u8* new_buffer = new u8[m_capacity];
		memcpy(new_buffer, m_buffer, m_tell);
		delete [] m_buffer;
		m_buffer = new_buffer;
	}
	m_size = min_size;
}

