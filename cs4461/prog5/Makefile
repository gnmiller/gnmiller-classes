FLAGS=-g -Wall -Wextra -std=gnu99
EXEC=dnsquery
SOURCE=${EXEC}.c

.PHONY: all clean
all: ${EXEC}

${EXEC}: ${SOURCE}
	gcc ${FLAGS} -o ${EXEC} ${SOURCE}

clean:
	rm -f  ${EXEC}

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