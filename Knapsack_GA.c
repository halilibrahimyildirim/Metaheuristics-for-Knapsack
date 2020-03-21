#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define FileCount 10
#define N 500
#define MutRate 0.05
#define Iteration 1000
#define CrossOverRate 0.85
#define ExeCount 10

/*Please do not change*/
#define MutCalibre 0.05
#define MStartRate 0.85
#define ElimgrationRate 0.1
#define StartRate 0.5
#define ErrorRate 0.00001
#define BestIterThres 100

typedef struct item{
	int value;
	int cons1;
	double ratio; /* bigger better */
}item;
typedef struct chromosome{
	int * gen;
	int fitness;
	int usedCapacity;
}chromosome;
int cmpfunc(const void *a , const void *b) /*Ascending*/
{
	return (*(int*)a - *(int*)b);
}
int fitnesscmpfunc(const void *a , const void *b) /*Descending*/
{
	return ((*(chromosome*)b).fitness - (*(chromosome*)a).fitness);
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

int itemCount;
int capacity;
item * itemPool;
chromosome population[N];
chromosome nextGeneration[N];
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
void Vaccinate(int index)/*If used capacity exceeds the max*/
{
	int i=itemCount-1;
	while(population[index].usedCapacity>capacity)
	{
		population[index].usedCapacity-=itemPool[i].cons1*population[index].gen[i];
		population[index].fitness-=itemPool[i].value*population[index].gen[i];
		population[index].gen[i]=0;
		i--;
	}
	while(population[index].usedCapacity<capacity)
	{
		i=rand()%itemCount;
		population[index].usedCapacity+=itemPool[i].cons1*(1-population[index].gen[i]);
		population[index].fitness+=itemPool[i].value*(1-population[index].gen[i]);
		population[index].gen[i]=1;
	}

	population[index].usedCapacity-=itemPool[i].cons1*population[index].gen[i];
	population[index].fitness-=itemPool[i].value*population[index].gen[i];
	population[index].gen[i]=0;	

}
void NonUniformCrossOver(int indexF,int indexS,int indexGen)/*Ratio based crossover on fitness*/
{
	int i=0;
	if(getRand()>CrossOverRate){
		if(population[indexF].fitness>population[indexS].fitness)
			memcpy(&nextGeneration[indexGen],&population[indexF],sizeof(nextGeneration[indexGen]));
		else
			memcpy(&nextGeneration[indexGen],&population[indexS],sizeof(nextGeneration[indexGen]));
		return;
	}
	double ratio = (double)population[indexF].fitness/(double)(population[indexF].fitness+population[indexS].fitness);
	for(i;i<itemCount;i++)
	{
		if(getRand()<ratio)
			nextGeneration[indexGen].gen[i]=population[indexF].gen[i];
		else
			nextGeneration[indexGen].gen[i]=population[indexS].gen[i];
	}
}
void Mutation(int index,int count)/*Adds #count piece items to bag*/
{
	int i,j=0;
	for(i=0;i<itemCount;i++)
	{
		if(population[index].gen[i]==0 && getRand()<MStartRate){
			population[index].gen[i]=1;
			j++;
			if(j==count) break;
		}
	}
}
void MutationS(int index,int count)/*Outdated mutation function*/
{
	int i,r;
	for(i=0;i<count;i++)
	{
		r=rand()%itemCount;
		population[index].gen[r]=!population[index].gen[r];
	}
}
void Selection(int selectCount,int candidates [])/*Roulette selection*/
{
	int i,j;
	double scores[N];
	double sSum=0;
	for(i=0;i<N;i++)
	{
		scores[i]=population[i].fitness;
		sSum+=scores[i];
	}
	for(i=0;i<N;i++)
	{
		scores[i]/=sSum;
	}
	double cumulative = 0;
	j=0;
	while(j<selectCount)
	{
		cumulative = 0;
		for(i=0;i<N;i++)
		{
			if(getRand()<(cumulative+scores[i]))
			{
				candidates[j]=i;
				j++;
				break;
			}
			cumulative+=scores[i];
		}
	}
}
void Fitness(int index)/*Calculates fitness*/
{
	int i;
	int score=0;
	int usedCapacity=0;
	for(i=0;i<itemCount;i++)
	{
		score+=population[index].gen[i]*itemPool[i].value;
		usedCapacity+=population[index].gen[i]*itemPool[i].cons1;
	}
	population[index].fitness=score;
	population[index].usedCapacity=usedCapacity;
}
void HeuristicStartPoint()/*Creates starting population*/
{
	int i=0,j;
	int tempCapacity=0;
	for(i=0;i<N;i++)
	{
		population[i].gen=(int*)calloc(itemCount,sizeof(int));
		nextGeneration[i].gen=(int*)calloc(itemCount,sizeof(int));
		for(j=0;j<itemCount;j++)
		{
			if(tempCapacity+itemPool[j].cons1<capacity)
			{
				if(getRand()<StartRate)
				{
					population[i].gen[j]=1;
					tempCapacity+=itemPool[j].cons1;	
				}
			}
		}
		tempCapacity=0;
	}
}
void PrintPopulation()/*Prints population*/
{
	int i=0,j;
	for(i;i<1;i++)
	{
		printf("[%d] ",population[i].fitness);
		for(j=0;j<itemCount;j++)
		{
			printf("%d ",population[i].gen[j]);	
		}
		printf("\n");
	}
}
int FindBestChr()
{
	int i,bestF=0,bestC=0,bestI;
	for(i=0;i<N;i++)
	{
		if(population[i].fitness>bestF || (population[i].fitness==bestF && population[i].usedCapacity<bestC))
		{
			bestF=population[i].fitness;
			bestC=population[i].usedCapacity;
			bestI=i;
		}
	}
	return bestI;
}
int main()
{
	int i=0,j,iter=0,elitCount,candidatesCount,mutCount,f;
	srand(time(NULL));
	int m;
	char results[]="_2_results.txt";
	char writeFileName[100];
	for(f=1;f<=FileCount;f++)
	{
		int best=0,bestCap=0,bestGlob=0,calibreCount,bestI,bestCount;
		double bestGlobTime=0,avg=0;
		char fileName[]="test";
		char numb[3];
		/*File name*/
		sprintf(numb,"%d",f);
		strcat(fileName,numb);
		strcpy(writeFileName,fileName);
		strcat(fileName,".txt");
		Read(fileName);
		clock_t t;
		printf("%s %d %d\n",fileName,itemCount,capacity);
		/*Sort items by ratio*/
		qsort(itemPool,itemCount,sizeof(item),ratiocmpfunc);
		elitCount=ceil(N*ElimgrationRate);
		candidatesCount=(N-elitCount)*2;
		mutCount=itemCount*MutCalibre;
		int candidates[candidatesCount];
		for(m=0;m<ExeCount;m++)
		{
			t=clock();/*timer starts*/
			HeuristicStartPoint();
			best=0;
			bestCap=0;
			bestCount=0;
			for(iter=0;iter<Iteration;iter++)/*GA algorithm*/
			{
				for(i=0;i<N;i++){
					Fitness(i);
					Vaccinate(i);
				}
				bestI=FindBestChr();
				/*qsort(population,N,sizeof(chromosome),fitnesscmpfunc);*/
				/*Possible illegal area :) 
				if(population[bestI].fitness-best<ceil(best*ErrorRate))
					bestCount++;
				else
					bestCount=0;
				if(bestCount==BestIterThres)
					break;
				Possible illegal area :) */
				if(population[bestI].fitness>best)
				{
					best = population[bestI].fitness;
					bestCap = population[bestI].usedCapacity;
				}
				for(i=0;i<elitCount;i++)
					memcpy(&nextGeneration[i],&population[i],sizeof(nextGeneration[i]));
				Selection(candidatesCount,candidates);
				for(i=0;i<candidatesCount;i+=2)
					NonUniformCrossOver(candidates[i],candidates[i+1],elitCount+(i/2));
				memcpy(&population,&nextGeneration,sizeof(population));
				for(i=0;i<N;i++)
					if(getRand()<MutRate)
						Mutation(i,mutCount);
			}
			for(i=0;i<N;i++)
			{
				Fitness(i);
				Vaccinate(i);
			}	
			bestI=FindBestChr();
			if(population[bestI].fitness>best)
			{
				best = population[bestI].fitness;
				bestCap = population[bestI].usedCapacity;
			}
			t = clock() - t;/*timer stops*/
			double time_taken = ((double)t)/CLOCKS_PER_SEC;
			avg+=best;
			if(best>bestGlob || (best==bestGlob && time_taken<bestGlobTime))
			{
				bestGlob=best;
				bestGlobTime=time_taken;
			}
			printf(" %d  %d  %lf\n",best,bestCap,time_taken);
		}
		strcat(writeFileName,results);
		Write(writeFileName,bestGlob,avg/ExeCount,bestGlobTime);
	}
	exit(0);
}
