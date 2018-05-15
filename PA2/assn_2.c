/*
========================================================================
Name: In-Memory Indexing with Availability Lists
========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{ 
	int key;  
	long off; 
}index_S;

typedef struct 
{ 
	int siz; /* Record's key */ 
	long off; /* Record's offset in file */ 
}avail_S;

int check_index(index_S arr[],int l, int h, int n);
int cmpfunc(const void *a, const void *b);
void worstfit(avail_S avail_arr[]);
void bestfit(avail_S avail_arr[]);
int cmpfunc_worst(const void *a, const void *b);
int cmpfunc_best(const void *a, const void *b);

index_S index_arr[10000];
avail_S avail_arr[10000];
index_S buffer;
avail_S buffer_avail;
int count=0;
int avail_count=0;
int type;

/*
========================================================================
			Main Function
========================================================================
*/

int main(int argc, char* argv[])
{
	FILE *ifp;
	FILE *afp;
	FILE *fp, *pp;
	if((fp=fopen(argv[2],"r+b"))==NULL)
	{
		fp=fopen(argv[2],"a+b");
		ifp=fopen("index.bin","w+b");
		afp=fopen("available.bin","w+b");
	}	
	if((ifp=fopen("index.bin","r+b"))==NULL)
	{
		;
	}
	else
	{
		while(fread(&buffer, sizeof(index_S), 1, ifp)/*!(feof(ifp))*/)
		{
			index_arr[count].key=buffer.key;
			index_arr[count].off=buffer.off;
			count++;
		}
	}
	if((afp=fopen("available.bin","r+b"))==NULL)
	{
		;
	}
	else
	{
		while(fread(&buffer_avail, sizeof(avail_S), 1, afp))
		{
			avail_arr[avail_count].siz=buffer_avail.siz;
                        avail_arr[avail_count].off=buffer_avail.off;
                        avail_count++;	
		}
	}


	if(strcmp(argv[1],"--first-fit")==0)
	{
		type=1;
	}
	else if(strcmp(argv[1],"--best-fit")==0)
	{
		type=2;
		bestfit(avail_arr);
	}
	else if(strcmp(argv[1],"--worst-fit")==0)
	{
		type=3;
	}
	else
	{
		printf("enter correct ordering strategy");
	}
	
	fp = fopen( argv[2], "a+b" );
	
	while(1)
	{
		char input1[10],input3[100];
		int input2;
		int ret;
			
		scanf("%s", input1);
		
		/* if given input is to add key*/
		if((strcmp(input1,"add"))==0)
		{
			scanf("%d", &input2);
			scanf("%s", input3);
			ret=check_index(index_arr,0, count, input2);
			long offset=0;
			if(ret==0)
			{
				index_arr[count].key=input2;
				int rec_size=strlen(input3);
				int rec_size_total = sizeof(int) + rec_size;
				int j,k;
				int check=0;
				for(j=0;j<avail_count;j++)
				{
					if(avail_arr[j].siz>=rec_size_total)
					{
						check=1;
						int rem_hole=avail_arr[j].siz-rec_size_total;
						long rem_hole_offset=avail_arr[j].off;
						offset=avail_arr[j].off;
						for(k=j;k<avail_count;k++)
						{
							avail_arr[k].siz=avail_arr[k+1].siz;
							avail_arr[k].off=avail_arr[k+1].off;
						}
						if(rem_hole>0)
						{
							if(type==1)
							{
								avail_arr[avail_count-1].siz=rem_hole;
								avail_arr[avail_count-1].off=rem_hole_offset + sizeof(int) + rec_size;
							}
							else if(type==2)
							{
								avail_arr[avail_count-1].siz=rem_hole;
								avail_arr[avail_count-1].off=rem_hole_offset + sizeof(int) + rec_size;
								bestfit(avail_arr);
							}
							else if(type==3)
							{
								avail_arr[avail_count-1].siz=rem_hole;
								avail_arr[avail_count-1].off=rem_hole_offset + sizeof(int) + rec_size;
								worstfit(avail_arr);		
							}
							break;
						}
						else
						{
							avail_count--;
							break;
						}
					}
					
				}	
				if(check==0)
				{			
					fseek(fp, 0, SEEK_END);
                                        offset=ftell(fp);
				}
				pp = fopen( argv[2], "r+b" );
				index_arr[count].off=offset;
				fseek(pp, offset, SEEK_SET);
                                fwrite(&rec_size, sizeof(int), 1, pp);
                                fwrite(&input3, rec_size, 1, pp);
				count++;
				qsort(index_arr, count, sizeof(index_S), cmpfunc);
				fclose(pp);
				
			}
			else
			{
				printf("Record with SID=%d exists\n", input2);
			}
		}
		/*If given input is to find key*/
		else if((strcmp(input1,"find"))==0)
		{
			scanf("%d", &input2);
			int find_res=check_index(index_arr,0, count, input2);			
			if(find_res==1)
			{
				int k,n,rec_size_find;
				long pos;
				char output[100];
				for(k=0;k<count;k++)
				{
					if(index_arr[k].key==input2)
					{
						pos=index_arr[k].off;
						break;
					}
				}
				fseek(fp, pos, SEEK_SET);
				fread(&rec_size_find, sizeof(int), 1, fp);
				fread(&output, rec_size_find, 1, fp);
				for(n=0;n<rec_size_find;n++)	
				{
					printf("%c", output[n]);
				}
				printf("\n");
			}
			else
			{
				printf("No record with SID=%d exists\n",input2);
			}
		}
		/*If given input is to delete a key*/
		else if((strcmp(input1,"del"))==0)
		{
			scanf("%d", &input2);
                        int find_res=check_index(index_arr,0, count, input2);
                        if(find_res==1)
                        {
				int j,k,rec_size_del;
                                long pos;
                                char output[100];
                                for(k=0;k<count;k++)
                                {
                                        if(index_arr[k].key==input2)
                                        {
                                                pos=index_arr[k].off;
                                        	break;
					}
                                }	
				fseek(fp, pos, SEEK_SET);
                                fread(&rec_size_del, sizeof(int), 1, fp);
				avail_arr[avail_count].siz = rec_size_del+sizeof(int);
				avail_arr[avail_count].off = pos;
				avail_count++;
				for(j=k;j<count;j++)
				{
					index_arr[j].key = index_arr[j+1].key;
					index_arr[j].off = index_arr[j+1].off;	
				}
				count--;

			}
			else
                        {
                                printf("No record with SID=%d exists\n",input2);
                        }
		}
		/*If input is to end the input arguments*/
		else if((strcmp(input1,"end"))==0)
		{
			int k;
			if(type==2)
			{
				bestfit(avail_arr);
			}
			else if(type==3)
			{
				worstfit(avail_arr);
			}
                        printf("Index:\n");
                        for(k=0;k<count;k++)
                        {
                                printf( "key=%d: offset=%ld\n", index_arr[k].key, index_arr[k].off );
                        }
                        printf("Availability:\n");
                        for(k=0;k<avail_count;k++)
                        {
                                printf( "size=%d: offset=%ld\n", avail_arr[k].siz, avail_arr[k].off );
                        }
                        printf( "Number of holes: %d\n", avail_count );
                        int total_hole_size=0;
                        for(k=0;k<avail_count;k++)
                        {
                                total_hole_size += avail_arr[k].siz;
                        }
                        printf( "Hole space: %d\n", total_hole_size );
			ifp=fopen("index.bin","w+b");
			fwrite(&index_arr,sizeof(index_S), count, ifp);
			fclose(ifp);	
			afp=fopen("available.bin","w+b");
			fwrite(&avail_arr,sizeof(avail_S), avail_count, afp);
			fclose(afp);				
			
			break;
		}		
	}	
}

