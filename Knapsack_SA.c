#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include<stdbool.h>

#define FileCount 10
#define TemprMax 1000.0
#define stopTempr 1
#define coolingRate 0.97
#define ExeCount 10

/*Please do not change*/
#define iteration 150
#define StartRate 1
#define NeighbourCoeff 0.002

typedef struct item{
	int value;
	int cons1;
	double ratio; 
}item;
typedef struct point{
	int fitness;
	int usedCapacity;
	int *set;
}point;

int cmpfunc(const void *a , const void *b) /*Ascending*/
{
	return (*(int*)a - *(int*)b);
}
int ratiocmpfunc(const void *a , const void *b) /*Descending*/
{
	double res=((*(item*)b).ratio - (*(item*)a).ratio);
	return (res>0) ? 1 : -1; 
}

void arrayCopy(int destination[],int original[],int n)
{
	int i;
	for(i=0;i<n;i++)
	{
		destination[i]=original[i];
	}
}

double Tempr=TemprMax;
int itemCount;
int capacity;
item * itemPool;
int bestn=0;
point current,neighbour;

double getRand()/*Returns [0-1] random*/
{
	return (double)rand()/(double)RAND_MAX;
}

void Read(char* fileName)/*Reads file*/
{
	FILE * f;
	f=fopen(fileName,"r");
	fscanf(f,"%d",&capacity);
	int temp;
	int i=0;
	while(!feof(f))
	{
		fscanf(f,"%d",&temp);
		i++;
	}
	rewind(f);
	fscanf(f,"%d",&capacity);
	itemCount=i/2;
	itemPool=(item*)calloc(itemCount,sizeof(item));
	for(i=0;i<itemCount;i++)
	{
		fscanf(f,"%d",&itemPool[i].cons1);
	}
	for(i=0;i<itemCount;i++)
	{
		fscanf(f,"%d",&itemPool[i].value);
		itemPool[i].ratio = (double)itemPool[i].value / (double)itemPool[i].cons1;
	}
}
void PrintPoint(point p,int mode)/*Prints point*/
{
	int i=0;
	if(mode==1 || mode==3)
	printf("[%d] ",p.fitness);
	if(mode==2 || mode==3)
	for(i;i<itemCount;i++)
	{
		printf("%d ",p.set[i]);
	}
	printf("\n------------------------\n");
}
void Write(char* fileName, int maxFit, double avgFit, double time)/*Write to file*/
{
	FILE *f=fopen(fileName,"w");
	fprintf(f,"%lf %d %lf",avgFit,maxFit,time);
}

bool Check(int delta)/*Checks if bad result is gonna be accepted*/
{
	double rand=getRand();
	double prob=exp(-(double)delta/Tempr);
	return (rand<prob)?true:false;
}

void DecrTemp()/*Decreases temperature*/
{
	Tempr*=coolingRate;
}
void Fitness(point *p)/*Calculates fitness for point p*/
{
	int i;
	p->fitness=0;
	p->usedCapacity=0;
	for(i=0;i<itemCount;i++)
	{
		p->fitness+=p->set[i]*itemPool[i].value;
		p->usedCapacity+=p->set[i]*itemPool[i].cons1;
	}

}

void OldFindNeighbour(int count)/*Outdated neighbour function 1*/
{
	int i,j=0;
	neighbour.set=calloc(itemCount,sizeof(int));
	/*memcpy(neighbour.set,current.set,sizeof(neighbour.set));*/
	arrayCopy(neighbour.set,current.set,itemCount);
	neighbour.usedCapacity=current.usedCapacity;
	for(i=0;i<itemCount;i++)
	{
		if(neighbour.set[i]==0)
		{
			neighbour.set[i]=1;
			neighbour.usedCapacity+=itemPool[i].cons1;
			j++;
			if(j==count) break;
		}
	}
	i=itemCount-1;
	while(neighbour.usedCapacity>capacity)
	{
		if(neighbour.set[i]==1)
		{
			neighbour.set[i]=0;
			neighbour.usedCapacity-=itemPool[i].cons1;
		}
		i--;
	}
}

