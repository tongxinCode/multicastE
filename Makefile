temp:
	rm -rf *.o send_sendfile receive_tcp
	gcc -o send_sendfile send_sendfile.c
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
	rm -rf *.o sendbuf receive sendfile send_sendfile send_splice receive_tcp receive_udp