//
// Created by kinit on 2021-11-05.
//

#ifndef MPEGMEASURE2021_RECOGNITION_H
#define MPEGMEASURE2021_RECOGNITION_H

#include <vector>
#include <tuple>
#include <opencv2/core/mat.hpp>

std::vector<std::tuple<cv::Rect, cv::Point, float>> findTargets(const cv::Mat &rawImage,
                                                                cv::Mat &lastBlurGreyFrame, cv::Point &lastPoint);

#endif //MPEGMEASURE2021_RECOGNITION_H
