#include <iostream>
#include <unistd.h>
#include <opencv2/opencv.hpp>

#include "MeasureView.h"
#include "mmtcp/TcpClientSocket.h"
#include "mmtcp/MmTcp.h"
#include "utils/Time.h"
#include "Recognition.h"
#include "ui/Widgets.h"
#include "MeasureSession.h"

using namespace std;

int main() {
    while (true) {
        TcpClientSocket serverA;
        TcpClientSocket serverB;
        MmTcp connA;
        MmTcp connB;
        {
            int err;
            err = serverA.connectToIpV4("192.168.24.205", 8001);
            if (err < 0) {
                cout << "unable to connect to A :" << err << endl;
                usleep(1000000);
                continue;
            }
            err = serverB.connectToIpV4("192.168.24.206", 8001);
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
        while (true) {
            uint64_t startTime1 = getRelativeTimeMs();
            cv::Mat imgA = connA.readImage();
            uint64_t startTime2 = getRelativeTimeMs();
            cv::Mat imgB = connB.readImage();
            uint64_t startTime3 = getRelativeTimeMs();
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
                cv::rectangle(imgA, rect, cv::Scalar(0, 0, 255), i == 0 ? 2 : 1);
                char buf[32] = {};
                snprintf(buf, 32, "(%d, %d) %.3f", point.x, point.y, confidence);
                lwk::DrawTextLeftCenter(imgA, buf, rect.x, rect.y - 16, cv::Scalar(0, 0, 0));
            }
            const auto targetsB = findTargets(imgB, lastMatB, lastPointB);
            for (int i = 0; i < targetsB.size(); i++) {
                auto const &[rect, point, confidence] = targetsB[i];
                cv::rectangle(imgB, rect, cv::Scalar(0, 0, 255), i == 0 ? 2 : 1);
                char buf[32] = {};
                snprintf(buf, 32, "(%d, %d) %.3f", point.x, point.y, confidence);
                lwk::DrawTextLeftCenter(imgB, buf, rect.x, rect.y - 16, cv::Scalar(0, 0, 0));
            }
            if (!targetsA.empty() && !targetsB.empty()) {
                const auto &[ra, pa, ca] = targetsA[0];
                const auto &[rb, pb, cb] = targetsB[0];
                measureSession.updateFrame(pa, {startTime1, startTime2}, pb, {startTime2, startTime3});
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
                    lwk::DrawTextCenterH(imgA, buf, r.point.x, r.point.y + 12 + 16 * i, Scalar(255, 0, 0));
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
                    lwk::DrawTextCenterH(imgB, buf, r.point.x, r.point.y + 12 + 16 * i, Scalar(255, 0, 0));
                }
            }
            {
                float periodTime = measureSession.calculateT() / 1000.0f;
                float lineLength = 100.0f * 0.24836f * pow(periodTime, 2.0f) - 7.5f;
                char buf[64];
                snprintf(buf, 64, "T=%.3fs length=%.1fcm", periodTime, lineLength);
                lwk::DrawTextLeftCenter(imgA, buf, 16, 16, Scalar(0, 0, 0));
                lwk::DrawTextLeftCenter(imgB, buf, 16, 16, Scalar(0, 0, 0));
            }
            imshow("test A", imgA);
            imshow("test B", imgB);
            if (cv::waitKey(1) == 'q') {
                exit(0);
            }
        }
        usleep(100000);
    }
}