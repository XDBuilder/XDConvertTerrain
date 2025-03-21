#include "readfile.h"
CReadFile::CReadFile( char* fileName,hdfsFS fs1,bool bhdfs) : File(0), FileSize(0) {  
		memset(Filename,0,512);
		sprintf(Filename,"%s",fileName); 
		fs=fs1;
		bHDFS=bhdfs; 
		//openFile();
}

//! opens the file
s32 CReadFile::read(void* buffer, u32 sizeToRead)
{  
	if (!isOpen())	return 0;
	//return fread(buffer, 1, sizeToRead, File);
	if(bHDFS==false) {
		return fread(buffer, 1, sizeToRead, File);
	}
	else {
		//hdfsPread(fs,FileHDFS,m_pos,buffer,sizeToRead);
		//m_pos += sizeToRead;
		//return sizeToRead;
		return hdfsRead(fs,FileHDFS,buffer,sizeToRead);	
		
fprintf(stderr,"CReadFile::read\n");			
		//m_mf->read(buffer,sizeToRead);
	}
}

bool CReadFile::seek(long finalPos, bool relativeMovement)
{
	if (!isOpen())	return false;
	if(bHDFS==false) 
		return fseek(File, finalPos, relativeMovement ? SEEK_CUR : SEEK_SET) == 0;
	else {
		int bret = -1;
		if(relativeMovement==false) {
			int ret = hdfsSeek(fs,FileHDFS,finalPos);
			m_pos=finalPos;
			//m_mf->seek(finalPos,ESM_BEGIN);
		}
		else {
			int off = hdfsTell(fs,FileHDFS);
			if(FileSize < off+finalPos) return false;
			bret = hdfsSeek(fs,FileHDFS,off+finalPos);
			m_pos=off+finalPos;
			//int off = m_mf->gettell();
			//m_mf->seek(off+finalPos,ESM_BEGIN);
		}
		if(bret==-1) return false;
		else return true;
	}
}

// returns size of file
long CReadFile::getSize()  { return FileSize; }

// returns if file is open
bool CReadFile::isOpen() 	{ 
//fprintf(stderr,"CReadFile - isOpen!!!!!!!!!!!!!!!\n");
	if(bHDFS==false){
		return File != 0;	
	}
	else {
		return FileHDFS != 0;
		//if(m_mf->gettell()>0) return true;
		//if(m_mf!=NULL) return true;
		//else false;
	}
}

// returns where in the file we are.
long CReadFile::getPos()  { 
	if(bHDFS==false) return ftell(File); 
	else return hdfsTell(fs,FileHDFS);
	//else return m_mf->gettell();
}

// returns name of file
char* CReadFile::getFileName() { return Filename; }

void* CReadFile::getFile()  { 
	if(bHDFS==false) return File; 
	else return FileHDFS;
}  
void CReadFile::closeFile()
{
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
void CReadFile::openFile()
{
	m_pos=0;	
	//m_mf = new MemoryFile(1000000);			
	if (strlen(Filename) == 0) // bugfix posted by rt
	{
		if(bHDFS==false) File = 0;
		else FileHDFS=0;
		return;
	}
	if(bHDFS==false) {
		File = fopen(Filename, "rb");
		if (File)
		{
			// get FileSize
			fseek(File, 0, SEEK_END);
			FileSize = getPos();
			fseek(File, 0, SEEK_SET);
		}
	}
	else {
fprintf(stderr,"openFile - bHDFS true\n");						
		hdfsFileInfo *finfo = hdfsGetPathInfo(fs,Filename);
fprintf(stderr,"openFile - bHDFS true  22 - ||%s||\n",Filename);							
if(fs==NULL) fprintf(stderr,"fs null\n");
		if(finfo) {
			fprintf(stderr,"openFile - finfo not null\n");						
			FileSize =finfo->mSize;
			fprintf(stderr,"openFile - finfo FileSize %d , size %s\n",(int)FileSize,Filename);						
			hdfsFreeFileInfo(finfo,1);
			fprintf(stderr,"openFile - 22\n");	
			//char buf[1000000];
			//memset(buf,0,1000000);					
			FileHDFS = hdfsOpenFile(fs,(const char*)Filename,O_RDONLY,0,0,0);
			//FileHDFS = hdfsOpenFile(fs,"/user/egiskorea/bldg363.3ds",O_RDONLY,0,0,0);
			//FileSize = hdfsAvailable(fs,FileHDFS);
			fprintf(stderr,"openFile - FileSize %ld\n",FileSize);			
			//hdfsPread(fs,FileHDFS,0,buf,FileSize);
			//hdfsCloseFile(fs,FileHDFS);
			//FileHDFS=0;
			//m_mf->write(buf, FileSize);
			fprintf(stderr,"openFile - 33\n");						
		}
	}
}