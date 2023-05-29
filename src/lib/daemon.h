/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef USER_DAEMON_DAEMON_H
#define USER_DAEMON_DAEMON_H

#include <user-server-lib/server.h>
#include <udp-server/udp_server.h>

typedef struct UserDaemon {
    UserServer server;
    UdpServerSocket socket;
} UserDaemon;

int userDaemonInit(UserDaemon* self);

#endif
