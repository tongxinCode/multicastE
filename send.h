/**
 * multicast send
 */
#ifdef  __cplusplus
extern "C" {
#endif

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
#include "error.h"

/**
 * init some options
 * need root privilege
 */
void send_init(int& sock, const char* local_addr);

/**
 * create socket
 */
void create_send_socket(int& sock, int &dstlen, struct sockaddr_in &dstaddr, const char* target_addr, uint target_port, const char* local_addr);

/**
 * send loop
 */
void send_m(int& sock, int dstlen, struct sockaddr_in &dstaddr, byte* &buffer, int buffer_length, int interval_us);

#ifdef  __cplusplus
}
#endif  /* end of __cplusplus */