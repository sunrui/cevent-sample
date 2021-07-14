//
//  ChatMessage.cpp
//  cevent
//
//  Created by 孙瑞 on 15/1/15.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#include "ChatMessage.h"

extern "C" {
#include "xhash.h"
}

ChatMessage::ChatMessage()
{
    packetId = hash_long(time(NULL), 4);
}

char * ChatMessageText::serial(int * size)
{
    return NULL;
}

ChatMessage * ChatMessageText::unserial(const char * from, int size)
{
    return NULL;
}

char * ChatMessageImage::serial(int * size)
{
    return NULL;
}

ChatMessage * ChatMessageImage::unserial(const char * from, int size)
{
    return NULL;
}

char * ChatMessageVoice::serial(int * size)
{
    return NULL;
}

ChatMessage * ChatMessageVoice::unserial(const char * from, int size)
{
    return NULL;
}

char * ChatMessageImageGroup::serial(int * size)
{
    return NULL;
}

ChatMessage * ChatMessageImageGroup::unserial(const char * from, int size)
{
    return NULL;
}

char * ChatMessageAnnounce::serial(int * size)
{
    return NULL;
}

ChatMessage * ChatMessageAnnounce::unserial(const char * from, int size)
{
    return NULL;
}