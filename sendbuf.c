/**
 * sendbuf
 * this c is used for udp transfer matching receive.c
 * also can be used for tcp if complemented
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h> //mmap()


#define MYPORT 8887
#define MAXBUF 16*8*1024
char *DSTIP = "127.0.0.1";

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)

void readfile(char * buf)
{
    FILE *fp;

    // open file
    fp = fopen("./data", "rb");

    // read data
    fgets(buf, MAXBUF, fp);
    fclose(fp);
}

int main(int argc, char const *argv[])
{
    // timer part
    struct timeval t1, t2;
    double elapsedTime;

    // socket part
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket error");
    struct sockaddr_in dstaddr;
    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_port = htons(MYPORT);
    dstaddr.sin_addr.s_addr = inet_addr(DSTIP);
    int dstlen = sizeof(dstaddr);
    
    /**
     * third version
     * use buf = mmap(file, len);
     * 
     * 19.47404 Gbps 7.78277 Gbps send && receive 
     * 
    char buf[MAXBUF] = {0};
     */
    
	char * buf = (char *)mmap(0, MAXBUF, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);
	if(buf == MAP_FAILED)
	{
		perror("mmap");
		close(sock);
		return 0;
	}

    float packetnum = 0;
    double packetsum = 0;

    // start timer
    gettimeofday(&t1, NULL);

    // socket loop
    /**
     * second version: one read multiple send
     * 2071.67426 MB/s just send
     * 1436.90906 MB/s send && receive (11.23Gbps)
     * do the less things in the loop , get the more rate
    */
    readfile(buf);
    int n = strlen(buf);
    while (1)
    {
        // read data from file
        /**
         * first version : the rate is limited by the disk
         * but can change dynamically
         * 903.25743 MB/s just send
        readfile(buf);
        int n = strlen(buf);
        */
        if(n<=0)
            ERR_EXIT("no data in file");
        /* printf("发送 %d B\n",n);*/
        // udp max 64KB==65507B
        sendto(sock, buf, n, 0,
                (struct sockaddr *)&dstaddr, dstlen);
        
        // snapshot timer
        gettimeofday(&t2, NULL);
        // compute and print the elapsed time in millisec
        elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        /*printf("time elapsed %5.5f \n", elapsedTime);*/
        packetnum ++;
        packetsum += n;
        // printf("total send rate %5.5f MB/s  ", packetsum*1000/elapsedTime/1024/1024);
        printf("band-width %5.5f Gbps \n", packetsum*1000/elapsedTime/1024/1024/1024*8);
        memset(buf, 0 , sizeof(buf));
    }

    // close socket
    close(sock);
    return 0;
}
