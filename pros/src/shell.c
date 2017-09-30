// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et

#include <string.h>

#include "shell.h"

#define CR '\r'
#define LF '\n'
#define CTRL_C 0x03
#define CTRL_D 0x04
#define CTRL_Q 0x11
#define BKSPACE_CHAR '\b'
#define ABORT_CHAR CTRL_C
#define QUIT_APPEND CTRL_Q
#define HISTORY_CHAR '!'
#define ESC 0x1B

#define xstr(a) str(a)
#define str(a) #a

// local shell variables
static TaskHandle shellThreadPointer = NULL;

// local shell flags
static bool shellEchoFlag = true;
static bool shellBufrFlag = true;

// Array of the default commands.
static void cmd_systime(PROS_FILE *chp, int argc, char *argv[]);
static const shellCommand_t shellLocalCommands[] = {
    // {"info",    cmd_info},
    {"systime", cmd_systime},
    // {"mem",     cmd_mem},
    // {"threads", cmd_threads},
    // {"reboot",  cmd_reboot},
    // {"flags",   cmd_flags},
    {NULL, NULL}};

static char *_strtok(char *str, const char *delim, char **saveptr);
static void shellThread(void *p);
static void shellThreadRun(shellConfig_t *p);
static void shellList(PROS_FILE *chp, const shellCommand_t *scp);
static bool shellExec(const shellCommand_t *scp, PROS_FILE *chp, char *name, int argc, char *argv[]);

void
shellInit(void)
{
    (void)usartInit(stdout, 115200, SERIAL_8N1); // 115200 or 230400
    return;
}

void
shellStart(const shellConfig_t *scp)
{
    if (shellThreadPointer != NULL) {
        return;
    }
    shellThreadPointer = taskCreate(shellThread, TASK_DEFAULT_STACK_SIZE, (void *)scp, TASK_PRIORITY_DEFAULT);
    return;
}

bool
shellGetLine(PROS_FILE *chp, char *line, unsigned size)
{
    static char history[SHELL_MAX_LINE_LENGTH] = {""};

    short escSeq = 0;
    char *p = line;
    char *q;
    char *r = line;
    unsigned long lasttime = millis();

    // not in escape sequence
    escSeq = 0;

    while (true) {
        char c;
        if ((c = (char)fgetc(chp)) == -1) {
            return true;
        }

        // In escape sequence part 1 ?
        if (escSeq == 1) {
            if (c == '[') {
                escSeq = 2;
                continue;
            } else
                escSeq = 0;
        }

        // In escape sequence part 2 ?
        if (escSeq == 2) {
            switch (c) {
            case 'A': // Cursor Up
                if (shellEchoFlag)
                    fprint(history, chp);

                // copy history buffer to line
                q = history;
                while ((p < line + size - 1) && (*q != 0))
                    *p++ = *q++;
                // assume no buffering for history
                r = p;
                break;

            default:
                break;
            }
            escSeq = 0;
            continue;
        }

        switch ((uint8_t)c) {
        // CTRL D quits shell
        case CTRL_D:
            fprint("^D", chp);
            return true;
            break;

        // Backspace
        case BKSPACE_CHAR:
        case 0x7F: // for some reason OSX screen outputs 7F
            if (p != line) {
                if (shellEchoFlag) {
                    // print any buffered characters
                    while (r != p)
                        fputc((uint8_t)*r++, chp);
                    // print backspace and wipe the previous character
                    fprintf(chp, "%c %c", (uint8_t)BKSPACE_CHAR, (uint8_t)BKSPACE_CHAR);
                }

                // one less character
                p--;
                // no more buffering
                r = p;
            }
            break;

        case CR:
            *p = 0;

            if (shellEchoFlag) {
                // print any remaining chars that came in quickly
                if (r != p)
                    fprint(r, chp);

                // copy to history buffer
                for (q = history, p = line; *p != 0; q++, p++)
                    *q = *p;
                *q = 0;
            }

            // send cr/lf
            fprint("\r\n", chp);
            return false;
            break;

        case ESC:
            // escape sequence ?
            escSeq = 1;
            break;

        case 0xC9:
            // start of VEX escape sequence
            // sleep - do not echo
            taskDelay(500);
            break;

        case 0x21:
            // ROBOTC uses 0x21 command to start user code
            // and does this before downloading its firmware
            // sleep - do not echo
            taskDelay(100);
            break;

        default:
            if (c >= 0x20) {
                if (p < line + size - 1) {
                    *p++ = (char)c;

                    if (shellEchoFlag) {
                        if (!shellBufrFlag || (millis() - lasttime) > 50) {
                            while (r != p)
                                fputc((uint8_t)*r++, chp);
                        }

                        lasttime = millis();
                    }
                }
            }
            break;
        }
    }
}

