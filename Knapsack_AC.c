#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include<stdbool.h>

#define FileCount 10
#define ExeCount 10
#define N 200
#define Iter 500
#define Alpha 0.5
#define Beta 0.5
#define Ro 0.2
#define Q 1/*Function Update Pheromone - Line 154*/

/*Please do not change*/
#define LimitRate 0.1
#define LowThreshRate 0.3
#define HighThreshRate 0.7
#define LowGitGudRate 0.7
#define HighGitGudRate 0.6
#define Calibre 0.1

typedef struct item{
	int value;
	int cons1;
	double ratio;
	double theta;
}item;
typedef struct ant{
	int fitness;
	int usedCapacity;
	int stability;
	int *food;
}ant;

item * itemPool;
int capacity;
ant colony[N];
int itemCount;
double *pheromone;
double threshold;

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
	pheromone=(double*)calloc(itemCount,sizeof(double));
	for(i=0;i<itemCount;i++)
	{
		fscanf(f,"%d",&itemPool[i].cons1);
		pheromone[i]=1;
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

void Fitness(int index)/*Calculates fitness*/
{
	int i;
	int score=0;
	int usedCapacity=0;
	for(i=0;i<itemCount;i++)
	{
		score+=colony[index].food[i]*itemPool[i].value;
		usedCapacity+=colony[index].food[i]*itemPool[i].cons1;
	}
	colony[index].fitness=score;
	colony[index].usedCapacity=usedCapacity;
	
}

double CalcThreshold()/*Calculates threshold*/
{
	int i;
	double total=0;
	for(i=0;i<itemCount;i++)
	{
		total+=itemPool[i].theta;		
	}
	return total/(double)itemCount;
}

void CalcItemThres()/*Calculates threshold values per items*/
{
	int i;
	for(i=0;i<itemCount;i++)
	{
		itemPool[i].theta=pow(pheromone[i],Alpha)*pow(itemPool[i].ratio,Beta);
		pheromone[i]*=(1-Ro);
	}
}

void UpdatePheromone(int index,int bestIndex)/*Updates Pheromones*/
{
	int j;
	double temp;
	for(j=0;j<itemCount;j++)
	{
		temp=(double)colony[index].fitness/(colony[bestIndex].fitness+1);
		pheromone[j]+=temp*itemPool[j].ratio/**Q*/;
	}
}

void PrintPher()
{
	int i;
	for(i=0;i<itemCount;i++)
	{
		if(i%5!=0)
			printf("%lf ",pheromone[i]);
		else
			printf("%lf\n",pheromone[i]);
	}
	printf("\n");
	getch();
}

void Walk(int index)/*Ant[index] completes a tour*/
{
	int i,tempCapacity=colony[index].usedCapacity;
	int tempFitness=colony[index].fitness;
	for(i=0;i<itemCount;i++)
	{
		if(colony[index].food[i]==0 && itemPool[i].cons1+tempCapacity<=capacity && ((itemPool[i].theta<threshold && getRand()<LowThreshRate) || (itemPool[i].theta>threshold && getRand()<HighThreshRate)))
		{
			colony[index].food[i]=1;
			tempCapacity+=itemPool[i].cons1;
		}
	}
	if(tempFitness<colony[index].fitness)
	{
		colony[index].stability=0;
	}
	else
	{
		colony[index].stability++;
	}
}

void GitGud(int index,int count)/*If stability==limit change tour*/
{
	int i,j=0,best=FindBestIndex();
	for(i=itemCount-1;i>=0 && j<count;i--)
	{
		if(colony[index].food[i]==1 && colony[best].food[i]==0  && getRand()<LowGitGudRate)
		{
			colony[index].fitness-=itemPool[i].value;
			colony[index].usedCapacity-=itemPool[i].cons1;
			colony[index].food[i]=0;
			j++;
		}
	}
	int tempCapacity=colony[index].usedCapacity;
	for(i=0;i<itemCount;i++)
	{
		if(colony[index].food[i]==0 && itemPool[i].cons1+tempCapacity<=capacity && ((itemPool[i].theta<threshold && getRand()<LowGitGudRate) || (itemPool[i].theta>threshold && getRand()<HighGitGudRate)))
		{
			colony[index].food[i]=1;
			tempCapacity+=itemPool[i].cons1;
		}
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
		int limit=ceil(LimitRate*Iter);
		int calibre=ceil(itemCount*Calibre);
		double bestGlobTime=0,avg=0;
		/*Sort items by ratio*/
		qsort(itemPool,itemCount,sizeof(item),ratiocmpfunc);
		clock_t t;
		printf("%s %d %d\n",fileName, itemCount, capacity);
		for(m=0;m<ExeCount;m++)
		{
			t=clock();/*timer starts*/
			for(i=0;i<N;i++)/*Variable reset*/
			{
				colony[i].food=(int*)calloc(itemCount,sizeof(int));
				colony[i].usedCapacity=0;
				colony[i].stability=0;
				colony[i].fitness=0;
			}
			for(i=0;i<itemCount;i++)/*Variable reset*/
				pheromone[i]=1;
			best=0;
			for(iter=0;iter<Iter;iter++)/*AC algorithm*/
			{
				CalcItemThres();
				threshold=CalcThreshold();
				for(i=0;i<N;i++)
				{
					Walk(i);
					Fitness(i);
				}
				bestIndex=FindBestIndex();
				for(i=0;i<N;i++)
					UpdatePheromone(i,bestIndex);
				for(i=0;i<N;i++)
				{
					if(colony[i].stability==limit)
					{
						colony[i].stability=0;
						GitGud(i,calibre);
						Fitness(i);
					}
				}
				bestIndex=FindBestIndex();
				if(colony[bestIndex].fitness>best)
				{
					best=colony[bestIndex].fitness;
					bestCap=colony[bestIndex].usedCapacity;
				}
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
