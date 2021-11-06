//
// Created by kinit on 2021-11-06.
//

#ifndef MPEGMEASURE2021_HWMANAGER_H
#define MPEGMEASURE2021_HWMANAGER_H

#include <array>

#include "SerialInterface.h"

using uchar = unsigned char;

class HwManager {
public:
    using CmdPacket = struct {
        uchar header;
        uchar cmd;
        uchar arg1;
        uchar arg2;
        uchar checksum;
    };

    static_assert(sizeof(CmdPacket) == 5, "CmdPacket size error");

    void setSerialManager(SerialInterface *serialInterface);

    SerialInterface *getSerialManager();

    bool nextCmdPacketAsync(CmdPacket &pk);

    int transactAndWaitForReply(uchar cmd, uchar arg1 = 0, uchar arg2 = 0,
                                CmdPacket *reply = nullptr, bool junk = false);

protected:
    int transactAndWaitForReply(const CmdPacket &pk, CmdPacket *reply = nullptr, bool junk = true);

    static CmdPacket &updateChecksum(CmdPacket &pk);

    static bool verifyChecksum(const CmdPacket &pk);

    void dropPendingRxPacket();


private:
    SerialInterface *mSerial = nullptr;
    uchar cmdbuf[256] = {};
    int cmdbuf_start = 0;
    int cmdbuf_len = 0;
};

#endif //MPEGMEASURE2021_HWMANAGER_H
