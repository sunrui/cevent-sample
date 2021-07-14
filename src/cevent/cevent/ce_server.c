//
//  ce_server.c
//  cevent
//
//  Created by 孙瑞 on 15/1/11.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#include "ce_server.h"
#include "ce_io.h"
#include "ce_alloctor.h"
#include "ce_assert.h"
#include "ce_sock.h"

#include "ae.h"
#include "anet.h"

#include <pthread.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#endif

struct ce_server {
    int fd;
    aeEventLoop * event_loop;
    pthread_t event_loop_thread;
    bool stop;
    
    ce_conf_t conf;
    ce_io_t * io;
};

int _ce_server_send(int fd, const char * buffer, packet_len_t size, void * extra)
{
    return (int)send(fd, buffer, size, 0);
}

void * _ce_server_routine(void * user_data) {
    ce_server_t * server;
    
    server = (ce_server_t *)user_data;
    
    server->stop = false;
    aeMain(server->event_loop);
    aeDeleteFileEvent(server->event_loop, server->fd, AE_ALL_EVENTS);
    aeDeleteEventLoop(server->event_loop);
    
    return NULL;
}

void _aeClientClose(aeEventLoop * el, int fd)
{
    ce_server_t * server;
    ce_io_param_t * param;
    
    server = (ce_server_t *)el->apidata;
    param = ce_io_param(server->io);
    
    aeDeleteFileEvent(el, fd, AE_ALL_EVENTS);
    
    ce_io_del(server->io, fd);
    ce_sock_close(fd);
    
    param->conf.on_close(fd, param->user_data);
}

void _aeReadFromClient(aeEventLoop * el, int fd, void * privdata, int mask)
{
    ce_server_t * server;
    
    char buffer[MAX_BUFFER];
    ssize_t size;
    int r;

    server = (ce_server_t *)el->apidata;

    size = recv(fd, buffer, sizeof(buffer), 0);
    if (size <= 0) {
        _aeClientClose(el, fd);
        return;
    }
    
    r = ce_io_enqueue(server->io, fd, buffer, size);
    if (!r) {
        _aeClientClose(el, fd);
        return;
    }
}

void _aeAcceptTcpHandler(aeEventLoop * el, int fd, void *privdata, int mask)
{
    ce_server_t * server;
    ce_io_param_t * param;
    
    char ip[64] = { 0 };
    char err[256];
    int c_fd, c_port;
    int r;
    
    server = (ce_server_t *)el->apidata;
    param = ce_io_param(server->io);
    
    c_fd = anetTcpAccept(err, fd, ip, sizeof(ip), &c_port);
    if (c_fd == ANET_ERR) {
        ce_sock_close(c_fd);
        ce_assert(0);
        
        return;
    }
    
    if (anetKeepAlive(err, c_fd, server->conf.keepalive) == ANET_ERR) {
        ce_sock_close(c_fd);
        ce_assert(0);
        
        return;
    };

    r = aeCreateFileEvent(el, c_fd, AE_READABLE, _aeReadFromClient, NULL);
    if (r == AE_ERR) {
        ce_sock_close(c_fd);
        ce_assert(0);
        return;
    }
    
    if (!ce_io_add(server->io, c_fd)) {
        ce_sock_close(c_fd);
        ce_assert(0);
        return;
    }
    
    param->conf.on_accept(c_fd, ip, c_port, param->user_data);
}

int _aeHeartbeatTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    ce_server_t * server;
    
    server = (ce_server_t *)clientData;
    ce_io_push(server->io, ce_push_except_fd, server->fd, NULL, 0);
    
    return server->conf.heartbeat;
}

ce_server_t * ce_server_startup(ce_conf_t * conf, char * err)
{
    ce_server_t * server;
    
    if (conf == NULL || err == NULL) {
        ce_assert(0);
        return NULL;
    }
    
    if (conf->on_accept == NULL ||
        conf->on_received == NULL ||
        conf->on_exception == NULL ||
        conf->on_close == NULL ||
        conf->per_cache_ringbuffer_size <= 0 ||
        conf->per_max_packet_body_size <= 0 ||
        conf->per_max_packet_body_size > conf->per_cache_ringbuffer_size ||
        conf->workers < 1 ||
        conf->ip[0] == '\0' ||
        conf->port <= 0 ||
        conf->timeout < 0 ||
        conf->keepalive < 0 ||
        conf->heartbeat < 1) {
        ce_assert(0);
        return NULL;
    }
    
    ce_sock_init(true);
    server = (ce_server_t *)ce_malloc(sizeof(ce_server_t));
    server->fd = anetTcpServer(err, conf->port, NULL);
    if (server->fd == ANET_ERR) {
        ce_free(server);
        ce_sock_init(false);
        ce_assert(0);
        return NULL;
    }
    
    if (anetKeepAlive(err, server->fd, conf->keepalive) == ANET_ERR) {
        ce_sock_close(server->fd);
        ce_free(server);
        ce_sock_init(false);
        ce_assert(0);
        return NULL;
    }
    
    server->event_loop = aeCreateEventLoop(10 * 1024);
    server->event_loop->apidata = server;
  
    if (aeCreateFileEvent(server->event_loop, server->fd, AE_READABLE, _aeAcceptTcpHandler, NULL) == ANET_ERR) {
        ce_sock_close(server->fd);
        ce_free(server);
        ce_sock_init(false);
        ce_assert(0);
        return NULL;
    }
    
    {
        ce_io_param_t param;
        
        param.send = _ce_server_send;
        param.send_extra = NULL;
        
        param.user_data = conf->user_data;
        memcpy(&param.conf, conf, sizeof(ce_conf_t));
        
        server->io = ce_io_new(&param);
    }
    
    server->stop = true;
    pthread_create(&server->event_loop_thread, NULL, _ce_server_routine, server);
    while (server->stop) {
        usleep(100 * 1000);
    }
    
    aeCreateTimeEvent(server->event_loop, server->conf.heartbeat, _aeHeartbeatTimeProc, server, NULL);
    
    return server;
}

bool ce_server_push(ce_server_t * server, int fd, const char * data, packet_len_t size)
{
    if (server == NULL || data == NULL || size <= 0) {
        ce_assert(0);
        return false;
    }
    
    return ce_io_push(server->io, ce_push_fd, fd, data, size);
}

void ce_server_wait(ce_server_t * server)
{
    if (server == NULL) {
        ce_assert(0);
        return;
    }
    
    pthread_join(server->event_loop_thread, NULL);
}

void ce_server_stop(ce_server_t * server)
{
    server->stop = true;
    aeStop(server->event_loop);
    pthread_join(server->event_loop_thread, NULL);
    ce_io_destory(server->io);
    ce_sock_init(false);
}