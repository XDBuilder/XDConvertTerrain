
#ifndef __C_READ_FILE_H_INCLUDED__
#define __C_READ_FILE_H_INCLUDED__

#include <stdio.h>
#include "xdstring.h"
#include "memfile.h"
#include "/hadoop/include/hdfs.h"

//int const MEM=1000000;
class CReadFile
{
	public:
		MemoryFile *m_mf;

		//CReadFile(const wchar_t* fileName,hdfsFS fs1,bool bhdfs) : File(0), FileSize(0) { Filename = fileName; fs=fs1; bHDFS=bhdfs; openFile();}
		CReadFile( char* fileName, hdfsFS fs1, bool bhdfs);

		virtual ~CReadFile() { 
			if(bHDFS==false){ 
				if (File) {
					fclose(File);
					File=NULL;
				}
			}
			else { 
				if(FileHDFS) {
					hdfsCloseFile(fs,FileHDFS);
					FileHDFS=NULL;
				}
			//	delete m_mf;
			}
		}

		s32 read(void* buffer, u32 sizeToRead);
		bool seek(long finalPos, bool relativeMovement = false);
		// returns size of file
		long getSize();

		// returns if file is open
		bool isOpen();

		// returns where in the file we are.
		long getPos();

		// returns name of file
		char* getFileName();

		void* getFile();

	
		//! opens the file
		void openFile();
		void closeFile();

		char Filename[512];
		FILE* File;
		long FileSize;
		
		hdfsFS fs;
		hdfsFile FileHDFS;
		bool bHDFS;		
		u32 m_pos;

};


#endif

