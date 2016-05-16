#include "globals.h"

void dumpBitmapHeaders(LPBITMAPFILEHEADER bmpHead, LPBITMAPINFOHEADER imgHead){
	char strBuff[512];
	writeConsoleFmt("\r\nBITMAP FILE\r\n\tBitmap type: 0x%04X\r\n\tFile Size: %d\r\n\tBits offset: +0x%08X\r\n",bmpHead->bfType,bmpHead->bfSize,bmpHead->bfOffBits);
		
	sprintf(strBuff,"\r\nBITMAP IMAGE\r\n\tHeader size: %d\r\n\tResolution: %dx%d\r\n\tPlanes: %d\r\n\tBit count: %d\r\n\tCompression: ",
	imgHead->biSize,imgHead->biWidth,imgHead->biHeight,imgHead->biPlanes,imgHead->biBitCount);
	
	switch(imgHead->biCompression){
		case 0:
			strcat(strBuff,"None");
			break;
		case 1:
			strcat(strBuff,"RLE-8");
			break;
		case 2:
			strcat(strBuff,"RLE-4");
			break;
		case 3:
			strcat(strBuff,"Bitfields");
			break;
		default:
			strcat(strBuff,"Unknown");
			break;
	}
	
	sprintf(strBuff,"%s\r\n\tImage size: %d\r\n\tPixels per meter: %dx%d\r\n\tCOLORS used: %d; important: %d\r\n",strBuff,imgHead->biSizeImage,imgHead->biXPelsPerMeter,imgHead->biXPelsPerMeter,imgHead->biClrUsed,imgHead->biClrImportant);
	writeConsole(strBuff);
}

void dumpColorTable(DWORD *colorTable, int colorCount){
	int i;
	char* dmpData = (char*)HeapAlloc(GetProcessHeap(), 0, 16*colorCount);
	char* strThis = dmpData+1;
	dmpData[0] = '\r';
	dmpData[1] = '\n';
	for(i = 0; i < colorCount; i++){
		strThis += sprintf(strThis,"%04X: ",i);
		switch(colorTable[i]){
				case 0x00000000:
					strcpy(strThis,"black");
					strThis += 5;
					break;
				case 0x00808080:
					strcpy(strThis,"gray");
					strThis += 4;
					break;
				case 0x00C0C0C0:
					strcpy(strThis,"silver");
					strThis += 6;
					break;
				case 0x00FFFFFF:
					strcpy(strThis,"white");
					strThis += 5;
					break;
				case 0x00800000:
					strcpy(strThis,"wine");
					strThis += 4;
					break;
				case 0x00FF0000:
					strcpy(strThis,"red");
					strThis += 3;
					break;
				case 0x00008000:
					strcpy(strThis,"lime");
					strThis += 4;
					break;
				case 0x0000FF00:
					strcpy(strThis,"green");
					strThis += 5;
					break;
				case 0x00000080:
					strcpy(strThis,"marine");
					strThis += 6;
					break;
				case 0x000000FF:
					strcpy(strThis,"blue");
					strThis += 4;
					break;
				case 0x00808000:
					strcpy(strThis,"olive");
					strThis += 5;
					break;
				case 0x00FFFF00:
					strcpy(strThis,"yellow");
					strThis += 6;
					break;
				case 0x00800080:
					strcpy(strThis,"purple");
					strThis += 6;
					break;
				case 0x00FF00FF:
					strcpy(strThis,"fuchsia");
					strThis += 7;
					break;
				case 0x00008080:
					strcpy(strThis,"teal");
					strThis += 4;
					break;
				case 0x0000FFFF:
					strcpy(strThis,"aqua");
					strThis += 4;
					break;
				default:
					strThis += sprintf(strThis,"%02X %02X %02X", (colorTable[i]&0x00FF0000)>>16, (colorTable[i]&0x0000FF00)>>8,(colorTable[i]&0x000000FF));
					break;
		}
		strcpy(strThis,"\r\n");	
		strThis += 2;
	}
	*strThis = '\0';
	writeConsole(dmpData);
}

