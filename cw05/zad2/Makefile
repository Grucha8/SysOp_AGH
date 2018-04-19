all:
	gcc prog.c -Wall -o prog
	gcc master.c -Wall -o master
	gcc slave.c -Wall -o slave
	./prog myfifo 5 5

clean:
	rm -rf prog master slave myfifo
