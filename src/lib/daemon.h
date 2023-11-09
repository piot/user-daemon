/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/user-daemon
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef GUISE_DAEMON_DAEMON_H
#define GUISE_DAEMON_DAEMON_H

#include <guise-server-lib/server.h>
#include <udp-server/udp_server.h>

typedef struct GuiseDaemon {
    UdpServerSocket socket;
} GuiseDaemon;

int guiseDaemonInit(GuiseDaemon* self, uint16_t port);

#endif
