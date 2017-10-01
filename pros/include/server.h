// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * server.h
 */

#ifndef SERVER_H_

#define SERVER_H_

#include <API.h>

#define SERVER_WAIT_MILLISECONDS 2

typedef struct serverIpv4_s {
    uint8_t v[4];
} serverIpv4_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void serverSetup(PROS_FILE *sd);
extern void serverInit(void);
extern void serverStart(void);
extern void serverStop(void);
extern int serverIsConnected(void);
extern serverIpv4_t serverGetIpv4(void);

#ifdef __cplusplus
}
#endif

#endif
