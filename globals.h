#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int writeConsoleFmt(const char *format, ...);
int writeConsole(const char* strBuffer);
int readConsoleString(char* strBuffer);
char waitKeyPress(bool bMessage);

typedef struct _imgDados{
	int width;
	int height;
	DWORD* bitmap;
	char* imagePath;
	char* MIMEtype;
	char* codecInfo;
}IMGDATA, *LPIMGDATA;

#pragma pack (push,1)
typedef struct _pixel{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char alpha;
}COLOR, *LPCOLOR;
#define COLOR_WHITE (COLOR){255,255,255,255}

typedef struct _shortVector{
	signed short x;
	signed short y;
}S_VECT, *PS_VECT;

#pragma pack (pop)

typedef DWORD (*IMGFN) (const char* szFileName, LPIMGDATA imgBuff);

#define NULLIMAGE (LPIMGDATA)NULL

#endif

