# Greg  Miller -- gnmiller
# Cs4461 -- Computer Networks
# Spring 15
# 2/11/15
# Program 2
# 
# FILE: Makefile
#
# In addition to the directives supplied an additional debug (make debug) is
# supported. When httpget is compiled using this directive it toggles the DEBUG
# flag and httpget will output various statements relating to what is being sent
# and received from the host.


EXEC=httpserve

$(EXEC): main.c
	gcc -o $(EXEC) -D_GNU_SOURCE -std=gnu99 -Wall -g main.c -pthread

debug: main.c
	gcc -o $(EXEC) -D_GNU_SOURCE -DDEBUG -std=c99 -Wall -g main.c

test: test.c
	gcc -o test -std=c99 -Wall -g test.c

.PHONY: clean clobber tar

clean:
	rm -f $(EXEC) *.o

clobber: clean

######################################################################
# All of the following is designed to help students create a file to
# submit.
SHELL:=/usr/bin/env bash
COURSENAME=cs4461
TAR_FILE_BASENAME=${USER}-$(shell date +"%Y%m%d-%H%M%S")-${COURSENAME}
TAR_FILE=${TAR_FILE_BASENAME}.tgz

tar:
	@echo
	@${MAKE} -s clean
	@echo
	@rm -f "${TAR_FILE}"
	@tar --exclude-vcs --exclude ${USER}-????????-??????-${COURSENAME}.tgz --exclude ${USER}-????????-??????-${COURSENAME} --transform 's,^,${TAR_FILE_BASENAME}/,' -czf "${TAR_FILE}" *
#	@tar -czf ${TAR_FILE} *
	@echo "======================================================="
	@tar --list -zf "${TAR_FILE}"
	@echo "======================================================="
	@echo -n "Size: "
	@du -h "${TAR_FILE}" | awk '{ print $$1 }'
	@echo "The files listed above are stored in: ${TAR_FILE}"
	@echo
	@echo "The digits in the filename represent the time the file was"
	@echo "created in YYYYMMDD-HHMMSS format."
	@echo
	@echo "Note: If you extract the files from ${TAR_FILE}, it will"
	@echo "extract the files into a subdirectory (keeping the existing"
	@echo "files in your directory)."
	@echo
	@echo "FINISHED? Submit ${TAR_FILE} on Canvas!"