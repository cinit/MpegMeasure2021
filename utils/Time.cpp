//
// Created by kinit on 2021-11-05.
//
#include <chrono>

#include "Time.h"

uint64_t getRelativeTimeMs() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}