int check_index(index_S arr[],int l, int h, int n)
{
	int mid;
	while(l<=h)
	{
		mid=l+(h-l)/2;
		if(arr[mid].key==n)
			return 1;
		else if(arr[mid].key>n)
			h=mid-1;
		else 
			l=mid+1;
	}
return 0;
}

int cmpfunc(const void *a, const void *b)
{
	return ( *(int*)a - *(int*)b );
}
int cmpfunc_best(const void *a, const void *b)
{
	int l=((avail_S *)a)->siz;
	int r=((avail_S *)b)->siz;
	if((l-r)==0)
	{
		long m=((avail_S *)a)->off;
		long n=((avail_S *)b)->off;
		return ((int)(m-n));
	}
	return (l-r);
}
int cmpfunc_worst(const void *a, const void *b)
{
	int l=((avail_S *)a)->siz;
        int r=((avail_S *)b)->siz;
        if((r-l)==0)
        {
                long m=((avail_S *)a)->off;
                long n=((avail_S *)b)->off;
                return ((int)(n-m));
        }
        return (r-l);

}
void bestfit(avail_S avail_arr[])
{
	qsort(avail_arr, avail_count, sizeof(avail_S), cmpfunc_best);		
}
void worstfit(avail_S avail_arr[])
{
	qsort(avail_arr, avail_count, sizeof(avail_S), cmpfunc_worst);
}
