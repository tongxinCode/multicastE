/**
 * multicast transfer extreme tool
 */
#include "cmdline/cmdline.h"
#include "send.h"
#include "recv.h"
#include <iostream> //~
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h> 
#include <regex.h>
#include <string.h>
#include "error.h"

#define CLOSED -1
#define UDP     0
#define ASM     1
#define SSM     2

typedef struct sendConn
{
    int state;
    std::string local_address;
    uint local_port;
    std::string target_address;
    uint target_port;
    byte* buffer;
    int buffer_length;
}sendConn;

typedef struct recvConn
{
    int state;
    std::string local_address;
    uint local_port;
    std::string target_address;
    std::string source_address;
    uint target_port;
    byte* buffer;
    int buffer_length;
}recvConn;

/**
 * get local nic interface
 * name and address
 */
int check_nic(const char* if_name)
{
    struct ifaddrs *ifa = NULL, *ifList;  
    if (getifaddrs(&ifList) < 0) {
        return -1;
    }
    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) {
            if (strcmp(ifa->ifa_name, if_name) ==0) {
                if (!(ifa->ifa_flags & IFF_UP)) {
                    printf("DEVICE_DOWN\r\n");
                    freeifaddrs(ifList);
                    return 1;
                }
                if (!(ifa->ifa_flags & IFF_RUNNING)) {
                    printf("DEVICE_UNPLUGGED\r\n");
                    freeifaddrs(ifList);
                    return 2;
                }
                printf("DEVICE_LINKED\r\n");
                freeifaddrs(ifList);
                return 3;
            }
        }  
    }  
    printf("%s", stderr, "DEVICE_NONE\r\n");
    freeifaddrs(ifList);
    return 0;
}

/**
 * split the address and port 
 * from 229.0.0.1:8888 to 229.0.0.1 8888
 */
void split_arg(std::string info, std::string address, uint port)
{
    int pos = -1;
    info.find_first_of(':', pos);
    if(pos == -1)
    {
        address = info;
    }else
    {
        address = info.substr(0, pos);
        port = atoi(info.substr(pos + 1, info.length() - pos).c_str());
    }
}

/**
 * parse the parameters
 */
void parse_arg(int argc, char *argv[], sendConn sc, recvConn rc)
{
    cmdline::parser cmd;
    cmd.add<std::string>("local", 'l', "local ip address", false, "");
    cmd.add<std::string>("sendinfo", 's', "the target ip muliticast address and port, e.g 229.0.0.1:8888", false, "");
    cmd.add<std::string>("recvinfo", 'r', "the source ip muliticast address and port, e.g 229.0.0.1:8888", false, "");
    cmd.add<std::string>("source", 'S', "the source ip address", false, "");
    cmd.parse_check(argc, argv);
    // ~
    std::cout << cmd.get<std::string>("local") << " "
		<< cmd.get<std::string>("sendinfo") << " "
		<< cmd.get<std::string>("recvinfo") <<  " "
		<< cmd.get<std::string>("source") << std::endl;
    split_arg(cmd.get<std::string>("local"), sc.local_address, sc.local_port);
    split_arg(cmd.get<std::string>("local"), rc.local_address, rc.local_port);
    split_arg(cmd.get<std::string>("sendinfo"), sc.target_address, sc.target_port);
    split_arg(cmd.get<std::string>("recvinfo"), rc.target_address, rc.target_port);
    rc.source_address = cmd.get<std::string>("source");
}

/**
 * check the ip property
 */
bool check_ip(const char* address)
{
    regex_t reg;
    regmatch_t pmatch;
    const char ip_pattern[] = "((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})(/.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}";
    regcomp(&reg, ip_pattern, REG_EXTENDED);
    int status = regexec(&reg, address, 1, &pmatch, 0);
    /* 匹配正则表达式，注意regexec()函数一次只能匹配一个，不能连续匹配，网上很多示例并没有说明这一点 */
    if(status == REG_NOMATCH)
    {
        printf("IP ADDRESS ERROR\n");
    	regfree(&reg);
        return false;
    }
	regfree(&reg);
    return true;
}

/**
 * check ip is ASM or SSM type
 */
int check_type(const char* address)
{
    regex_t reg;
    regmatch_t pmatch;
    const char SSM_pattern[] = "232(/.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}";
    regcomp(&reg, SSM_pattern, REG_EXTENDED);
    int status = regexec(&reg, address, 1, &pmatch, 0);
    if(status == REG_NOERROR)
    {
	    regfree(&reg);
        return 2;
    }
    const char ASM_pattern[] = "2((2[4-9])|(3[0-9]))(/.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}";
    regcomp(&reg, ASM_pattern, REG_EXTENDED);
    status = regexec(&reg, address, 1, &pmatch, 0);
    if(status == REG_NOERROR)
    {
	    regfree(&reg);
        return 1;
    }
    return 0;
}

/**
 * check the port property
 */
bool check_port(int port)
{
    if(port<=65535 && port>299)
        return true;
    return false;
}

/**
 * check the property
 */
bool check_arg(sendConn sc, recvConn rc)
{
    bool res;
    if(sc.target_address != "")
    {
        if(check_ip(sc.target_address.c_str()))
            if(check_nic(sc.local_address.c_str()))
                if(check_port(sc.target_port))
                {
                    res = true;
                    sc.state = check_type(sc.target_address.c_str());
                }
        else
            ERR_EXIT("ARGUEMENTS CHECK ERROR");
    }
    if(rc.target_address != "")
    {
        if(check_ip(rc.target_address.c_str()))
            if(check_nic(rc.local_address.c_str()))
                if(check_port(rc.target_port))
                {
                    res = res & true;
                    if (rc.source_address != "")
                        if (check_ip(rc.source_address.c_str()))
                            res = res & true;
                        else
                            ERR_EXIT("ARGUEMENTS CHECK ERROR");
                    rc.state = check_type(rc.target_address.c_str());
                }    
        else
            ERR_EXIT("ARGUEMENTS CHECK ERROR");
    }
    return res;
}

int main(int argc, char *argv[])
{
    sendConn sc;
    sc.state = -1;
    sc.buffer = (byte *)malloc(sc.buffer_length*sizeof(byte));
    recvConn rc;
    rc.state = -1;
    rc.buffer = (byte *)malloc(sc.buffer_length*sizeof(byte));
    parse_arg(argc, argv, sc, rc);
    check_arg(sc, rc);
    printf("%d\n", sc.state);
    printf("%d\n", rc.state);
    if(sc.state != -1)
    {
        int sock_send_fd;
        int dst_len;
        sockaddr_in dst_addr;
        create_send_socket(sock_send_fd, dst_len, dst_addr, sc.target_address.c_str(), sc.target_port);
        send_m(sock_send_fd, dst_len, dst_addr, sc.local_address.c_str(), sc.buffer, sc.buffer_length);
    }
    if(rc.state != -1)
    {
        int sock_rev_fd;
        int peer_len;
        sockaddr_in rev_addr;
        sockaddr_in peer_addr;
        create_recv_socket(sock_rev_fd, rev_addr, rc.target_address.c_str(), rc.target_port);
        recv_m(sock_rev_fd, peer_addr, peer_len, rc.local_address.c_str(), rc.buffer, rc.buffer_length);
    }
    return 0;
}
