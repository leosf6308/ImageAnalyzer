#include "globals.h"
#include <math.h>

typedef struct _clusteredImage{
	int width;
	int height;
	unsigned int* pixClus;
}CLUSIMG, *LPCLUSIMG;

class Cluster{
	private:
		int id;
		Cluster* next;
		int pixelCount;
		int top;
		int left;
		int right;
		int bottom;
		COLOR avgTone;
		LPCLUSIMG clustImage;
	public:
		Cluster(int ID, int Top, int Left, COLOR Tone, LPCLUSIMG Image);
		void mergePoint(int x, int y, COLOR color);
		void mergeCluster(Cluster* c2, Cluster* head);
		void setNext(Cluster* Next){ next = Next; }
		Cluster* getNext(){ return next; }
		int getID(){ return (int)id; }
		COLOR getTone() { return avgTone; }		
};

Cluster::Cluster(int ID, int Left, int Top, COLOR Tone, LPCLUSIMG Image){
	this->id = ID;
	this->top = Top;
	this->left = Left;
	this->right = Left+1;
	this->bottom = Top+1;
	this->avgTone = Tone;
	this->clustImage = Image;
	this->pixelCount = 1;
	Image->pixClus[Top*Image->width+Left] = ID;
	next = (Cluster*)NULL;
}

void Cluster::mergePoint(int x, int y, COLOR color){
	int newR = (avgTone.red*pixelCount + color.red)/(pixelCount+1);
	int newG = (avgTone.green*pixelCount + color.green)/(pixelCount+1);
	int newB = (avgTone.blue*pixelCount + color.blue)/(pixelCount+1);
	avgTone.red = (unsigned char)newR&0x00FF;
	avgTone.green = (unsigned char)newG&0x00FF;
	avgTone.blue = (unsigned char)newB&0x00FF;
	if(x < left)
		left = x;
	if(x > right)
		right = x+1;
	if(y < top)
		top = y;
	if(y > bottom)
		bottom = y+1;
	clustImage->pixClus[y*clustImage->width+x] = id;
	pixelCount++;
}

void Cluster::mergeCluster(Cluster* c2, Cluster* head){
	int idC2, i;
	idC2 = c2->getID();
	if(id == c2->getID())
		return;
	
	pixelCount += c2->pixelCount;
	
	for(i = 0; i < clustImage->width*clustImage->height; i++)
		if(clustImage->pixClus[i] == idC2)
			clustImage->pixClus[i] = id;
	
	while(head != (Cluster*)NULL && head->next != c2)
    	head = head->next;
    
    head->next = c2->next;
    writeConsoleFmt("Merged cluster %d with %d.\nBefore: %d...\n",id,idC2,head->id);
}

Cluster* getPixelCluster(Cluster* clus, LPCLUSIMG clusImg,  int x, int y){
	if(x < 0 || x >= clusImg->width || y < 0 || y >= clusImg->height || clus == (Cluster*)NULL){
		writeConsoleFmt("Failed to find (%d,%d) pixel's cluster.\n",x,y);
		*((char*)NULL) = '\0';
		return (Cluster*)NULL;
	}
    int id = clusImg->pixClus[y*clusImg->width+x];
    while(clus != (Cluster*)NULL && clus->getID() != id)
    	clus = clus->getNext();
    if(clus == (Cluster*)NULL){
    	writeConsoleFmt("Failed to find (%d,%d) pixel's cluster. ID: %d.\n",x,y,id);
    	*((char*)NULL) = '\0';
	}
    return clus;
}

inline float getColorDistance(COLOR c1, COLOR c2){
	int dr = c1.red-c2.red;
	int dg = c1.green-c2.green;
	int db = c1.blue-c2.blue;
	float dist = dr*dr + dg*dg + db*db;
	return sqrt(dist);
}

COLOR getClusterColorByID(Cluster* head, int id){
	while(head != (Cluster*)NULL && head->getID() != id)
    	head = head->getNext();
    return head->getTone();
}

void convertClustersBitmap(Cluster* head, LPCLUSIMG clusImg, LPIMGDATA imgData){
	int i;
	unsigned int* inPx = clusImg->pixClus;
	LPCOLOR outPx = (LPCOLOR)imgData->bitmap;
	for(i = 0; i < clusImg->width*clusImg->height; i++)
		*outPx++ = getClusterColorByID(head, *inPx++);
}

void DKHFastScanning(LPIMGDATA imgData){
	int i, j, lastID, clusterCount;
	CLUSIMG clusImg;
	float threshold, dist;
	Cluster *clusterList, *Ci, *lastClus;
	
	lastID = 1;
	clusterCount = 1;
	threshold = 45.0f;
	clusImg.width = imgData->width;
	clusImg.height = imgData->height;
	clusImg.pixClus = (unsigned int*)HeapAlloc(GetProcessHeap(),0,clusImg.width*clusImg.height*sizeof(unsigned int));
	
	LPCOLOR thisPx = (LPCOLOR)imgData->bitmap;
	Ci = new Cluster(lastID++,0,0,*thisPx,&clusImg);
	lastClus = clusterList = Ci;
	thisPx++;
	for(j = 1; j < clusImg.width; j++){
		Ci = getPixelCluster(clusterList,&clusImg,j-1,0);
		dist = getColorDistance(Ci->getTone(),*thisPx);
		if(dist < threshold)
			Ci->mergePoint(j,0,*thisPx);
		else{
			Ci = new Cluster(lastID++,j,0,*thisPx,&clusImg);
			lastClus->setNext(Ci);
			lastClus = Ci;
			clusterCount++;
		}
		thisPx++;
	}
	
	for(i = 1; i < clusImg.height; i++){
		Ci = getPixelCluster(clusterList,&clusImg,0,i-1);
		
		dist = getColorDistance(Ci->getTone(),*thisPx);
		if(dist < threshold)
			Ci->mergePoint(0,i,*thisPx);
		else{
			Ci = new Cluster(lastID++,0,i,*thisPx,&clusImg);
			lastClus->setNext(Ci);
			lastClus = Ci;
			clusterCount++;
		}
		thisPx++;
		
		for(j = 1; j < clusImg.width; j++){
			Cluster* Cu = getPixelCluster(clusterList,&clusImg,j,i-1);
			Cluster* Cl = getPixelCluster(clusterList,&clusImg,j-1,i);
			dist = getColorDistance(Cu->getTone(),*thisPx);
			if(dist < threshold){
				Cu->mergePoint(j,i,*thisPx);
				dist = getColorDistance(Cl->getTone(),*thisPx);
				if(dist < threshold){
					if(Cu != Cl){
						clusterCount--;
						if(Cl != clusterList)
							Cu->mergeCluster(Cl,clusterList);
						else
							Cl->mergeCluster(Cu,clusterList);
						lastClus = clusterList;
						
						while(lastClus != (Cluster*)NULL && lastClus->getNext() != (Cluster*)NULL)
					    	lastClus = lastClus->getNext();
				    }
				}
			}else{
				dist = getColorDistance(Cl->getTone(),*thisPx);
				if(dist < threshold)
					Cl->mergePoint(j,i,*thisPx);
				else{
					Ci = new Cluster(lastID++,j,i,*thisPx,&clusImg);
					lastClus->setNext(Ci);
					lastClus = Ci;
					clusterCount++;
				}
			}
			thisPx++;
		}
		writeConsoleFmt("Line %d done. Cluster count %d.\n",i,clusterCount);
	}
	
	convertClustersBitmap(clusterList, &clusImg, imgData);
	
}


