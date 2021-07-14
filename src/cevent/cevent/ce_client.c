//
//  ce_client.c
//  cevent
//
//  Created by 孙瑞 on 15/1/11.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#include "ce_client.h"
#include "ce_io.h"
#include "ce_alloctor.h"
#include "ce_assert.h"
#include "ce_sock.h"

#include <pthread.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

struct ce_client {
    ce_io_t * io;
    int fd;
    pthread_t recv_thread;
    bool stop;
};

int _ce_client_send(int fd, const char * buffer, packet_len_t size, void * extra)
{
    return (int)send(fd, buffer, size, 0);
}

void * _ce_client_routine(void * user_data)
{
    ce_client_t * client;
    ce_io_param_t * param;
    char buffer[MAX_BUFFER];
    int size;
    bool r;
    
    client = (ce_client_t *)user_data;
    param = ce_io_param(client->io);
    
    client->stop = false;
    for (; !client->stop;) {
        size = (int)recv(client->fd, buffer, MAX_BUFFER, 0);
        if (size > 0) {
            r = ce_io_enqueue(client->io, client->fd, buffer, size);
            if (!r) {
                ce_assert(0);
                break;
            }
        } else {
            break;
        }
    }
    
    param->conf.on_close(client->fd, param->user_data);
    
    return NULL;
}

ce_client_t * ce_client_connect(ce_conf_t * conf, ce_connect_result * result)
{
    ce_client_t * client;
    struct sockaddr_in addr;
    int ret;
    
    if (conf == NULL || result == NULL) {
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
    
    client = (ce_client_t *)ce_malloc(sizeof(ce_client_t));
    client->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client->fd == -1) {
        *result = ce_connect_socket_error;
        ce_free(client);
        return NULL;
    }
    
    addr.sin_family = AF_INET;
#ifdef _WIN32
    addr.sin_addr.S_un.S_addr = inet_addr(conf->ip);
#else
    addr.sin_addr.s_addr = inet_addr(conf->ip);
#endif
    addr.sin_port = htons(conf->port);
    
    ret = connect(client->fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret != 0) {
        *result = ce_connect_server_no_response;
        ce_freep(client);
        return NULL;
    }
    
    {
        ce_io_param_t param;
        
        param.send = _ce_client_send;
        param.send_extra = NULL;
        
        param.user_data = conf->user_data;
        memcpy(&param.conf, conf, sizeof(ce_conf_t));
        
        client->io = ce_io_new(&param);
    }
    
    {
        bool r;
    
        r = ce_io_add(client->io, client->fd);
        if (!r) {
            *result = ce_connect_init_error;
            ce_io_destory(client->io);
            ce_sock_close(client->fd);
            ce_freep(client);
            return NULL;
        }
    }
    
    ce_sock_keepalive(client->fd, 1, conf->keepalive);
    client->stop = true;
    
    pthread_create(&client->recv_thread, NULL, _ce_client_routine, client);
    while (!client->stop) {
        usleep(100 * 1000);
    }
    
    return client;
}

bool ce_client_send(ce_client_t * client, const char * data, packet_len_t size)
{
    if (client == NULL || data == NULL || size <= 0) {
        ce_assert(0);
        return false;
    }
    
    return ce_io_push(client->io, ce_push_fd, client->fd, data, size);
}

void ce_client_disconnect(ce_client_t * client)
{
    client->stop = 1;
    ce_sock_close(client->fd);
    pthread_join(client->recv_thread, NULL);
    ce_io_destory(client->io);
    ce_freep(client);
}
