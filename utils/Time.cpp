//
// Created by kinit on 2021-11-05.
//
#include <chrono>
#include <cerrno>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>

#include "Time.h"

uint64_t getRelativeTimeMs() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

int getCpuTemperature() {
    int thermalFd;
    thermalFd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
    if (thermalFd < 0) {
        return -errno;
    }
    char buf[32] = {};
    if (read(thermalFd, buf, 32) < 0) {
        int err = errno;
        close(thermalFd);
        return -err;
    }
    close(thermalFd);
    char *outpos = nullptr;
    return int(strtol(buf, &outpos, 10));
}
