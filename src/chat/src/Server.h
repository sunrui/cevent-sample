//
//  Server.h
//  chat
//
//  Created by 孙瑞 on 15/1/15.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//
#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <stdbool.h>

class Server {
public:
    virtual bool startup(int argc, char * argv[]) = 0;
    
public:
    static Server * instance();
};

#endif