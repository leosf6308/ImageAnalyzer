#include "globals.h"
#include <math.h>

#define GEOM_SQUARE 1
#define GEOM_RECTANGLE 2
#define GEOM_CIRCLE 3
#define GEOM_TRIANGLE 4

#define MINDOUBLE 0.0000000001

typedef struct _allocationData{
	HANDLE hHeap;
	void* allocPtr;
	DWORD allocSize;
}ALLOCDATA, *LPALLOCDATA;

typedef struct _clusteredImage{
	int width;
	int height;
	void** pixClus; //Why void** ?? Because these guys point to a list of Cluster* object.
					//But Cluster also references this structure... So... You got it.
}CLUSIMG, *LPCLUSIMG;

class Cluster{
	private:
		Cluster* before;
		Cluster* next;
		int pixelCount;
		int top;
		int left;
		int right;
		int bottom;
		COLOR avgTone;
		LPCLUSIMG clustImage;
	public:
		Cluster(int Top, int Left, COLOR Tone, LPCLUSIMG Image);
		void mergePoint(int x, int y, COLOR color);
		void mergeCluster(Cluster* c2, Cluster* head);
		void mergeClosest(Cluster*);
		void setBefore(Cluster* Before){ before = Before; }
		void setNext(Cluster* Next){ next = Next; }
		Cluster* getNext(){ return next; }
		Cluster* getBefore(){ return before; }
		COLOR getTone() { return avgTone; }	
		int getCount() {return pixelCount; }
		int getTop() { return top; }
		int getBottom() { return bottom; }
		int getLeft() { return left; }
		int getRight() { return right; }
		void testClusImg(LPCLUSIMG clusImg){
			if(clusImg != clustImage)
				writeConsoleFmt("Wrong clustered image at cluster %08x",this);
		}
		int detectShape();
};

union ColorValue{
	COLOR color;
	DWORD value;
};

typedef struct meanValues{
	int sum, count, mean;
}MEANVALS;

typedef struct _point2d{
	int x;
	int y;
}POINT2D, *PPOINT2D;;

typedef struct ClusterList{
	Cluster* cluster;
	struct ClusterList* next;
	int left;
	int top;
	int width;
	int height;
	int count;
}CLUSLIST, *PCLUSLIST;

typedef struct _pixelCount{
	Cluster* cluster;
	int count;
	int distance;
}PIXCNT, *LPPIXCNT;

/*
C    = n!/(p!*(n-p)!)
 n,p

C         5!         5!
 5,3   -------- = -------- = 5!/3!*2! = (5*4)/2!
       3!(5-3)!    3! 2!
	   
C     = (5*4*3*2*1)/(3*2*1)*(2*1)
 5,3
 
C    = 3!/(4!*(n-p)!)
 3,4

C      = 20!/(15!*(20-15)!) = (20*19*18*17*16)/(5*4*3*2*1) = 1860480/120 = 15504
 20,15
Se n == p #DIV0
	Ret nan("");
Se n < p 
	Ret -INFINITY
Senão
	r1 = n! (conceitualmente)
	r2 = p! (conceitualmente)
	r3 = (n-p)! (conceitualmente)
	Se r1 > r2
		Cortar. Mas como?
		r1 = MultiplicaSequencial(r2+1 até r1)
		r2 = 0
		C = r1/r3!
	Se r1 > r3

*/

double sequentialMultiply(int start, int end) {
	double result;
	if(start == end)
		return (double)start;
	if(start > end)
		return (double)nan("");
	result = (double)start++;
	while(start <= end){
		result *= start;
		start++;
	}
	return result;
}

double calcCombinations(int n, int k){
	if(n == k)
		return (double)nan("");
	else if(n < k)
		return (double)INFINITY;
	else{
		double result;
		result = sequentialMultiply(k+1, n);
		result /= sequentialMultiply(1,n-k);
		return result;
	}
}

