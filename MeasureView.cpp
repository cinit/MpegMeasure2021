//
// Created by kinit on 2021-11-04.
//

#include <cmath>
#include <opencv2/imgproc.hpp>

#include "MeasureView.h"
#include "ui/Widgets.h"

using namespace std;
using namespace cv;
using namespace lwk;

void MeasureView::onDraw() {
    rectangle(mFrameBuffer, Point(0, 0), Point(WIDTH, HEIGHT), Scalar(0, 0, 0), cv::FILLED);
    {
        Mat rect = mFrameBuffer(Rect(0, 0, POS_RECT_A, POS_RECT_A));
        line(rect, Point(0, POS_RECT_A / 2), Point(POS_RECT_A, POS_RECT_A / 2), Scalar(255, 255, 255), 1);
        line(rect, Point(POS_RECT_A / 2, 0), Point(POS_RECT_A / 2, POS_RECT_A), Scalar(255, 255, 255), 1);
        float dOffset = float(POS_RECT_A / 2) / MAX_VALUE_ABS_OFFSET;
        for (int i = 0; i < mDataLength; ++i) {
            PointRecord p = mDataArray[i];
            float x = max(min(p.x, float(MAX_VALUE_ABS_OFFSET)), float(-MAX_VALUE_ABS_OFFSET));
            float y = max(min(p.y, float(MAX_VALUE_ABS_OFFSET)), float(-MAX_VALUE_ABS_OFFSET));
            int dispX = int(x * dOffset) + POS_RECT_A / 2;
            int dispY = int(y * dOffset) + POS_RECT_A / 2;
            circle(rect, Point(dispX, dispY), 1, Scalar(120, 120, 120), 2);
        }
        if (mLastDataIndex != -1) {
            PointRecord p = mDataArray[mLastDataIndex];
            float x = max(min(p.x, float(MAX_VALUE_ABS_OFFSET)), float(-MAX_VALUE_ABS_OFFSET));
            float y = max(min(p.y, float(MAX_VALUE_ABS_OFFSET)), float(-MAX_VALUE_ABS_OFFSET));
            int dispX = int(x * dOffset) + POS_RECT_A / 2;
            int dispY = int(y * dOffset) + POS_RECT_A / 2;
            circle(rect, Point(dispX, dispY), 2, Scalar(64, 64, 255), 2);
        }
    }
    {
        if (mLastDataIndex != -1) {
            char buf[64];
            PointRecord p = mDataArray[mLastDataIndex];
            float x = max(min(p.x, float(MAX_VALUE_ABS_OFFSET)), float(-MAX_VALUE_ABS_OFFSET));
            float y = max(min(p.y, float(MAX_VALUE_ABS_OFFSET)), float(-MAX_VALUE_ABS_OFFSET));
            float r = hypot(x, y);
            float theta = 90.0f - atan2(y, x) * 180.0f / 3.1415926f;
            snprintf(buf, 64, "   (%.1f, %.1f) ", x, y);
            DrawTextLeftCenter(mFrameBuffer, buf, 20, POS_RECT_A + 10, Scalar(255, 255, 255));
            snprintf(buf, 64, " r=%.1f  theta=%.1f", r, theta);
            DrawTextLeftCenter(mFrameBuffer, buf, 20, POS_RECT_A + 26, Scalar(255, 255, 255));
        } else {
            DrawTextLeftCenter(mFrameBuffer, "NO DATA", 20, POS_RECT_A + 10, Scalar(255, 255, 255));
        }
    }
    mBufferDirty = false;
}

void MeasureView::updateData(float x, float y) {
    if (mDataLength != mDataArray.size()) {
        // push ++
        mDataArray[mDataLength] = {x, y};
        mLastDataIndex = mDataLength;
        mDataLength++;
    } else {
        mDataStartIndex++;
        mDataStartIndex = mDataStartIndex % int(mDataArray.size());
        mDataArray[mDataStartIndex] = {x, y};
        mLastDataIndex = mDataStartIndex;
    }
    mBufferDirty = true;
}

void MeasureView::clearData() {
    mDataStartIndex = 0;
    mDataLength = 0;
    mLastDataIndex = -1;
    mBufferDirty = true;
}

void MeasureView::updateNoData() {
    mLastDataIndex = -1;
    mBufferDirty = true;
}
