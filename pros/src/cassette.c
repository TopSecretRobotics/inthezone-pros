// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et

#include "cassette.h"

#define CASSETTE_MAX 20

static const char cassetteNames[20][3] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
                                          "10", "11", "12", "13", "14", "15", "16", "17", "18", "19"};

uint8_t
cassetteCount(void)
{
    PROS_FILE *fp = NULL;
    uint8_t count = 0;
    for (int i = 0; i < CASSETTE_MAX; i++) {
        fp = fopen(cassetteNames[i], "r");
        if (fp != NULL) {
            count++;
            (void)fclose(fp);
            fp = NULL;
        }
    }
    return count;
}

uint8_t
cassetteFree(void)
{
    return (CASSETTE_MAX - cassetteCount());
}

uint8_t
cassetteMax(void)
{
    return CASSETTE_MAX;
}

PROS_FILE *
cassetteOpenWrite(uint8_t index)
{
    if (index >= CASSETTE_MAX) {
        return NULL;
    }
    return fopen(cassetteNames[index], "w");
}

PROS_FILE *
cassetteOpenRead(uint8_t index)
{
    if (index >= CASSETTE_MAX) {
        return NULL;
    }
    return fopen(cassetteNames[index], "r");
}
