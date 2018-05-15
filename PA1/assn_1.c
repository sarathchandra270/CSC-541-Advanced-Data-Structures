/*
========================================================================
Name: In-Memory VS Disk-Based Searching
========================================================================
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>

void memoryLinearSearch(char *,char *);
void memoryBinarySearch(char *,char *);
void diskLinearSearch(char *,char *);
void diskBinarySearch(char *,char *);
int binarySearch(int * ,int ,  int , int );
int dBinarySearch(FILE *, int , int, int);

struct timeval tm1,tm2;

/*
========================================================================
			Main Function
========================================================================
*/

int main(int argc, char* argv[])
{
	char *seekFile,*keyFile;
	if(argc!=4)
		printf("enter correct number of arguments\n");
	else
	{
		keyFile=argv[2];
		seekFile=argv[3];
	
		if(strcmp(argv[1],"--mem-lin")==0)
			memoryLinearSearch(keyFile,seekFile);
		else if(strcmp(argv[1],"--mem-bin")==0)
			memoryBinarySearch(keyFile,seekFile);
		else if(strcmp(argv[1],"--disk-lin")==0)
			diskLinearSearch(keyFile,seekFile);
		else if(strcmp(argv[1],"--disk-bin")==0)
			diskBinarySearch(keyFile,seekFile);
	}
return 0;
}

/*
=========================================================================
			In Memory Linear Search
=========================================================================
*/

void memoryLinearSearch(char *keyFile,char *seekFile)
{
	FILE *kp,*sp;
	int i,p,q;
	int n1,n2;
	int time=0;
	kp=fopen(keyFile,"rb");
	sp=fopen(seekFile,"rb");
	
	if(kp==NULL)
		printf("KeyFile not found\n");
	else if(sp==NULL)
		printf("SeekFile not found\n");	
	else
	{
		fseek(kp, 0L, SEEK_END);
		n1=ftell(kp)/sizeof(int);
		fseek(kp, 0L, SEEK_SET);

		fseek(sp, 0L, SEEK_END);
        	n2=ftell(sp)/sizeof(int);
        	fseek(sp, 0L, SEEK_SET);
	
		int K[n1],S[n2],hit[n2];
	
		fread(&S, sizeof(int), n2, sp);
		gettimeofday(&tm1, NULL);
		fread(&K, sizeof(int), n1, kp);
        

		for(p=0;p<n2;p++)
		{
			for(q=0;q<n1;q++)
			{
				if(S[p]==K[q])
				{
					hit[p]=1;
					break;
				}
				else
					hit[p]=0;						
			}	
		}
		gettimeofday(&tm2, NULL);
		for(i=0;i<n2;i++)
		{
			if(hit[i]==1)
				printf("%12d: Yes\n",S[i]);
			else if(hit[i]==0)
                        	printf("%12d: No\n",S[i]);
		}
		if(tm2.tv_usec<tm1.tv_usec)
		{
			tm2.tv_sec=tm2.tv_sec-1;
			time=tm2.tv_usec+1000000-tm1.tv_usec;
		}
		else
			time=tm2.tv_usec-tm1.tv_usec;
		
		printf( "Time: %ld.%06ld\n", (tm2.tv_sec-tm1.tv_sec), time);
		fclose(kp);
		fclose(sp);
	}
}

/*
==============================================================================
			In Memory Binary Search
==============================================================================
*/

void memoryBinarySearch(char *keyFile,char *seekFile)
{
	FILE *kp,*sp;
        int i,p,res;
	int n1,n2;
	int time=0;

	kp=fopen(keyFile,"rb");
        sp=fopen(seekFile,"rb");
	if(kp==NULL)
                printf("KeyFile not found\n");
        else if(sp==NULL)
                printf("SeekFile not found\n");
        else
	{
		fseek(kp, 0L, SEEK_END);
        	n1=ftell(kp)/sizeof(int);
        	fseek(kp, 0L, SEEK_SET);

        	fseek(sp, 0L, SEEK_END);
        	n2=ftell(sp)/sizeof(int);
        	fseek(sp, 0L, SEEK_SET);

        	int K[n1],S[n2],hit[n2];
   
        	fread(&S, sizeof(int), n2, sp);
		gettimeofday(&tm1, NULL);
		fread(&K, sizeof(int), n1, kp);
		for(p=0;p<n2;p++)
		{
			res=binarySearch(K, 0, n1-1, S[p]);
			if(res==1)
				hit[p]=1;
			else
				hit[p]=0;
		}
		gettimeofday(&tm2, NULL);
		for(i=0;i<n2;i++)
        	{
                	if(hit[i]==1)
                        	printf("%12d: Yes\n",S[i]);
                	else if(hit[i]==0)
                        	printf("%12d: No\n",S[i]);
        	}
		if(tm2.tv_usec<tm1.tv_usec)
        	{
        		tm2.tv_sec=tm2.tv_sec-1;
        		time=tm2.tv_usec+1000000-tm1.tv_usec;
        	}
        	else
        		time=tm2.tv_usec-tm1.tv_usec;

        	printf( "Time: %ld.%06ld\n", (tm2.tv_sec-tm1.tv_sec), time);
		fclose(kp);	
		fclose(sp);
	}
}