float getColorDistance(COLOR c1, COLOR c2){
	int dr = c1.red-c2.red;
	int dg = c1.green-c2.green;
	int db = c1.blue-c2.blue;
	float dist = dr*dr + dg*dg + db*db;
	return sqrtf(dist);
}

void dbgShow(Cluster *head, LPCLUSIMG clusImg){
	int i, j, pixCnt;
	Cluster *thisClus, *lastClus, **thisPx;
	writeConsoleFmt("\n\nhead: %08x. clusImg: %08x\n",head,clusImg);
	lastClus = thisClus = head;
	pixCnt = clusImg->width*clusImg->height;
	while(thisClus != (Cluster*)NULL){
		writeConsoleFmt("%08x->",thisClus);
		thisClus->testClusImg(clusImg);
		if(thisClus == thisClus->getNext()){
			writeConsole("SELF REFERENCE!\n");
			waitKeyPress(true);
			exit(-1);
			return;
		}
				
		if(thisClus->getCount() == 0 || thisClus->getCount() > pixCnt){
    		writeConsoleFmt("DAMMIT! %08x: CORRUPTED CLUSTER! PIXCNT: %d\n",thisClus,thisClus->getCount());
			waitKeyPress(true);
			exit(-1);
			return;
		}
		
		thisClus = thisClus->getNext();
		if(thisClus != (Cluster*)NULL && thisClus->getBefore() != lastClus){
			writeConsole("LIST IS NOT RIGTH LINKED!\n");
			waitKeyPress(true);
			exit(-1);
			return;
		}
		lastClus = thisClus;
	}
    writeConsole("NULL\n\n");
    
    thisPx = (Cluster**)clusImg->pixClus;
    for(i = 0; i < clusImg->height; i++){
    	for(j = 0; j < clusImg->width; j++){
	    	if(*thisPx != NULL && ((*thisPx)->getCount() == 0 || (*thisPx)->getCount() > pixCnt)){
	    		writeConsoleFmt("DAMMIT! ORPHAN PIXEL %dx%d! CLUS->PIXCNT %d\n",j+1,i+1,(*thisPx)->getCount());
				waitKeyPress(true);
				exit(-1);
				return;
			}
		}
	}
}

Cluster::Cluster(int Left, int Top, COLOR Tone, LPCLUSIMG Image){
	this->top = Top;
	this->left = Left;
	this->right = Left+1;
	this->bottom = Top+1;
	this->avgTone = Tone;
	this->clustImage = Image;
	this->pixelCount = 1;
	Image->pixClus[Top*Image->width+Left] = (void*)this;
	next = (Cluster*)NULL;
	before = (Cluster*)NULL;
}

void Cluster::mergePoint(int x, int y, COLOR color){
	float newR = (avgTone.red*pixelCount + color.red);
	float newG = (avgTone.green*pixelCount + color.green);
	float newB = (avgTone.blue*pixelCount + color.blue);
	pixelCount++;
	newR /= pixelCount;
	newG /= pixelCount;
	newB /= pixelCount;
	avgTone.red = (unsigned char)round(newR);
	avgTone.green = (unsigned char)round(newG);
	avgTone.blue = (unsigned char)round(newB);
	
	if(x < left)
		left = x;
	if(x >= right)
		right = x+1;
	if(y < top)
		top = y;
	if(y >= bottom)
		bottom = y+1;
	clustImage->pixClus[y*clustImage->width+x] = (void*)this;
}

