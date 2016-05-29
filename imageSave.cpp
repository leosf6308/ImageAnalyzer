#include "globals.h"

int saveImage(const char* szFileName, LPIMGDATA imgData){
	int i, j, si, di;
	char *fileData, *bmpDest, *bmpSrc;
	HANDLE hFile;
	LPBITMAPFILEHEADER bmpHead;
	LPBITMAPINFOHEADER imgHead;
	DWORD dwLineSz, dwFileSize, nWritten;
	
	if(imgData == (LPIMGDATA)NULL)
		return ERROR_INVALID_PARAMETER;
	
	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return GetLastError();
	/*
	if(imgData->width&3)
		imgData->width += 4-(imgData->width&3);
	*/
	dwLineSz = imgData->width*3;
	if(dwLineSz&3)
		dwLineSz += 4-(dwLineSz&3);
	
	dwFileSize = 54+dwLineSz*imgData->height;
	fileData = (char*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, dwFileSize);
	
	bmpHead = (LPBITMAPFILEHEADER)fileData;
	bmpHead->bfType = 0x4D42;
	bmpHead->bfSize = dwFileSize;
	bmpHead->bfReserved1 = 0;
	bmpHead->bfReserved2 = 0;
	bmpHead->bfOffBits = 54;
	
	imgHead = (LPBITMAPINFOHEADER)(fileData+14);
	imgHead->biSize = 40;
	imgHead->biWidth = imgData->width;
	imgHead->biHeight = -imgData->height;
	imgHead->biPlanes = 1;
	imgHead->biBitCount = 24;
	imgHead->biCompression = 0;
	imgHead->biSizeImage = dwFileSize-54;
	imgHead->biXPelsPerMeter = 0;
	imgHead->biYPelsPerMeter = 0;
	imgHead->biClrUsed = 0;
	imgHead->biClrImportant = 0;
	
	di = 0;
	j = 0;
	bmpSrc = (char*)imgData->bitmap;
	while(j < imgData->height){
		i = 0;
		if(di&3)
			di += 4-(di&3);
		while(i < imgData->width){
			fileData[54+di+0] = *bmpSrc++;
			fileData[54+di+1] = *bmpSrc++;
			fileData[54+di+2] = *bmpSrc++;
			di += 3;
			bmpSrc++;
			i++;
		}
		j++;
	}
	
	WriteFile(hFile,fileData,dwFileSize,&nWritten,NULL);
	CloseHandle(hFile);
	
	HeapFree(GetProcessHeap(),0,fileData);
	writeConsole("Image saved!\n");
	return ERROR_SUCCESS;
}