// String to token
static char *
_strtok(char *str, const char *delim, char **saveptr)
{
    char *token;
    if (str)
        *saveptr = str;
    token = *saveptr;

    if (!token)
        return NULL;

    token += strspn(token, delim);
    *saveptr = strpbrk(token, delim);
    if (*saveptr)
        *(*saveptr)++ = '\0';

    return (*token ? token : NULL);
}

static void
shellThread(void *p)
{
    unsigned long now = millis();
    while (true) {
        (void)shellThreadRun((void *)p);
        taskDelayUntil(&now, 50);
    }
    shellThreadPointer = NULL;
    return;
}

static void
shellThreadRun(shellConfig_t *p)
{
    int n;
    PROS_FILE *chp = p->channel;
    const shellCommand_t *scp = p->commands;
    char *lp, *cmd, *tokp, line[SHELL_MAX_LINE_LENGTH];
    char *args[SHELL_MAX_ARGUMENTS + 1];

    fprint("\033[2J", chp); // clearscreen
    fprint("\r\nVEX-PROS Shell V1.00\r\n", chp);

    while (true) {
        fprint("ch> ", chp);
        if (shellGetLine(chp, line, sizeof(line))) {
            fprint("\r\nlogout", chp);
            break;
        }

        lp = _strtok(line, " \009", &tokp);
        cmd = lp;
        n = 0;

        // Break line up into arguments
        while ((lp = _strtok(NULL, " \009", &tokp)) != NULL) {
            if (n >= SHELL_MAX_ARGUMENTS) {
                fprint("too many arguments\r\n", chp);
                cmd = NULL;
                break;
            }
            args[n++] = lp;
        }
        args[n] = NULL;

        if (cmd != NULL) {
            if (strcasecmp(cmd, "exit") == 0) {
                if (n > 0) {
                    fprint("Usage: exit\r\n", chp);
                    continue;
                }
                break;
            } else if (strcasecmp(cmd, "help") == 0) {
                if (n > 0) {
                    fprint("Usage: help\r\n", chp);
                    continue;
                }

                fprint("Commands: help exit ", chp);
                shellList(chp, shellLocalCommands);
                if (scp != NULL)
                    shellList(chp, scp);
                fprint("\r\n", chp);
            } else if (shellExec(shellLocalCommands, chp, cmd, n, args) && ((scp == NULL) || shellExec(scp, chp, cmd, n, args))) {
                fprint(cmd, chp);
                fprint(" ?\r\n", chp);
            }
        }
    }

    return;
}

static void
shellList(PROS_FILE *chp, const shellCommand_t *scp)
{
    while (scp->name != NULL) {
        fprint(scp->name, chp);
        fputc(' ', chp);
        scp++;
    }
}

static bool
shellExec(const shellCommand_t *scp, PROS_FILE *chp, char *name, int argc, char *argv[])
{
    while (scp->name != NULL) {
        if (strcasecmp(scp->name, name) == 0) {
            scp->function(chp, argc, argv);
            return false;
        }
        scp++;
    }
    return true;
}

static void
cmd_systime(PROS_FILE *chp, int argc, char *argv[])
{
    (void)argv;
    if (argc > 0) {
        fprint("Usage: systime\r\n", chp);
        return;
    }
    fprintf(chp, "%lu\r\n", (unsigned long)millis());
}
