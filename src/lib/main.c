/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <clog/console.h>
#include "daemon.h"
#include "version.h"
#include <flood/in_stream.h>
#include <flood/out_stream.h>
#include <imprint/default_setup.h>

#include <guise-server-lib/utils.h>

#if !TORNADO_OS_WINDOWS
#include <unistd.h>
#endif

clog_config g_clog;

typedef struct UdpServerSocketSendToAddress
{
    struct sockaddr_in *sockAddrIn;
    UdpServerSocket *serverSocket;
} UdpServerSocketSendToAddress;

static int sendToAddress(void *self_, const NetworkAddress *address, const uint8_t *buf, size_t count)
{
    UdpServerSocketSendToAddress *self = (UdpServerSocketSendToAddress *)self_;

    return udpServerSend(self->serverSocket, buf, count, self->sockAddrIn);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_DEBUG;

    CLOG_OUTPUT("guise daemon v%s starting up", USER_DAEMON_VERSION)

    GuiseDaemon daemon;

    int err = guiseDaemonInit(&daemon);
    if (err < 0)
    {
        return err;
    }

    UdpServerSocketSendToAddress socketSendToAddress;
    socketSendToAddress.serverSocket = &daemon.socket;

    GuiseServerSendDatagram sendDatagram;
    sendDatagram.send = sendToAddress;
    sendDatagram.self = &socketSendToAddress;

    GuiseServerResponse response;
    response.sendDatagram = sendDatagram;

    GuiseServer server;

    ImprintDefaultSetup memory;
    imprintDefaultSetupInit(&memory, 16 * 1024 * 1024);

    // TODO:    ConclaveSerializeVersion applicationVersion = {0x10, 0x20, 0x30};

    Clog serverLog;
    serverLog.constantPrefix = "ClvServer";
    serverLog.config = &g_clog;

    guiseServerInit(&server, &memory.tagAllocator.info, serverLog);

#define UDP_MAX_SIZE (1200)

    uint8_t buf[UDP_MAX_SIZE];
    size_t size;
    struct sockaddr_in address;
    int errorCode;

#define UDP_REPLY_MAX_SIZE (UDP_MAX_SIZE)

    uint8_t reply[UDP_REPLY_MAX_SIZE];
    FldOutStream outStream;
    fldOutStreamInit(&outStream, reply, UDP_REPLY_MAX_SIZE);

    CLOG_OUTPUT("ready for incoming UDP packets")

    while (true)
    {
        size = UDP_MAX_SIZE;
        errorCode = udpServerReceive(&daemon.socket, buf, &size, &address);
        if (errorCode < 0)
        {
            CLOG_WARN("problem with receive %d", errorCode);
        }
        else
        {
            socketSendToAddress.sockAddrIn = &address;

            fldOutStreamRewind(&outStream);
#if 0
            nimbleSerializeDebugHex("received", buf, size);
#endif
            errorCode = guiseServerFeed(&server, &address, buf, size, &response);
            if (errorCode < 0)
            {
                CLOG_WARN("clvServerFeed: error %d", errorCode);
            }
        }
    }

    imprintDefaultSetupDestroy(&memory);

    return 0;
}
