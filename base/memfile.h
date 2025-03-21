#ifndef __S_MEMORY_FILE_H_INCLUDED__
#define __S_MEMORY_FILE_H_INCLUDED__
#include "xdtypes.h"

enum E_SEEK_MODE // Enumeration seek mode.
{
	ESM_BEGIN = 0,   
	ESM_CURRENT ,
	ESM_END    
};


class MemoryFile // �޸𸮸� ���� ó�� ����ϴ� Ŭ����
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
		int		m_position; //���� ���� �б� ��ġ
		int		m_size; // �ʱ� �Ҵ緮

		int		m_capacity; //��밡���� �뷮
		int 	m_tell;// ���� ��ϵ� ������ ��

}; 



#endif 

