#pakowanie biblioteki do archiwium
libmylibstatic.a: mylibstatic.o
	ar rcs libmylibstatic.a mylibstatic.o

#tworzy zasemblowany kod biblioteki statycznej
#tworzy dzielona biblioteke
mylibstatic.o libmylibshared.so: mylib.c
	gcc -c -Wall mylib.c -o mylibstatic.o					
	gcc -fPIC -Wall -shared mylib.c -o libmylibshared.so	
		
#usuwanie wszystkich plikow bibliotek		
clean:
	rm -rf *.o *.a *.so	