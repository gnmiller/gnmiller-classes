#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "defs.h"

/*
 * Test driver for pack.
 */
int main() {

    // get the field widths
    printf("Enter bit field widths: ");
    sizes sizeData; 
    sizeData.numWidths = 0;
    while ( 1 ) {
	int value;
	scanf("%d", &value);
	if (value <= 0)
	    break;
	while (value > 31) {
	    printf("bit field width must be less than 32, try again: ");
	    scanf("%d", &value);
	    if (value == 0)
		break;
	    }
	if (value == 0)
	    break;
	sizeData.numWidths += 1;
	if (sizeData.numWidths == 1) {
	    sizeData.fieldWidths = (int *) malloc(sizeof(int));
	}
	else {
	    sizeData.fieldWidths = (int *) realloc(sizeData.fieldWidths,sizeData.numWidths*sizeof(int));
	}
	sizeData.fieldWidths[sizeData.numWidths-1] = value;
    }
    if (sizeData.numWidths == 0) {
        printf("you must enter at least one field width\n");
	return 1;
    }

    // get the field values
    unpacked input;
    input.n = sizeData.numWidths;
    printf("Enter %d field values: ", sizeData.numWidths);
    input.values = (int *)malloc(sizeData.numWidths*sizeof(int));
    
	for (int i=0; i<sizeData.numWidths; i+=1) {
		scanf("%d", &(input.values[i]));
	
		while (input.values[i] < 0 || input.values[i] >= pow(2.0,sizeData.fieldWidths[i])) {
	    
			if (input.values[i] < 0) {
				printf("negative values are not allowed, try again: ");
			}
			
			else {
				printf("the value is too big to fit in the field, try again: ");
			}
			
			scanf("%d", &(input.values[i]));
		}
    }
	
    // call the pack function to pack the values into int
    packed p = pack(sizeData,input);
	
    // print the resulting packed array of ints
    for (int i=0; i<p.n; i+=1){
		printf("0x%08x ", p.fieldValues[i]);
    }
    printf("\n");
} // end of main function