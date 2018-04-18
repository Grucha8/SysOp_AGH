#stworzenie biblioteki dzielonej i kompilacja
#programu pod nia
shared:
	gcc -fPIC -Wall -shared mylib.c -o libmylibshared.so
	gcc prog.c -Wall -L. -Wl,-rpath=. -lmylibshared -o prog
	./prog 1000 1000 d c f mb db mdb

#tworzenie biblioteki statycznej i kompilacja
#programu za pomocom jej
static: 
	gcc -c -Wall mylib.c -o mylibstatic.o
	ar rcs libmylibstatic.a mylibstatic.o
	gcc prog.c -Wall -L. -lmylibstatic -o prog
	./prog 1000 1000 d c f mb db mdb

#tworzenie biblioteki dynamicznej i kompilacja
#programu za pomocom jej
dynamic:
	gcc -fPIC -Wall -shared mylib.c -o libmylibdynamic.so
	gcc progdynamic.c -Wall -ldl -o progdynamic
	./progdynamic 1000 1000 d c f mb db mdb

#usuwanie wszystkich plikow bibliotek
clean:
	rm -rf *.o *.a *.so prog progdynamic