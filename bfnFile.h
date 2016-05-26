#ifndef BITMAPFONT_H
#define BITMAPFONT_H
#include <windows.h>
typedef struct _bitmapFontFile{
	char signature[4];//BFN1
	WORD fonSizeH;
	WORD format;
	DWORD fileSize;
	char* charsData;
}BFNFILE, *LPBFNFILE;
/* Format:
      1: ASCII Standard
      2: ASCII Extended (ISO 8859-1)
      3: Unicode
*/

int openFon(LPBFNFILE bfnFile, const char* szFileName){
	char* fontData = NULL;
	DWORD dwFileSize, nRead, fonH;
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();
	
	if(!ReadFile(hFile,bfnFile,12,&nRead,NULL)){
		CloseHandle(hFile);
		return GetLastError();
	}
	
	if(memcmp(bfnFile->signature,"BFN1",4) != 0){
		CloseHandle(hFile);
		return -1;
	}
	
	if(bfnFile->fonSizeH&0x0000FFFF != 12 || bfnFile->fonSizeH&0x0000FFFF != 14){
		CloseHandle(hFile);
		return -1;
	}
	
	fonH = bfnFile->fonSizeH&0x0000FFFF;
	
	if(bfnFile->format != 1){
		CloseHandle(hFile);
		return -1;
	}
	
	dwFileSize = GetFileSize(hFile,NULL);
	
	if(bfnFile->fileSize != dwFileSize){
		CloseHandle(hFile);
		return -1;
	}
	
	if(bfnFile->fileSize > fonH*96+12){
		CloseHandle(hFile);
		return -1;
	}
	
	if(fontData == NULL){
		fontData = (char*)GlobalAlloc(GPTR, fonH*96);
		memset(fontData,0, fonH*96);
	}
	
	if(!ReadFile(hFile,fontData,fonH*96,&nRead,NULL)){
		CloseHandle(hFile);
		return GetLastError();
	}
	
	if(nRead != dwFileSize-12){
		fontData = (char*)GlobalFree(fontData);
		CloseHandle(hFile);
		return -1;
	}
	bfnFile->charsData = fontData;
	CloseHandle(hFile);
	return 0;
}


void closeFon(LPBFNFILE bfnFile){
	GlobalFree(bfnFile->charsData);
	ZeroMemory(bfnFile,sizeof(BFNFILE));
}

#endif

