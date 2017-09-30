// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * convex_compat.h
 */

#ifndef CONVEX_COMPAT_H_

#define CONVEX_COMPAT_H_

#include <API.h>

#define chTimeNow() millis()
#define chTimeElapsedSince(start) (chTimeNow() - (start))
#define kVexMotor_1 0
#define kVexMotor_2 1
#define kVexMotor_3 2
#define kVexMotor_4 3
#define kVexMotor_5 4
#define kVexMotor_6 5
#define kVexMotor_7 6
#define kVexMotor_8 7
#define kVexMotor_9 8
#define kVexMotor_10 9
#define kVexMotorNum 10
#define vexSleep(n) taskDelay(n)

#endif
