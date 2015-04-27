#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "dfile.h"

/**
 * Close an existing DFile.
 *
 * @param df A DFile structure.
 *
 * @return 0 if successful, -1 otherwise.
 */
int dclose(DFile* df) {
	return close(df->fd);
}