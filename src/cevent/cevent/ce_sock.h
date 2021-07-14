/**
 * sock util
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2012-12-22
 */
#ifndef CE_SOCK_H
#define CE_SOCK_H

#include <stdbool.h>

bool ce_sock_init(bool init);
bool ce_sock_setblock(bool fd, bool lock);
bool ce_sock_close(int fd);
bool ce_sock_keepalive(int fd, bool on, int delay);
bool ce_sock_ip(int fd, char ip[17], int * port);

#endif