int binarySearch(int K[],int l,  int h, int value)
{
	int mid;
	while(l<=h)
	{
		mid=l+(h-l)/2;
		if(K[mid]==value)
			return 1;
		else if(K[mid]>value)
			h=mid-1;
		else 
			l=mid+1;
	}
return 0;			
}

/*
===============================================================================
			In Disk Linear Search
===============================================================================
*/

void diskLinearSearch(char *keyFile,char *seekFile)
{
	FILE *kp,*sp;
        int n1,n2;
	int i,p,q;
	int time=0;

	kp=fopen(keyFile,"rb");
        sp=fopen(seekFile,"rb");
	if(kp==NULL)
                printf("KeyFile not found\n");
        else if(sp==NULL)
                printf("SeekFile not found\n");
        else
	{
		fseek(kp, 0L, SEEK_END);
        	n1=ftell(kp)/sizeof(int);
        	fseek(kp, 0L, SEEK_SET);

        	fseek(sp, 0L, SEEK_END);
        	n2=ftell(sp)/sizeof(int);
        	fseek(sp, 0L, SEEK_SET);

        	int S[n2],hit[n2];

        	fread(&S, sizeof(int), n2, sp);
		gettimeofday(&tm1, NULL);
		for(p=0;p<n2;p++)
		{
			int K=0;
			fseek(kp, 0L, SEEK_SET);	
			while(!feof(kp))
			{		
				
				fread(&K,sizeof(int),1,kp);
				if(K==S[p])
				{
					hit[p]=1;
					break;
				}
				else
					hit[p]=0;		
			}
		}
		gettimeofday(&tm2, NULL);
		for(i=0;i<n2;i++)
        	{
        	        if(hit[i]==1)
                        	printf("%12d: Yes\n",S[i]);
                	else if(hit[i]==0)
                	        printf("%12d: No\n",S[i]);
       	 	}
		if(tm2.tv_usec<tm1.tv_usec)
        	{
        		tm2.tv_sec=tm2.tv_sec-1;
        		time=tm2.tv_usec+1000000-tm1.tv_usec;
        	}
        	else
	    		time=tm2.tv_usec-tm1.tv_usec;
 
        	printf( "Time: %ld.%06ld\n", (tm2.tv_sec-tm1.tv_sec), time);
		fclose(kp);
		fclose(sp);
	}
}

/*
==============================================================================
			In Disk Binary Search
==============================================================================
*/

void diskBinarySearch(char *keyFile,char *seekFile)
{
	FILE *kp,*sp;
        int n1,n2;
	int i,p;
	int res;
	int time=0;

	kp=fopen(keyFile,"rb");
        sp=fopen(seekFile,"rb");
	if(kp==NULL)
                printf("KeyFile not found\n");
        else if(sp==NULL)
                printf("SeekFile not found\n");
        else 
	{
		fseek(kp, 0L, SEEK_END);
        	n1=ftell(kp)/sizeof(int);
        	fseek(kp, 0L, SEEK_SET);

        	fseek(sp, 0L, SEEK_END);
        	n2=ftell(sp)/sizeof(int);
        	fseek(sp, 0L, SEEK_SET);

        	int S[n2],hit[n2];
	
        	fread(&S, sizeof(int), n2, sp);
		gettimeofday(&tm1, NULL);
        	for(p=0;p<n2;p++)
        	{
                	res=dBinarySearch(kp,0,n1-1,S[p]);
			if(res==1)
				hit[p]=1;
			else 
				hit[p]=0;
        	}
		gettimeofday(&tm2, NULL);
		for(i=0;i<n2;i++)
        	{
        	        if(hit[i]==1)
        	                printf("%12d: Yes\n",S[i]);
                	else if(hit[i]==0)
                	        printf("%12d: No\n",S[i]);
        	}	
		if(tm2.tv_usec<tm1.tv_usec)
        	{
        		tm2.tv_sec=tm2.tv_sec-1;
        		time=tm2.tv_usec+1000000-tm1.tv_usec;
        	}
        	else
        		time=tm2.tv_usec-tm1.tv_usec;
        	
        	printf( "Time: %ld.%06ld\n", (tm2.tv_sec-tm1.tv_sec), time);
		fclose(kp);
		fclose(sp);	
	}
}

int dBinarySearch(FILE *kp,int l,int h, int value)
{
	int K=0,mid;
	while(l<=h)
	{
		fseek(kp, 0L, SEEK_SET);
		mid = l+ (h-l)/2;
		fseek(kp, mid * sizeof(int), SEEK_SET);
		fread(&K, sizeof(int), 1, kp);
		if(K==value)
			return 1;	
		else if(K>value)
			h=mid-1;
		else
			l=mid+1;
	}
	
return 0;
}
