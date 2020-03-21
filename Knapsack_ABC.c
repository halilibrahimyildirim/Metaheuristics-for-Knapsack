#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include<stdbool.h>

#define FileCount 10
#define ExeCount 10
#define N 500
#define Iter 1000

/*Please do not change*/
#define StartRate 0.314
#define MinStartRate 0
#define AddRate 0.85
#define SubRate 0.618
#define NeighAddCalibre 0.1
#define NeighSubCalibre 0.1
#define LimitRate 0.1

typedef struct item{
	int value;
	int cons1;
	double ratio; 
}item;
typedef struct bee{
	int fitness;
	int usedCapacity;
	int stability;
	int* flower;
}bee;

item * itemPool;
int capacity;
int itemCount;
bee colony[N];

int cmpfunc(const void *a , const void *b) /*Ascending*/
{
	return (*(int*)a - *(int*)b);
}

int ratiocmpfunc(const void *a , const void *b) /*Descending*/
{
	double res=((*(item*)b).ratio - (*(item*)a).ratio);
	return (res>0) ? 1 : -1; 
}

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

void Write(char* fileName, int maxFit, double avgFit, double time)/*Write to file*/
{
	FILE *f=fopen(fileName,"w");
	fprintf(f,"%lf %d %lf",avgFit,maxFit,time);
}

int SelectArea()/*Roulette selection*/
{
	int i,j;
	double scores[N];
	double sSum=0;
	for(i=0;i<N;i++)
	{
		scores[i]=colony[i].fitness;
		sSum+=scores[i];
	}
	for(i=0;i<N;i++)
	{
		scores[i]/=sSum;
	}
	double cumulative = 0;
	double r=getRand();
	for(i=0;i<N;i++)
	{
		if(r<(cumulative+scores[i]))
			return i;
		cumulative+=scores[i];
	}
	return N-1;
}

int FindBestIndex()/*Finds best bee*/
{
	int bestI=0,bestF=0,i;
	for(i=0;i<N;i++)
	{
		if(colony[i].fitness>bestF)
		{
			bestF=colony[i].fitness;
			bestI=i;
		}
	}
	return bestI;
}

void PrintBee(bee temp)
{
	int i;
	printf("Fit:%d Cap:%d\n",temp.fitness,temp.usedCapacity);
	for(i=0;i<itemCount;i++)
		printf("%d ",temp.flower[i]);
	printf("\n");
	//getch();
}

void Scout(int index,int count)/*If stability==limit remove some pollen*/
{
	int i,j=0;
	for(i=itemCount-1;i>=0 && j<count;i--)
	{
		if(colony[index].flower[i]==1 && getRand()<SubRate)
		{
			colony[index].flower[i]=0;
			colony[index].usedCapacity-=itemPool[i].cons1;
			colony[index].fitness-=itemPool[i].value;
			j++;
		}
	}
	int tempCapacity=colony[index].usedCapacity;
	for(i=0;i<itemCount;i++)
	{
		if(colony[index].flower[i]==0 && itemPool[i].cons1+tempCapacity<=capacity && getRand()<AddRate)
		{
			colony[index].flower[i]=1;
			tempCapacity+=itemPool[i].cons1;
		}
	}	
}

void TreatBee(int index)/*If used capacity exceeds the max*/
{
	int r=itemCount-1;
	while(colony[index].usedCapacity>capacity)
	{
		//r=rand()%itemCount;
		if(colony[index].flower[r]==1)
		{
			colony[index].flower[r]=0;
			colony[index].usedCapacity-=itemPool[r].cons1;
			colony[index].fitness-=itemPool[r].value;
		}
		r--;
		r=(r+itemCount)%itemCount;
	}
}

void Fitness(int index)/*Calculates fitness*/
{
	int i;
	int score=0;
	int usedCapacity=0;
	for(i=0;i<itemCount;i++)
	{
		score+=colony[index].flower[i]*itemPool[i].value;
		usedCapacity+=colony[index].flower[i]*itemPool[i].cons1;
	}
	colony[index].fitness=score;
	colony[index].usedCapacity=usedCapacity;
	
}

