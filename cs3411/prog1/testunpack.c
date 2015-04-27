# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <math.h>
# include "defs.h"



/*

 * Test driver for unpack.

 */

int main(){
	// get the field widths

	printf("Enter bit field widths: ");
	sizes sizeData;
	sizeData.numWidths = 0;
	int totalBits = 0;
	while (true) {
		int value;
		scanf("%d", &value);
		if (value <= 0)	break;

		while (value > 31) {
			printf("bit field width must be less than 32, try again: ");
			scanf("%d", &value);
			if (value == 0)	break;
		}

		if (value == 0)	break;
		totalBits += value;
		sizeData.numWidths += 1;
		if (sizeData.numWidths == 1) {
			sizeData.fieldWidths = (int *) malloc(sizeof(int));
		} else {
			sizeData.fieldWidths = (int *) realloc(sizeData.fieldWidths,sizeData.numWidths*sizeof(int));
		}
		sizeData.fieldWidths[sizeData.numWidths-1] = value;
	}

	if (sizeData.numWidths == 0) {
		printf("you must enter at least one field width\n");
		return 1;
	}



	// get the packed words

	int totalWords = (totalBits+31)/32;
	packed input;
	printf("Enter %d word values (in hex): ", totalWords);

	input.fieldValues = (int *)malloc(totalWords*sizeof(int));
	input.n = totalWords;

	for (int i=0; i<totalWords; i+=1) {
		scanf("%x", &(input.fieldValues[i]));
	}
	// call the unpack function to unpack the values into ints

	unpacked un = unpack(sizeData,input);

	// print the resulting unpacked array of ints

	for (int i=0; i<un.n; i+=1) {
		printf("%d ", un.values[i]);
	}

	printf("\n");


} // end of main function
