//
// Created by kinit on 2021-11-05.
//

#include <vector>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "Recognition.h"

using namespace cv;
using namespace std;

constexpr int REF_LP_HEIGHT_PX = 55; // height in px

vector<tuple<Rect, Point, float>> findTargets(const Mat &rawImage, Mat &lastBlurGreyFrame, cv::Point &lastPoint) {
    const int STEP_X = rawImage.cols / 20;
    const int STEP_Y = rawImage.rows / 20;
    int sumVal[3] = {0, 0, 0};
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            const auto &p = rawImage.at<Vec3b>(STEP_Y / 2 + j * STEP_Y, STEP_X / 2 + i * STEP_X);
            sumVal[0] += p[0];
            sumVal[1] += p[1];
            sumVal[2] += p[2];
        }
    }
    sumVal[0] /= 19 * 19;
    sumVal[1] /= 19 * 19;
    sumVal[2] /= 19 * 19;
    Mat blurGreyImg;
    {
        Mat tmp;
        cvtColor(rawImage, tmp, COLOR_BGR2GRAY);
        GaussianBlur(tmp, blurGreyImg, Size(3, 3), 0);
    }
    Mat binaryImg;
    int th = (sumVal[0] + sumVal[1] + sumVal[2]) / 3;
//    cout << "avg" << th << endl;
    th = (th / 2 + 70) / 2;
//    cout << "th" << th << endl;
    threshold(blurGreyImg, binaryImg, th, 255, THRESH_BINARY_INV);
    Mat tmp;
    erode(binaryImg, tmp, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
    dilate(tmp, binaryImg, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
    dilate(binaryImg, tmp, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));
    erode(tmp, binaryImg, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(binaryImg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
//    imshow("te", binaryImg);
    vector<tuple<vector<Point>, Rect, float>> candidateContours;
    Mat diffImg;
    if (!lastBlurGreyFrame.empty()) {
        absdiff(blurGreyImg, lastBlurGreyFrame, diffImg);
//        imshow("diff", diffImg);
    }
    for (const auto &contour: contours) {
        Rect r = boundingRect(contour);
        if (r.height > REF_LP_HEIGHT_PX * 2 || r.width > REF_LP_HEIGHT_PX * 2) {
            continue;
        }
        if (r.height < REF_LP_HEIGHT_PX / 2 || r.width < REF_LP_HEIGHT_PX / 8) {
            continue;
        }
        if (r.x == 0 || r.y == 0 || r.x + r.width >= rawImage.cols - 1 || r.y + r.height >= rawImage.rows - 1) {
            continue;
        }
        float hwRatio = float(r.height) / float(r.width);
        if (hwRatio < 1.2f || hwRatio > 5.0f) {
            continue;
        }
        vector<Point> hull;
        convexHull(contour, hull);
        float hullArea = float(contourArea(hull));
        float cntArea = float(contourArea(contour));
        if (cntArea / hullArea < 0.7) [[unlikely]] {
            continue;
        }
        float motionMultiplexer = 1.0f;
        if (!diffImg.empty()) {
            int startX = max(0, r.x - r.width / 2);
            int endX = min(r.x + r.width, r.x + r.width * 3 / 2);
            int pixelCount = r.height * (endX - startX);
            Scalar pxSum = sum(diffImg(Rect(startX, r.y, endX - startX, r.height)));
            float avgMotion = float(pxSum[0]) / float(pixelCount);
            motionMultiplexer = (1.0f + 1.4f * atan(avgMotion / 10.0f));
//            printf("%.3f -> %.2f\n", avgMotion, motionMultiPower);
        }
        float proxiMultiplexer = 1.0f;
        if (lastPoint.x >= 1 && lastPoint.y >= 1) {
            Point center = Point(r.x + r.width / 2, r.y + r.height / 2);
            auto distance = int(hypot(lastPoint.x - center.x, lastPoint.y - center.y));
            proxiMultiplexer = 1.0f / (1.0f + pow(float(distance) / (rawImage.cols / 2), 2.0f));
//            printf("%d, %0.2f\n", distance, proxiMultiplexer);
        }
        float confidence = 10.0f * max(1.0f - float(abs(r.height - REF_LP_HEIGHT_PX) / 40.0f), 0.3f)
                           * pow(1.0f - abs(3.0f - hwRatio), 1.0f)
                           * pow(cntArea / float(r.width) / float(r.height), 1.0f)
                           * pow(cntArea / hullArea, 2.0f)
                           * motionMultiplexer * proxiMultiplexer;
        candidateContours.emplace_back(contour, r, confidence);
    }
    lastBlurGreyFrame = blurGreyImg;
    if (candidateContours.empty()) [[unlikely]] {
        return {};
    }
    vector<tuple<Rect, Point, float>> resultSet;
    for (const auto&[con, r, c]: candidateContours) {
        Point center = Point(r.x + r.width / 2, r.y + r.height / 2);
        resultSet.emplace_back(r, center, c);
    }
    sort(resultSet.begin(), resultSet.end(),
         [](const tuple<Rect, Point, float> &a, const tuple<Rect, Point, float> &b) {
             return (get<2>(a) - get<2>(b)) > 0;
         });
    float smSum = 0;
    for (int i = int(resultSet.size()) - 1; i >= 0; i--) {
        smSum += exp(get<2>(resultSet[i]));
    }
    for (int i = int(resultSet.size()) - 1; i >= 0; i--) {
        get<2>(resultSet[i]) = exp(get<2>(resultSet[i])) / smSum;
    }
    lastPoint = get<1>(resultSet[0]);
    return resultSet;
}
