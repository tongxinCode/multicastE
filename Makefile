temp:
	rm -rf *.o sendfile receive_tcp
	gcc -o sendfile sendfile.c
	gcc -o receive_tcp receive_tcp.c

all:
	gcc -o sendbuf sendbuf.c
	gcc -o receive receive.c

sendbuf:
	gcc -o sendbuf sendbuf.c

sendfile:
	gcc -o sendfile sendfile.c

receive:
	gcc -o receive receive.c

clean:
	rm -rf *.o sendbuf receive sendfile