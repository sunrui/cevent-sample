//
//  ClientUI.h
//  chat
//
//  Created by 孙瑞 on 15/1/16.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#ifndef CHAT_CLIENT_UI_H
#define CHAT_CLIENT_UI_H

#include "ChatMessage.h"

#define MESSAGE_SYSTEM_UUID "____MESSAGE_SYSTEM_UUID";
#define MESSAGE_VISIT_UUID = "____MESSAGE_VISIT_UUID";
#define MESSAGE_ADD_UUID = "____MESSAGE_ADD_UUID";
#define MESSAGE_HISTORY_UUID = "____MESSAGE_HISTORY_UUID";
#define MESSAGE_COMMENT_UUID = "____MESSAGE_COMMENT_UUID";

// 消息记数相关
class Tally {
public:
    virtual int getTallySystem() = 0;
    virtual void zeroTallySystem() = 0;
    
    virtual int getTallyChatUnread(int uid) = 0;
    virtual void zeroTallyChatUnread(int uid) = 0;
};

class LoadListener {
public:
    enum LoadResult {
        LoadResultOk,
        LoadResultNetworkError,
    };
    
    virtual void onLoad(LoadResult result, bool hasMore);
};

// 聊天列表操作相关
class ChatMessager
{
public:
    virtual void removeChat(int uid, unsigned long packetId) = 0;
    virtual void removeUser(int uid) = 0;
    virtual int getChatCount(int uid) = 0;
    virtual ChatMessage * getChat(int uid, int index) = 0;
    virtual ChatMessage * getChatFromPacketId(int uid, int packetId) = 0;
    virtual void load(int uid, int count, LoadListener listener);
};

// UI 操作对象类
class ClientUI {
public:
    virtual Tally * getTally() = 0;
    virtual ChatMessager * getChatMessager() = 0;
    virtual ChatMessager * getGroupChatMessager() = 0;
    virtual ChatMessager * getHistoryMessager() = 0;
    virtual ChatMessager * getSystemMessager() = 0;
};




#endif