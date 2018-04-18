#zmienna odpowiedzialna za poziom optymalizancji
#O0 - domyslny
#O, O1, O2, O3(progo uzywany)
#Os 
opt=O0

#stworzenie biblioteki dzielonej i kompilacja
#programu pod nia
shared:
	gcc -fPIC -Wall -shared mylib.c -o libmylibshared.so
	gcc prog.c -Wall -$(opt) -L. -Wl,-rpath=. -lmylibshared -o prog
	./prog 1000 1000 d c f mb db mdb

#tworzenie biblioteki statycznej i kompilacja
#programu za pomocom jej
static: 
	gcc -c mylib.c -Wall -o mylibstatic.o
	ar rcs libmylibstatic.a mylibstatic.o
	gcc prog.c -Wall -$(opt) -L. -lmylibstatic -o prog
	./prog 1000 1000 d c f mb db mdb

#tworzenie biblioteki dynamicznej i kompilacja
#programu za pomocom jej
dynamic:
	gcc -fPIC -Wall -shared mylib.c -o libmylibdynamic.so
	gcc progdynamic.c -Wall -$(opt) -ldl -o progdynamic
	./progdynamic 1000 1000 d c f mb db mdb

#usuwanie wszystkich plikow bibliotek
clean:
	rm -rf *.o *.a *.so prog progdynamic