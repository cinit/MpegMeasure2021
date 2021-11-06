//
// Created by kinit on 2021-11-05.
//

#ifndef MPEGMEASURE2021_MEASURESESSION_H
#define MPEGMEASURE2021_MEASURESESSION_H

#include <cstdint>
#include <array>
#include <tuple>
#include <vector>
#include <opencv2/core/mat.hpp>

class MeasureSession {
public:
    enum class EdgePointType {
        LEFT_EDGE = 1,
        RIGHT_EDGE = 2,
    };

    using EdgeRecord = struct {
        cv::Point point;
        int deltaTimeMs;
        EdgePointType type;
    };

    [[nodiscard]]
    bool isInitialized() const noexcept;

    void init(int width, int height);

    void updateFrame(const cv::Point &posA, const std::array<uint64_t, 2> &measureTimeA,
                     const cv::Point &posB, const std::array<uint64_t, 2> &measureTimeB);

    void reset();

    [[nodiscard]]
    float calculateT() const;

    [[nodiscard]]
    float calculateTheta() const;

    [[nodiscard]]
    const std::vector<EdgeRecord> &getPeriodDataA() const;

    [[nodiscard]]
    const std::vector<EdgeRecord> &getPeriodDataB() const;

private:
    enum class PointMotionStatus {
        RESET = 0,
        GOING_LEFT = 1,
        GOING_RIGHT = 2,
    };

    struct LastCamInfo {
        PointMotionStatus status = PointMotionStatus::RESET;
        int lastX = 0;
        int lastDeltaX = 0;
        uint64_t lastTimePoint = 0;
    };

    cv::Size mImageSize = {0, 0};
    LastCamInfo mCamInfoA = {};
    LastCamInfo mCamInfoB = {};
    std::vector<EdgeRecord> mPeriodDataA;
    std::vector<EdgeRecord> mPeriodDataB;
};

#endif //MPEGMEASURE2021_MEASURESESSION_H
