/*
========================================================================
Name: B-Trees
========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int order;
long q[2000000];   
int front =-1;
int rear =-1;

typedef struct{
	/* B-tree node */ 
	int n; /* Number of keys in node */ 
	int *key; /* Node's keys */
	long *child; /* Node's child subtree offsets */ 
} btree_node; 
btree_node *node;

void searchBTree(long root, int target, int *flag, FILE *fp);
btree_node* get_root(long off, FILE *fp);
int cmpfunc(const void *a, const void *b);
void writing_to_file(btree_node* node,long off,FILE *fp);
long search(btree_node* curr, int target, int *flag, FILE *fp,long root, long *routeoffsets, int *routecount);
void add(btree_node* temp,long off, long root,FILE* fp, int input2, long *routeoffsets, int routecount);
void addToParent(int promoteNode, FILE *fp, long childoff, long *routeoffsets, int routecount);
void newCurrentNode(btree_node* temp,int *leftList,int split);
void rightListCreate(int *rightList, int *duplicatekey, int split);
void leftListCreate(int *leftList,int *list,int split);
void updateChild(long *duplicatechild,btree_node *nnode,long childoff,int len,int mov);
void push( long off);
long pop();
int q_size();
void print ( long root, FILE *fp);

int main(int argc, char* argv[])
{
	FILE *fp, *wp;
	long root;
	fp = fopen(argv[1], "r+b" );
	order = atoi(argv[2]);
	/* If file doesn't exist, set root offset to unknown and create
	* file, otherwise read the root offset at the front of the file */
	if ( fp == NULL ) 
	{ 
		root = -1; 
		fp = fopen( argv[1], "w+b" ); 
		fwrite( &root, sizeof( long ), 1, fp ); 
	}
	else 
	{ 
		fseek(fp,0,SEEK_SET);
		fread( &root, sizeof( long ), 1, fp ); 
	}
	while(1)
	{
		char input1[10];
		int input2;
		scanf("%s", input1);
		if((strcmp(input1,"add"))==0)
		{
			int flag =0;
			scanf("%d", &input2);
			long *routeoffsets = malloc(100000*sizeof(long));
			int routecount =1;
			fseek(fp,0,SEEK_SET);
			fread( &root, sizeof( long ), 1, fp );
			btree_node* temp = get_root ( root, fp);
			*routeoffsets = root;
			long node_off = search(temp, input2, &flag, fp, root, routeoffsets, &routecount);
			if( flag ==1)
			{
				printf("Entry with key=%d already exists\n",input2);
			}
			else
			{
					temp = get_root(node_off,fp);
				if(temp->n < order-1)
				{
					if(root ==-1)
					{
						root = 8;
						fseek(fp,0,SEEK_SET);
						fwrite( &root, sizeof( long ), 1, fp );
					}
					int c = temp->n;
					*(temp->key+c) = input2;
					temp->n++;
					qsort( temp->key, temp->n, sizeof(int), cmpfunc);
					writing_to_file(temp,node_off, fp);
				}
				else
				{
					fseek(fp,0,SEEK_SET);
					fread( &root, sizeof( long ), 1, fp );
					add(temp, node_off, root, fp, input2, routeoffsets, routecount);
				}				
			}
		}
		/*if input is to find key in BTREE */
		else if((strcmp(input1,"find"))==0)
		{
			
			int flag =0;
			fseek(fp,0,SEEK_SET);
			fread( &root, sizeof( long ), 1, fp );
			scanf("%d", &input2);
			searchBTree(root, input2, &flag, fp);  /*search BTREE for key */
			if(flag ==1)
			{
				printf("Entry with key=%d exists\n",input2);
			}
			else
			{
				printf("Entry with key=%d does not exist\n",input2);
			}
		}
		/*if input is to print nodes in BTREE */
		else if((strcmp(input1,"print"))==0)
		{
			fseek(fp,0,SEEK_SET);
			fread( &root, sizeof( long ), 1, fp );
			if(root!=-1)
			{
				print( root, fp);
			}
		}
		/*if input is to end the program */
		 else if((strcmp(input1,"end"))==0)
		{
			fseek(fp, 0, SEEK_SET);
			fread( &root, sizeof( long ), 1, fp );
			fclose(fp);
			break;
		}
		else
		{
			printf("\nenter correct operation to perform on BTREE\n");
		}
	}
}

