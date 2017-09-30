// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * shell.h
 */

#ifndef SHELL_H_

#define SHELL_H_

#include <API.h>

#if !defined(SHELL_MAX_LINE_LENGTH)
#define SHELL_MAX_LINE_LENGTH 64
#endif

#if !defined(SHELL_MAX_ARGUMENTS)
#define SHELL_MAX_ARGUMENTS 4
#endif

typedef void (*shellFunction_t)(PROS_FILE *chp, int argc, char *argv[]);

typedef struct shellCommand_s {
    const char *name;
    shellFunction_t function;
} shellCommand_t;

typedef struct shellConfig_s {
    PROS_FILE *channel;
    const shellCommand_t *commands;
} shellConfig_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void shellInit(void);
extern void shellStart(const shellConfig_t *scp);
extern bool shellGetLine(PROS_FILE *chp, char *line, unsigned size);

#ifdef __cplusplus
}
#endif

#endif