void save24Bit(int width, int height, char *fileData, LPIMGDATA imgData){
	int i, j, si, step, align;
	DWORD* dwThisPixel = imgData->bitmap;
	/*char* dmpData = (char*)HeapAlloc(GetProcessHeap(), 0, (11*width+2)*height+16);
	char* strThis = dmpData+1;
	dmpData[0] = '\r';
	dmpData[1] = '\n';*/
	DWORD color;
	i = j = si = 0;
	align = ((width*3)&3?4-(width*3)&3:0);
	if(height < 0)
		step = width + align;
	else{
		step = -( width*6 + align );
		si = ( width*3 + align )*(height-1);
	}
	
	while(j < (height>0?height:-height)){
		i = 0;
		while(i < width){
			color = ( *(DWORD*)(fileData+si) ) & 0x00FFFFFF;
			*dwThisPixel++ = color;
			
			si += 3;
			i++;
		}
		//*strThis++ = '\r';
		//*strThis++ = '\n';
		si += step;
			
		j++;
	}
	//*(++strThis) = '\0';
	//writeConsole(dmpData);	
}

void save8Bit(int width, int height, DWORD *colorTable, char *fileData, LPIMGDATA imgData){
	int i, j, si, index, step;
	int align = (width&3?4-width&3:0);
	char* dmpData = (char*)HeapAlloc(GetProcessHeap(), 0, (3*width+2)*height+16);
	DWORD* dwThisPixel = imgData->bitmap;
	char* strThis = dmpData+1;
	dmpData[0] = '\r';
	dmpData[1] = '\n';
	si = j = 0;
	if(height < 0)
		step = width + align;
	else{
		step = -( width*2 + align );
		si = ( width + align )*(height-1);
	}
	
	while(j < (height>0?height:-height)){
		i = 0;
		while(i < width){
			
			index = (((int)fileData[si])&0xFF);
			strThis += sprintf(strThis, "%02X ",index);
			
			*(dwThisPixel++) = colorTable[index];
			si++;
			i++;
		}
		*strThis++ = '\r';
		*strThis++ = '\n';
		si += step;
			
		j++;
	}
	*strThis++ = '\0';
	writeConsole(dmpData);
	HeapFree(GetProcessHeap(),0,dmpData);
}

void save4Bit(int width, int height, DWORD *colorTable, char *fileData, LPIMGDATA imgData){
	char codes[] = "0123456789ABCDEF";
	int i, j, si, index1, index2, step;
	int align = ((width>>1)&3?4-(width>>1)&3:0);
	char* dmpData = (char*)HeapAlloc(GetProcessHeap(), 0, (2*width+2)*height+16);
	//DWORD* dwThisPixel = (height>0?imgData->bitmap+width*(height-1):imgData->bitmap);
	DWORD* dwThisPixel = imgData->bitmap;
	char* strThis = dmpData+1;
	dmpData[0] = '\r';
	dmpData[1] = '\n';
	si = j = 0;
	if(height < 0)
		step = (width>>1) + align;
	else{
		step = -( width + align );
		si = ( (width>>1) + align )*(height-1);
	}
	writeConsoleFmt("Align: %d. Start Index: %d. Step: %d.\n",align,si,step);
	
	while(j < (height>0?height:-height)){
		i = 0;
		while(i < width){
			
			index1 = (((int)fileData[si])&0xF0)>>4;
			index2 = (((int)fileData[si])&0x0F);
			
			*strThis++ = codes[index1];
			*strThis++ = ' ';
			*strThis++ = codes[index2];
			*strThis++ = ' ';
			
			*(dwThisPixel++) = colorTable[index1];
			*(dwThisPixel++) = colorTable[index2];
						
			si++;
			i += 2;
		}
		*strThis++ = '\r';
		*strThis++ = '\n';
		si += step;
		//dwThisPixel -= height*2;
		j++;
	}
	*strThis++ = '\0';
	writeConsole(dmpData);
	HeapFree(GetProcessHeap(),0,dmpData);
}

