#include <iostream>
#include <unistd.h>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "MeasureView.h"
#include "mmtcp/TcpClientSocket.h"
#include "mmtcp/MmTcpV2.h"
#include "utils/Time.h"
#include "Recognition.h"
#include "ui/Widgets.h"
#include "MeasureSession.h"
#include "binder/HwManager.h"
#include "binder/LinuxSerial.h"

using namespace std;

static const std::array<const char *, 4> SERIAL_DEVICES = {
        "/dev/ttyUSB0",
        "/dev/ttyACM0",
        "/dev/ttyUSB1",
        "/dev/ttyACM1",
};

static const char *const AIO_WINDOW_NAME = "MpegMeasure";
static const char *const IP_NODE_A = "192.168.24.205";
static const char *const IP_NODE_B = "192.168.24.206";
static constexpr uint16_t NODE_PORT = 8003;

class TargetStatusInfo {
public:
    bool serialStatus = false;
    bool nodeStatusA = false;
    bool nodeStatusB = false;

    [[nodiscard]]
    inline bool isReady() const noexcept {
        return (serialStatus)
               && (nodeStatusA)
               && (nodeStatusB);
    }
};

int main() {
    LinuxSerial serial;
    namedWindow(AIO_WINDOW_NAME);
    moveWindow(AIO_WINDOW_NAME, 0, 60);
    {
        int retry = 0;
        while (true) {
            retry++;
            TargetStatusInfo info;
            if (!serial.isOpened()) {
                for (const char *path: SERIAL_DEVICES) {
                    if (access(path, F_OK) == 0) {
                        if (int err; (err = serial.open(path) != 0)) {
                            cout << "unable to open serial port" << path << " , err=" << err << endl;
                        }
                        if (serial.isOpened()) {
                            break;
                        }
                    }
                }
            }
            info.serialStatus = serial.isOpened();
            info.nodeStatusA = testIpPort(IP_NODE_A, 22);
            info.nodeStatusB = testIpPort(IP_NODE_B, 22);
            if (info.isReady()) {
                break;
            }
            serial.close();
            cv::Mat tmpBuf(cv::Size(600, 600), CV_8UC3, cv::Scalar(0, 0, 0));
            char buf[256] = {};
            Scalar color = Scalar(255, 255, 255);
            lwk::DrawTextLeftCenter(tmpBuf, (info.serialStatus ? "serial: true" : "serial: FALSE"), 32, 32, color);
            lwk::DrawTextLeftCenter(tmpBuf, (info.nodeStatusA ? "A: true" : "A: FALSE"), 32, 64, color);
            lwk::DrawTextLeftCenter(tmpBuf, (info.nodeStatusB ? "B: true" : "B: FALSE"), 32, 96, color);
            snprintf(buf, 64, "retry = %d", retry);
            lwk::DrawTextLeftCenter(tmpBuf, buf, 32, 128, color);
            imshow(AIO_WINDOW_NAME, tmpBuf);
            waitKey(1);
            usleep(100000);
        }
    }
    HwManager hwManager;
    hwManager.setSerialManager(&serial);
    while (true) {
        TcpClientSocket serverA;
        TcpClientSocket serverB;
        MmTcpV2 connA;
        MmTcpV2 connB;
        {
            int err;
            err = serverA.connectToIpV4(IP_NODE_A, NODE_PORT);
            if (err < 0) {
                cout << "unable to connect to A :" << err << endl;
                usleep(1000000);
                continue;
            }
            err = serverB.connectToIpV4(IP_NODE_B, NODE_PORT);
            if (err < 0) {
                cout << "unable to connect to B :" << err << endl;
                usleep(1000000);
                continue;
            }
            connA.setSocket(serverA.detach());
            connB.setSocket(serverB.detach());
        }
        // start video
        Mat lastMatA;
        Point lastPointA;
        Mat lastMatB;
        Point lastPointB;
        MeasureSession measureSession;
        bool showResult = false;
        uint64_t hwCtlStartTime = 0;
        struct {
            uint64_t lastTime = 0;
            int counter = 0;
            int fps = 0;
            float coreTemperature = 0.0f;
        } fpsCounter;
        while (true) {
            uint64_t startTime1 = getRelativeTimeMs();
            struct {
                cv::Mat img;
                uint64_t frameTime;
                uint32_t frameCost;
            } imgInfoA, imgInfoB;
            if (connA.readImage(imgInfoA.img, imgInfoA.frameTime, imgInfoA.frameCost) != 0) {
                cout << "raed A error" << endl;
                break;
            }
            if (connB.readImage(imgInfoB.img, imgInfoB.frameTime, imgInfoB.frameCost) != 0) {
                cout << "raed B error" << endl;
                break;
            }
            fpsCounter.counter++;
            if (startTime1 - fpsCounter.lastTime >= 1000) {
                fpsCounter.fps = fpsCounter.counter;
                fpsCounter.counter = 0;
                fpsCounter.lastTime = startTime1;
                int temp = getCpuTemperature();
                fpsCounter.coreTemperature = temp > 0 ? (float(temp) / 1000.0f) : NAN;
            }
            cv::Mat &imgA = imgInfoA.img;
            cv::Mat &imgB = imgInfoB.img;
            if (imgA.empty()) {
                cout << "read img error" << endl;
                break;
            }
            if (imgB.empty()) {
                cout << "read img error" << endl;
                break;
            }
            if (!measureSession.isInitialized()) {
                measureSession.init(imgA.cols, imgA.rows);
            }
            const auto targetsA = findTargets(imgA, lastMatA, lastPointA);
            for (int i = 0; i < targetsA.size(); i++) {
                auto const &[rect, point, confidence] = targetsA[i];
                cv::rectangle(imgA, rect, cv::Scalar(0, 0, i == 0 ? 255 : 0), i == 0 ? 2 : 1);
                char buf[32] = {};
                snprintf(buf, 32, "(%d, %d) %.3f", point.x, point.y, confidence);
                lwk::DrawTextLeftCenter(imgA, buf, rect.x, rect.y - 16, cv::Scalar(0, 0, i == 0 ? 255 : 0));
            }
            const auto targetsB = findTargets(imgB, lastMatB, lastPointB);
            for (int i = 0; i < targetsB.size(); i++) {
                auto const &[rect, point, confidence] = targetsB[i];
                cv::rectangle(imgB, rect, cv::Scalar(0, 0, i == 0 ? 255 : 0), i == 0 ? 2 : 1);
                char buf[32] = {};
                snprintf(buf, 32, "(%d, %d) %.3f", point.x, point.y, confidence);
                lwk::DrawTextLeftCenter(imgB, buf, rect.x, rect.y - 16, cv::Scalar(0, 0, i == 0 ? 255 : 0));
            }
            if (!targetsA.empty() && !targetsB.empty()) {
                const auto &[ra, pa, ca] = targetsA[0];
                const auto &[rb, pb, cb] = targetsB[0];
                if (hwCtlStartTime != 0) {
                    measureSession.updateFrame(pa, {imgInfoA.frameTime, imgInfoA.frameTime + imgInfoA.frameCost},
                                               pb, {imgInfoB.frameTime, imgInfoB.frameTime + imgInfoB.frameCost});
                }
            }
            if (const auto &periods = measureSession.getPeriodDataA(); !periods.empty()) {
                for (int i = 0; i < periods.size(); i++) {
                    const auto &r = periods[i];
                    char buf[32];
                    cv::circle(imgA, r.point, 1, Scalar(255, 128, 128), 2);
                    constexpr auto LEFT_EDGE = MeasureSession::EdgePointType::LEFT_EDGE;
                    constexpr auto RIGHT_EDGE = MeasureSession::EdgePointType::RIGHT_EDGE;
                    snprintf(buf, 32, "%d%s:%dms", i, r.type == LEFT_EDGE ? "L" : (r.type == RIGHT_EDGE ? "R" : "?"),
                             r.deltaTimeMs);
//                    lwk::DrawTextCenterH(imgA, buf, r.point.x, r.point.y + 12 + 16 * i, Scalar(255, 0, 0));
                }
            }
            if (const auto &periods = measureSession.getPeriodDataB(); !periods.empty()) {
                for (int i = 0; i < periods.size(); i++) {
                    const auto &r = periods[i];
                    char buf[32];
                    cv::circle(imgB, r.point, 1, Scalar(255, 128, 128), 2);
                    constexpr auto LEFT_EDGE = MeasureSession::EdgePointType::LEFT_EDGE;
                    constexpr auto RIGHT_EDGE = MeasureSession::EdgePointType::RIGHT_EDGE;
                    snprintf(buf, 32, "%d%s:%dms", i, r.type == LEFT_EDGE ? "L" : (r.type == RIGHT_EDGE ? "R" : "?"),
                             r.deltaTimeMs);
//                    lwk::DrawTextCenterH(imgB, buf, r.point.x, r.point.y + 12 + 16 * i, Scalar(255, 0, 0));
                }
            }
            float periodTime = measureSession.calculateT() / 1000.0f;
            float motionDegree = measureSession.calculateTheta();
            float lineLength = 100.0f * 0.24836f * pow(periodTime, 2.0f) - 7.5f;
            float fakeLineLength = 0.9881f * lineLength + 0.9509f;
            char buf[256];
            Mat windowBuffer = Mat(Size(imgA.cols * 2, imgA.rows), CV_8UC3);
            imgA.copyTo(windowBuffer(Rect(0, 0, imgA.cols, imgA.rows)));
            imgB.copyTo(windowBuffer(Rect(imgB.cols, 0, imgB.cols, imgB.rows)));
            if (showResult) {
                snprintf(buf, 64, "length= %.1f cm", lineLength);
                putText(windowBuffer, buf, Point(300, 200), FONT_HERSHEY_TRIPLEX, 2, Scalar(0, 0, 0), 2);
                snprintf(buf, 64, "theta = %.1f deg", motionDegree);
                putText(windowBuffer, buf, Point(300, 300), FONT_HERSHEY_TRIPLEX, 2, Scalar(0, 0, 0), 2);
            }
            snprintf(buf, 256, "T= %.3f s | length= %.1f(%.1f) cm | theta= %.1f | %d/%d epoch(s) | %d fps | %.1f'C",
                     periodTime, lineLength, fakeLineLength, motionDegree,
                     int(measureSession.getPeriodDataA().size()), int(measureSession.getPeriodDataB().size()),
                     fpsCounter.fps, fpsCounter.coreTemperature);
            lwk::DrawTextLeftCenter(windowBuffer, buf, 16, 16, Scalar(0, 0, 0));
            if (hwCtlStartTime != 0) {
                snprintf(buf, 32, "Time=%.03fs", float(getRelativeTimeMs() - hwCtlStartTime) / 1000.0f);
                lwk::DrawTextLeftCenter(windowBuffer, buf, 16, 32, Scalar(0, 0, 0));
            }
            {
                int usableWidth = windowBuffer.cols - 100;
                int startX = 16;
                float dXpMs = float(usableWidth) / float(2000);
                if (periodTime > 0) {
                    // measure line
                    Scalar color = Scalar(255, 255, 255);
                    float halfTMs = periodTime * 1000.0f / 2.0f;
                    int targetX = startX + int(dXpMs * float(min(2000.0f, halfTMs)));
                    snprintf(buf, 32, "%.1f", halfTMs);
                    lwk::DrawTextLeftCenter(windowBuffer, buf, targetX, windowBuffer.rows - 48 - 16, color);
                    cv::line(windowBuffer, Point(targetX, windowBuffer.rows - 48),
                             Point(targetX, windowBuffer.rows - 48 + 32), color, 1);
                }
                for (int i = 0; i < 2; ++i) {
                    const auto &epochs = (i == 0) ? measureSession.getPeriodDataA() : measureSession.getPeriodDataB();
                    Scalar color = (i == 0) ? Scalar(0, 255, 0) : Scalar(255, 0, 0);
                    int startY = windowBuffer.rows - 48 + i * 16;
                    lwk::DrawTextLeftCenter(windowBuffer, i == 0 ? "A" : "B", 8, startY, color);
                    for (const auto &epoch: epochs) {
                        constexpr auto RIGHT_EDGE = MeasureSession::EdgePointType::RIGHT_EDGE;
                        constexpr auto LEFT_EDGE = MeasureSession::EdgePointType::LEFT_EDGE;
                        const char *typeName = epoch.type == RIGHT_EDGE ? ">" : (epoch.type == LEFT_EDGE ? "<" : "?");
                        int targetX = startX + int(dXpMs * float(min(2000, epoch.deltaTimeMs)));
                        lwk::DrawTextLeftCenter(windowBuffer, typeName, targetX, startY, color);
                    }
                }
                for (int i = 0; i < 2000; i += 100) {
                    auto color = Scalar(0, 0, 0);
                    int startY = windowBuffer.rows - 16;
                    int targetX = startX + int(dXpMs * float(min(2000, i)));
                    snprintf(buf, 32, "%d", i);
                    lwk::DrawTextLeftCenter(windowBuffer, i == 0 ? "T/2" : buf, targetX, startY, color);
                }
            }
            imshow(AIO_WINDOW_NAME, windowBuffer);
            int keyCode = cv::waitKey(1);
            if (keyCode == 'q') {
                exit(0);
            }
            HwManager::CmdPacket packet = {};
            if (hwManager.nextCmdPacketAsync(packet) || keyCode == 'm' || keyCode == 'M') {
                if (packet.cmd == 1) {
                    hwManager.transactAndWaitForReply(1, 1, 0);
                } else if (packet.cmd == 2 || keyCode == 'm' || keyCode == 'M') {
                    showResult = false;
                    measureSession.reset();
                    hwCtlStartTime = getRelativeTimeMs();
                }
            }
            if (hwCtlStartTime != 0 && (getRelativeTimeMs() - hwCtlStartTime) >= 25000) {
                hwManager.transactAndWaitForReply(3, 5);
                hwCtlStartTime = 0;
                showResult = true;
            }
        }
        usleep(100000);
    }
}
