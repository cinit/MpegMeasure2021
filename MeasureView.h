//
// Created by kinit on 2021-11-04.
//

#ifndef MPEGMEASURE2021_MEASUREVIEW_H
#define MPEGMEASURE2021_MEASUREVIEW_H

#include <array>
#include <opencv2/core/mat.hpp>

class MeasureView {
public:
    using PointRecord = struct {
        float x;
        float y;
    };
    static constexpr int HEIGHT = 400;
    static constexpr int WIDTH = 600;
    static constexpr int POS_RECT_A = 200;
    static constexpr int MAX_VALUE_ABS_OFFSET = 20;


private:
    cv::Mat mFrameBuffer = cv::Mat(cv::Size(WIDTH, HEIGHT), CV_8UC3, cv::Scalar(0, 0, 0));
    int mDataStartIndex = 0;
    int mDataLength = 0;
    int mLastDataIndex = -1;
    std::array<PointRecord, 20> mDataArray;
    bool mBufferDirty = true;

private:
    void onDraw();

public:
    void updateData(float x, float y);

    void updateNoData();

    void clearData();

    inline void invalidate() {
        mBufferDirty = true;
    }

    [[nodiscard]]
    inline cv::Mat getFrameBuffer() {
        if (mBufferDirty) {
            onDraw();
        }
        return mFrameBuffer;
    }
};

#endif //MPEGMEASURE2021_MEASUREVIEW_H
