all:
	gcc -Wall -lpthread main.c -lm -o prog
	rm -rf Times.txt
	./prog 1 mona1.pgm filter1.txt out1.pgm >> Times.txt
	./prog 2 mona1.pgm filter1.txt out1.pgm >> Times.txt
	./prog 4 mona1.pgm filter1.txt out1.pgm >> Times.txt
	./prog 8 mona1.pgm filter1.txt out1.pgm >> Times.txt
	./prog 1 mona1.pgm filter2.txt out1.pgm >> Times.txt
	./prog 2 mona1.pgm filter2.txt out1.pgm >> Times.txt
	./prog 4 mona1.pgm filter2.txt out1.pgm >> Times.txt
	./prog 8 mona1.pgm filter2.txt out1.pgm >> Times.txt
	./prog 1 mona1.pgm filter3.txt out1.pgm >> Times.txt
	./prog 2 mona1.pgm filter3.txt out1.pgm >> Times.txt
	./prog 4 mona1.pgm filter3.txt out1.pgm >> Times.txt
	./prog 8 mona1.pgm filter3.txt out1.pgm >> Times.txt
