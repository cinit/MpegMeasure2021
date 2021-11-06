//
// Created by kinit on 2021-11-06.
//

#include <unistd.h>
#include <cstring>
#include <cstdio>

#include "HwManager.h"

static constexpr bool DEBUG_PRINT_RX = true;

static inline void msleep(int ms) {
    usleep(1000 * ms);
}

void HwManager::setSerialManager(SerialInterface *serialInterface) {
    mSerial = serialInterface;
}

SerialInterface *HwManager::getSerialManager() {
    return mSerial;
}

HwManager::CmdPacket &HwManager::updateChecksum(CmdPacket &pk) {
    pk.checksum = (uchar) (pk.cmd + pk.arg1 + pk.arg2);
    return pk;
}

int HwManager::transactAndWaitForReply(uchar cmd, uchar arg1, uchar arg2, CmdPacket *reply, bool junk) {
    CmdPacket pk = {0x75, cmd, arg1, arg2, 0xFF};
    updateChecksum(pk);
    return transactAndWaitForReply(pk, reply, junk);
}

int HwManager::transactAndWaitForReply(const CmdPacket &pk, CmdPacket *reply, bool junk) {
    if (mSerial == nullptr) {
        return 0xFF;
    }
    int maxWait = 10;
    int maxTry = 2;
    dropPendingRxPacket();
    do {
        mSerial->transmit(&pk, 5);
        CmdPacket resp = {};
        while (!nextCmdPacketAsync(resp) && maxWait > 0) {
            msleep(5);
            maxWait--;
        }
        if (maxWait <= 0) {
            maxTry--;
//            if (DEBUG_PRINT_RX) {
//                printf("stage 1 out\n");
//            }
            continue;
        }
        if (junk) {
            maxWait = 10;
            do {
                maxWait--;
                msleep(5);
            } while ((!nextCmdPacketAsync(resp)) && maxWait > 0);
        }
        if (maxWait <= 0) {
//            if (DEBUG_PRINT_RX) {
//                printf("stage 2 out\n");
//            }
            maxTry--;
            continue;
        }
        if (verifyChecksum(resp)) {
            if (resp.cmd != 0) {
                printf("E: cmd 0x%02x(%d,%d) get invalid resp: 0x%02x(%d,%d)\n",
                       pk.cmd, pk.arg1, pk.arg2, resp.cmd, resp.arg1, resp.arg2);
                maxTry--;
                maxWait = 5;
            } else {
                if (resp.arg1 != 0) {
                    printf("I: cmd 0x%02x(%d,%d) result 0x%02x\n",
                           pk.cmd, pk.arg1, pk.arg2, resp.arg1);
                }
                if (reply != nullptr) {
                    memcpy(&reply, &resp, 5);
                }
                return resp.cmd;
            }
        } else {
            printf("W: recv checksum mismatch\n");
            maxTry--;
            maxWait = 5;
        }
        if ((maxTry <= 0) || (maxWait-- <= 0)) {
            printf("E: wait for cmd 0x%02x(%d,%d) reply timeout\n", pk.cmd, pk.arg1, pk.arg2);
            return 0xFF;
        }
    } while (maxTry > 0);
    printf("E: wait for cmd 0x%02x(%d,%d) reply timeout\n", pk.cmd, pk.arg1, pk.arg2);
    return 0xFF;
}

bool HwManager::verifyChecksum(const CmdPacket &pk) {
    return pk.checksum == pk.cmd + pk.arg1 + pk.arg2;
}

void HwManager::dropPendingRxPacket() {
    char dummy[64];
    mSerial->receive(dummy, 64);
    cmdbuf_len = cmdbuf_start = 0;
    if (DEBUG_PRINT_RX) {
        printf("drop rx\n");
    }
}

bool HwManager::nextCmdPacketAsync(CmdPacket &pk) {
    //check buffer
    if (cmdbuf_len < 32 && (256 - cmdbuf_start - cmdbuf_len < 64)) {
        printf("Reset buffer\n");
        memcpy(cmdbuf, cmdbuf + cmdbuf_start, cmdbuf_len);
        cmdbuf_start = 0;
    }
    int rl = mSerial->receive(cmdbuf + cmdbuf_start + cmdbuf_len, 256 - cmdbuf_start - cmdbuf_len);
    cmdbuf_len += rl;
    if (rl > 0) {
        if (DEBUG_PRINT_RX) {
            for (int i = 0; i < rl; i++) {
                printf("%02x, ", cmdbuf[(cmdbuf_start + i) % 256]);
            }
        }
        if (DEBUG_PRINT_RX) {
            printf("\n");
        }
    }
    //skip bad value
    while (cmdbuf_len > 0 && *(cmdbuf + cmdbuf_start) != 0x75) {
        cmdbuf_len--;
        cmdbuf_start++;
    }
    if (cmdbuf_len >= 5 && *(cmdbuf + cmdbuf_start) == 0x75) {
        //start from 6
        memcpy(&pk, cmdbuf + cmdbuf_start, 5);
        cmdbuf_len -= 5;
        cmdbuf_start += 5;
//        if (DEBUG_PRINT_RX) {
//            printf("consume5\n");
//        }
        return true;
    }
    return false;
}