void Cluster::mergeCluster(Cluster* c2, Cluster* head){
	int i;
	Cluster *clusBefore, **thisPx;
	if(c2 == this){
		writeConsole("SELF-MERGE????\n");
		waitKeyPress(true);
		exit(-1);
		return;
	}
	
	float newR = (avgTone.red*pixelCount + c2->avgTone.red*c2->pixelCount);
	float newG = (avgTone.green*pixelCount + c2->avgTone.green*c2->pixelCount);
	float newB = (avgTone.blue*pixelCount + c2->avgTone.blue*c2->pixelCount);
	
	pixelCount += c2->pixelCount;
	newR /= pixelCount;
	newG /= pixelCount;
	newB /= pixelCount;
	avgTone.red = (unsigned char)round(newR);
	avgTone.green = (unsigned char)round(newG);
	avgTone.blue = (unsigned char)round(newB);
		
	if(left > c2->left)
        left = c2->left;
    if(top > c2->top)
        top = c2->top;
    if(right < c2->right)
        right = c2->right;
    if(bottom < c2->bottom)
        bottom = c2->bottom;
	
	thisPx = (Cluster**)clustImage->pixClus;
	for(i = 0; i < clustImage->width*clustImage->height; i++){
		if(*thisPx == c2)
			*thisPx = this;
		thisPx++;
	}
	
    c2->avgTone = (COLOR){0,0,0};
    c2->pixelCount = 0;
    c2->left = 0;
    c2->top = 0;
    c2->right = 0;
    c2->bottom = 0;
    
    clusBefore = c2->before;
    clusBefore->next = c2->next;
    if(clusBefore->next != (Cluster*)NULL)
    	clusBefore->next->before = clusBefore;
    //writeConsoleFmt("List: %08x->%08x. Delete: %08x. Merged with %08x.\n",clusBefore,c2->next,c2,this);
    delete c2;
}

//void Cluster::mergeClosest(Cluster* head, HANDLE hHeap){
void Cluster::mergeClosest(Cluster* head){
	int i, j, n, sizex, sizey, step, currCnt, maxCnt, maxWeight, maxDist;
	Cluster **thisPx, *Ci;
	LPPIXCNT pixCnt;
	thisPx = (Cluster**)clustImage->pixClus;
	thisPx += (top*clustImage->width+left);
	sizex = right-left;
	sizey = bottom-top;
	step = clustImage->width-sizex;
	maxCnt = 128;
	maxDist = 0;
	currCnt = 0;
	
	//pixCnt = (LPPIXCNT)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,maxCnt*sizeof(PIXCNT));
	pixCnt = (LPPIXCNT)malloc(maxCnt*sizeof(PIXCNT));
	i = top;
	while(i < bottom){
		j = left;
		while(j < right){
			if(*thisPx == this){
				Cluster* neighbour;
				for(int c = 0; c < 4; c++){
					switch(c){
						case 0: //Top
							if(i == 0)
								continue;
							neighbour = *(thisPx-clustImage->width);
							break; 
						case 1: //Left
							if(j == 0)
								continue;
							neighbour = *(thisPx-1);
							break;
						case 2: //Botton							
							if(i+1 == clustImage->height)
								continue;
							neighbour = *(thisPx+clustImage->width);
							break;
						case 3: //Right
							if(j+1 == clustImage->width)
								continue;
							neighbour = *(thisPx+1);
							break;
					}
					bool found = false;
					if(neighbour == this)
						continue;
					
					for(n = 0; n < currCnt; n++)
						if(pixCnt[n].cluster == neighbour){
							pixCnt[n].count++;
							found = true;
							break;
					}
					
					if(!found){
						if(currCnt+1 >= maxCnt){
							maxCnt += 128;
							//pixCnt = (LPPIXCNT)HeapReAlloc(hHeap,HEAP_ZERO_MEMORY,pixCnt,maxCnt*sizeof(PIXCNT));
							pixCnt = (LPPIXCNT)realloc(pixCnt,maxCnt*sizeof(PIXCNT));
							//writeConsoleFmt("List reallocation. %d current items. %d max items.",currCnt,maxCnt);
						}
						pixCnt[currCnt].cluster = neighbour;
						pixCnt[currCnt].count = 1;
						pixCnt[currCnt].distance = (int)trunc(getColorDistance(avgTone,neighbour->avgTone));
						pixCnt[currCnt].distance = abs(pixCnt[currCnt].distance);
						if(pixCnt[currCnt].distance > maxDist)
							maxDist = pixCnt[currCnt].distance;
						currCnt++;
					}
					
				}
			}
			thisPx++;
			j++;
		}
		thisPx += step;
		i++;
	}
	
	i = 0xFFFFFFFF;
	maxWeight = 0;
		
	for(n = 0; n < currCnt; n++){
		int thisWeight = pixCnt[n].count*(maxDist-pixCnt[n].distance);
		if(thisWeight > maxWeight || pixCnt[n].distance-maxDist < 1.01){
			i = n;
			maxWeight = thisWeight;
		}
		//writeConsoleFmt("%08x: %d, %d.\n",pixCnt[n].cluster,pixCnt[n].count,pixCnt[n].distance);
	}
	if(i != 0xFFFFFFFF)
		Ci = pixCnt[i].cluster;
	else{
		Ci = (Cluster*)NULL;
		writeConsoleFmt("FAILED TO FIND CLOSEST CLUSTER FOR %08X.\n",this);
	}
	writeConsoleFmt("Merge %08x (%d,%d,%d) with %08x (%d,%d,%d)\n",Ci,Ci->avgTone.red,Ci->avgTone.green,Ci->avgTone.blue,this,avgTone.red,avgTone.green,avgTone.blue);
	//HeapFree(hHeap,0,pixCnt);
	free(pixCnt);
	
	if(Ci != (Cluster*)NULL)
		Ci->mergeCluster(this,head);
}

