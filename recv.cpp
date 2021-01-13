/**
 * multicast receive
 */
#include "recv.h"

void create_recv_socket(int& sock, struct sockaddr_in &revaddr, const char* target_addr, uint target_port)
{
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket error");
    memset(&revaddr, 0, sizeof(revaddr));
    revaddr.sin_family = AF_INET;
    revaddr.sin_port = htons(target_port);
    revaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("监听%d端口\n", target_port);
    if (bind(sock, (struct sockaddr *)&revaddr, sizeof(revaddr)) < 0)
        ERR_EXIT("bind error");
}

void recv_m(int& sock, struct sockaddr_in &peeraddr, socklen_t &peerlen, const char* local_addr, byte* &buffer, int buffer_length)
{
    // timer part
    struct timeval t1, t2;
    double elapsedTime;

    int n;
    float packetnum = 0;
    double packetsum = 0;

    // start timer
    gettimeofday(&t1, NULL);

    // socket loop
    while (1)
    {
        peerlen = sizeof(peeraddr);
        n = recvfrom(sock, buffer, buffer_length, 0,
                     (struct sockaddr *)&peeraddr, &peerlen);
        if (n <= 0)
        {

            if (errno == EINTR)
                continue;

            ERR_EXIT("recvfrom error");
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
        memset(buffer, 0, sizeof(buffer));
    }
    close(sock);
}