void add(btree_node* temp,long off, long root,FILE* fp, int input2, long *routeoffsets, int routecount)
{
	int *list = realloc(temp -> key, order * sizeof(int));
    *(list + temp->n) = input2;
    qsort(list,  order, sizeof(int), cmpfunc);
    int split;
	if(order%2==0)
			{
				split = 1 + ((order-1)/2) ;
			}
			else
			{
				split = ((order-1)/2) ;
			}
	/*key value that will be promoted */
	int promoteNode = *(list + split);
	
	/*building left side node list*/
    int *leftList = malloc( split * sizeof(int));
	leftListCreate(leftList,list,split);
	
	/*building the right side node list*/
    int *rightList = malloc((order-split-1)* sizeof(int));
	rightListCreate(rightList, list, split);
	
	/*changing the current node to left side node list*/
	newCurrentNode(temp,leftList,split);
    
	temp->n = split;
	
	/*writting left nodes back to file*/
	writing_to_file( temp, off, fp);
	fseek(fp,0,SEEK_END);
	long rightNode_off = ftell(fp);
	
	/*right side node creation*/
	btree_node *rightNode = (btree_node *) malloc(sizeof(btree_node));
	rightNode->n = order-split-1;
	rightNode->key = (int *)realloc(rightList, (order - 1)*sizeof(int));
	rightNode->child = (long *) calloc(order, sizeof( long ) );
	
	writing_to_file(rightNode, rightNode_off, fp);
	
	/*function to add promoted key value to upper node and update childs*/
	addToParent( promoteNode, fp, rightNode_off, routeoffsets, routecount);
}

void addToParent(int promoteNode, FILE *fp, long childoff, long *routeoffsets, int routecount)
{
	/*if parent node is not root node*/
	if( routecount != 1)
	{
		long nnodeoff = *(routeoffsets+routecount-2);
		btree_node *nnode = get_root(nnodeoff, fp);
		int k;
		int len = nnode->n;
		int mov = 0;
		for(k=0;k<len;k++)
		{
			if(*(nnode->key+k)<promoteNode)
			{
				mov++;
			}
		}
		if(len < order-1)
		{
			nnode->n++;
			*(nnode->key+len) = promoteNode;
			qsort(nnode->key, nnode->n, sizeof(int), cmpfunc);
			if( mov == len )
			{
				*(nnode->child+mov+1) = childoff;
			}
			else
			{
				long *duplicatechild = malloc((len+2)*sizeof(long));
				updateChild(duplicatechild,nnode,childoff,len,mov);
                nnode -> child = duplicatechild;
			}
			routecount--;
			writing_to_file(nnode, nnodeoff, fp);
		}
		else
		{
			int *duplicatekey = malloc(order *sizeof(int));
			int m;
			for(m=0;m<len; m++)
			{
				*(duplicatekey+m) = *(nnode->key+m);
			}
			*(duplicatekey+len) = promoteNode;
			qsort(duplicatekey, order, sizeof(int), cmpfunc);
			int split;
			if(order%2==0)
			{
				split = 1 + ((order-1)/2) ;
			}
			else
			{
				split = ((order-1)/2) ;
			}
			
			/*building the left side node list*/
			int *leftList = malloc( split * sizeof(int));
			leftListCreate(leftList,duplicatekey,split);
			newCurrentNode(nnode,leftList,split);
			nnode->n = split;
			/*building the right side node list*/
			int *rightList = malloc((order-split-1)* sizeof(int));
			rightListCreate(rightList,duplicatekey,split);
			
			long *newchild = malloc((len+2)*sizeof(long));
			if( mov == len )
			{
				newchild = nnode->child;
				*(newchild+len+1) = childoff;
			}
			else
			{
				updateChild(newchild,nnode,childoff,len,mov);
			}

			btree_node *temp_node = (btree_node *) malloc(sizeof(btree_node));
			temp_node->n = order - split -1;
			temp_node->key = (int *)realloc(rightList,(order-1)*sizeof(int));
			temp_node->child = (long *)calloc(order, sizeof(long));
			int ii;
			for(ii =0;ii<order-split;ii++)
			{
				*(temp_node->child+ii) = *(newchild+ii+split +1);
			}
			int q;
			for(q=0;q<order;q++)
			{
				if(q<=split)
				{
					*(nnode->child+q) = *(newchild +q);
				}
				else
				{
					*(nnode->child +q) = 0;
				}
			}
			writing_to_file(nnode, *(routeoffsets+routecount-2), fp);
			fseek(fp,0,SEEK_END);
			long f_off = ftell(fp);
			writing_to_file(temp_node, f_off, fp);
			
			int promoteNode = *(duplicatekey+split);
			routecount--;
			addToParent(promoteNode, fp, f_off, routeoffsets, routecount);
		}
	}
	/*if we need to split the root node*/
	else
	{
		btree_node *newroot = (btree_node *) malloc(sizeof(btree_node));
		newroot->n = 1;
		newroot->key = (int *) calloc( order - 1, sizeof( int ) ); 
		newroot->child = (long *) calloc( order, sizeof( long ) );
		
		*(newroot->key) = promoteNode;
		*(newroot->child) = *routeoffsets;
		*(newroot->child +1) = childoff;
		fseek(fp, 0, SEEK_END);
		long rootoff = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fwrite(&rootoff, sizeof(long), 1 , fp);
		writing_to_file(newroot, rootoff, fp);
	}
}