int Cluster::detectShape(){
	//Returns probable geometric shape (square, triangle, circle) of the object.
	//http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/MORSE/region-props-and-moments.pdf Section 9.8.2
	int i, j, S, lnStep, width, height, ptIdx, perimeter;
	double ux, uy, u20, u02, u11, u30, u03, u12, u21, C, metric, cicularity;
	double n20, n02, n11, n30, n03, n12, n21, I2, I3, Spwr;
	Cluster** thisPx = ((Cluster**)clustImage->pixClus)+(top*clustImage->width+left);
	POINT2D center;
	
	width = right-left;
	height = bottom-top;
	lnStep = clustImage->width-width;
	uy = ux = 0.0;
	perimeter = 0;
	j = 0;
	S = 0;
	//j = y; x = i; S = cluster pixel count; f(x,y) returns 1 for pixels who belong to the cluster.
	while(j < height){
		i = 0;
		while(i < width){
			if(*thisPx == this){
				ux += i;
				uy += j;
				S++;
			}
			thisPx++;
			i++;
		}
		thisPx += lnStep;
		j++;
	}
	ux /= S;
	uy /= S;
	center.x = (int)ux;
	center.y = (int)uy;
	thisPx = ((Cluster**)clustImage->pixClus)+(top*clustImage->width+left);
	u20 = u02 = u11 = u30 = u03 = u12 = u21 = 0.0;
	j = 0;
	while(j < height){
		i = 0;
		while(i < width){
			if(*thisPx == this){
				int xvar, yvar, outerNeighbouhr;
				Cluster** neighbouhr;
				xvar = i - center.x;
				yvar = j - center.y;
				u20 += (xvar*xvar);
				u02 += (yvar*yvar);
				u11 += (xvar*yvar);
				u30 += (xvar*xvar*xvar);
				u03 += (yvar*yvar*yvar);
				u12 += (xvar*yvar*yvar);
				u21 += (xvar*xvar*yvar);
				
				outerNeighbouhr = 0;
				if(j != 0){
					neighbouhr = thisPx-(lnStep+1);
					if(i == 0 || *neighbouhr++ != this) //Test top left
						outerNeighbouhr++;
					if(*neighbouhr++ != this) //Test top middle
						outerNeighbouhr++;
					if(i == width-1 || *neighbouhr != this) //Test top right
						outerNeighbouhr++;
				}else
					outerNeighbouhr += 3;
				
				neighbouhr = thisPx-1;
				if(i == 0 || *neighbouhr++ != this) //Test left
					outerNeighbouhr++;
				if(i == width-1 || *++neighbouhr != this) //Test right
					outerNeighbouhr++;
				
				
				if(j < height-1){
					neighbouhr = thisPx+(lnStep-1);
					if(i == 0 || *neighbouhr++ != this) //Test bottom left
						outerNeighbouhr++;
					if(*neighbouhr++ != this) //Test bottom middle
						outerNeighbouhr++;
					if(i == width-1 || *neighbouhr != this) //Test bottom right
						outerNeighbouhr++;
				}else
					outerNeighbouhr += 3;
				
				if(outerNeighbouhr > 2)
					perimeter++;
			}
			thisPx++;
			i++;
		}
		thisPx += lnStep;
		j++;
	}
	u20 /= S;
	u02 /= S;
	u11 /= S;
	u03 /= S;
	u30 /= S;
	u12 /= S;
	u21 /= S;
	C = 0.5*atan2(2*u11,u20-u02);
	I2 = (u20/(S*S))-(u02/(S*S));
	I2 *= I2;
	n11 = u11/(S*S);
	I2 += 4.0*n11*n11;
	Spwr = pow(S,2.5);
	I3 = (u30/Spwr)-3*(u12*Spwr);
	I3 *= I3;
	I3 += (3*(u21/Spwr)-(u21/Spwr))*(3*(u21/Spwr)-(u21/Spwr));
	metric = (4*M_PI*S)/(perimeter*perimeter);
	cicularity = (perimeter*perimeter)/(4*M_PI*S);
	//sequentialMultiply(1,5)
	//writeConsoleFmt("%08x (%3d,%3d,%3d) %9d %dx%d,%dx%d",lastClus,lastClus->getTone().red,lastClus->getTone().green,lastClus->getTone().blue,
	//lastClus->getCount(),lastClus->getLeft(),lastClus->getTop(),lastClus->getRight()-lastClus->getLeft(),lastClus->getBottom()-lastClus->getTop());
	//ID	Color	Count	Tx	Ty	Sx	Sy	ux	uy	u20	u02	u11	Angle	u30	u03	u12	u21	I2	I3	Metric	Circularity	Perimeter
	writeConsoleFmt("%08x\t(%03d|%03d|%03d)\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.4e\t%.4e\t%.4e\t%.3f\t%.4e\t%.4e\t%.4e\t%.4e\t%.4e\t%.4e\t%.2f\t%.2f\t%d",
	this,avgTone.red,avgTone.green,avgTone.blue,pixelCount,left,top,right-left,bottom-top,center.x,center.y,u20,u02,u11,(C*180.0)/M_PI,u30,u03,u12,u21,I2,I3,metric,cicularity,perimeter);
	//writeConsoleFmt("S: %d\tux:%.4f\tuy:%.4f\tu20:%.4f\tu02:%.4f\tu11:%.4f\tC:%.4f;",S,ux,uy,u20,u02,u11,C);
	
	/*
	uij = sum x ( sum y ((x-x')^p*(y-y')^q*f(x,y)) )
		
	nij = (uij)/(u00^(1+(i+j)/2)
		
	https://en.wikipedia.org/wiki/Image_moment#Rotation_invariants
	I2 = (n20-n02)^2 + 4*n11^2
		((u20/u00^2)-(u02/u00^2))^2 + 4*(u11/u00^2)^2
		
	I3 = (n30-3*n12)^2+(3*n21-n03)^2
		((u30/u00^2.5)-3*((u12/u00^2.5)))^2+(3*(u21/u00^2.5)-(u03/u00^2.5))^2
		xvar = x-x'
		yvar = y-y'
		u30 = sum x ( sum y (xvar^3*f(x,y)))
		u12 = sum x ( sum y (xvar*yvar^2*f(x,y)))
		u21 = sum x ( sum y (xvar^2*yvar*f(x,y)))
		u03 = sum x ( sum y (yvar^3*f(x,y)))
	*/
	
	/*
	//Old algorithm.
	int i, j, step, size, width, height, p;
	double u20, u02, u11, vx, vy, angle, num, den;
	Cluster** thisPx = ((Cluster**)clustImage->pixClus)+(top*clustImage->width+left);
	POINT2D *point, center;
	width = right-left;
	height = bottom-top;
	step = clustImage->width-width;
	point = (POINT2D*)malloc(sizeof(POINT2D)*pixelCount);
	p = 0;
	center.x = 0;
	center.y = 0;
	i = 0;
	while(i < height){
		j = 0;
		while(j < width){
			if(*thisPx == this){
				point[p].x = j;
				point[p].y = i;
				p++;
				center.x += j;
				center.y += i;
			}
			thisPx++;
			j++;
		}
		thisPx += step;
		i++;
	}
	center.x /= p;
	center.y /= p;
	
	//u20: x variance; u02: y variance; u11: xy covariance;
	u20 = u02 = u11 = 0.0;
	
	//f(x,y) = 1, because every pixel has the same weight
	//Also copied from MATLAB regionprops->ComputeEllipseParams
	for(p = 0; p < pixelCount; p++){
		vx = point[p].x-center.x;
		vy = -(point[p].y-center.y);
		u20 += pow(vx,2);
		u02 += pow(vy,2);
		u11 += (vx*vy);
	}
	
	u20 /= p;
	u02 /= p;
	u11 /= p;
	
	if(u02 > u20){
		num = u02 - u20 + sqrt(pow(u02-u20,2)+4.0*pow(u11,2));
		den = 2.0*u11;
	}else{
		num = 2.0*u11;
		den = u02 - u20 + sqrt(pow(u02-u20,2)+4.0*pow(u11,2));
	}
	
	//num = 2*u11;
	//den = u20-u02;
	if(abs(num)<MINDOUBLE ||abs(num)<MINDOUBLE)
		angle = 0.0;
	else
		angle = (180.0/M_PI)*atan(num/den);
	
	writeConsoleFmt("Mean point: %dx%d. Angle: %.3f\n",center.x+left,center.y+top,angle);
	free(point);*/
}

