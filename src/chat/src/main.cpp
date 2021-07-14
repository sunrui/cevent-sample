//
//  main.c
//  cevent
//
//  Created by 孙瑞 on 15/1/10.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>

#include "Server.h"
#include "Client.h"

void * client_routine(void * param)
{
    return NULL;
}

int main(int argc, char * argv[])
{
    Server::instance()->startup(argc, argv);
    
    pthread_t thead;
    pthread_create(&thead, NULL, client_routine, NULL);
    
    getchar();
    
    return 0;
}