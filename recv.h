/**
 * multicast receive
 */
#ifdef  __cplusplus
extern "C" {
#endif

#include "error.h"
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

/**
 * init some options
 */
void init();

/**
 * recv loop
 */
void recv_m(int sock, struct sockaddr_in peeraddr, socklen_t peerlen, const char* local_addr, byte* buffer, int buffer_length);

/**
 * create recv socket
 */
void create_recv_socket(int sock, struct sockaddr_in revaddr, const char* target_addr, uint target_port);

/**
 * join asm
 */
void joinASM();

/**
 * join ssm
 */
void joinSSM();

#ifdef  __cplusplus
}
#endif  /* end of __cplusplus */