void FindNeighbour(int count)/*Outdated neighbour function 2*/
{
	neighbour.set=calloc(itemCount,sizeof(int));
/*	memcpy(neighbour.set,current.set,sizeof(neighbour.set));*/
	arrayCopy(neighbour.set,current.set,itemCount);
	neighbour.usedCapacity=current.usedCapacity;
	int i=0,r;
	for(i;i<count;i++)
	{
		r=rand()%itemCount;
		if(neighbour.set[r]==0)
		{
			neighbour.set[r]=1;
			neighbour.usedCapacity+=itemPool[r].cons1;
		}
	}
	while(neighbour.usedCapacity>capacity)
	{
		r=rand()%itemCount;
		if(neighbour.set[r]==1)
		{
			neighbour.set[r]=0;
			neighbour.usedCapacity-=itemPool[r].cons1;
		}
	}
	
}

void HybridFindNeighbour(int count)/*Finds neighbour*/
{
	memcpy(&neighbour,&current,sizeof(neighbour));
	
	int i=0,j=0,r;
	for(i=0;i<itemCount;i++)
	{
		if(neighbour.set[i]==0 && getRand()<StartRate)
		{
			neighbour.set[i]=1;
			neighbour.usedCapacity+=itemPool[i].cons1;
			j++;
			if(j==count) break;
		}
	}
	while(neighbour.usedCapacity>capacity)
	{
		r=rand()%itemCount;
		if(neighbour.set[r]==1)
		{
			neighbour.set[r]=0;
			neighbour.usedCapacity-=itemPool[r].cons1;
		}
	}
	
}

void HeuristicStartPoint()/*Finds starting position*/
{
	int j;
	current.usedCapacity = 0;
	for(j=0;j<itemCount;j++)
	{
		if(current.usedCapacity+itemPool[j].cons1<capacity)
		{
			if(getRand()<StartRate)
			{
				current.set[j]=1;
				current.usedCapacity+=itemPool[j].cons1;
			}
		}
	}
} 

int main()
{
	int i,m,f;
	srand(time(NULL));
	char results[]="_2_results.txt";
	char writeFileName[100];
	for(f=1;f<=FileCount;f++)
	{	
		char fileName[]="test";
		char numb[3];
		/*File name*/
		sprintf(numb,"%d",f);
		strcat(fileName,numb);
		strcpy(writeFileName,fileName);
		strcat(fileName,".txt");
		Read(fileName);
		int delta,best=0,bestCap=0,bestGlob=0,calibreCount,bestGlobCap=0;
		double bestGlobTime=0,avg=0;
		/*Sort items by ratio*/
		qsort(itemPool,itemCount,sizeof(item),ratiocmpfunc);
		clock_t t;
		printf("%s %d %d\n",fileName, itemCount, capacity);
		for(m=0;m<ExeCount;m++)
		{
			t=clock();/*timer starts*/
			Tempr=TemprMax;
			best=0;
			current.set=(int*)calloc(itemCount,sizeof(int));
			HeuristicStartPoint();
			neighbour.set=(int*)calloc(itemCount,sizeof(int));
			calibreCount = ceil(Tempr/TemprMax*(double)itemCount*NeighbourCoeff);
			do/*SA algorithm*/
			{
				for(i=0;i<iteration;i++)
				{
					HybridFindNeighbour(calibreCount);
					Fitness(&neighbour);
					Fitness(&current);
					delta=current.fitness-neighbour.fitness;
					if(delta<0 || (delta>0 && Check(delta)))
						memcpy(&current,&neighbour,sizeof(current));
					if(current.fitness>best)
					{
						best=current.fitness;
						bestCap=current.usedCapacity;
					} 
				}
				DecrTemp();
			}while(Tempr>stopTempr);
			t = clock() - t;/*timer stops*/
			double time_taken = ((double)t)/CLOCKS_PER_SEC;
			if(best>bestGlob || (best==bestGlob && time_taken<bestGlobTime))
			{
				bestGlob=best;
				bestGlobTime=time_taken;
				bestGlobCap=bestCap;
			}
			avg+=best;
			printf(" %d %d %lf\n",best,bestGlobCap,time_taken);
		}
		strcat(writeFileName,results);
		Write(writeFileName,bestGlob,avg/ExeCount,bestGlobTime);
	}
	exit(0);
}
