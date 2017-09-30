// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    server.c                                                          */
/** @brief   The Raspberry Pi 3 server for the robot                           */
/*-----------------------------------------------------------------------------*/

#include "server.h"
#include "rpc.h"
#include "convex_compat.h"

#include <stdlib.h>
#include <string.h>

// types for server
typedef enum serverState_t { serverStateConnected = 0, serverStateDisconnected } serverState_t;

typedef struct server_s {
    rpc_t rpc;
    PROS_FILE *sd;
    serverState_t state;
    SFPcontext sfp;
} server_t;

// storage for server
static server_t server;

// thread pointer
static TaskHandle serverThreadPointer = NULL;

// private functions
static void serverThread(void *arg);
static void serverReset(server_t *ctx);
static void serverRead(uint8_t *buf, size_t len, void *userdata);
static int serverWrite(uint8_t *octets, size_t len, size_t *outlen, void *userdata);
static int serverWritePacket(const uint8_t *octets, size_t len, size_t *outlen, void *userdata);
static void serverCheckConnection(server_t *ctx);

// compatability with convex
static size_t
sdAsynchronousRead(PROS_FILE *sd, uint8_t *buf, size_t len)
{
    int read;
    size_t rlen = 0;
    while (1) {
        if (len == 0 || fcount(sd) == 0) {
            return rlen;
        }
        if ((read = fgetc(sd)) != EOF) {
            *buf++ = (uint8_t)read;
            len--;
            rlen++;
        } else {
            break;
        }
    }
    return rlen;
}

