.phony all:
all: ACS

ACS: ACS.c
	gcc -Wall ACS.c -pthread -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe