#ifndef __S_MEMORY_FILE_H_INCLUDED__
#define __S_MEMORY_FILE_H_INCLUDED__
#include "xdtypes.h"

enum E_SEEK_MODE // Enumeration seek mode.
{
	ESM_BEGIN = 0,   
	ESM_CURRENT ,
	ESM_END    
};


class MemoryFile // 메모리를 파일 처럼 사용하는 클래서
{
	public:
		MemoryFile( int size ); 

		virtual ~MemoryFile();

		int  read( void* buffer, int size );
		int  write( const void* buffer, int size );
		bool seek( int position, int mode );

		int  getcurrent() { return m_position; }
		int  gettell() { return m_tell; }

		u8* getdata() { return m_buffer; } 
		int getsize() { return m_size; } 
		int getposition() { return m_position; } 
		void clearBuffer();
		void clearBufferFrom(int pos);

	private:

		void ensureSize(int min_size);

		u8*	m_buffer;
		int		m_position; //현재 쓰고 읽기 위치
		int		m_size; // 초기 할당량

		int		m_capacity; //사용가능한 용량
		int 	m_tell;// 실제 기록된 정보의 끝

}; 



#endif 

