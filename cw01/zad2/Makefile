all:
	gcc -c -Wall mylib.c -o mylibstatic.o
	ar rcs libmylibstatic.a mylibstatic.o
	gcc prog.c -Wall -L. -lmylibstatic -o prog
	./prog 1000 1000 d c f mb db mdb

#usuwanie wszystkich plikow bibliotek
clean:
	rm -rf *.o *.a *.so prog