Cluster* getPixelCluster(Cluster* clus, LPCLUSIMG clusImg,  int x, int y){
	if(x < 0 || x >= clusImg->width || y < 0 || y >= clusImg->height || clus == (Cluster*)NULL){
		writeConsoleFmt("Invalid parameters to find (%d,%d) pixel's cluster.\n",x,y);
		return (Cluster*)NULL;
	}
    Cluster* Ci = (Cluster*)clusImg->pixClus[y*clusImg->width+x];
    return Ci;
}

void convertClustersBitmap(Cluster* head, LPCLUSIMG clusImg, LPIMGDATA imgData){
	int i;
	void** inPx = clusImg->pixClus;
	LPCOLOR outPx = (LPCOLOR)imgData->bitmap;
	for(i = 0; i < clusImg->width*clusImg->height; i++){
		Cluster* Ci = (Cluster*)(*inPx);
		*outPx++ = Ci->getTone();
		inPx++;
	}
}

PCLUSLIST bubbleSortList(PCLUSLIST start, PCLUSLIST end){
	bool swap = true;
	PCLUSLIST thisItem, lastItem, nextItem;
	while(swap){
		thisItem = start;
		lastItem = NULL;
		swap = false;
		while(thisItem->next != NULL){
			nextItem = thisItem->next;
			if(thisItem->left > nextItem->left){
				if(lastItem == NULL)
					start = nextItem;
				else
					lastItem->next = nextItem;
				thisItem->next = nextItem->next;
				nextItem->next = thisItem;
				swap = true;
			}
			lastItem = thisItem;
			thisItem = thisItem->next;
		}
	}
	return start;
}

