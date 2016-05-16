#include "globals.h"
#include <math.h>

COLOR color16[] = {
	{0x00,0x00,0x00,0x00},
	{0x80,0x80,0x80,0x00},
	{0xC0,0xC0,0xC0,0x00},
	{0xFF,0xFF,0xFF,0x00},
	{0x00,0x00,0x80,0x00},
	{0x00,0x00,0xFF,0x00},
	{0x00,0x80,0x00,0x00},
	{0x00,0xFF,0x00,0x00},
	{0x80,0x00,0x00,0x00},
	{0xFF,0x00,0x00,0x00},
	{0x00,0x80,0x80,0x00},
	{0x00,0xFF,0xFF,0x00},
	{0x80,0x00,0x80,0x00},
	{0xFF,0x00,0xFF,0x00},
	{0x80,0x80,0x00,0x00},
	{0xFF,0xFF,0x00,0x00},
};

void quantizeColors16(LPIMGDATA imgData){
	
	/*
	Loop through pixels checking for 16 bit colorspace proximity.
	*/
	int i, n, size;
	LPCOLOR thisPx = (LPCOLOR)imgData->bitmap;
	i = 0;
	size = imgData->width*imgData->height;
	writeConsole("Color quantization process started...");
	while(i < size){
		int low = 32;
		float dist, lowdist = 260.0f; //Euclidean distance, lowest Euclidean distance.
		for(n = 0; n < 16; n++){
			dist = pow((float)(thisPx->red-color16[n].red), 2.0f) + pow((float)(thisPx->green-color16[n].green), 2.0f) + pow((float)(thisPx->blue-color16[n].blue), 2.0f);
			dist = sqrt(dist);
			if(dist < lowdist){
				low = n;
				lowdist = dist;
			}
		}
		*thisPx = color16[low];
		/*thisPx->alpha = 0;
		thisPx->red = color16[low].red;
		thisPx->green = color16[low].green;
		thisPx->blue = color16[low].blue;*/
		
		thisPx++;
		i++;
	}
	writeConsole("Color quantization done!\n");
}

void sobelOperator(LPIMGDATA imgData, bool isGradient){
	int x, y, i, j, sumX, sumY, res;
	int GX [3][3] = { {-1,0,1}, {-2,0,2}, {-1,0,1} };
	int GY [3][3] = { {1,2,1}, {0,0,0}, {-1,-2,-1} };
	DWORD *oldBuff = imgData->bitmap;
	LPCOLOR thisPx = (LPCOLOR)imgData->bitmap;
	imgData->bitmap = (DWORD*)GlobalAlloc(GPTR, imgData->width*imgData->height*4);
    LPCOLOR dest = (LPCOLOR)imgData->bitmap;
    
	for(y = 0; y < imgData->height; y++){
        for(x = 0; x < imgData->width; x++){
        	if(x == 0 || y == 0 || x >= imgData->width-1 || y >= imgData->height-1 ){
                if(isGradient)
                    res = 128;
                else
                    res = 0;
            }else{
                sumX = sumY = 0;
                for(i = -1; i <= 1; i++){
                    for(j = -1; j <= 1; j++){
                    	LPCOLOR color = thisPx + (j*imgData->width+i);
                        sumX += (((color->red+color->green+color->blue)/3) * GX[i+1][j+1]);
                        sumY += (((color->red+color->green+color->blue)/3) * GY[i+1][j+1]);
                    }
                }
                
                if(isGradient){
                    //res = sumX+sumY;
                    float percent = ((sumX+sumY)/4.0f)/255.0f;
                    if(percent > 100.0f)
                        percent = 100.0f;
                    res = 128+ (int)(percent*128.0f);
                }else
                    res = abs(sumX)+abs(sumY);   
            }
            
            if(res < 0){
                //writeConsoleFmt("[WARNING] Got a negative value: %d.\n",res);
                res = 0;
            }else if(res > 255){
            	//writeConsoleFmt("[WARNING] Got a way too big value: %d.\n",res);
                res = 255;
            }
            dest->alpha = 255;
            dest->red = res;
            dest->green = res;
            dest->blue = res;
            thisPx++;
            dest++;
        }   
    }
    GlobalFree(oldBuff);
}

void detectBorder(LPIMGDATA imgData){
	sobelOperator(imgData,false);
	writeConsole("Border detection done!\n");
}

void fastScanning(LPIMGDATA imgData){
	
}

