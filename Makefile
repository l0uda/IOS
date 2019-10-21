proj2: proj2.c
	gcc -std=gnu99 -g -O2 -Wall -Wextra -Werror -pedantic proj2.c -o proj2 -lpthread -lrt

zip:
	zip proj2.zip proj2.c *.h Makefile