/* Seek highest color.
 * Create list of clusters whose color is greater than or equal.
 * Order them by X position order, then by Y order.
 * Save average X and Z position.
 * Detect distance between them.
 *	If the distance varies on both X and Y directions, the cluster might not belong to thrust.
 * 
 * 
 */
void seekDiamonds(LPCLUSIMG clusImg, Cluster* head){
	Cluster* thisClus = head;
	ColorValue maxColor, thisColor;
	PCLUSLIST clusList, thisItem, lastItem;
	int count = 0;
	float minDist, thisDist;
	minDist = 500.0f;
	while(thisClus != NULL){
		thisDist = getColorDistance(COLOR_WHITE,thisClus->getTone());
		if(thisDist < minDist){
			count = 1;
			minDist = thisDist;
			maxColor.color = thisClus->getTone();
		}
		thisClus = thisClus->getNext();
	}
	//create CLUSLIST items.
	writeConsoleFmt("Highest color: %3d,%3d,%3d\n",maxColor.color.red,maxColor.color.green,maxColor.color.blue);
	
	int xmin, xmax, ymin, ymax;
	clusList = lastItem = (PCLUSLIST)NULL;
	thisClus = head;
	while(thisClus != NULL){
		thisColor.color = thisClus->getTone();
		if(thisColor.value == maxColor.value){
			thisItem = (PCLUSLIST)malloc(sizeof(CLUSLIST));
			thisItem->cluster = thisClus;
			thisItem->next = NULL;
			thisItem->left = thisClus->getLeft();
			thisItem->top = thisClus->getTop();
			thisItem->width = thisClus->getRight()-thisClus->getLeft();
			thisItem->height = thisClus->getBottom()-thisClus->getTop();
			thisItem->count = thisClus->getCount();
			
			if(clusList == NULL){
				clusList = thisItem;
				xmin = thisItem->left;
				xmax = thisItem->left;
				ymin = thisItem->top;
				ymax = thisItem->top;
			}else{
				lastItem->next = thisItem;
				if(thisItem->left < xmin)
					xmin = thisItem->left;
				if(thisItem->left > xmax)
					xmax = thisItem->left;
				if(thisItem->top < ymin)
					ymin = thisItem->top;
				if(thisItem->top > ymax)
					ymax = thisItem->top;
			}
			lastItem = thisItem;
		}
		thisClus = thisClus->getNext();
	}
		
	lastItem = clusList;
	while(lastItem != NULL){
		writeConsoleFmt("Cluster: %08x. Left: %d\n",lastItem->cluster,lastItem->left);
		lastItem = lastItem->next;
	}
	writeConsoleFmt("Variation: x:%d(%d-%d); y:%d(%d-%d);\n",xmax-xmin,xmax,xmin,ymax-ymin,ymax,ymin);
	
	clusList = lastItem = bubbleSortList(clusList,lastItem);
	while(lastItem != NULL){
		writeConsoleFmt("Cluster: %08x. Left: %d. DistNxt: %d\n",lastItem->cluster,lastItem->left,
		(lastItem->next!=NULL?lastItem->next->left-(lastItem->left+lastItem->width):0));
		clusList = lastItem;
		lastItem = lastItem->next;
		free(clusList);
	}
}