static size_t
sdAsynchronousWrite(PROS_FILE *sd, const uint8_t *buf, size_t len)
{
    size_t wlen = 0;
    while (len > 0) {
        (void)fputc((int)(*buf++), sd);
        len--;
        wlen++;
    }
    return wlen;
    // return fwrite((const void *)buf, 1, len, sd);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Assign UART port to the server.                                */
/** @param[in]  sd The serial driver to use for the server                     */
/*-----------------------------------------------------------------------------*/
void
serverSetup(PROS_FILE *sd)
{
    server.sd = sd;
    server.rpc.writePacket = serverWritePacket;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the Raspberry Pi 3 server.                          */
/*-----------------------------------------------------------------------------*/
void
serverInit(void)
{
    serverReset(&server);
    // (void)usartInit(server.sd, 115200, SERIAL_STOPBITS_1); // 115200 or 230400
    (void)usartInit(server.sd, 115200, SERIAL_8N1); // 115200 or 230400
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the server system thread                                 */
/*-----------------------------------------------------------------------------*/
void
serverStart(void)
{
    if (serverThreadPointer != NULL) {
        return;
    }
    serverThreadPointer = taskCreate(serverThread, 1024, NULL, TASK_PRIORITY_DEFAULT - 1);
    // serverThreadPointer = taskCreate(serverThread, 1024, NULL, TASK_PRIORITY_HIGHEST);
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Stop the server system thread                                  */
/*-----------------------------------------------------------------------------*/
void
serverStop(void)
{
    serverThreadPointer = NULL;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Checks whether the server is connected or not.                 */
/*-----------------------------------------------------------------------------*/
int
serverIsConnected(void)
{
    return (server.state == serverStateConnected);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Checks whether the server is connected or not.                 */
/*-----------------------------------------------------------------------------*/
serverIpv4_t
serverGetIpv4(void)
{
    serverIpv4_t ipv4;
    (void)memcpy(&ipv4, &server.rpc.ipv4, 4);
    return ipv4;
}

// extern void usartFlushBuffers(void);

/*-----------------------------------------------------------------------------*/
/** @brief      The server thread                                              */
/** @param[in]  arg Unused                                                     */
/*-----------------------------------------------------------------------------*/
static void
serverThread(void *arg)
{
    // Unused
    (void)arg;

    // Register the task
    // vexTaskRegister("server");

    // (void)usartInit(server.sd, 115200, SERIAL_8N1);
    // (void)usartFlushBuffers();

    // taskDelay(5000);
    // printf("Foo\r\n");
    // printf("Foo\r\n");
    // printf("Foo\r\n");
    // printf("Foo\r\n");
    // printf("Foo\r\n");

    // local variables
    size_t rlen = 0;
    uint8_t *rbuf = NULL;
    server_t *srv = &server;

    // reset the heartbeat and published timers
    srv->rpc.timestamp = srv->rpc.heartbeat = srv->rpc.published = srv->rpc.sendstats = chTimeNow();

    while (1) {
        // printf("0 READ\r\n");
        rlen = sdAsynchronousRead(srv->sd, srv->rpc.in.buf, SFP_CONFIG_MAX_PACKET_SIZE);
        rbuf = srv->rpc.in.buf;
        // printf("1 READ: rlen=%lu\r\n", rlen);
        // printf("0 DELIVER\r\n");
        while (rlen-- > 0) {
            (void)sfpDeliverOctet(&srv->sfp, *rbuf, NULL, 0, NULL);
            rbuf++;
        }
        // printf("1 DELIVER\r\n");
        // printf("0 CHECK\r\n");
        (void)serverCheckConnection(srv);
        // printf("1 CHECK\r\n");
        if (serverIsConnected()) {
            // printf("0 LOOP\r\n");
            (void)rpcLoop(&srv->rpc);
            // printf("1 LOOP\r\n");
        }
        // printf("0 SLEEP\r\n");
        vexSleep(2);
        // printf("1 SLEEP\r\n");
    }

    (void)serverReset(srv);
    serverThreadPointer = NULL;
    (void)taskDelete(NULL);

    return;
}

static void
serverReset(server_t *srv)
{
    serverIpv4_t ipv4Empty = {{0, 0, 0, 0}};
    srv->state = serverStateDisconnected;
    srv->rpc.seq_id = 0;
    srv->rpc.timestamp = chTimeNow();
    (void)memcpy(&srv->rpc.ipv4, &ipv4Empty, 4);
    (void)sfpInit(&srv->sfp);
    (void)sfpSetDeliverCallback(&srv->sfp, serverRead, (void *)srv);
    (void)sfpSetWriteCallback(&srv->sfp, serverWrite, (void *)srv);
    return;
}

static void
serverRead(uint8_t *buf, size_t len, void *userdata)
{
    server_t *srv = (void *)userdata;
    if (message_deserialize(&srv->rpc.in.msg, buf, len) == 0) {
        (void)rpcRecv(&srv->rpc, &srv->rpc.in.msg);
    }
    (void)serverCheckConnection(srv);
    return;
}

static int
serverWrite(uint8_t *octets, size_t len, size_t *outlen, void *userdata)
{
    server_t *srv = (void *)userdata;
    size_t wlen = 0;
    size_t wcnt = 0;
    // printf("WANT TO WRITE %u BYTES:", len);
    // for (int i = 0; i < len; i++) {
    //     printf(" %u", (uint8_t)octets[i]);
    // }
    // printf("\r\n");
    wlen = sdAsynchronousWrite(srv->sd, octets, len);
    // printf("WROTE         %u BYTES\r\n", wlen);
    wcnt += wlen;
    len -= wlen;
    while (len > 0) {
        octets += wlen;
        vexSleep(2);
        // printf("NEED TO WRITE %u BYTES:", len);
        // for (int i = 0; i < len; i++) {
        //     printf(" %u", (uint8_t)octets[i]);
        // }
        // printf("\r\n");
        wlen = sdAsynchronousWrite(srv->sd, octets, len);
        // printf("WROTE         %u BYTES\r\n", wlen);
        // if (wlen == 0) {
        //     vexSleep(1000);
        // }
        wcnt += wlen;
        len -= wlen;
    }
    if (outlen != NULL) {
        *outlen = wcnt;
    }
    (void)serverCheckConnection(srv);
    return 0;
}

static int
serverWritePacket(const uint8_t *octets, size_t len, size_t *outlen, void *userdata)
{
    server_t *srv = (void *)userdata;
    return sfpWritePacket(&srv->sfp, octets, len, outlen);
}

static void
serverCheckConnection(server_t *srv)
{
    if (srv->state == serverStateDisconnected) {
        if (sfpIsConnected(&srv->sfp)) {
            srv->rpc.timestamp = srv->rpc.heartbeat = srv->rpc.published = srv->rpc.sendstats = chTimeNow();
            srv->state = serverStateConnected;
        }
    } else if (srv->state == serverStateConnected) {
        if (!sfpIsConnected(&srv->sfp) || (chTimeElapsedSince(srv->rpc.heartbeat) > 5000) ||
            (chTimeElapsedSince(srv->rpc.timestamp) > 2147483647)) {
            (void)serverReset(srv);
        }
    }
}
