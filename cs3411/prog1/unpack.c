/**
* 
* Greg Miller
* CS3411 Fall 14
* Program 1
* unpack.c
* 
* Contains the function definition for unpack().
*
*/

#ifndef gnmiller_unpack_c
#define gnmiller_unpack_c

#include <math.h>
#include <stdlib.h>
#include "defs.h"

#define LIMIT 32
#define ZERO 0

/*
 * Unpack values from bit fields.
 *
 * Parameters:
 *   s - The bit field widths.
 *   p - The packed values.
 *
 * Returns - unpacked values.
 */

 
unpacked unpack(sizes s, packed p) {
	int cb, i, k, temp1, temp2, b_p, b_n;
	cb = i = k = temp1 = temp2 = b_p = b_n = 0;
	unsigned int j;
	unpacked un;
	un.n = s.numWidths;
	un.values = (int*) malloc( sizeof(int) * un.n );
	
	cb=0;
	k=0;

	for ( i = 0; i < s.numWidths; ++i ){
		
		cb += s.fieldWidths[i];
		
		if( cb > LIMIT ){
			
			unsigned int temp1shift,temp2shift;
			temp1shift = temp2shift = 0;
			
			temp2shift = cb - LIMIT;
			temp1shift = s.fieldWidths[i] - temp2shift;
			
			temp1 = p.fieldValues[k];
			temp2 = p.fieldValues[k+1];
			
			temp1 = temp1 << LIMIT - temp1shift;
			temp1 = temp1 >> LIMIT - temp1shift;
			temp1 = temp1 << temp2shift; // make room for those bits
			
			temp2 = (unsigned)temp2 >> LIMIT - temp2shift;
			// temp2 = temp2 << LIMIT - temp2shift;
						
			un.values[i] = (unsigned)temp1+temp2;
			cb = temp2shift;			
			
			k += 1;	
		} else {
			j = p.fieldValues[k] << (cb-s.fieldWidths[i]);	
			j = j >> (cb-s.fieldWidths[i]);
			j = j >> (LIMIT-cb);
			un.values[i] = j;		
		}
		
	}
	return un;
	
} // end of unpack function 

#endif