LPIMGDATA loadBitmap(const char* szFileName){
	DWORD dwFileSize, nRead;
	HANDLE hFile = NULL;
	BITMAPFILEHEADER bmpHead;
	BITMAPINFOHEADER imgHead;
	LPIMGDATA imgData = NULLIMAGE;
	char *colorTable, *fileData, *bitmap;
	ZeroMemory(&bmpHead,sizeof(BITMAPFILEHEADER));
    ZeroMemory(&imgHead,sizeof(BITMAPINFOHEADER));
    
	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		writeConsoleFmt("Can't open '%s'.\n",szFileName);
		return NULLIMAGE;
	}
	
	dwFileSize = GetFileSize(hFile,NULL);
	if(dwFileSize < sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)){
		writeConsoleFmt("File size %dBytes is too low for a bitmap.\n",dwFileSize);
		CloseHandle(hFile);
		return NULLIMAGE;
	}
	
	ReadFile(hFile,(char*)&bmpHead,sizeof(BITMAPFILEHEADER),&nRead,NULL);
	ReadFile(hFile,(char*)&imgHead,sizeof(BITMAPINFOHEADER),&nRead,NULL);
	
	if(bmpHead.bfSize != dwFileSize || bmpHead.bfOffBits > dwFileSize){
		writeConsoleFmt("Data out of bounds. File Size (real): %d. File reported size: %d. Bitmap offset: %d.\n",dwFileSize,bmpHead.bfSize,bmpHead.bfOffBits);
		CloseHandle(hFile);
		return NULLIMAGE;
	}
	
	if(bmpHead.bfType == 0x4D42){
		if(imgHead.biSize == 40){
			int readSize = imgHead.biSizeImage;
			int offsetBits = bmpHead.bfOffBits;
			int tableSize = bmpHead.bfOffBits-(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER));
			if(readSize == 0){
				readSize = imgHead.biWidth*imgHead.biHeight;
				switch(imgHead.biBitCount){
					case 1:
						if(readSize&7) //Aren't bits byte-aligned?
							readSize += 8;
						readSize >>= 4; //Divide by 8.
						break;
					case 4:
						if(readSize&1) //Aren't nibbles(4bits group) bye-aligned?
							readSize += 2;
						readSize >>= 1; //Div 2.
						break;
					case 8: //Byte size (256 colors, very common). We are OK. 
						break;
					case 16: //Word size. We need to double it. This is really rare.
						readSize <<= 1;
						break;
					case 24: //Worst (includes padding...). Mul 3.
						readSize *= 3;
						break;
					case 32: //DWord sized. Multiply by 4. Uncommon format.
						readSize <<= 2;
						break;
					default:
						writeConsoleFmt("%d is a weird Bit count.\n",imgHead.biBitCount);
						return NULLIMAGE;
				}
				
				if(readSize+offsetBits > dwFileSize){
					writeConsoleFmt("The file structure excceds the original size in %d bytes\n",(readSize+offsetBits)-dwFileSize);
					CloseHandle(hFile);
					return NULLIMAGE;
				}
			}
						
			if(offsetBits+readSize > dwFileSize){
				writeConsoleFmt("File computed data size (%dBytes) spans out of data content (%dBytes).\n",offsetBits+readSize,dwFileSize);
				CloseHandle(hFile);
				return NULLIMAGE;
			}
			
			if((sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER))+tableSize+readSize > dwFileSize ){
				writeConsoleFmt("File computed data size (headers+table+bitmap: %dBytes) spans out of data content (%dBytes).\n",(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER))+tableSize+readSize,dwFileSize);
				CloseHandle(hFile);
				return NULLIMAGE;
			}
			
			if(imgHead.biBitCount == 24){
				imgData = (LPIMGDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IMGDATA));
				fileData = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, readSize);
				SetFilePointer(hFile,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),NULL,FILE_BEGIN);
				ReadFile(hFile,colorTable,bmpHead.bfOffBits-(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)),&nRead,NULL);
				ReadFile(hFile,fileData,readSize,&nRead,NULL);
				imgData->width = imgHead.biWidth;
				imgData->height = imgHead.biHeight;
				imgData->bitmap = (DWORD*)GlobalAlloc(GPTR, imgHead.biWidth*imgHead.biHeight*4);
				dumpBitmapHeaders(&bmpHead,&imgHead);
				save24Bit(imgHead.biWidth,imgHead.biHeight,fileData,imgData);
				HeapFree(GetProcessHeap(),0,fileData);
				writeConsole("Dump is done!\n");
			}else if(imgHead.biBitCount == 8){
				if(imgHead.biClrUsed == 0)
					imgHead.biClrUsed = 256;
				imgData = (LPIMGDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IMGDATA));
				colorTable = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
				fileData = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, readSize);
				SetFilePointer(hFile,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),NULL,FILE_BEGIN);
				ReadFile(hFile,colorTable,bmpHead.bfOffBits-(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)),&nRead,NULL);
				ReadFile(hFile,fileData,readSize,&nRead,NULL);
				imgData->width = imgHead.biWidth;
				imgData->height = imgHead.biHeight;
				imgData->bitmap = (DWORD*)GlobalAlloc(GPTR, imgHead.biWidth*imgHead.biHeight*4);
				dumpBitmapHeaders(&bmpHead,&imgHead);
				dumpColorTable((DWORD*)colorTable,imgHead.biClrUsed);
				save8Bit(imgHead.biWidth,imgHead.biHeight,(DWORD*)colorTable,fileData,imgData);
				HeapFree(GetProcessHeap(),0,fileData);
				HeapFree(GetProcessHeap(),0,colorTable);
				writeConsole("Dump is done!\n");
			}else if(imgHead.biBitCount == 4){
				if(imgHead.biClrUsed == 0)
					imgHead.biClrUsed = 16;
				imgData = (LPIMGDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IMGDATA));
				colorTable = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 64);
				fileData = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, readSize);
				SetFilePointer(hFile,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),NULL,FILE_BEGIN);
				ReadFile(hFile,colorTable,bmpHead.bfOffBits-(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)),&nRead,NULL);
				ReadFile(hFile,fileData,readSize,&nRead,NULL);
				imgData->width = imgHead.biWidth;
				imgData->height = imgHead.biHeight;
				imgData->bitmap = (DWORD*)GlobalAlloc(GPTR, imgHead.biWidth*imgHead.biHeight*4);
				dumpBitmapHeaders(&bmpHead,&imgHead);
				dumpColorTable((DWORD*)colorTable,imgHead.biClrUsed);
				save4Bit(imgHead.biWidth,imgHead.biHeight,(DWORD*)colorTable,fileData,imgData);
				HeapFree(GetProcessHeap(),0,fileData);
				HeapFree(GetProcessHeap(),0,colorTable);
				writeConsole("Dump is done!\n");
			}else
				writeConsoleFmt("Dump not implemented for %dBpp...\n",imgHead.biBitCount);
			
		}else
			writeConsoleFmt("Not supported image header: %dBytes\n",imgHead.biSize);
	}else
		writeConsoleFmt("Header is not 0x4D42. It's 0x%04X.\n",bmpHead.bfType&0x0000FFFF);
	CloseHandle(hFile);
	return imgData;
}

