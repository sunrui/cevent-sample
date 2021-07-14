/**
 * sock util
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2012-12-22
 */
#include "ce_sock.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <string.h>

bool ce_sock_init(bool init)
{
#ifdef _WIN32
    static int init_ref = 0;
    
    if (init) {
        WSADATA wsaData;
        
        init_ref++;
        return WSAStartup(MAKEWORD(2,2), &wsaData) == NO_ERROR;
    } else {
        if (--init_ref == 0) {
            return (WSACleanup() == NO_ERROR);
        }
    }
#else
    return true;
#endif
}

bool ce_sock_setblock(bool fd, bool lock)
{
#ifdef _WIN32
    unsigned long flags = lock ? 0 : 1;
    return ioctlsocket(fd, FIONBIO, &flags) != -1;
#else
    int flags;
    
    flags = fcntl(fd, F_GETFL, 0);
    flags = lock ? flags & ~O_NONBLOCK : flags | O_NONBLOCK;
    
    return fcntl(fd, F_SETFL, flags ) != -1;
#endif
}

bool ce_sock_close(int fd)
{
#ifdef _WIN32
    return closesocket(fd) == NO_ERROR;
#else
    return close(fd) == 0;
#endif
}

bool ce_sock_keepalive(int fd, bool on, int delay)
{
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on, sizeof(on))) {
        return false;
    }
    
#ifdef TCP_KEEPIDLE
    if (on && setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &delay, sizeof(delay))) {
        return false;
    }
#endif
    
    /* Solaris/SmartOS, if you don't support keep-alive,
     * then don't advertise it in your system headers...
     */
    /* FIXME(bnoordhuis) That's possibly because sizeof(delay) should be 1. */
#if defined(TCP_KEEPALIVE) && !defined(__sun)
    if (on && setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, (const char *)&delay, sizeof(delay))) {
        return false;
    }
#endif
    
    return true;
}

bool ce_sock_ip(int fd, char ip[17], int * port)
{
    struct sockaddr_in sin;
    socklen_t len;
    bool r;
    
    *ip = 0;
    *port = 0;
    len = sizeof(sin);
    
    r = !getpeername(fd, (struct sockaddr *)&sin, &len);
    if (r) {
        strncpy(ip, inet_ntoa(sin.sin_addr), 16);
        ip[16] = '\0';
        *port = ntohs(sin.sin_port);
    }
    
    return r;
}
