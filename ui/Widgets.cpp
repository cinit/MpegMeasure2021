//
// Created by kinit on 2019/12/4.
//

#include <opencv2/imgcodecs.hpp>
#include "opencv2/photo.hpp"
#include <opencv2/highgui/highgui.hpp>

#include "Widgets.h"
#include "VgaFont.h"

using namespace cv;


namespace lwk {

const Scalar COLOTR_DEFAULT = Scalar(200, 208, 212);

Button::Button(int _x, int _y, int _w, int _h, const char *_text, void (*click)(Button *), int enabled) {
    r = Rect(_x, _y, _w, _h);
    text = _text;
    isClicked = 0;
    isEnabled = enabled;
    mOnClickListener = click;
}

void Button::draw(Mat buf) {
    rectangle(buf, r, COLOTR_DEFAULT, FILLED);
    if (isEnabled && isClicked) {
        line(buf, Point(r.x, r.y), Point(r.x, r.y + r.height), Scalar(64, 64, 64));
        line(buf, Point(r.x, r.y), Point(r.x + r.width, r.y), Scalar(64, 64, 64));
        line(buf, Point(r.x + +1, r.y + 1), Point(r.x + 1, r.y + r.height - 1),
             Scalar(128, 128, 128));
        line(buf, Point(r.x + 1, r.y + 1), Point(r.x + r.width - 1, r.y + 1),
             Scalar(128, 128, 128));
        line(buf, Point(r.x + r.width, r.y + r.height), Point(r.x + 1, r.y + r.height), Scalar(255, 255, 255));
        line(buf, Point(r.x + r.width, r.y + r.height), Point(r.x + r.width, r.y + 1), Scalar(255, 255, 255));
    } else {
        line(buf, Point(r.x, r.y), Point(r.x + r.width - 1, r.y), Scalar(255, 255, 255));
        line(buf, Point(r.x, r.y), Point(r.x, r.y + r.height - 1), Scalar(255, 255, 255));
        line(buf, Point(r.x + r.width, r.y + r.height), Point(r.x, r.y + r.height), Scalar(64, 64, 64));
        line(buf, Point(r.x + r.width, r.y + r.height), Point(r.x + r.width, r.y), Scalar(64, 64, 64));
        line(buf, Point(r.x + r.width - 1, r.y + r.height - 1), Point(r.x + 1, r.y + r.height - 1),
             Scalar(128, 128, 128));
        line(buf, Point(r.x + r.width - 1, r.y + r.height - 1), Point(r.x + r.width - 1, r.y + 1),
             Scalar(128, 128, 128));
    }
    if (text != nullptr) {
        int len = strlen(text);
        if (isEnabled) {
            int startx = r.x + r.width / 2 - len * 4 + (isClicked ? 1 : 0);
            int starty = r.y + r.height / 2 - 8 + (isClicked ? 1 : 0);
            for (int i = 0; i < len; i++) {
                RasterGlyph::renderGlyph(buf, FIXEDSYS.load(text[i]), startx + 8 * i, starty, Scalar(0, 0, 0));
            }
        } else {
            int startx = r.x + r.width / 2 - len * 4;
            int starty = r.y + r.height / 2 - 8;
            for (int i = 0; i < len; i++) {
                RasterGlyph::renderGlyph(buf, FIXEDSYS.load(text[i]), startx + 8 * i + 1, starty + 1,
                                         Scalar(255, 255, 255));
                RasterGlyph::renderGlyph(buf, FIXEDSYS.load(text[i]), startx + 8 * i, starty,
                                         Scalar(128, 128, 128));
            }
        }
    }
}

bool Button::onMouseAction(int event, int x, int y, int flags, void *ustc, int *dirty = nullptr) {
    int redraw = 0;
    if (!isEnabled)return false;
    switch (event) {
        case EVENT_LBUTTONDOWN:
            if (r.contains(Point(x, y))) {
                isClicked = 1;
                redraw = 1;
            } else {
                return false;
            }
            break;
        case EVENT_LBUTTONUP:
            if (isClicked && r.contains(Point(x, y))) {
                isClicked = 0;
                redraw = 1;
                if (mOnClickListener != nullptr) {
                    mOnClickListener(this);
                }
            } else {
                return false;
            }
            break;
        case EVENT_MOUSEMOVE:
            if (isClicked) {
                if (!r.contains(Point(x, y))) {
                    isClicked = 0;
                } else {
                    isClicked = 1;
                }
                redraw = 1;
            } else {
                return false;
            }
            break;
        default:
            return false;
    }
    if (redraw && dirty != nullptr) {
        *dirty = 1;
    }
    return true;
}


LevelTrackBar::LevelTrackBar(int _x, int _y, int _w, int _h, int _maxLevel, int currLevel,
                             void (*click)(LevelTrackBar *, int)) {
    r = Rect(_x, _y, _w, _h);
    mOnLevelChangedListener = click;
    levelCount = _maxLevel;
    mShowLevel = level = currLevel;
    mDragging = 0;
}


void LevelTrackBar::draw(Mat buf) {
    rectangle(buf, r, COLOTR_DEFAULT, FILLED);
    int width = r.width;
    int ybar = r.y + r.height / 2 - 4;
    line(buf, Point(r.x, ybar - 2), Point(r.x + r.width - 2, ybar - 2), Scalar(128, 128, 128));
    line(buf, Point(r.x, ybar - 2), Point(r.x, ybar), Scalar(128, 128, 128));
    line(buf, Point(r.x + 1, ybar - 1), Point(r.x + r.width - 3, ybar - 1), Scalar(64, 64, 64));
    line(buf, Point(r.x, ybar + 1), Point(r.x + r.width - 1, ybar + 1), Scalar(255, 255, 255));
    line(buf, Point(r.x + r.width - 1, ybar - 2), Point(r.x + r.width - 1, ybar + 1), Scalar(255, 255, 255));
    //2刻度
    if (levelCount == 0)return;
    line(buf, Point(r.x + 5, ybar + 13), Point(r.x + 5, ybar + 17), Scalar(0, 0, 0));
    line(buf, Point(r.x + r.width - 6, ybar + 13), Point(r.x + r.width - 6, ybar + 17), Scalar(0, 0, 0));
    float len = r.width - 11;
    int startx, starty;
    uchar *pc;
    float stepLen = len / ((float) (levelCount - 1));
    for (int i = 0; i < levelCount; i++) {
        int delta = i * stepLen + 0.5f;
        if (i)line(buf, Point(r.x + delta + 5, ybar + 13), Point(r.x + delta + 5, ybar + 16), Scalar(0, 0, 0));
        if ((mDragging ? mShowLevel : level) == i) {
            //draw block
            rectangle(buf, Rect(r.x + delta, ybar - 2, 11, 4), COLOTR_DEFAULT, FILLED);
            startx = r.x + delta;
            starty = ybar - 9;
            line(buf, Point(startx, starty), Point(startx, starty + 15), Scalar(255, 255, 255));
            line(buf, Point(startx, starty), Point(startx + 10, starty), Scalar(255, 255, 255));
            line(buf, Point(startx + 10, starty), Point(startx + 10, starty + 15), Scalar(64, 64, 64));
            line(buf, Point(startx + 9, starty + 1), Point(startx + 9, starty + 15), Scalar(128, 128, 128));
            for (int i = 0; i < 5; i++) {
                pc = buf.ptr(ybar + 11 - i, startx + 5 - i);
                pc[0] = pc[1] = pc[2] = 255;
                pc = buf.ptr(ybar + 11 - i, startx + 5 + i);
                pc[0] = pc[1] = pc[2] = 64;
                pc = buf.ptr(ybar + 11 - i - 1, startx + 5 + i);
                pc[0] = pc[1] = pc[2] = 128;
            }
        }
    }
}

bool LevelTrackBar::onMouseAction(int event, int x, int y, int flags, void *ustc, int *dirty = nullptr) {
    int redraw = 0;
    if (levelCount < 1)return false;
    int width = r.width;
    int ybar = r.y + r.height / 2 - 4;
    int rely = y - ybar;
    int relx = x - r.x;
    if (!mDragging && (rely > 8 || rely < -16 || relx < 0 || relx > r.width)) {
        return false;
    }
    float len = r.width - 10;
    float stepLen = len / ((float) (levelCount - 1));
    int currentPos = (int) (stepLen * (float) level + 0.5);
    int newlev;
    switch (event) {
        case EVENT_LBUTTONDOWN:
            if (rely > 8 || rely < -16 || relx < 0 || relx > r.width) {
                mDragging = 0;
                mShowLevel = level;
                break;
            }
            if (abs(currentPos - relx + 5) < 6) {
                mDragging = 1;
                mShowLevel = level;
                break;
            }
            newlev = (int) (((float) relx - 5.0f) / stepLen + 0.5f);
            if (newlev < 0)newlev = 0;
            if (newlev >= levelCount)newlev = levelCount - 1;
            if (level != newlev) {
                mShowLevel = level = newlev;
                redraw = 1;
                if (mOnLevelChangedListener != nullptr) {
                    mOnLevelChangedListener(this, newlev);
                }
            }
            mDragging = 0;
            break;
        case EVENT_LBUTTONUP:
            if (!mDragging)break;
            mDragging = 0;
            newlev = (int) (((float) relx - 5.0f) / stepLen + 0.5f);
            if (newlev < 0)newlev = 0;
            if (newlev >= levelCount)newlev = levelCount - 1;
            if (level != newlev) {
                mShowLevel = level = newlev;
                redraw = 1;
                if (mOnLevelChangedListener != nullptr) {
                    mOnLevelChangedListener(this, newlev);
                }
            }
            break;
        case EVENT_MOUSEMOVE:
            if (mDragging) {
                newlev = (int) (((float) relx - 5.0f) / stepLen + 0.5f);
                if (newlev < 0)newlev = 0;
                if (newlev >= levelCount)newlev = levelCount - 1;
                if (mShowLevel != newlev) {
                    mShowLevel = newlev;
                    redraw = 1;
                }
            }
            break;
        default:
            return false;
    }
    if (redraw && dirty != nullptr) {
        *dirty = 1;
    }
    return true;
}

void DrawTextCenterH(const Mat &canvas, const char *text, int x, int y, const Scalar &color) {
    int len = strlen(text);
    int startx = x - len * 4;
    int starty = y - 8;
    for (int i = 0; i < len; i++) {
        RasterGlyph::renderGlyph(canvas, FIXEDSYS.load(text[i]), startx + 8 * i, starty, color);
    }
}

void DrawTextLeftCenter(const Mat &canvas, const char *text, int x, int y, const Scalar &color) {
    int len = strlen(text);
    int startx = x;
    int starty = y - 8;
    for (int i = 0; i < len; i++) {
        RasterGlyph::renderGlyph(canvas, FIXEDSYS.load(text[i]), startx + 8 * i, starty, color);
    }
}

void DrawTextLeftCenterAutoColor(const Mat &canvas, const char *text, int x, int y) {
    int len = strlen(text);
    int startx = x;
    int starty = y - 8;
    for (int i = 0; i < len; i++) {
        RasterGlyph::renderGlyphAutoColor(canvas, FIXEDSYS.load(text[i]), startx + 8 * i, starty);
    }
}

}
