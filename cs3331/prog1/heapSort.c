#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define IS_LESS(v1, v2)  (v1 < v2)
#define SWAP(r,s)  do{int t=r; r=s; s=t; } while(0)

void heapSort(int* heap, int size);
void heapDown(int* heap, int start, int end);

int main(int argc, char** argv){
    char* print;
    int* rand_arr;
    char* buf;
    buf=malloc(sizeof(char)*100);
    int buf_max;
    int i;
    print="";
    rand_arr=malloc(10*sizeof(int));
    for (i = 0; i < 10; ++i) rand_arr[i] = i;
    for (i = 0; i < 10; ++i) {      
     
      if (strlen(buf) >= buf_max-1 ){
	buf_max = buf_max*2;
	//buf = realloc( buf, sizeof(char)*buf_max );
     }
      char* tmp;
      tmp = malloc(sizeof(char)*100);
      sprintf(tmp,"%d ",rand_arr[i]);
      
      if (strlen(buf) >= buf_max-1 ){
	buf_max = buf_max*2;
	//buf = realloc( buf, sizeof(char)*buf_max );
     }
     
      buf = strcat(buf,tmp);
    }
    
    write(1,buf,strlen(buf));
    write(1,"\n",1);
}

// ----------------------------------------------------------- 
// FUNCTION  heapSort                         
//     Sorts the given array (heap) using the heapsort
//	algorithm.
// PARAMETER USAGE :                                           
//    int* heap: The array to sort
//    int size: The size of the array to sort
// FUNCTION CALLED :                                           
//    heapDown()          
// ----------------------------------------------------------- 
void heapSort(int* heap, int size){
  int start,end;
  for ( start = (size-2)/2; start >= 0; --start ){
   
    heapDown(heap, start, size);
    
  }
  
  for ( end = size-1; end > 0; --end ) {
  
    SWAP(heap[end],heap[0]);
    heapDown(heap,0,end);
    
  }
    
}

// ----------------------------------------------------------- 
// FUNCTION  heapDown                        
//     Rebuilds the tree after an insertion
// PARAMETER USAGE :                                           
//    int* heap: The array being heapSorted
//    int start: The index to begin building at
//    int size: The index to stop building at
// FUNCTION CALLED :                                           
//         
// -----------------------------------------------------------
void heapDown( int* heap, int start, int end){
  int root = start;
  
  while ( root*2+1 < end ) {
    int child = 2*root+1;
    
    if ( ( child + 1 < end) && IS_LESS(heap[child], heap[child+1] ) ){
    
      child += 1;
      
    }
    
    if( IS_LESS(heap[root], heap[child] ) ){
    
      SWAP( heap[child], heap[root] );
      root = child;
  
    } else return;
  
  }  
  
}