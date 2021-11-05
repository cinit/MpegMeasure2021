//
// Created by kinit on 2019/12/5.
//

#ifndef OPENCV_LJL0002_VGAFONT_H
#define OPENCV_LJL0002_VGAFONT_H

#include <opencv2/core/mat.hpp>

class RasterGlyph {

public:
    RasterGlyph();

    RasterGlyph(int c, int width, int gly[], int valid, int gly_length);

    int chr;
    int valid;
    int data[16];
    int width;

    int getHeight() {
        //;return data.length;
        return 16;
    }

    int getWidth() {
        return valid;
    }

    static float renderGlyph(const cv::Mat &canvas, RasterGlyph glyph, int stx, int sty, cv::Scalar color);

    static float renderGlyphAutoColor(const cv::Mat &canvas, RasterGlyph glyph, int stx, int sty);
};

class RasterFont {
public:
    int version;
    char copyright[60];
    char facename[16];
    int pointsize;
    int ascent;
    int width;
    int height;
    int italic;
    int underline;
    int strikeout;
    int weight;
    int charset;
    RasterGlyph chars[256];
public:
    RasterGlyph load(int c);
};

extern RasterFont FIXEDSYS;

#endif //OPENCV_LJL0002_VGAFONT_H
