//
//  Server.cpp
//  chat
//
//  Created by 孙瑞 on 15/1/15.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#include "Server.h"

extern "C" {
#include "ce_server.h"
}

#include <string.h>
#include <stdio.h>

class ServerImpl : public Server {
public:
    virtual bool startup(int argc, char * argv[]);
    
public:
    static ServerImpl * instance();
    
protected:
    friend void on_s_accept(int fd, char * ip, int port, void * user_data);
    friend void on_s_received(int fd, const char * data, packet_len_t size, void * user_data);
    friend void on_s_exception(int fd, ce_exception exception, void * user_data);
    friend void on_s_close(int fd, void * user_data);
    
private:
    ServerImpl();
    
    ce_server_t * _server;
    
    static ServerImpl * _impl;
};

ServerImpl * ServerImpl::_impl = NULL;

Server * Server::instance()
{
    return ServerImpl::instance();
}

ServerImpl::ServerImpl()
{
}

void on_s_accept(int fd, char * ip, int port, void * user_data)
{
}

void on_s_received(int fd, const char * data, packet_len_t size, void * user_data)
{
    ServerImpl * server = (ServerImpl *)user_data;
    
    ce_server_push(server->_server, fd, "我收到了亲", strlen("我收到了亲"));
}

void on_s_exception(int fd, ce_exception exception, void * user_data)
{
}

void on_s_close(int fd, void * user_data)
{
}

bool ServerImpl::startup(int argc, char * argv[])
{
    ce_conf conf;
    conf.on_accept = on_s_accept;
    conf.on_received = on_s_received;
    conf.on_exception = on_s_exception;
    conf.on_close = on_s_close;
    conf.user_data = this;
    
    conf.per_max_packet_body_size = 4096;
    conf.per_cache_ringbuffer_size = 1024 * 1024;
    conf.workers = 2;
    
    strcpy(conf.ip, "127.0.0.1");
    conf.port = 9087;
    conf.timeout = 5;
    conf.keepalive = 10;
    conf.heartbeat = 60;
    
    char err[256];
    
    _server = ce_server_startup(&conf, err);
    if (_server == NULL) {
        fprintf(stderr, "%s\n", err);
        return false;
    }
    
    return true;
}

ServerImpl * ServerImpl::instance()
{
    if (_impl == NULL) {
        _impl = new ServerImpl();
    }
    
    return _impl;
}