void FindNeighbour(int index,int neighIndex,int count)/*Finds new solution by using solution of neighIndex*/
{
	int i=0,j=0,r;
	bee neighbour;
	memcpy(&neighbour,&colony[neighIndex],sizeof(neighbour));
	for(i=0;i<itemCount && j<count;i++)
	{
		if(neighbour.flower[i]==0 && getRand()<AddRate && neighbour.usedCapacity+itemPool[i].cons1<=capacity)
		{
			neighbour.flower[i]=1;
			neighbour.usedCapacity+=itemPool[i].cons1;
			neighbour.fitness+=itemPool[i].value;
			j++;
		}
	}
	if(neighbour.fitness>colony[index].fitness)
	{
		memcpy(&colony[index],&neighbour,sizeof(neighbour));
		colony[index].stability=0;
	}
	else
		colony[index].stability++;
	
}

void HeuristicStartPoint()/*Creates starting population*/
{
	int i=0,j;
	int tempCapacity=0;
	double decr=(StartRate-MinStartRate)/(double)N;
	double rate=StartRate;
	for(i=0;i<N;i++)
	{
		colony[i].stability=0;
		colony[i].flower=(int*)calloc(itemCount,sizeof(int));
		for(j=0;j<itemCount;j++)
		{
			if(tempCapacity+itemPool[j].cons1<=capacity && getRand()<rate)
			{
					colony[i].flower[j]=1;
					tempCapacity+=itemPool[j].cons1;
			}
		}
		rate-=decr;
		tempCapacity=0;
	}
}

int main()
{
	int i,m,f,iter;
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
		int best=0,bestCap=0,bestGlob=0,bestIndex;
		int neighAddCalibre=ceil(NeighAddCalibre*itemCount);/*Used for FindNeighbour function*/
		int limit=LimitRate*Iter;
		int neighSubCalibre=ceil(NeighSubCalibre*itemCount);/*Used for Scout Function*/
		double bestGlobTime=0,avg=0;
		/*Sort items by ratio*/
		qsort(itemPool,itemCount,sizeof(item),ratiocmpfunc);
		clock_t t;
		printf("%s %d %d\n",fileName, itemCount, capacity);
		for(m=0;m<ExeCount;m++)
		{
			t=clock();/*timer starts*/
			HeuristicStartPoint();
			best=0;
			for(iter=0;iter<Iter;iter++)/*ABC algorithm*/
			{
				for(i=0;i<N;i++)
				{
					Fitness(i);
					if(colony[i].usedCapacity>capacity)
						TreatBee(i);
				}
				for(i=0;i<N;i++)
					FindNeighbour(i,SelectArea(),neighAddCalibre);
				bestIndex=FindBestIndex();
				if(colony[bestIndex].fitness>best)
				{
					best=colony[bestIndex].fitness;
					bestCap=colony[bestIndex].usedCapacity;
				}
				for(i=0;i<N;i++)
				{
					if(colony[i].stability==limit)
					{
						Scout(i,neighSubCalibre);
						colony[i].stability=0;
					}
				}
				
			}
			for(i=0;i<N;i++)
			{
				Fitness(i);
				if(colony[i].usedCapacity>capacity)
					TreatBee(i);
			}
			bestIndex=FindBestIndex();
			if(colony[bestIndex].fitness>best)
			{
				best=colony[bestIndex].fitness;
				bestCap=colony[bestIndex].usedCapacity;
			}
			t = clock() - t;/*timer stops*/
			double time_taken = ((double)t);
			if(best>bestGlob || (best==bestGlob && time_taken<bestGlobTime))
			{
				bestGlob=best;
				bestGlobTime=time_taken;
			}
			avg+=best;
			printf(" %d %d %lf\n",best,bestCap,time_taken);
		}
		strcat(writeFileName,results);
		Write(writeFileName,bestGlob,avg/ExeCount,bestGlobTime);
	}
	exit(0);
}
