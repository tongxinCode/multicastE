temp:
	rm -rf *.o send_splice receive_udp
	gcc -o send_splice send_splice.c
	gcc -o receive_udp receive_udp.c

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