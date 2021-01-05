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

#define MYPORT 8887
#define MAXBUF 16*8*1024

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

    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket error");

    struct sockaddr_in saddr,caddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(MYPORT);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("监听TCP %d端口\n", MYPORT);
    if (bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
        ERR_EXIT("bind error");

    listen(sock,5);

    int len = sizeof(caddr);
    int c = accept(sock,(struct sockaddr*)&caddr,&len);

    char recvbuf[MAXBUF] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;//the number read
    float packetnum = 0;
    double packetsum = 0;

    // start timer
    gettimeofday(&t1, NULL);

    /**
     * 504.12269 MB/s send && receive
     * do the less things in the loop , get the more rate
     */
    // socket loop
    while (1)
    {
        if(c < 0)
            continue;
        peerlen = sizeof(peeraddr);
        n = recv(c,recvbuf,MAXBUF,0);
        printf("recv length is %d\n",n);
        if (n <= 0)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom end or error");
        }
        else if (n > 0)
        {
            // printf("接收到的数据：%s\n", recvbuf);
            // printf("接收到的数据大小 %d B\n", n);

            // snapshot timer
            gettimeofday(&t2, NULL);
            // compute and print the elapsed time in millisec
            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
            /*printf("time elapsed %5.5f \n", elapsedTime);*/
            packetnum ++;
            packetsum += n;
            // printf("total receive rate %5.5f MB/s  ", packetsum*1000/elapsedTime/1024/1024);
            printf("band-width %5.5f Gbps \n", packetsum*1000/elapsedTime/1024/1024/1024*8);
        }
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sock);
    return 0;
}
