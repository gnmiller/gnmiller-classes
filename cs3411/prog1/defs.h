/**
* 
* Greg Miller
* CS3411 Fall 14
* Program 1
* defs.h
*
* Contains function prototypes and struct definitions.
*
*/

#ifndef gnmiller_defs_h
#define gnmiller_defs_h

// a structure that contains field widths
typedef struct {
    int * fieldWidths;   // a pointer to an array of bit field widths (in bits)
    int numWidths;       // the number of elements in the array (i.e., the number of bit fields)
} sizes;

// a structure that contains an array of ints containing packed data fields
typedef struct {
    int * fieldValues; // a pointer to an array of ints containing packed bit fields
    int n;                   // the number of elements in the array
} packed;

// a structure that contains an array of ints containing individual data values (one per int)
typedef struct {
    int * values;      // a pointer to an array of ints containing values for bit fields (one per element)
    int n;                 // the number of elements in the array
} unpacked;

//func prototypes
packed pack(sizes s, unpacked un); 
unpacked unpack(sizes s, packed p);

#endif
