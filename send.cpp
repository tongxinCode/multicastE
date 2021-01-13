/**
 * multicast send
 */
#include "send.h"

void create_send_socket(int& sock, int &dstlen, struct sockaddr_in &dstaddr, const char* target_addr, uint target_port)
{
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("Socket Error");
    /* Construct local address structure */
    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_port = htons(target_port);
    dstaddr.sin_addr.s_addr = inet_addr(target_addr);
    dstlen = sizeof(dstaddr);
    printf("发送到%s的%d端口\n", target_addr, target_port);
}

void send_m(int& sock, int dstlen, struct sockaddr_in &dstaddr, const char* local_addr, byte* &buffer, int buffer_length)
{
    // timer part
    struct timeval t1, t2;
    double elapsedTime;
    float packetnum = 0;
    double packetsum = 0;

    int n;
    // start timer
    gettimeofday(&t1, NULL);
    while (1)
    {
        n = sendto(sock, buffer, buffer_length, 0,
                (struct sockaddr *)&dstaddr, dstlen);
        if(n <= 0)
            ERR_EXIT("Send error.");
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
        memset(buffer, 0 , buffer_length);
    }
    // close socket
    close(sock);
}