long search(btree_node* curr, int target, int *flag, FILE *fp,long root, long *routeoffsets, int *routecount)
{
	int s =0;
	long off = root;
	while(s < curr->n)
	{
		if(*(curr->key+s)==target)
		{
			*flag = 1;
			return off;
		}
		else if(*(curr->key+s)>target)
			break;
		else
			s++;
	}
	if(*(curr->child+s)!=0)
	{
		off = *(curr->child+s);
		*(routecount) += 1;
		long *route = realloc(routeoffsets, (*(routecount))* sizeof(long)); /*if not worked change long to int*/
		*(route+*(routecount)-1) = off;
		routeoffsets= route;
		btree_node* temp = get_root(off,fp);
		return search(temp, target, flag, fp, off, routeoffsets, routecount); /*changed here root to off*/
	}
	else
		return off;
	return off;
}

void searchBTree(long root, int target, int *flag, FILE *fp)
{
	int sub_tree =0;
	long off;
	btree_node *curr = get_root(root, fp);
	while(sub_tree < curr->n)
	{
		if(*(curr->key+sub_tree)==target)
		{
			*flag = 1;
			return;
		}
		else if(*(curr->key+sub_tree)>target)
			break;
		else
			sub_tree++;
	}
	if(*(curr->child+sub_tree)!=0)
	{
		off = *(curr->child+sub_tree);
		return searchBTree(off, target, flag, fp);
	}
	else
		return;
	return;
}

btree_node* get_root(long off, FILE *fp)
{
	btree_node *temp =  (btree_node *) malloc(sizeof(btree_node));
	temp->n = 0;
	temp->key = (int *) calloc( order - 1, sizeof( int ) ); 
	temp->child = (long *) calloc( order, sizeof( long ) );
	if(off==-1)
	{
		;
	}
	else
	{
		fseek(fp, off, SEEK_SET);
		fread( &(temp->n), sizeof( int ), 1, fp );	
		fread( temp->key, sizeof( int ), order - 1, fp ); 
		fread( temp->child, sizeof( long ), order, fp );
	}
	return temp;
}

void writing_to_file(btree_node* node, long off, FILE *fp)
{
	fseek(fp, off, SEEK_SET);
	fwrite( &(node->n), sizeof( int ), 1, fp ); 
	fwrite( node->key, sizeof( int ), order - 1, fp ); 
	fwrite( node->child, sizeof( long ), order, fp);
}

void print ( long root, FILE *fp)
{
	
	push(root);
	int level=1;
	while(front != -1 && rear !=-1)
	{
		int size = q_size();
		int k;
		printf(" %d: ",level);
		for(k=0;k<size;k++)
		{
			int m;
			long off = pop();
			btree_node* temp = get_root(off, fp);
			int len = temp->n;
			for(m=0;m<len-1;m++)
			{
				printf( "%d,", *(temp->key+m));
			}
			printf( "%d", *(temp->key+m));
			for(m=0;m<=len;m++)
			{
				if(*(temp->child+m) != 0)
				{
					push( *(temp->child+m));
				}
			}
			printf(" ");
		}
		level++;
		printf("\n");
	}
}

void leftListCreate(int *leftList,int *list,int split)
{
	int i=0;
    while(i<split)
	{
        *(leftList+i) = *(list+i);
		i++;
    }
}

void rightListCreate(int *rightList, int *duplicatekey, int split)
{
	int j=0;
	while(j < order - split - 1)
	{
		*(rightList+j) = *(duplicatekey+split+1+j);
		j++;
	}
}
void newCurrentNode(btree_node* temp,int *leftList,int split)
{
	int k=0;
	while(k<(order-1))
	{
		if(k < split)
		{
			*(temp -> key + k) = *(leftList + k);
		}	
		else
		{
            *(temp -> key + k) = 0;
        }
		k++;
    }
}

void updateChild(long *duplicatechild,btree_node *nnode,long childoff,int len,int mov)
{
	int a=0;
	int b=0;
	while(a < len + 1)
    {
        if(a == mov + 1)
		{
            *(duplicatechild+b) = childoff;
            mov = -3;
            b++;
        }
        else
		{
			*(duplicatechild+b) = *(nnode -> child + a);
            a++;
            b++;
        }
    }
}

void push( long off)
{
	if( front == -1 && rear ==-1)
	{
		front = rear =0;
	}
	else
		rear++;
	q[rear] = off;
}

long pop()
{
	long res = q[front];
	front++;
	if(front> rear)
		front = rear = -1;
	return res;
}

int q_size()
{
	return (rear+1 - front);
}

int cmpfunc(const void *a, const void *b)
{
	return ( *(int*)a - *(int*)b );
}
