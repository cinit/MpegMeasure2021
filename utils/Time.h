//
// Created by kinit on 2021-11-05.
//

#ifndef MPEGMEASURE2021_TIME_H
#define MPEGMEASURE2021_TIME_H

#include <cstdint>

uint64_t getRelativeTimeMs();

int getCpuTemperature();

bool testIpPort(const char *ip, uint16_t port);

#endif //MPEGMEASURE2021_TIME_H
