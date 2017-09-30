// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/** @file init.c
 * @brief File for initialization code
 *
 * This file should contain the user initialize() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "main.h"
#include "server.h"
#include "shell.h"

#include "apollo.h"

// static const ShellCommand commands[] = {{"adc", vexAdcDebug},
//                                         {"spi", vexSpiDebug},
//                                         {"motor", vexMotorDebug},
//                                         {"lcd", vexLcdDebug},
//                                         {"enc", vexEncoderDebug},
//                                         {"son", vexSonarDebug},
//                                         {"ime", vexIMEDebug},
//                                         {"test", vexTestDebug},
//                                         {"sm", cmd_sm},
//                                         {"apollo", cmd_apollo},
//                                         {NULL, NULL}};

static void
cmd_apollo(PROS_FILE *chp, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    apolloInit();

    // run until any key press
    while (fcount(chp) == 0) {
        apolloUpdate();
    }

    apolloDeinit();
}

// configuration for the shell
static const shellCommand_t shellCommands[] = {{"apollo", cmd_apollo}, {NULL, NULL}};
static const shellConfig_t shellConfig = {stdout, shellCommands};

/*
 * Runs pre-initialization code. This function will be started in kernel mode one time while the
 * VEX Cortex is starting up. As the scheduler is still paused, most API functions will fail.
 *
 * The purpose of this function is solely to set the default pin modes (pinMode()) and port
 * states (digitalWrite()) of limit switches, push buttons, and solenoids. It can also safely
 * configure a UART port (usartOpen()) but cannot set up an LCD (lcdInit()).
 */
void
initializeIO()
{
    (void)setTeamName("TopSecret");
    (void)watchdogInit();
    (void)serverSetup(uart2);
    (void)serverInit();
    (void)shellInit();
    return;
}

/*
 * Runs user initialization code. This function will be started in its own task with the default
 * priority and stack size once when the robot is starting up. It is possible that the VEXnet
 * communication link may not be fully established at this time, so reading from the VEX
 * Joystick may fail.
 *
 * This function should initialize most sensors (gyro, encoders, ultrasonics), LCDs, global
 * variables, and IMEs.
 *
 * This function must exit relatively promptly, or the operatorControl() and autonomous() tasks
 * will not start. An autonomous mode selection menu like the pre_auton() in other environments
 * can be implemented in this task if desired.
 */
void
initialize()
{
    (void)lcdInit(uart1);
    (void)lcdSetBacklight(uart1, true);
    (void)lcdSetText(uart1, 1, "PROS V2.12.0    ");
    (void)lcdSetText(uart1, 2, "VEX CORTEX LCD1 ");
    (void)serverStart();
    (void)shellStart(&shellConfig);
    return;
}
