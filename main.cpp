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
#include <boost/regex.hpp> // **
#include <string.h>
#include "error.h"

#define CLOSED -1
#define UDP     0
#define ASM     1
#define SSM     2

#define SEND_FIT_BUF_LEN 1472
#define SEND_MAX_BUF_LEN 65507
#define RECV_MAX_BUF_LEN 131072

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
int check_nic(const char* address)
{
    struct ifaddrs *ifa = NULL, *ifList;  
    if (getifaddrs(&ifList) < 0) {
        return -1;
    }
    for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sin = NULL;
            sin = (struct sockaddr_in *)ifa->ifa_addr;
            if (strcmp(inet_ntoa(sin->sin_addr), address) ==0) {
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
                printf("DEVICE_LINKED. The interface is %s.\r\n", ifa->ifa_name);
                freeifaddrs(ifList);
                return 3;
            }
        }  
    }  
    printf("DEVICE_NONE\r\n");
    freeifaddrs(ifList);
    return 0;
}

/**
 * split the address and port 
 * from 229.0.0.1:8888 to 229.0.0.1 8888
 */
void split_arg(std::string info, std::string &address, uint &port)
{
    int pos = -1;
    pos = info.find_first_of(":", 0);
    if(pos == -1)
    {
        address.assign(info);
    }else
    {
        address.assign(info.substr(0, pos));
        port = atoi(info.substr(pos + 1, info.length() - pos).c_str());
    }
    // ~
    // std::cout<< address << " " << port << std::endl;
}

/**
 * parse the parameters
 */
void parse_arg(int argc, char *argv[], sendConn &sc, recvConn &rc)
{
    cmdline::parser cmd;
    cmd.add<std::string>("local", 'l', "local ip address", false, "");
    cmd.add<std::string>("sendinfo", 's', "the target ip muliticast address and port, e.g 229.0.0.1:8888", false, "");
    cmd.add<std::string>("recvinfo", 'r', "the source ip muliticast address and port, e.g 229.0.0.1:8888", false, "");
    cmd.add<std::string>("source", 'S', "the source ip address", false, "");
    cmd.parse_check(argc, argv);
    // ~
    // std::cout << cmd.get<std::string>("local") << " "
	// 	<< cmd.get<std::string>("sendinfo") << " "
	// 	<< cmd.get<std::string>("recvinfo") <<  " "
	// 	<< cmd.get<std::string>("source") << std::endl;
    split_arg(cmd.get<std::string>("local"), sc.local_address, sc.local_port);
    split_arg(cmd.get<std::string>("local"), rc.local_address, rc.local_port);
    split_arg(cmd.get<std::string>("sendinfo"), sc.target_address, sc.target_port);
    split_arg(cmd.get<std::string>("recvinfo"), rc.target_address, rc.target_port);
    rc.source_address = cmd.get<std::string>("source");
    // ~
    // printf("%s\n",sc.target_address.c_str());
    // printf("%d\n",sc.target_port);
}

/**
 * check the ip property
 */
bool check_ip(std::string address)
{
    boost::cmatch what;
    boost::regex ip_pattern("((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})(\\.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}");
    bool ret = boost::regex_match(address.c_str(), what, ip_pattern);
    if(ret == false) ERR_EXIT("Wrong IP Address.");
    return ret;
}

/**
 * check ip is ASM or SSM type
 */
int check_type(std::string address)
{
    boost::cmatch what;
    boost::regex SSM_pattern("232(\\.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}");
    if(boost::regex_match(address.c_str(), what, SSM_pattern))
    {
        return 2;
    }
    boost::regex ASM_pattern("2((2[4-9])|(3[0-9]))(\\.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}");
    if(boost::regex_match(address.c_str(), what, ASM_pattern))
    {
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
bool check_arg(sendConn &sc, recvConn &rc)
{
    bool res;
    if(!sc.target_address.empty())
    {
        if(check_ip(sc.target_address))
            if(check_nic(sc.local_address.c_str()))
                if(check_port(sc.target_port))
                {
                    res = true;
                    sc.state = check_type(sc.target_address);
                }
        else
            ERR_EXIT("ARGUEMENTS CHECK ERROR");
    }
    if(!rc.target_address.empty())
    {
        if(check_ip(rc.target_address))
            if(check_nic(rc.local_address.c_str()))
                if(check_port(rc.target_port))
                {
                    res = res & true;
                    if (rc.source_address != "")
                        if (check_ip(rc.source_address))
                            res = res & true;
                        else
                            ERR_EXIT("ARGUEMENTS CHECK ERROR");
                    rc.state = check_type(rc.target_address);
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
    sc.buffer_length = SEND_FIT_BUF_LEN;
    sc.buffer = (byte *)malloc(sc.buffer_length*sizeof(byte));
    memset(sc.buffer, 0, sc.buffer_length);
    recvConn rc;
    rc.state = -1;
    rc.buffer_length = RECV_MAX_BUF_LEN;
    rc.buffer = (byte *)malloc(rc.buffer_length*sizeof(byte));
    memset(rc.buffer, 0, rc.buffer_length);
    parse_arg(argc, argv, sc, rc);
    check_arg(sc, rc);
    // ~
    // printf("%d\n", sc.state);
    // printf("%d\n", rc.state);
    if(sc.state != -1)
    {
        int sock_send_fd;
        int dst_len;
        sockaddr_in dst_addr;
        create_send_socket(sock_send_fd, dst_len, dst_addr, sc.target_address.c_str(), sc.target_port);
        send_m(sock_send_fd, dst_len, dst_addr, sc.local_address.c_str(), sc.buffer, sc.buffer_length);
    }
    if(rc.state == 0)
    {
        int sock_rev_fd;
        uint peer_len;
        sockaddr_in rev_addr;
        sockaddr_in peer_addr;
        create_recv_socket(sock_rev_fd, rev_addr, rc.target_address.c_str(), rc.target_port);
        recv_m(sock_rev_fd, peer_addr, peer_len, rc.local_address.c_str(), rc.buffer, rc.buffer_length);
    }
    else if(rc.state == 1)
    {

    }
    else if(rc.state == 2)
    {
        
    }
    return 0;
}
