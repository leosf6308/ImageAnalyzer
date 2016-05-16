#include "globals.h"
#include "log.h"

LOG hLog;
HANDLE outConsole, inConsole;
char* strConsole;

LPIMGDATA loadFile(const char* szFileName);
int saveImage(const char* szFileName, LPIMGDATA imgData);
void quantizeColors16(LPIMGDATA imgData);
void detectBorder(LPIMGDATA imgData);
void DKHFastScanning(LPIMGDATA imgData);

int writeConsoleFmt(const char *format, ...){
	int scrSz, strSz;
	DWORD cCharsWritten;
	strSz = strlen(format);
	va_list vaArg;
	va_start(vaArg, format);
	
	strSz = vsprintf(strConsole, format, vaArg);
	va_end(vaArg);
	
	WriteConsole(outConsole, strConsole, strSz, &cCharsWritten, NULL);
	logMessage(&hLog,strConsole);
	return strSz;
}


int writeConsole(const char* strBuffer){
	int strSize = strlen(strBuffer);
	DWORD cCharsWritten;
	WriteConsole(outConsole, strBuffer, strSize, &cCharsWritten, NULL);
	logMessage(&hLog,strBuffer);
	return cCharsWritten;
}


int readConsoleString(char* strBuffer){
	int strSize;
	DWORD cCharsRead = 0;
	ReadConsole(inConsole, strBuffer, MAX_PATH, &cCharsRead, NULL);
	strSize = strlen(strBuffer);
	strBuffer[strSize-2] = '\0';
	return cCharsRead;
}

void clearConsole(){
	DWORD nWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(outConsole, &csbi);
	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;
	FillConsoleOutputCharacter(outConsole, ' ', csbi.dwMaximumWindowSize.X*csbi.dwMaximumWindowSize.Y, csbi.dwCursorPosition, &nWritten);
	SetConsoleCursorPosition(outConsole, csbi.dwCursorPosition);
}

char waitKeyPress(bool bMessage){
	char ch;
	DWORD  mode;
	DWORD  count;
	if (bMessage){
		DWORD cCharsWritten;
		WriteConsole(outConsole, "Press any key to continue...", 28, &cCharsWritten, NULL);
	}

	// Switch to raw mode
	GetConsoleMode(inConsole, &mode);
	SetConsoleMode(inConsole, 0);

	// Wait for the user's response
	WaitForSingleObject(inConsole, INFINITE);

	// Read the (single) key pressed
	ReadConsole(inConsole, &ch, 1, &count, NULL);

	// Restore the console to its previous state
	SetConsoleMode(inConsole, mode);

	// Return the key code
	return ch;
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	int option = 0;
	LPIMGDATA imgData;
	AllocConsole();
	outConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	inConsole = GetStdHandle(STD_INPUT_HANDLE);
	strConsole = (char*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, 2048);
	logStart(&hLog, "bmpdump");
	if(lpCmdLine == NULL || strcmp(lpCmdLine,"") == 0){
		lpCmdLine = (char*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, MAX_PATH);
		writeConsole("No file was given. Type a file name (RETURN for load dialog): ");
		readConsoleString(lpCmdLine);
		if(strcmp(lpCmdLine,"") == 0){
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			lpCmdLine[0] = 0;
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = HWND_DESKTOP;
			ofn.lpstrFilter = "Todos os formatos suportados()\0*.png;*.jpg;*.jpeg;*.bmp\0Imagem PNG(*.png)\0*.png\0Foto JPEG(*.jpg;*.jpeg)\0*.jpg;*.jpeg\0Arquivo de bitmap(*.bmp;*.dib)\0*.bmp;*.dib\0Todos os arquivos (*.*)\0*.*\0\0";
			ofn.lpstrFile = lpCmdLine;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrDefExt = "bmp";
			ofn.Flags = OFN_FILEMUSTEXIST;
			GetOpenFileName(&ofn);	
		}
	}
	imgData = loadFile(lpCmdLine);
	if(imgData != NULLIMAGE){
		writeConsoleFmt("Image was loaded. Size: %dx%d\n",imgData->width,imgData->height);
	}else{
		waitKeyPress(true);
		logClose(&hLog);
		FreeConsole();
		return 0;
	}
	
	do{
		writeConsole("\nAvailable operations:\n\t1. Save image.\n\t2. Quantize colors (16bits)\n\t3. Detect border\n\t4. Scan for clusters.\n\t5. Show histogram.\n\t0. Leave\nYour choice:");
		option = (int)waitKeyPress(false)-0x30;
		writeConsoleFmt("%d\n",option);
		switch(option){
			case 0:
				break;
			case 1:
				sprintf(strConsole,"C:\\temp\\dumpRes-%dx%d.bmp",imgData->width,imgData->height);
				saveImage(strConsole, imgData);
				break;
			case 2:
				quantizeColors16(imgData);
				break;
			case 3:
				detectBorder(imgData);
				break;
			case 4:
				DKHFastScanning(imgData);
				break;
			case 5:
				writeConsole("In development...");
				break;
			default:
				writeConsole("Unknown option...");
		}
	}while(option != 0);
	
	GlobalFree(imgData->bitmap);
	if(imgData->codecInfo != (char*)NULL)
		GlobalFree(imgData->codecInfo);
	if(imgData->MIMEtype != (char*)NULL)
		GlobalFree(imgData->MIMEtype);
	HeapFree(GetProcessHeap(),0,imgData);
	logClose(&hLog);
	FreeConsole();
	return 0;
}


