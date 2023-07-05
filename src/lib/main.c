/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include "daemon.h"
#include "version.h"
#include <clog/console.h>
#include <flood/in_stream.h>
#include <flood/out_stream.h>
#include <flood/text_in_stream.h>
#include <guise-serialize/parse_text.h>
#include <guise-server-lib/user.h>
#include <guise-server-lib/utils.h>
#include <imprint/default_setup.h>
#include <inttypes.h>

#if !defined TORNADO_OS_WINDOWS
#include <errno.h>
#include <unistd.h>
#endif

clog_config g_clog;
char g_clog_temp_str[CLOG_TEMP_STR_SIZE];

typedef struct UdpServerSocketSendToAddress {
    struct sockaddr_in* sockAddrIn;
    UdpServerSocket* serverSocket;
} UdpServerSocketSendToAddress;

static int sendToAddress(void* self_, const NetworkAddress* address, const uint8_t* buf, size_t count)
{
    (void) address;
    UdpServerSocketSendToAddress* self = (UdpServerSocketSendToAddress*) self_;

    return udpServerSend(self->serverSocket, buf, count, self->sockAddrIn);
}

static int readOneUserLine(GuiseUsers* users, FldTextInStream* stream)
{
    GuiseSerializeUserInfo userInfo;
    int userErr = guiseTextStreamReadUser(stream, &userInfo);
    if (userErr < 0) {
        return userErr;
    }
    if (userInfo.userId == 0) {
        return -1;
    }
    if (userInfo.passwordHash == 0) {
        return -48;
    }

    GuiseUser* user;
    int err = guiseUsersCreate(users, userInfo.userId, &user);
    if (err < 0) {
        return err;
    }

    user->name = userInfo.userName;
    user->passwordHash = userInfo.passwordHash;
    user->roles = userInfo.roles;

    CLOG_C_VERBOSE(&users->log, "Read User '%s' (%" PRIx64 ") (ends with %02x)", user->name.utf8, user->id,
                   (uint8_t) user->passwordHash & 0xff)

    return 0;
}

static int readUsersFile(GuiseUsers* users)
{
    CLOG_C_DEBUG(&users->log, "reading users file")
    FILE* fp = fopen("users.txt", "r");
    if (fp == 0) {
        CLOG_ERROR("could not find users.txt")
        // return -4;
    }

#if defined CONFIGURATION_DEBUG
    size_t usersRead = 0;
#endif

    while (1) {
        char line[1024];
        char* ptr = fgets(line, 1024, fp);
        if (ptr == 0) {
            CLOG_C_DEBUG(&users->log, "read (%zu) users", usersRead)
            return 0;
        }

        FldTextInStream textInStream;
        FldInStream inStream;

        fldInStreamInit(&inStream, (const uint8_t*) line, tc_strlen(line));
        fldTextInStreamInit(&textInStream, &inStream);

        int lineErr = readOneUserLine(users, &textInStream);
        if (lineErr < 0) {
            fclose(fp);
            return lineErr;
        }
#if defined CONFIGURATION_DEBUG
        usersRead++;
#endif
    }
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_VERBOSE;

    CLOG_OUTPUT("guise daemon v%s starting up", USER_DAEMON_VERSION)

    GuiseDaemon daemon;

    int err = guiseDaemonInit(&daemon);
    if (err < 0) {
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
    serverLog.constantPrefix = "GuiseServer";
    serverLog.config = &g_clog;

    guiseServerInit(&server, &memory.tagAllocator.info, serverLog);
    readUsersFile(&server.guiseUsers);

#define UDP_MAX_SIZE (1200)

    uint8_t buf[UDP_MAX_SIZE];
    struct sockaddr_in address;
    int errorCode;

#define UDP_REPLY_MAX_SIZE (UDP_MAX_SIZE)

    uint8_t reply[UDP_REPLY_MAX_SIZE];
    FldOutStream outStream;
    fldOutStreamInit(&outStream, reply, UDP_REPLY_MAX_SIZE);

    CLOG_OUTPUT("ready for incoming UDP packets")

    while (true) {
        ssize_t receivedOctetCount = udpServerReceive(&daemon.socket, buf, UDP_MAX_SIZE, &address);
        if (receivedOctetCount < 0) {
            CLOG_WARN("problem with receive %zd", receivedOctetCount)
        } else {
            socketSendToAddress.sockAddrIn = &address;

            fldOutStreamRewind(&outStream);
#if 0
            nimbleSerializeDebugHex("received", buf, size);
#endif
            errorCode = guiseServerFeed(&server, &address, buf, (size_t) receivedOctetCount, &response);
            if (errorCode < 0) {
                CLOG_WARN("guiseServerFeed: error %d", errorCode)
            }
        }
    }

    // imprintDefaultSetupDestroy(&memory);
    // return 0;
}
