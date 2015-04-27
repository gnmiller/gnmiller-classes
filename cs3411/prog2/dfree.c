#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "dfile.h"

/**
 * Free space in an open file.
 *
 * @param df A pointer to the DFile structure for the file.
 * @ param freeptr A file pointer to the free block (the data part).
 *
 * @return -1 if an error, 0 otherwise
 */
int dfree(DFile* df, off_t freeptr) {
	// find the nodes place in the list and update relevant pointers
	// check if its the new biggest as well
	if (df == NULL) {
		errno = EINVAL;
		return -1;
	}
	
	if(df->fd <= 0) return -1;
	
	off_t _free,nextptr,prevptr,myfptr;
	int mysize,nextsize,fd,freesize,prevsize;
	fd = df->fd;
	myfptr = freeptr - sizeof(int) - sizeof(off_t); // change freeptr to reference the header and not the data
	
	// get some info on our node
	lseek(fd,myfptr,SEEK_SET);
	lseek(fd,sizeof(off_t),SEEK_CUR);
	read(fd,&mysize,sizeof(int));
	lseek(fd,-sizeof(int),SEEK_CUR);
	mysize = mysize * -1;
	write(fd,&mysize,sizeof(int)); // update
	lseek(fd,mysize,SEEK_CUR);
	write(fd,&mysize,sizeof(int)); // update
	lseek(fd,sizeof(off_t),SEEK_CUR);
	
	// get free node
	lseek(fd,0,SEEK_SET);
	read(fd,&_free,sizeof(off_t));
	
	// search for a node such that node.size > size > node.next.size
	lseek(fd,_free,SEEK_SET);
	lseek(fd,sizeof(off_t),SEEK_CUR);
	read(fd,&nextsize,sizeof(int));
	
	lseek(fd,_free,SEEK_SET);
	lseek(fd,sizeof(off_t),SEEK_CUR);
	lseek(fd,sizeof(int),SEEK_CUR);
	lseek(fd,freesize,SEEK_CUR);
	lseek(fd,sizeof(int),SEEK_CUR);
	read(fd,&nextptr,sizeof(off_t));
	freesize = nextsize;
	// next == free.next
	// nextsize == free.size
	
	if(nextptr != 0 || mysize > freesize){ // search for the place to free to (if were bigger than free skip this crap)
		lseek(fd,nextptr,SEEK_SET);
		off_t holdptr;
		// we are at free.next
 		// we are def smaller than free and there is def other nodes in the list
		while( mysize < nextsize && nextptr > 0){
			lseek(fd,nextptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			read(fd,&nextsize,sizeof(int));
			lseek(fd,nextsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			if(nextsize < mysize) break;
			holdptr = nextptr;
			read(fd,&nextptr,sizeof(off_t));
		} // nextptr.size > size > nextptr.next.size
		
		
		if(nextptr == 0 && holdptr != 0){ // we are the last node
			nextptr = holdptr;
			
			// update freeptr.prev
			lseek(fd,nextptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			read(fd,&nextsize,sizeof(int));
			lseek(fd,nextsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			read(fd,&prevptr,sizeof(off_t));
			
			lseek(fd,nextptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			lseek(fd,nextsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&myfptr,sizeof(off_t));
			
			// update free
			lseek(fd,myfptr,SEEK_SET);
			write(fd,&nextptr,sizeof(off_t));
			lseek(fd,sizeof(int),SEEK_CUR);
			lseek(fd,mysize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			off_t tmp;
			tmp = (off_t)0;
			write(fd,&tmp,sizeof(off_t));
			
			//update next
			//there is no next...
			return 0;
		} else { // we go in the middle
			nextptr = holdptr; // sets it such that freed -> next
			
			// update the node after freed node
			lseek(fd,nextptr,SEEK_SET);
			read(fd,&prevptr,sizeof(off_t));
			lseek(fd,nextptr,SEEK_SET);
			write(fd,&myfptr,sizeof(off_t));
			
			// update the node before the freed node
			lseek(fd,prevptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			read(fd,&prevsize,sizeof(int));
			lseek(fd,prevsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			read(fd,&prevptr,sizeof(off_t));
			
			lseek(fd,prevptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			lseek(fd,prevsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&myfptr,sizeof(off_t));
			
			// update the free node
			lseek(fd,myfptr,SEEK_SET);
			write(fd,&prevptr,sizeof(off_t));
			lseek(fd,sizeof(int),SEEK_CUR); // updated size already..
			lseek(fd,mysize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&nextptr,sizeof(off_t));
			
			return 0;		
		}
	} else { // only one node in the list
		if(mysize > freesize) {
 			// free node is not biggest now
			
			// update the free node
			lseek(fd,_free,SEEK_SET);
			write(fd,&myfptr,sizeof(off_t));
			
			// update the freed node
			lseek(fd,myfptr,SEEK_SET);
			off_t tmp;
			tmp = (off_t)0;
			write(fd,&tmp,sizeof(off_t));
			lseek(fd,sizeof(int),SEEK_CUR);
			lseek(fd,mysize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&myfptr,sizeof(off_t));
			
			// update the free pointer
			lseek(fd,0,SEEK_SET);
			write(fd,&myfptr,sizeof(off_t));
			
			return 0;
		} else {
			// free node holds position
			
			// update free node
			lseek(fd,_free,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			lseek(fd,freesize,SEEK_SET);
			lseek(fd,sizeof(int),SEEK_CUR);
			read(fd,&nextptr,sizeof(off_t));
			
			lseek(fd,_free,SEEK_SET);
			lseek(fd,sizeof(int)+sizeof(off_t),SEEK_CUR);
			lseek(fd,freesize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&myfptr,sizeof(off_t));
			
			// update next node in free list eg freed.next
			if(nextptr!=0){
				lseek(fd,nextptr,SEEK_SET);
				write(fd,&myfptr,sizeof(off_t));
			}
			
			// update freed node
			lseek(fd,myfptr,SEEK_SET);
			write(fd,&_free,sizeof(off_t));
			read(fd,&nextsize,sizeof(int));
			lseek(fd,nextsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&nextptr,sizeof(off_t)); // we either get 0 or the next here signifying our spot as last or not
			
			return 0;
		}
	
	}
	return -1; // if we get here its an error (jsut to be safe)
}