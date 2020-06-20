
main: version.h main.c
	gcc main.c -o main

version.h:
	sh ./genversion.sh

.PHONY: clean

clean:
	@rm -fr main version.h