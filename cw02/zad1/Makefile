ROZMIAR1=8192 4
ROZMIAR2=4096 4
ROZMIAR3=4096 512
ROZMIAR4=2048 512
ROZMIAR5=4096 2048
ROZMIAR6=2048 2048
ROZMIAR7=4096 8192
ROZMIAR8=2048 8192

all:
	gcc -Wall prog.c -o prog
	./prog generate dane $(ROZMIAR1) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR1) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR1) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR1) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR1) sys >> Wyniki.txt
	./prog generate dane $(ROZMIAR2) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR2) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR2) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR2) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR2) sys >> Wyniki.txt
	./prog generate dane $(ROZMIAR3) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR3) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR3) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR3) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR3) sys >> Wyniki.txt
	./prog generate dane $(ROZMIAR4) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR4) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR4) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR4) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR4) sys >> Wyniki.txt
	./prog generate dane $(ROZMIAR5) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR5) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR5) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR5) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR5) sys >> Wyniki.txt
	./prog generate dane $(ROZMIAR6) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR6) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR6) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR6) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR6) sys >> Wyniki.txt
	./prog generate dane $(ROZMIAR7) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR7) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR7) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR7) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR7) sys >> Wyniki.txt
	./prog generate dane $(ROZMIAR8) >> Wyniki.txt
	./prog copy dane daneLib $(ROZMIAR8) lib >> Wyniki.txt
	./prog sort daneLib $(ROZMIAR8) lib >> Wyniki.txt
	./prog copy dane daneSys $(ROZMIAR8) sys >> Wyniki.txt
	./prog sort daneSys $(ROZMIAR8) sys >> Wyniki.txt


clean:
	rm -rf prog Wyniki.txt
