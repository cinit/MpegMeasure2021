//
// Created by kinit on 2021-11-05.
//

#include <cmath>

#include "MeasureSession.h"

using namespace std;
using cv::Point, cv::Mat;

constexpr float NaN = NAN;

constexpr std::array<float, 10> WEIGHT_FIRST_CYCLES =
        {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

void MeasureSession::init(int width, int height) {
    mImageSize = {width, height};
    mPeriodDataA.clear();
    mPeriodDataB.clear();
    mCamInfoA = {};
    mCamInfoB = {};
}

void MeasureSession::updateFrame(const cv::Point &posA_, const std::array<uint64_t, 2> &measureTimeA_,
                                 const cv::Point &posB_, const std::array<uint64_t, 2> &measureTimeB_) {
    // 2fps error
    for (int i: std::array<int, 2>{1, 2}) {
        LastCamInfo &mLastInfoR = (i == 1) ? mCamInfoA : mCamInfoB;
        std::vector<EdgeRecord> &mPeriodDataR = (i == 1) ? mPeriodDataA : mPeriodDataB;
        const cv::Point &posR = (i == 1) ? posA_ : posB_;
        const std::array<uint64_t, 2> &measureTimeR = (i == 1) ? measureTimeA_ : measureTimeB_;
        PointMotionStatus newStatus = PointMotionStatus::RESET;
        if (mLastInfoR.lastX != 0 && mLastInfoR.lastDeltaX != 0) {
            int thisDelta = posR.x - mLastInfoR.lastX;
            if (thisDelta > 0 && mLastInfoR.lastDeltaX > 0) {
                newStatus = PointMotionStatus::GOING_RIGHT;
            } else if (thisDelta < 0 && mLastInfoR.lastDeltaX < 0) {
                newStatus = PointMotionStatus::GOING_LEFT;
            }
        }
        if (mLastInfoR.status == PointMotionStatus::RESET) {
            mLastInfoR.status = newStatus;
        } else if ((mLastInfoR.status == PointMotionStatus::GOING_LEFT
                    && newStatus == PointMotionStatus::GOING_RIGHT)
                   || (mLastInfoR.status == PointMotionStatus::GOING_RIGHT
                       && newStatus == PointMotionStatus::GOING_LEFT)) {
            EdgePointType currentEdgeType = (mLastInfoR.status == PointMotionStatus::GOING_LEFT) ?
                                            EdgePointType::LEFT_EDGE : EdgePointType::RIGHT_EDGE;
            // we are at left
            mLastInfoR.status = newStatus;
            uint64_t recordTime = (measureTimeR[0] + measureTimeR[1]) / 2;
            if (mLastInfoR.lastTimePoint == 0) {
                // first
                mPeriodDataR.push_back(EdgeRecord{posR, 0, currentEdgeType});
            } else {
                int deltaTime = int(recordTime - mLastInfoR.lastTimePoint);
                mPeriodDataR.push_back(EdgeRecord{posR, deltaTime, currentEdgeType});
            }
            mLastInfoR.lastTimePoint = recordTime;
        }
        // update last
        if (mLastInfoR.lastX != 0) {
            if (mLastInfoR.lastX - posR.x != 0) {
                mLastInfoR.lastDeltaX = mLastInfoR.lastX - posR.x;
            }
        }
        mLastInfoR.lastX = posR.x;
    }
}

void MeasureSession::reset() {
    mCamInfoA = {};
    mCamInfoB = {};
    mPeriodDataA.clear();
    mPeriodDataB.clear();
}

float MeasureSession::calculateT() const {
    std::vector<const std::vector<EdgeRecord> *> targetRecordSets;
    float thetaDeg = calculateTheta();
    if (!isnan(thetaDeg) && thetaDeg > 80) {
        // use A only
        targetRecordSets.push_back(&mPeriodDataA);
    } else if (!isnan(thetaDeg) && thetaDeg < 10) {
        // use B only
        targetRecordSets.push_back(&mPeriodDataB);
    } else {
        targetRecordSets.push_back(&mPeriodDataA);
        targetRecordSets.push_back(&mPeriodDataB);
    }
    std::array<float, 2> measuredHalfTPerSEt = {-1.0f, -1.0f};
    for (int setNo = 0; setNo < targetRecordSets.size(); setNo++) {
        const std::vector<EdgeRecord> *pPeriodData = targetRecordSets[setNo];
        const std::vector<EdgeRecord> &periodData = *pPeriodData;
        std::array<float, 2> weightedSumT = {0, 0};
        std::array<float, 2> weightSum = {0, 0};
        for (int i = 1; i < periodData.size(); i++) {
            float biasMultiplexer = (i - 1 < WEIGHT_FIRST_CYCLES.size()) ? (WEIGHT_FIRST_CYCLES[i - 1]) : 1.0f;
            const Point &p1 = periodData[i - 1].point;
            const Point &p2 = periodData[i].point;
            auto distance = float(abs(p1.x - p2.x));
            if (distance < 5) {
                continue;
            }
            float weight = 1.0f;
            int timei = periodData[i].deltaTimeMs;
            float timef = float(timei);
            if (timei < 500 || timei > 1400) {
                continue;
            }
            if (periodData[i].type == EdgePointType::LEFT_EDGE) {
                weightedSumT[0] += biasMultiplexer * weight * timef;
                weightSum[0] += biasMultiplexer * weight;
            } else if (periodData[i].type == EdgePointType::RIGHT_EDGE) {
                weightedSumT[1] += biasMultiplexer * weight * timef;
                weightSum[1] += biasMultiplexer * weight;
            }
        }
        if (weightSum[0] * weightSum[1] != 0) {
            float leftT = weightedSumT[0] / weightSum[0];
            float rightT = weightedSumT[1] / weightSum[1];
            measuredHalfTPerSEt[setNo] = (leftT + rightT) / 2.0f;
        } else {
            measuredHalfTPerSEt[setNo] = NaN;
        }
    }
    if (targetRecordSets.size() == 2) {
        return (isnan(measuredHalfTPerSEt[0]) || isnan(measuredHalfTPerSEt[1])) ? NaN : (
                (measuredHalfTPerSEt[0] + measuredHalfTPerSEt[1]) / 2.0f * 2.0f);
    } else {
        return measuredHalfTPerSEt[0] * 2.0f;
    }
}

float MeasureSession::calculateTheta() const {
    float weightedSumA = 0;
    float weightedSumB = 0;
    float weightA = 0;
    float weightB = 0;
    for (int i = 1; i < mPeriodDataA.size(); i++) {
        float biasMultiplexer = (i - 1 < WEIGHT_FIRST_CYCLES.size()) ? (WEIGHT_FIRST_CYCLES[i - 1]) : 1.0f;
        const Point &p1 = mPeriodDataA[i - 1].point;
        const Point &p2 = mPeriodDataA[i].point;
        float weight = 1.0f;
        float value = float(abs(p1.x - p2.x));
        weightedSumA += biasMultiplexer * weight * value;
        weightA += biasMultiplexer * weight;
    }
    for (int i = 1; i < mPeriodDataB.size(); i++) {
        float biasMultiplexer = (i - 1 < WEIGHT_FIRST_CYCLES.size()) ? (WEIGHT_FIRST_CYCLES[i - 1]) : 1.0f;
        const Point &p1 = mPeriodDataB[i - 1].point;
        const Point &p2 = mPeriodDataB[i].point;
        float weight = 1.0f;
        float value = float(abs(p1.x - p2.x));
        weightedSumB += biasMultiplexer * weight * value;
        weightB += biasMultiplexer * weight;
    }
    float relA = weightA == 0 ? 0 : (weightedSumA / weightA);
    float relB = weightB == 0 ? 0 : (weightedSumB / weightB);
    if (relA == 0 && relB == 0) {
        return NaN;
    }
    float resultRad = atan2(relA, relB) * 180.0f / 3.14159265f;
    return resultRad;
}

const vector<MeasureSession::EdgeRecord> &MeasureSession::getPeriodDataA() const {
    return mPeriodDataA;
}

const vector<MeasureSession::EdgeRecord> &MeasureSession::getPeriodDataB() const {
    return mPeriodDataB;
}

bool MeasureSession::isInitialized() const noexcept {
    return mImageSize.area() > 0;
}
