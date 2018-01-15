#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>

#define true 1
#define false !true

extern char * optarg;

const char UNVALID_PARA_ERROR[]="%s: invalid option -- '%c'\n";
const char PARA_MISSING_ERROR[]="%s: Missing required command line argument\n";
const char OPTION_REQUIRED_ARGUMENT_ERROR[]="%s: option requires an augument -- '%c'\n";
const char MISS_REQUIRED_PARA_ERROR[]="%s: Missing required command line argument\n";


//actually the length of values is capacity+1 because we use an extra position to represent the tail
typedef struct LRU{
	//start pointer and end pointer
	int sp,ep;
	//values
	long * values;
	//capacity
	int capacity;
} LRU;

//get a LRU cache
//capacity:single LRU's capacity
//count: total LRU count
LRU * initLRU(int capacity,int count);

//put value into lru
//if lru is full and need to evict item then return -1
//if lru contains the key then return 1
//else return 0
int LRUPut(LRU * lru,long key);

//get value from lru
//if the key in lru then return 1
//else return 0
int LRUGet(LRU * lru,long key,int touch);

//touch an item to the item the lastest
int LRUTouch(LRU * lru,int position);

//print help message when passing -h
void printHelpMess();

//extract parameters from the command
int extractParameter(int argc,char * argv[], int * b,int * s,int *E,int*h,int* v,char * fileName);

//extract the address information,mark and group, with given s,E and b
int extractAddressFormat(long address,int s,int E,int b,long * mark,int * group);

int main(int argc,char * argv[])
{	
	int b,s,E,h,v;
	b = s = E = h = v = -1;
	char fileName[128];
	int re = extractParameter(argc,argv,&b,&s,&E,&h,&v,fileName);
	if(re==-1){
		return 1;	
	}
	printf("s=%d,b=%d,E=%d,h=%d,v=%d,fileName=%s\n",s,b,E,h,v,fileName);
	//-h
	if(h==1){
		printHelpMess();
		return 0;
	}
	if(b==-1||s==-1||E==-1||strlen(fileName)==0){
		printf(MISS_REQUIRED_PARA_ERROR,argv[0]);
		printHelpMess();
		return 1;
	}
	LRU * lru = initLRU(E,pow(2,s));
	FILE * fp = fopen(fileName,"r");
	char c[1];
	int hit,miss,eviction;
	long address;
	int size;
	hit=miss=eviction=0;
	while((fscanf(fp,"%s %lx,%d",c,&address,&size)!=-1)){
		long mark;
		int group;
		extractAddressFormat(address,s,E,b,&mark,&group);
		int temRe;
		//printf("%lx,%d\n",mark,group);
		if(v==1)
			printf("%s %lx,%d ",c,address,size);
		switch(c[0]){
			case 'M':
				temRe = LRUPut(lru+group,mark);
				if(temRe==1){
					hit+=2;			
					if(v==1)
						printf("hit hit");		
				}else if(temRe==0){
					miss++;
					hit++;
					if(v==1)
						printf("miss hit");	
				}else{
					miss++;
					eviction++;
					hit++;
					if(v==1)
						printf("miss eviction hit");	
				}
				break;
			case 'L':
				temRe = LRUGet(lru+group,mark,1);
				if(temRe==-1){
					miss++;
					if(v==1)
						printf("miss");	
					temRe = LRUPut(lru+group,mark);
					if(temRe==-1){
						eviction++;
						if(v==1)
							printf(" eviction");
					}
				}else{
					hit++;
					if(v==1)
						printf("hit");
				}
				break;
			case 'S':
				temRe = LRUPut(lru+group,mark);
				if(temRe==1){
					hit++;			
					if(v==1)
						printf("hit");		
				}else if(temRe==0){
					miss++;
					if(v==1)
						printf("miss");	
				}else{
					miss++;
					eviction++;
					if(v==1)
						printf("miss eviction");	
				}
				break;
			default:
				continue;
		}
		if(v==1)
			printf("\n");
	}
	fclose(fp);
    printSummary(hit, miss, eviction);
    return 0;
}


void printHelpMess(){
	printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n");
	printf("\n");
	printf("Examples:\n");
	printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int extractParameter(int argc,char * argv[], int * b,int * s,int *E,int * h,int * v,char * fileName){
	char ch;
	while((ch=getopt(argc,argv,"s:b:E:t:vh"))!=-1){
		switch(ch){
			case 's':
				*s = atoi(optarg);
				break;
			case 'b':
				*b = atoi(optarg);
				break;
			case 'E':
				*E = atoi(optarg);
				break;
			case 't':
				strcpy(fileName,optarg);
				break;
			case 'v':
				*v=1;
				break;
			case 'h':
				*h=1;
				break;
		}
	}
	return 0;
}

int extractAddressFormat(long address,int s,int E,int b,long * mark,int * group){
	long groupMask = ((1L<<s)-1)<<b;
	long markMask = ~((1L<<(s+b))-1);
	*group = (int)((address & groupMask)>>b);
	*mark = (address & markMask)>>(s+b);
	return 0;
}


LRU * initLRU(int capacity,int count){
	LRU * lru=NULL;
	lru =(LRU*) malloc(count*sizeof(LRU));
	if(lru==NULL){
		printf("malloc lru space failed");
		exit(1);
	}
	for(int i=0;i<count;i++){
		(lru+i)->capacity=capacity;
		long * values = NULL;
		values = (long *)malloc((capacity+1)*sizeof(long));
		if(values==NULL){
			printf("malloc values of LRU failed.");
			for(int j=0;j<i;j++){
				free((lru+j)->values);
			}
			free(lru);
			exit(1);
		}
		(lru+i)->values = values;
		(lru+i)->sp = 0;
		(lru+i)->ep = 0;
	}
	return lru;
}

int LRUPut(LRU * lru,long key){
	int pos = LRUGet(lru,key,0);
	int nextEp = (lru->ep + 1)% (lru-> capacity+1);
	int nextSp = (lru->sp + 1)% (lru-> capacity+1);
	//already in the cache	
	if(pos != -1){
		LRUTouch(lru,pos);
		return 1;
	}
	if(lru->sp != nextEp){ //not full
		*(lru->values+lru->ep) = key;
		//printf("put %ld at position %d\n",key,lru->ep);
		lru->ep = nextEp;
		return 0;
	}else{//full
		*(lru->values+lru->ep) = key;
		//printf("put %ld at position %d\n",key,lru->ep);
		//printf("evict %ld at position %d\n",*(lru->values+lru->sp),lru->sp);
		lru->ep = nextEp;
		lru->sp = nextSp;
		return -1;
	}
}


int LRUGet(LRU * lru,long key,int touch){
	int sp = lru->sp;
	int ep = lru->ep;
	long * values = lru->values;
	while(sp != ep){
		if(*(values+sp)==key){
			if(touch)
				LRUTouch(lru,sp);
			return sp;
		}
		sp=(sp+1) % (lru->capacity+1);
	}
	return -1;
}

int LRUTouch(LRU * lru,int position){
	int ep = lru->ep;
	long * values = lru->values;
	while(true){
		int nextPos = (position+1) % (lru->capacity+1);
		if(position==ep||nextPos==ep)
			break;
		long tem = *(values+nextPos);
		*(values+nextPos) = *(values+position);
		*(values+position) = tem;
		position = nextPos;
	}
	return 1;
}

