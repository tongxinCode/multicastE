main:
	rm -f muticastE
	g++ *.cpp *.c -o muticastE

test:
	rm -f *.o *.i *.S
	rm -f muticastE
	g++ -E send.cpp -o send.i 
	g++ -S send.i -o send.S
	g++ -c send.S -o send.o 

	g++ -E recv.cpp -o recv.i 
	g++ -S recv.i -o recv.S
	g++ -c recv.S -o recv.o 

	g++ -E main.cpp -o main.i 
	g++ -S main.i -o main.S
	g++ -c main.S -o main.o 

	g++ -L. main.o -o muticastE -lsend -lrecv

clean:
	rm -f *.o *.i *.S