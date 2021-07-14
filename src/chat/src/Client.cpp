//
//  ClientImpl.cpp
//  chat
//
//  Created by 孙瑞 on 15/1/15.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#include "Client.h"

extern "C" {
#include "ce_client.h"
#include "ce_assert.h"
}

#include <string.h>

class ClientImpl : public Client {
public:
    virtual void setServer(const char * ip, int port);
    virtual void setSession(const char * session);
    
    virtual void setMessageListener(MessageListener * listener);
    virtual void setConnectChangeListener(ConnectChangeListener * listener);
    virtual void setNotificationListener(NotificationListener * listener);
    virtual void setLoginListener(LoginListener * listener);
    
    virtual bool login();
    virtual bool disconnect();
    
    virtual bool isAuthed();
    
    virtual bool chatText(SendToType type, int uid, const char * text);
    virtual bool chatTextAgain(unsigned long packetId);
    
    virtual bool chatImage(SendToType type, int uid, const char imageFile[], int imageCount, void * userData);
    virtual bool chatImageAgain(unsigned long packetId, void * userData);
    
    virtual bool chatVoice(SendToType type, int uid, const char imageFile[], int imageCount, void * userData);
    virtual bool chatVoice(unsigned long packetId, void * userData);
    
    virtual bool changeOnline(bool online);
    
    virtual ClientUI * getClientUI();
    
protected:
    friend void on_c_accept(int fd, char * ip, int port, void * user_data);
    friend void on_c_received(int fd, const char * data, packet_len_t size, void * user_data);
    friend void on_c_exception(int fd, ce_exception exception, void * user_data);
    friend void on_c_close(int fd, void * user_data);
    
public:
    static ClientImpl * instance();

private:
    ClientImpl();
    
    char _session[33];
    char _ip[33];
    int _port;
    
    MessageListener * _messageListener;
    ConnectChangeListener * _connectChangeListener;
    NotificationListener * _notificationListener;
    LoginListener * _loginListener;
    
    ce_client_t * _client;
    
    static ClientImpl * _impl;
};

Client * Client::instance()
{
    return ClientImpl::instance();
}

ClientImpl * ClientImpl::_impl = NULL;

ClientImpl::ClientImpl()
{
    _ip[0] = '\0';
    _port = 0;
    
    _messageListener = NULL;
    _connectChangeListener = NULL;
    _notificationListener = NULL;
    _loginListener = NULL;
    
    _client = NULL;
}

void ClientImpl::setServer(const char * ip, int port)
{
    strcpy(_ip, ip);
    _port = port;
}

void ClientImpl::setSession(const char * session)
{
    strcpy(_session, session);
}

void ClientImpl::setMessageListener(MessageListener * listener)
{
    _messageListener = listener;
}

void ClientImpl::setConnectChangeListener(ConnectChangeListener * listener)
{
    _connectChangeListener = listener;
}

void ClientImpl::setNotificationListener(NotificationListener * listener)
{
    _notificationListener = listener;
}

void ClientImpl::setLoginListener(LoginListener * listener)
{
    _loginListener = listener;
}

void on_c_accept(int fd, char * ip, int port, void * user_data)
{
}

void on_c_received(int fd, const char * data, packet_len_t size, void * user_data)
{
}

void on_c_exception(int fd, ce_exception exception, void * user_data)
{
}

void on_c_close(int fd, void * user_data)
{
}

bool ClientImpl::login()
{
    if (isAuthed()) {
        return true;
    }
    
    if (_ip[0] == '\0' || _session[0] == '\0') {
        ce_assert(0);
        return false;
    }
    
    ce_conf conf;
    conf.on_accept = on_c_accept;
    conf.on_received = on_c_received;
    conf.on_exception = on_c_exception;
    conf.on_close = on_c_close;
    conf.user_data = this;
    
    conf.per_max_packet_body_size = 4096;
    conf.per_cache_ringbuffer_size = 1024 * 1024;
    conf.workers = 2;
    
    strcpy(conf.ip, _ip);
    conf.port = _port;
    conf.timeout = 5;
    conf.keepalive = 10;
    conf.heartbeat = 60;
    
    _connectChangeListener->onConnectChanged(CONNECT_STATUS__CONNECTING);
    
    ce_connect_result result;
    _client = ce_client_connect(&conf, &result);
    
    switch (result) {
        case ce_connect_ok: {
            _connectChangeListener->onConnectChanged(CONNECT_STATUS__CONNECTED);
            break;
        }
        case ce_connect_server_no_response: {
            _connectChangeListener->onConnectChanged(CONNECT_STATUS__DISCONNECTED);
            _loginListener->onNoResponse();
            break;
        }
        case ce_connect_init_error: {
            _loginListener->onInitError();
            break;
        }
        case ce_connect_socket_error: {
            _loginListener->onInitError();
            break;
        }
    }
    
    return true;
}

bool ClientImpl::disconnect()
{
    return false;
}

bool ClientImpl::isAuthed()
{
    return false;
}

bool ClientImpl::chatText(SendToType type, int uid, const char * text)
{
    return false;
}

bool ClientImpl::chatTextAgain(unsigned long packetId)
{
    return false;
}

bool ClientImpl::chatImage(SendToType type, int uid, const char imageFile[], int imageCount, void * userData)
{
    return false;
}

bool ClientImpl::chatImageAgain(unsigned long packetId, void * userData)
{
    return false;
}

bool ClientImpl::chatVoice(SendToType type, int uid, const char imageFile[], int imageCount, void * userData)
{
    return false;
}

bool ClientImpl::chatVoice(unsigned long packetId, void * userData)
{
    return false;
}

bool ClientImpl::changeOnline(bool online)
{
    return false;
}

ClientUI * ClientImpl::getClientUI()
{
    return NULL;
}

ClientImpl * ClientImpl::instance()
{
    if (_impl == NULL) {
        _impl = new ClientImpl();
    }
    
    return _impl;
}