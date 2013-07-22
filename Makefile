ssserver: ssserver.o stringlist.o
	gcc -o ssserver ssserver.o stringlist.o -ansi -pedantic

ssserver.o: ssserver.c
	gcc -o ssserver.o -c ssserver.c -ansi -pedantic
stringlist.o: stringlist.c
	gcc -o stringlist.o -c stringlist.c -ansi -pedantic

clean:
	rm ssserver *.o
