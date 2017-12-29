// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/** @file opcontrol.c
 * @brief File for operator control code
 *
 * This file should contain the user operatorControl() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "main.h"
#include "server.h"

static void replay(void);

/*
 * Runs the user operator control code. This function will be started in its own task with the
 * default priority and stack size whenever the robot is enabled via the Field Management System
 * or the VEX Competition Switch in the operator control mode. If the robot is disabled or
 * communications is lost, the operator control task will be stopped by the kernel. Re-enabling
 * the robot will restart the task, not resume it from where it left off.
 *
 * If no VEX Competition Switch or Field Management system is plugged in, the VEX Cortex will
 * run the operator control task. Be warned that this will also occur if the VEX Cortex is
 * tethered directly to a computer via the USB A to A cable without any VEX Joystick attached.
 *
 * Code running in this task can take almost any action, as the VEX Joystick is available and
 * the scheduler is operational. However, proper use of delay() or taskDelayUntil() is highly
 * recommended to give other tasks (including system tasks such as updating LCDs) time to run.
 *
 * This task should never exit; it should end with some kind of infinite loop, even if empty.
 */
void
operatorControl()
{
    (void)lcdClear(uart1);

    serverIpv4_t ipv4;
    unsigned int lcdButtons;

    while (1) {
        (void)lcdPrint(uart1, 1, "%s", serverIsConnected() ? "CONNECTED" : "DISCONNECTED");
        ipv4 = serverGetIpv4();
        if (ipv4.v[0] == 0) {
            (void)lcdSetText(uart1, 2, "                ");
        } else {
            (void)lcdPrint(uart1, 2, "%u.%u.%u.%u", ipv4.v[0], ipv4.v[1], ipv4.v[2], ipv4.v[3]);
        }
        lcdButtons = lcdReadButtons(uart1);
        if (lcdButtons & LCD_BTN_LEFT) {
            replay();
        }
        delay(20);
    }
}

static void
replay(void)
{
    motorSet(1, 0);
    motorSet(2, 0);
    motorSet(3, 0);
    motorSet(4, 0);
    motorSet(5, 0);
    motorSet(6, 0);
    motorSet(7, 0);
    motorSet(8, 0);
    motorSet(9, 0);
    motorSet(10, 0);
    taskDelay(25);
    motorSet(1, 25);
    motorSet(9, 25);
    taskDelay(312);
    motorSet(1, -89);
    motorSet(2, 127);
    motorSet(9, -89);
    motorSet(10, -127);
    taskDelay(104);
    motorSet(1, -90);
    motorSet(9, -90);
    taskDelay(676);
    motorSet(1, -127);
    motorSet(9, -127);
    taskDelay(2340);
    motorSet(1, 0);
    motorSet(2, 0);
    motorSet(9, 0);
    motorSet(10, 0);
    taskDelay(1144);
    motorSet(1, -21);
    motorSet(9, -21);
    taskDelay(286);
    motorSet(1, -67);
    motorSet(2, -127);
    motorSet(9, -67);
    motorSet(10, 127);
    taskDelay(104);
    motorSet(1, -65);
    motorSet(9, -65);
    taskDelay(234);
    motorSet(1, -84);
    motorSet(9, -84);
    taskDelay(104);
    motorSet(1, -88);
    motorSet(9, -88);
    taskDelay(1170);
    motorSet(1, -89);
    motorSet(9, -89);
    taskDelay(598);
    motorSet(1, 0);
    motorSet(2, 0);
    motorSet(9, 0);
    motorSet(10, 0);
    taskDelay(858);
    motorSet(2, -25);
    motorSet(10, 25);
    taskDelay(286);
    motorSet(1, 90);
    motorSet(2, -127);
    motorSet(9, 90);
    motorSet(10, 127);
    taskDelay(104);
    motorSet(1, 89);
    motorSet(9, 89);
    taskDelay(416);
    motorSet(1, 90);
    motorSet(9, 90);
    taskDelay(104);
    motorSet(1, 127);
    motorSet(9, 127);
    taskDelay(338);
    motorSet(2, -90);
    motorSet(10, 90);
    taskDelay(1326);
    motorSet(1, 0);
    motorSet(2, 0);
    motorSet(9, 0);
    motorSet(10, 0);
    taskDelay(962);
    motorSet(1, 21);
    motorSet(2, 30);
    motorSet(9, 21);
    motorSet(10, -30);
    taskDelay(260);
    motorSet(1, 127);
    motorSet(2, 90);
    motorSet(9, 127);
    motorSet(10, -90);
    taskDelay(624);
    motorSet(2, 127);
    motorSet(10, -127);
    taskDelay(1378);
    motorSet(1, 0);
    motorSet(2, 0);
    motorSet(9, 0);
    motorSet(10, 0);
    taskDelay(25);
    motorStopAll();
}
