//
//  ChatMessage.h
//  cevent
//
//  Created by 孙瑞 on 15/1/15.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <time.h>
#include <stdint.h>

enum MessageType {
    MessageTypeText,
    MessageTypeImage,
    MessageTypeVoice,
    MessageTypeImageGroup,
    MessageTypeAnnounce
};

enum MessageStatus {
    MessageStatusDelivering,
    MessageStatusDelivered,
    MessageStatusDeliverFailed,
    MessageStatusDisplayed
};

class ChatMessage {
public:
    unsigned long packetId;
    MessageType type;
    time_t ts;
    bool tsDisplay;
    int uid;
    bool postByMe;
    MessageStatus status;
    
public:
    virtual char * serial(int * size) = 0;
    virtual ChatMessage * unserial(const char * from, int size) = 0;
    
protected:
    ChatMessage();
};

class ChatMessageText : public ChatMessage {
public:
    char text[4096];
    
public:
    virtual char * serial(int * size);
    virtual ChatMessage * unserial(const char * from, int size);
};

class ChatMessageImage : public ChatMessage {
public:
    char imageUrl[2048];
    char localFile[2048];
    
public:
    virtual char * serial(int * size);
    virtual ChatMessage * unserial(const char * from, int size);
};

class ChatMessageVoice : public ChatMessage {
public:
    char voiceUrl[2048];
    char localFile[2048];
    
public:
    virtual char * serial(int * size);
    virtual ChatMessage * unserial(const char * from, int size);
};

class ChatMessageImageGroup : public ChatMessage {
public:
    char imageUrl[64][2048];
    char localFile[64][2048];
    int imageCount;
    
public:
    virtual char * serial(int * size);
    virtual ChatMessage * unserial(const char * from, int size);
};

class ChatMessageAnnounce : public ChatMessage {
public:
    char text[4096];
    
public:
    virtual char * serial(int * size);
    virtual ChatMessage * unserial(const char * from, int size);
};

#endif