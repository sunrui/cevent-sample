//
//  Client.h
//  chat
//
//  Created by 孙瑞 on 15/1/15.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//
#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "ChatMessage.h"
#include "ClientUI.h"

enum SendErrorType {
    SendErrorFileNotExsit,
    SendErrorNetwork,
    SendErrorToken,
    SendErrorUnkown
};

class MessageListener {
public:
    virtual void onMessageReceived(ChatMessage * msg) = 0;
    
    virtual void onTextSending(ChatMessage * msg) = 0;
    virtual void onTextSendSuccess(ChatMessage * msg) = 0;
    virtual void onTextSendFailed(ChatMessage * msg);
    
    virtual void onImageSending(ChatMessage * msg) = 0;
    virtual void onImageSendSuccess(ChatMessage * msg) = 0;
    virtual void onImageSendFailed(ChatMessage * msg, SendErrorType error) = 0;
    
    virtual void onVoiceSending(ChatMessage * msg) = 0;
    virtual void onVoiceSendSuccess(ChatMessage * msg) = 0;
    virtual void onVoiceSendFailed(ChatMessage * msg, SendErrorType error) = 0;
};

enum ConnectStatus {
    CONNECT_STATUS__CONNECTING,
    CONNECT_STATUS__CONNECTED,
    CONNECT_STATUS__LOGIN_SUCCESS,
    CONNECT_STATUS__DISCONNECTED,
    CONNECT_STATUS__CONFILECT
};

class ConnectChangeListener {
public:
    virtual void onConnectChanged(ConnectStatus status) = 0;
};

class LoginListener {
public:
    virtual void onLoginSuccess();
    virtual void onPasswordError();
    virtual void onForbidden();
    virtual void onNoResponse();
    virtual void onInitError();
};

class NotificationListener {
public:
    virtual void onChatMessageReceived(ChatMessage * msg) = 0;
};

enum SendToType {
    SendToUser,
    SendToGroup
};

class Client {
public:
    virtual void setServer(const char * ip, int port) = 0;
    virtual void setSession(const char * session) = 0;

    virtual void setMessageListener(MessageListener * listener) = 0;
    virtual void setConnectChangeListener(ConnectChangeListener * listener) = 0;
    virtual void setNotificationListener(NotificationListener * listener) = 0;
    virtual void setLoginListener(LoginListener * listener) = 0;

    virtual bool login() = 0;
    virtual bool disconnect() = 0;

    virtual bool isAuthed() = 0;
    
    virtual bool chatText(SendToType type, int uid, const char * text) = 0;
    virtual bool chatTextAgain(unsigned long packetId) = 0;
    
    virtual bool chatImage(SendToType type, int uid, const char imageFile[], int imageCount, void * userData) = 0;
    virtual bool chatImageAgain(unsigned long packetId, void * userData) = 0;
    
    virtual bool chatVoice(SendToType type, int uid, const char imageFile[], int imageCount, void * userData) = 0;
    virtual bool chatVoice(unsigned long packetId, void * userData) = 0;
    
    virtual bool changeOnline(bool online) = 0;
    
    virtual ClientUI * getClientUI() = 0;
    
public:
    static Client * instance();
};

#endif