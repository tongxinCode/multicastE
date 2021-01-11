/**
 * sendfile
 * just used in tcp
 * use sendfile (sendfile只适用于将数据从文件拷贝到套接字)
 * ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
 * 20-40Gbps send && receive
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
#include <sys/sendfile.h>
#include <fcntl.h>

/**
 * use tcp_cork
 * this modification may be useful in WWW、FTP or file-transfer by Nagle mode
 * this step may be harmful when it comes to small packets
#include <netinet/tcp.h> 
*/

#define MYPORT 8887
#define MAXLEN 131072

char *DSTIP = "127.0.0.1";

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)


int main(int argc, char const *argv[])
{
    // timer part
    struct timeval t1, t2;
    double elapsedTime;

    // socket part
    int sock, client;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket error");
    /**
     * 4-2nd version
     * use tcp_cork?
    int on = 1;
    setsockopt(sock,SOL_TCP, TCP_CORK, &on, sizeof (on));
    */
    struct sockaddr_in dstaddr;
    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_port = htons(MYPORT);
    dstaddr.sin_addr.s_addr = inet_addr(DSTIP);
    int dstlen = sizeof(dstaddr);
    // tcp--connect sendfile
    int res = connect(sock,(struct sockaddr*)&dstaddr,sizeof(dstaddr));
    if(res<0)
        ERR_EXIT("socket error");

    float packetnum = 0;
    double packetsum = 0;

    // start timer
    gettimeofday(&t1, NULL);

    // socket loop
    /**
     * 4th version
     * remove readfile()
    readfile(buf);
    */
    int fd;
    int n;//the number read
    off_t offset = 0;
    fd = open("./data1MB", O_RDONLY);
    if(fd < 0)
        ERR_EXIT("open error");
    while (1)
    {
        // tcp max 128KB==131072B
        /**
         * in linux kernal config
        net.ipv4.tcp_wmem = 8192 131072(**) 16777216
        net.ipv4.tcp_rmem = 32768 131072(**) 16777216
        */
        /**
         * 4-1st version
         * remove sendto()
        sendto(sock, buf, n, 0,
                (struct sockaddr *)&dstaddr, dstlen);
        */
        n = sendfile(sock, fd, &offset, MAXLEN);
        if(n == 0)
            ERR_EXIT("send end or error");
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
    }
    /**
     * 4-2nd version
     * use tcp_cork?
    on = 0;
    setsockopt (sock, SOL_TCP, TCP_CORK, &on, sizeof (on)); 
    */
    // close socket
    close(sock);
    return 0;
}
