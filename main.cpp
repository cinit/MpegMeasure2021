#include <iostream>
#include <unistd.h>
#include <opencv2/opencv.hpp>

#include "MeasureView.h"
#include "mmtcp/TcpClientSocket.h"
#include "mmtcp/MmTcp.h"
#include "utils/Time.h"
#include "Recognition.h"
#include "ui/Widgets.h"

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
            const auto targetsA = findTargets(imgA, lastMatA, lastPointA);
            for (int i = 0; i < targetsA.size(); i++) {
                auto const &[rect, point, confidence] = targetsA[i];
                cv::rectangle(imgA, rect, cv::Scalar(0, 0, 255), i == 0 ? 2 : 1);
                char buf[32] = {};
                snprintf(buf, 32, "(%d, %d) %.3f", point.x, point.y, confidence);
                lwk::DrawTextLeftCenter(imgA, buf, rect.x, rect.y - 16, cv::Scalar(0, 0, 0));
            }
//            const auto targetsB = findTargets(imgB);
//            for (int i = 0; i < targetsB.size(); i++) {
//                auto const &[rect, point, confidence] = targetsB[i];
//                cv::rectangle(imgB, rect, cv::Scalar(0, 0, 255), i == 0 ? 2 : 1);
//                char buf[32] = {};
//                snprintf(buf, 32, "(%d, %d) %.3f", point.x, point.y, confidence);
//                lwk::DrawTextLeftCenter(imgB, buf, rect.x, rect.y - 16, cv::Scalar(0, 0, 0));
//            }
            imshow("test A", imgA);
//            imshow("test B", imgB);
            if (cv::waitKey(1) == 'q') {
                exit(0);
            }
        }
        usleep(100000);
    }
}
