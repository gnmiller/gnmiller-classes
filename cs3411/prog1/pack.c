/**
* 
* Greg Miller
* CS3411 Fall 14
* Program 1
* pack.c
* 
* Contains the function definition for pack().
*
*/

#ifndef gnmiller_pack_c
#define gnmiller_pack_c

#include <math.h>
#include <stdlib.h>
#include "defs.h"

#define LIMIT 32

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

/*
 * Pack values into bit fields.
 *
 * Parameters:
 *   s - The bit field widths.
 *   un - The unpacked values.
 *
 * Returns - packed values.
 */
packed pack(sizes s, unpacked un){
	int cb,b_w1,b_w2,w1_val,w2_val,temp,i,j;
	cb=b_w1=b_w2=w1_val=w2_val=temp=i=j=0;
	packed p;
	
	
	//calculate and alloc space
	for ( i = 0; i < s.numWidths; ++i ){
		temp+=s.fieldWidths[i];
	}
	
	temp = max(1,ceil(temp/32)); //word count = bitcount/32 (ceil to get whole #)
	p.fieldValues = malloc(sizeof(int)*temp);
	
	for ( i = 0; i < s.numWidths; ++i ){
		p.fieldValues[i]=0;
	}
	
	for( i = 0; i < s.numWidths; ++i ){
		cb += s.fieldWidths[i]; //increment bit counter
		
		if( i == 0 ){
			
			p.fieldValues[0] += un.values[i];
			
		} else if ( cb <= LIMIT ){
			p.fieldValues[j] = p.fieldValues[j] << s.fieldWidths[i];
			p.fieldValues[j] += un.values[i];			
			
		} else { // cb > 32
		
			b_w1 = LIMIT-(cb-s.fieldWidths[i]);
			b_w2 = s.fieldWidths[i]-b_w1;
			
			w1_val = w2_val = un.values[i];
			
			//trixy hobbits
			//get rid of bits for the first words val...
			w1_val = w1_val >> b_w2;
			//get rid of bits for the second words val...
			w2_val = w2_val << (LIMIT-b_w2);
			w2_val = w2_val >> (LIMIT-b_w2);
			
			p.fieldValues[j] = p.fieldValues[j] << b_w1;
			p.fieldValues[j] += w1_val;
			
			p.fieldValues[j+1] = p.fieldValues[j+1] << b_w2;
			p.fieldValues[j+1] += w2_val;
			
			j+=1;			
			cb = b_w2;			
		}
	}
	
	//clean up
	int leftover = LIMIT-cb;
	p.fieldValues[j] = p.fieldValues[j] << leftover;
	p.n=j+1;
	return p;
}

#endif