void DKHFastScanning(LPIMGDATA imgData){
	int i, j, clusterCount, maxSize;
	CLUSIMG clusImg;
	//ALLOCDATA delLists;
	float threshold, dist;
	Cluster *clusterList, *Ci, *lastClus;
	char strThreshold[16];
	
	clusterCount = 1;
	threshold = 45.0f;
	clusImg.width = imgData->width;
	clusImg.height = imgData->height;
	clusImg.pixClus = (void**)HeapAlloc(GetProcessHeap(),0,clusImg.width*clusImg.height*sizeof(void*));
	writeConsole("\nThreshold distance: ");
	readConsoleString(strThreshold);
	threshold = atof(strThreshold);
	if(threshold < 1.0f)
		threshold = 50.0f;
	writeConsoleFmt("We'll be using threshold %.2f\n",threshold);
	
	LPCOLOR thisPx = (LPCOLOR)imgData->bitmap;
	Ci = new Cluster(0,0,*thisPx,&clusImg);
	lastClus = clusterList = Ci;
	thisPx++;
	for(j = 1; j < clusImg.width; j++){
		Ci = getPixelCluster(clusterList,&clusImg,j-1,0);
		dist = getColorDistance(Ci->getTone(),*thisPx);
		if(dist < threshold)
			Ci->mergePoint(j,0,*thisPx);
		else{
			Ci = new Cluster(j,0,*thisPx,&clusImg);
			lastClus->setNext(Ci);
			Ci->setBefore(lastClus);
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
			Ci = new Cluster(0,i,*thisPx,&clusImg);
			lastClus->setNext(Ci);
			Ci->setBefore(lastClus);
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
						if(Cl != clusterList){
							if(Cl == lastClus)
								lastClus = lastClus->getBefore();
							Cu->mergeCluster(Cl,clusterList);
						}else{
							if(Cu == lastClus)
								lastClus = lastClus->getBefore();
							Cl->mergeCluster(Cu,clusterList);
						}
				    }
				}
			}else{
				dist = getColorDistance(Cl->getTone(),*thisPx);
				if(dist < threshold)
					Cl->mergePoint(j,i,*thisPx);
				else{
					Ci = new Cluster(j,i,*thisPx,&clusImg);
					lastClus->setNext(Ci);
					Ci->setBefore(lastClus);
					lastClus = Ci;
					clusterCount++;
				}
			}
			thisPx++;
		}
		writeConsoleFmt("Line %d done. Cluster count %d.\n",i,clusterCount);
		//dbgShow(clusterList,&clusImg);
	}
	
	/*maxSize = clusImg.width+clusImg.height*sizeof(PIXCNT);
	delLists.hHeap = HeapCreate(0,128*sizeof(PIXCNT), maxSize*sizeof(PIXCNT));
	delLists.allocPtr = NULL;
	delLists.allocSize = 0;*/
	
	j = 0;
	//Remove small clusters (less than 1%)
	writeConsoleFmt("Wich it's the minimum cluster size(size threshold in percent)?");
	readConsoleString(strThreshold);
	threshold = atof(strThreshold)/100.0f;
	if(threshold > 0.1f){
		writeConsole("Too big. You should had inserted values lower than 10.0");
		threshold = 0.01f;
	}else if(threshold < 0.0001f){
		writeConsole("Too small. You should had inserted values greater than 0.01");
		threshold = 0.01f;
	}
	writeConsoleFmt("Minimal cluster size to image ratio: %.4f%%",threshold*100.0f);
	i = (int)((clusImg.width*clusImg.height)*threshold);
	i = (clusImg.width*clusImg.height)/400; //Get how much is 1%
	Ci = clusterList->getNext();
	while(Ci != (Cluster*)NULL){
		if(Ci->getCount() < i){
			Cluster* delClus = Ci;
			j++;
			writeConsoleFmt("Merging cluster %08x because of low pixel count: %d. Clusters deleted: %d\n",Ci,Ci->getCount(),j);
			Ci = Ci->getNext();
			clusterCount--;
			//delClus->mergeClosest(clusterList,delLists.hHeap);
			delClus->mergeClosest(clusterList);
			continue;
		}
		Ci = Ci->getNext();
	}
	//HeapDestroy(delLists.hHeap);
	convertClustersBitmap(clusterList, &clusImg, imgData);
	
	writeConsoleFmt("%d clusters found.\n",clusterCount);
	
	seekDiamonds(&clusImg,clusterList);
	
	lastClus = clusterList;
	//writeConsole("   ID         RGB        Count   Position,Box\n");
	while(clusterList != (Cluster*)NULL){
		clusterList = clusterList->getNext();
		//writeConsoleFmt("%08x (%3d,%3d,%3d) %9d %dx%d,%dx%d",lastClus,lastClus->getTone().red,lastClus->getTone().green,lastClus->getTone().blue,
		//lastClus->getCount(),lastClus->getLeft(),lastClus->getTop(),lastClus->getRight()-lastClus->getLeft(),lastClus->getBottom()-lastClus->getTop());
		lastClus->detectShape();
		delete lastClus;
		lastClus = clusterList;
	}
	
}


