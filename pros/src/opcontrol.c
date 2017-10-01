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

// static void replay(void);

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
    // unsigned int lcdButtons;

    while (1) {
        (void)lcdPrint(uart1, 1, "%s", serverIsConnected() ? "CONNECTED" : "DISCONNECTED");
        ipv4 = serverGetIpv4();
        if (ipv4.v[0] == 0) {
            (void)lcdSetText(uart1, 2, "                ");
        } else {
            (void)lcdPrint(uart1, 2, "%u.%u.%u.%u", ipv4.v[0], ipv4.v[1], ipv4.v[2], ipv4.v[3]);
        }
        // lcdButtons = lcdReadButtons(uart1);
        // if (lcdButtons & LCD_BTN_LEFT) {
        //     replay();
        // }
        delay(20);
    }
}

// static void
// replay(void)
// {
//     motorSet(1, 0);
//     motorSet(2, 0);
//     motorSet(3, 0);
//     motorSet(4, 0);
//     motorSet(5, 0);
//     motorSet(6, 0);
//     motorSet(7, 0);
//     motorSet(8, 0);
//     motorSet(9, 0);
//     motorSet(10, 0);
//     taskDelay(25);
//     motorSet(1, -21);
//     motorSet(2, -25);
//     motorSet(9, -21);
//     motorSet(10, 25);
//     taskDelay(78);
//     motorSet(1, -25);
//     motorSet(2, 0);
//     motorSet(9, -25);
//     motorSet(10, 0);
//     taskDelay(26);
//     motorSet(1, -32);
//     motorSet(2, 22);
//     motorSet(9, -32);
//     motorSet(10, -22);
//     taskDelay(26);
//     motorSet(1, -43);
//     motorSet(2, 36);
//     motorSet(9, -43);
//     motorSet(10, -36);
//     taskDelay(26);
//     motorSet(1, -72);
//     motorSet(2, 57);
//     motorSet(9, -72);
//     motorSet(10, -57);
//     taskDelay(26);
//     motorSet(1, -127);
//     motorSet(2, 88);
//     motorSet(9, -127);
//     motorSet(10, -88);
//     taskDelay(52);
//     motorSet(2, 90);
//     motorSet(10, -90);
//     taskDelay(26);
//     motorSet(2, 127);
//     motorSet(10, -127);
//     taskDelay(78);
//     motorSet(1, -90);
//     motorSet(9, -90);
//     taskDelay(598);
//     motorSet(1, -127);
//     motorSet(9, -127);
//     taskDelay(988);
//     motorSet(1, 0);
//     motorSet(2, 0);
//     motorSet(9, 0);
//     motorSet(10, 0);
//     taskDelay(702);
//     motorSet(1, -22);
//     motorSet(9, -22);
//     taskDelay(52);
//     motorSet(1, 0);
//     motorSet(9, 0);
//     taskDelay(52);
//     motorSet(1, 42);
//     motorSet(2, -52);
//     motorSet(9, 42);
//     motorSet(10, 52);
//     taskDelay(52);
//     motorSet(1, 88);
//     motorSet(2, -127);
//     motorSet(9, 88);
//     motorSet(10, 127);
//     taskDelay(52);
//     motorSet(1, 90);
//     motorSet(9, 90);
//     taskDelay(52);
//     motorSet(1, 127);
//     motorSet(9, 127);
//     taskDelay(104);
//     motorSet(2, -90);
//     motorSet(10, 90);
//     taskDelay(1534);
//     motorSet(1, 0);
//     motorSet(2, 0);
//     motorSet(9, 0);
//     motorSet(10, 0);
//     taskDelay(936);
//     motorSet(1, -23);
//     motorSet(9, -23);
//     taskDelay(208);
//     motorSet(1, 87);
//     motorSet(2, 127);
//     motorSet(9, 87);
//     motorSet(10, -127);
//     taskDelay(52);
//     motorSet(1, 88);
//     motorSet(9, 88);
//     taskDelay(1014);
//     motorSet(1, 89);
//     motorSet(9, 89);
//     taskDelay(468);
//     motorSet(1, 0);
//     motorSet(2, 0);
//     motorSet(9, 0);
//     motorSet(10, 0);
//     taskDelay(780);
//     motorSet(1, -22);
//     motorSet(9, -22);
//     taskDelay(156);
//     motorSet(1, -127);
//     motorSet(2, -127);
//     motorSet(9, -127);
//     motorSet(10, 127);
//     taskDelay(52);
//     motorSet(2, -90);
//     motorSet(10, 90);
//     taskDelay(52);
//     motorSet(2, -127);
//     motorSet(10, 127);
//     taskDelay(1560);
//     motorSet(1, 0);
//     motorSet(2, 0);
//     motorSet(9, 0);
//     motorSet(10, 0);
//     taskDelay(25);
//     motorStopAll();
// }
