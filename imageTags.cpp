#include "globals.h"
#include "bfnFile.h"
/*
	Instantiate ImageClass object
	Print descriptors into the image (may be inserted into for loops...)
	
	DestroyClass.
*/

class ImageTags{
	private:
		LPIMGDATA imgData;
		BFNFILE bfnFile;
	public:
		ImageTags(LPIMGDATA image, char* fontPath);
		void drawLine(int x1, int y1, int x2, int y2, COLOR color);
		void drawText(int top, int left, COLOR color, char* strText);
		~ImageTags();
};

ImageTags::ImageTags(LPIMGDATA image, char* fontPath){
	int result;
	if(image->bitmap == NULL || image->width == 0 || image->height == 0)
		return;
	imgData = image;
	result = openFon(&bfnFile,fontPath);
	if(result != 0){
		writeConsole("Can't load font file.");
		memset(&bfnFile,0,sizeof(BFNFILE));
	}
}

void ImageTags::drawLine(int x1, int y1, int x2, int y2, COLOR color){
	if(x1 == x2){
		if(y1 > y2){
			int tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
		}
		if(y2 > imgData->height)
			y2 = imgData->height;
		COLOR* thisColor = ((COLOR*)imgData->bitmap)+(y1*imgData->width+x1);
		while(y1 <= y2){
			*thisColor = color;
			thisColor += imgData->width;
			y1++;
		}
	}else if(y1 == y2){
		if(x1 > x2){
			int tmp;
			tmp = x1;
			x1 = x2;
			x2 = tmp;
		}
		if(x2 > imgData->width)
			x2 = imgData->width;
		COLOR* thisColor = ((COLOR*)imgData->bitmap)+(y1*imgData->width+x1);
		while(x1 <= x2){
			*thisColor = color;
			thisColor++;
			x1++;
		}
	}else{
		int offset;
		COLOR* screenBuff = (COLOR*)imgData->bitmap;
		//Using Bresenham Line Algorithm
		int x, y, xEnd;
		int dx = x2-x1; //deltaX
		int dy = y2-y1; //deltaY
		int d = 2*dy-dx;
		int inc1 = 2*dy;
		int inc2 = 2*(dy-dx);
		if(x1 > x2){
			x = x2;
			y = y2;
			xEnd = x1;
		}else{
			x = x1;
			y = y1;
			xEnd = x2;
		}
		
		while(x < xEnd){
			x++;
			if(d < 0)
				d = d+inc1;
			else{
				y++;
				d = d+inc2;
			}
			offset = y*imgData->width+x;
			*(screenBuff+offset) = color;
		}
	}
}

void ImageTags::drawText(int top, int left, COLOR color, char* strText){
	if(bfnFile.fileSize == 0)
		return;
}

ImageTags::~ImageTags(){
	imgData = (LPIMGDATA)NULL;
	closeFon(&bfnFile);
}
