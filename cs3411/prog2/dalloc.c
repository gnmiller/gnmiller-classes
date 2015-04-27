#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "dfile.h"

/**
 * Allocate space in an open file.
 *
 * @param df A pointer to the DFile structure for the file.
 * @param size The number of bytes to allocate (must be a multiple of 4).
 *
 * @return a file pointer to the first data byte, or -1 if an error.
 *         "errno" is set to EINVAL if the size is invalid.
 *         "errno" is set to ENOMEM if there isn't enough space.
 * @note Currently does not function correctly after a call to dfree().
 *		- Does not correctly find where to allocate a node
 */
off_t dalloc(DFile* df, int size) {
	
	off_t freeptr,nextptr,prevptr,holdptr,ret;
	int freesize, nextsize, invsize;
	int fd;
	fd = df->fd;
	invsize = size * -1;
	
	if( fd <= 0 ){
		errno = ENOENT;
		return -1;
	} // error
	
	if( size % 4 != 0 ){
		errno = EINVAL;
		return -1;
	} // error
	
	lseek(fd,0,SEEK_SET);
	read(fd,&freeptr,sizeof(off_t));
	lseek(fd,freeptr,SEEK_SET);
	lseek(fd,sizeof(off_t),SEEK_CUR);
	read(fd,&nextsize,sizeof(int));
	freesize = nextsize;	
	
	if(freeptr == 0){
		errno = ENOMEM;
		return -1;
	}
	
	if(freesize < size){ // too big ...
		errno = ENOMEM;
		return -1;
	}
	
	if(size == freesize){ // fit perfectly in first node
		ret = freeptr + sizeof(off_t) + sizeof(int);
		
		// alloc the node
		lseek(fd,freeptr,SEEK_SET);
		lseek(fd,sizeof(off_t),SEEK_CUR);
		write(fd,&invsize,sizeof(int));
		lseek(fd,sizeof(size),SEEK_CUR);
		write(fd,&invsize,sizeof(int));
		read(fd,&nextptr,sizeof(off_t));
		
		// update the free list
		if(nextptr == 0){
			lseek(fd,0,SEEK_SET);
			write(fd,0,sizeof(off_t)); // free had no next thus it was the only free node
		} else {
			lseek(fd,nextptr,SEEK_SET);
			write(fd,0,sizeof(off_t));
			lseek(fd,0,SEEK_SET);
			write(fd,&nextptr,sizeof(off_t));
		}
		
	} else { // find our spot
		lseek(fd,freeptr,SEEK_SET);
		lseek(fd,sizeof(off_t),SEEK_CUR);
		lseek(fd,sizeof(int),SEEK_CUR);
		lseek(fd,freesize,SEEK_CUR);
		lseek(fd,sizeof(int),SEEK_CUR);
		while(size <= nextsize){ // find the next largest node
			read(fd,&nextptr,sizeof(off_t));
			if(nextptr == 0) break;
			lseek(fd,nextptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			read(fd,&nextsize,sizeof(int));
			lseek(fd,nextsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
		} // nextptr > free (by one slot)
		if(nextptr == 0){ // split the free node
				
			int sizeNewNode,sizeNewFree;
			sizeNewNode = sizeof(off_t)*2+sizeof(int)*2+size; // size + overhead
			sizeNewFree = (freesize - sizeNewNode) + sizeof(int)*2 + sizeof(off_t)*2;// + sizeof(off_t)*2 + sizeof(int)*2;
			
			// offset from free pointer to the new node...
			ret = freeptr + sizeNewFree + sizeof(off_t) + sizeof(int);

			// get free pointers size and next
			lseek(fd,freeptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			read(fd,&freesize,sizeof(int));
			lseek(fd,freesize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			read(fd,&holdptr,sizeof(off_t)); // hold onto free.next
			lseek(fd,-sizeNewNode,SEEK_CUR); // go back from the end of free
			
			// allocate the node
			lseek(fd,sizeof(off_t),SEEK_CUR);
			write(fd,&invsize,sizeof(int));
			lseek(fd,size,SEEK_CUR);
			write(fd,&invsize,sizeof(int));
			lseek(fd,sizeof(off_t),SEEK_CUR);
			
			if(holdptr != 0){
				// search for a node such that node.size > size > node.next.size
				lseek(fd,holdptr,SEEK_SET);
				lseek(fd,sizeof(off_t),SEEK_CUR);
				read(fd,&nextsize,sizeof(int));
				
				if(nextsize > freesize){ // if the free node is no longer the largest we need to move it
					
					while( (freesize-size) < nextsize){ // find the next largest node
						lseek(fd,nextsize,SEEK_CUR);
						lseek(fd,size,SEEK_CUR);
						read(fd,&nextptr,sizeof(off_t));
						if(nextptr == 0) break;
						lseek(fd,nextptr,SEEK_SET);
						lseek(fd,sizeof(off_t),SEEK_CUR);
						read(fd,&nextsize,sizeof(int));
					} // nextptr.size > size > nextptr.next.size

					// edit nextptr (free.prev)
					lseek(fd,nextptr,SEEK_SET);
					lseek(fd,sizeof(off_t),SEEK_CUR);
					lseek(fd,sizeof(int),SEEK_CUR);
					lseek(fd,nextsize,SEEK_CUR);
					lseek(fd,sizeof(int),SEEK_CUR);
					read(fd,&prevptr,sizeof(off_t));
					lseek(fd,-sizeof(int),SEEK_CUR);
					write(fd,&freeptr,sizeof(off_t));
					
					// edit prevptr (free.next)
					lseek(fd,prevptr,SEEK_SET);
					read(fd,&nextptr,sizeof(off_t));
					lseek(fd,-sizeof(off_t),SEEK_CUR);
					write(fd,&freeptr,sizeof(off_t));
					
					// edit free ptr now
					int freesizenew;
					freesizenew = freesize-size;
					lseek(fd,freeptr,SEEK_SET);
					write(fd,&prevptr,sizeof(off_t));
					write(fd,&freesizenew,sizeof(int));
					lseek(fd,freesize-size,SEEK_CUR);
					write(fd,&freesizenew,sizeof(int));
					write(fd,&nextptr,sizeof(off_t));
					
					// update the free ptr and the new free node
					lseek(fd,0,SEEK_SET);
					write(fd,&holdptr,sizeof(off_t));
					lseek(fd,holdptr,SEEK_SET);
					write(fd,0,sizeof(off_t)); // you are now last...
					
					return ret;
				} else { // free node is still the largest node
					int freesizenew;
					freesizenew = freesize-size;
					// update the free node (just size, prev/next and free pointer all stay the same here)
					lseek(fd,freeptr,SEEK_SET);
					lseek(fd,sizeof(off_t),SEEK_CUR);
					write(fd,&freesizenew,sizeof(int));
					lseek(fd,freesize-size,SEEK_CUR);
					write(fd,&freesizenew,sizeof(int));
					
					return ret;
				}
			} else { // only 1 node in the free list ...
				int freesizenew;
				freesizenew = freesize-(size + sizeof(int)*2 + sizeof(off_t)*2);
				// update the free node (just size, prev/next and free pointer all stay the same here)
				lseek(fd,freeptr,SEEK_SET);
				lseek(fd,sizeof(off_t),SEEK_CUR);
				write(fd,&freesizenew,sizeof(int));
				lseek(fd,freesizenew,SEEK_CUR);
				write(fd,&freesizenew,sizeof(int));
				
				return ret;		
			}
		} else if (nextsize == size){ // we found a match
			ret = nextptr + sizeof(off_t) + sizeof(int);
			
			// allocate the node
			holdptr = nextptr;
			lseek(fd,holdptr,SEEK_SET);
			read(fd,&prevptr,sizeof(off_t));
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&invsize,sizeof(int));
			lseek(fd,size,SEEK_CUR);
			write(fd,&invsize,sizeof(int));
			read(fd,&nextptr,sizeof(off_t));
			
			// update the previous node in the list
			lseek(fd,prevptr,SEEK_SET);
			lseek(fd,sizeof(off_t),SEEK_CUR);
			read(fd,&nextsize,sizeof(int));
			lseek(fd,nextsize,SEEK_CUR);
			lseek(fd,sizeof(int),SEEK_CUR);
			write(fd,&nextptr,sizeof(off_t));
			
			// update the next node
			lseek(fd,nextptr,SEEK_SET);
			write(fd,&prevptr,sizeof(off_t));
			
			return ret;
		} else { // we iterated over the whole list and got an error (some node has incorrect next)
			errno = EINVAL;
			return -1;
		}
	}
}