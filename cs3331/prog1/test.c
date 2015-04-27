#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include <stdio.h>
#include <string.h>

long fib(long n);

int main(){
	printf("%d\n",fib(10));
	return 0;
}



long fib(long x) 
{
    if (x < 2)
      return x;
    else 
      return (fib(x - 1) + fib(x - 2));
}


/*
long long fib(int n){
  return n<=2?1:fib(n-1)+fib(n-2);
}*/