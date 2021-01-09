/**
 * sendfile
 * just used in tcp
 * use splice (splice用于在两个文件描述符中移动数据)
 * ssize_t splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);
 * splice调用利用了Linux提出的管道缓冲区机制， 所以至少一个描述符要为管道
 * 
 */

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // for pipe

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
    int socket_fd, client;
    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket error");
    
    struct sockaddr_in dstaddr;
    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_port = htons(MYPORT);
    dstaddr.sin_addr.s_addr = inet_addr(DSTIP);
    int dstlen = sizeof(dstaddr);
    // tcp--connect
    int res = connect(socket_fd,(struct sockaddr*)&dstaddr,sizeof(dstaddr));
    if(res<0)
        ERR_EXIT("socket error");

    float packetnum = 0;
    double packetsum = 0;

    // start timer
    gettimeofday(&t1, NULL);

    // socket loop
    int file_fd;
    int n;//the number read
    off_t offset = 0;
    file_fd = open("./data1MB", O_RDONLY);
    if(file_fd < 0)
        ERR_EXIT("open error");
    
    int pfd[2];
    pipe(pfd);

    while (1)
    {
        // tcp max 128KB==131072B
        /**
         * in linux kernal config
        net.ipv4.tcp_wmem = 8192 131072(**) 16777216
        net.ipv4.tcp_rmem = 32768 131072(**) 16777216
        */
        ssize_t bytes = splice(file_fd, NULL, pfd[1], NULL, MAXLEN, SPLICE_F_MOVE);
        if(bytes < 0)
            ERR_EXIT("splice error");
        n = splice(pfd[0], NULL, socket_fd, NULL, bytes, SPLICE_F_MOVE | SPLICE_F_MORE);
        if(n <= 0)
            ERR_EXIT("send end or splice error");
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
    // close socket
    close(socket_fd);
    return 0;
}