LPIMGDATA imageLoaderDLL(const char* DLLName, const char* fileName){
	int nRet;
	LPIMGDATA imgData;
	HINSTANCE hLib = LoadLibraryEx(DLLName,NULL,0);
	if(hLib == (HINSTANCE)NULL){
		writeConsoleFmt("DLL '%s' couldn't be loaded. Error %d.\n",DLLName, GetLastError());
		return NULLIMAGE;
	}
	
	IMGFN getImage = (IMGFN)GetProcAddress(hLib,"getImage");
	if(getImage == (IMGFN)NULL){
		writeConsoleFmt("DLL '%s' doesn't contains 'getImage' function. Error %d.\n",DLLName, GetLastError());
		FreeLibrary(hLib);
		return NULLIMAGE;
	}
	
	imgData = (LPIMGDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IMGDATA));
	if((nRet = getImage(fileName,imgData)) != ERROR_SUCCESS){
		writeConsoleFmt("Error reading file '%s'. Function error code: %d. System error code: %d.\n",fileName,nRet,GetLastError());
		FreeLibrary(hLib);
		return NULLIMAGE;
	}
	
	FreeLibrary(hLib);
	return imgData;
}

LPIMGDATA loadFile(const char* szFileName){
	LPIMGDATA imgData;
	if((imgData = loadBitmap(szFileName)) != NULLIMAGE)
		return imgData;
	else{
		writeConsole("Searching for external formats...");
		
		imgData = imageLoaderDLL("libpng.dll",szFileName);
		if(imgData != NULLIMAGE)
			return imgData;
		
		imgData = imageLoaderDLL("libjpeg.dll",szFileName);
		if(imgData != NULLIMAGE)
			return imgData;
	}
	return NULLIMAGE;
}
