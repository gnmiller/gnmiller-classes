.PHONY: all clean
all: server client

FLAGS=-g -Wall -std=gnu99 -D_GNU_SOURCE
EXEC=server client

server: server.c shared.c
	gcc ${FLAGS} -o server server.c unreliable.c shared.c

client: client.c shared.c
	gcc ${FLAGS} -o client client.c unreliable.c shared.c

clean:
	rm -f client server

######################################################################
# All of the following is designed to help students create a file to
# submit.
SHELL:=/usr/bin/env bash
COURSENAME=cs4461
TAR_FILE_BASENAME=${USER}-$(shell date +"%Y%m%d-%H%M")-${COURSENAME}
TAR_FILE=${TAR_FILE_BASENAME}.tgz

tar:
	@echo
	@if [[ `find . -maxdepth 1 -type f -name ${USER}-????????-????-${COURSENAME}.tgz | wc -l` -gt 0 ]]; then ls ${USER}-????????-????-${COURSENAME}.tgz; echo -n "Delete old .tgz files listed above? [y/n]: "; read YN; if [[ $$YN == 'y' ]]; then rm -v ${USER}-????????-????-${COURSENAME}.tgz; fi; fi;
	@echo
	@${MAKE} -s clean
	@echo
	@rm -f "${TAR_FILE}"
	@tar --exclude ${USER}-????????-????-${COURSENAME}.tgz --exclude ${USER}-????????-????-${COURSENAME} --transform 's,^,${TAR_FILE_BASENAME}/,' -czf "${TAR_FILE}" *
#	@tar -czf ${TAR_FILE} *
	@echo "======================================================="
	@tar --list -zf "${TAR_FILE}"
	@echo "======================================================="
	@echo -n "Size: "
	@du -h "${TAR_FILE}" | awk '{ print $$1 }'
	@echo "The files listed above are stored in: ${TAR_FILE}"
	@echo
	@echo "The digits in the filename represent the time the file was"
	@echo "created in YYYYMMDD-HHMM format."
	@echo
	@echo "Note: If you extract the files from ${TAR_FILE}, it will"
	@echo "extract the files into a subdirectory (keeping the existing"
	@echo "files in your directory)."
	@echo
	@echo "FINISHED? Submit ${TAR_FILE} on Canvas!"
