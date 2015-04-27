// ----------------------------------------------------------- 
// NAME : Greg Miller                         User ID: gnmiller 
// DUE DATE : 9/26/14                                      
// PROGRAM ASSIGNMENT 1                                        
// FILE NAME : prog1.h
// PROGRAM PURPOSE :                                           
//		Header file for the associated prog1.c, containing 
//		macros for and function definitions.
// -----------------------------------------------------------

#ifndef gnmiller_prog1_defs
#define gnmiller_prog1_defs

// Define 2 macros for usage in heapSort.
// IS_LESS ... returns the lesser of the values v1, and v2
// SWAP ... sets r = s and s = r
#define IS_LESS(v1, v2)  (v1 < v2)
#define SWAP(r,s)  do{int t=r; r=s; s=t; } while(0)

// function defs
void heapSort(int* heap, int size);
void heapDown(int* heap, int start, int size);
long fibonacci(long n);

#endif