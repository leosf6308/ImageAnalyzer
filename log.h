#ifndef LOG_H
#define LOG_H
/*Log.h file
 *This file is used by everyone who needs to keep track of what was done during execution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef struct _logHandler{
	HANDLE hFile;
	char*  logData;
	DWORD  dwSize;
}LOG, *LPLOG;

void showFmtMessage(const char* szFormat, ...){
	int sz;
	char* strTxt;
	va_list vaArg;
	va_start(vaArg,szFormat);
	strTxt = (char*)GlobalAlloc(GPTR, 1024);
	sz = vsprintf(strTxt,szFormat,vaArg);
	va_end(vaArg);
	if(sz > 1024)
		*((char*)NULL) = '\0'; // DEAD END! Drops a SEGFAULT.
	MessageBox(0,strTxt,"Message",MB_ICONEXCLAMATION|MB_OK);
}

void logMessageFmt(LPLOG logStruct, const char* szFormat, ...){
	int nSize;
	DWORD nWritten;
	SYSTEMTIME sysTime;
	
	va_list vaArg;
	if(logStruct->hFile == INVALID_HANDLE_VALUE)
		return;
	
	GetLocalTime(&sysTime);
	nSize = sprintf(logStruct->logData,"[%02d:%02d:%02d.%04d %02d/%02d/%04d]",sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds,sysTime.wDay,sysTime.wMonth,sysTime.wYear);
	WriteFile(logStruct->hFile,logStruct->logData,nSize,&nWritten,NULL);
	
	va_start(vaArg,szFormat);
	nSize = vsnprintf(logStruct->logData,4096,szFormat,vaArg);
	va_end(vaArg);
	
	WriteFile(logStruct->hFile,logStruct->logData,nSize,&nWritten,NULL);
	WriteFile(logStruct->hFile,"\r\n",2,&nWritten,NULL);
}

void logMessage(LPLOG logStruct, const char* string){
	int nSize;
	DWORD nWritten;
	SYSTEMTIME sysTime;
	if(logStruct->hFile == INVALID_HANDLE_VALUE)
		return;
	GetLocalTime(&sysTime);
	nSize = sprintf(logStruct->logData,"[%02d:%02d:%02d.%04d %02d/%02d/%04d]",sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds,sysTime.wDay,sysTime.wMonth,sysTime.wYear);
	WriteFile(logStruct->hFile,logStruct->logData,nSize,&nWritten,NULL);
	nSize = strlen(string);
	WriteFile(logStruct->hFile,string,nSize,&nWritten,NULL);
	if(string[nSize-1] != '\n')
		WriteFile(logStruct->hFile,"\r\n",2,&nWritten,NULL);
}

void logClose(LPLOG logStruct){
	logMessage(logStruct,"The log was ended.\r\n");
	CloseHandle(logStruct->hFile);
	GlobalFree(logStruct->logData);
}

DWORD logStart(LPLOG logStruct, const char* appName){
	char strFileName[512];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	ZeroMemory(strFileName,512);
	
    sprintf(strFileName,"%s_%02d-%02d-%04d.log",appName,sysTime.wDay,sysTime.wMonth,sysTime.wYear);
	logStruct->hFile = CreateFile(strFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	logStruct->dwSize = 0;
	
	if(logStruct->hFile == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_EXISTS){
		logStruct->hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(logStruct->hFile != INVALID_HANDLE_VALUE){
			logStruct->dwSize = GetFileSize(logStruct->hFile,NULL);
			SetFilePointer(logStruct->hFile,0,NULL,FILE_END);
		}
	}
	
	if(logStruct->hFile == INVALID_HANDLE_VALUE){
		MessageBox(0,"Can't create log file.",strFileName,MB_ICONERROR|MB_OK);
		return 0xFFFFFFFF;
	}
	
	logStruct->logData = (char*)GlobalAlloc(GPTR,4096);
	logMessage(logStruct,"The log was started.");
	
	return 0;
}

#endif

