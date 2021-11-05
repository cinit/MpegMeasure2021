//
// Created by kinit on 2019/12/5.
//

#ifndef OPENCV_LJL0002_WIDGETS_H
#define OPENCV_LJL0002_WIDGETS_H


#include <opencv2/core/mat.hpp>

using namespace cv;

namespace lwk {

extern const Scalar COLOTR_DEFAULT;

class Button {
public:
    Button() {}

    Rect r;
    int isClicked;
    int isEnabled;
    const char *text;
    int id;

    void (*mOnClickListener)(Button *);

    bool onMouseAction(int event, int x, int y, int flags, void *ustc, int *dirty);

public:
    Button(int _x, int _y, int _w, int _h, const char *_text, void (*click)(Button *) = nullptr, int enabled = 1);

    void draw(Mat buf);
};


class LevelTrackBar {
    int mDragging;
    int mShowLevel;
public:
    LevelTrackBar() {}

    LevelTrackBar(int _x, int _y, int _w, int _h, int _maxLevel, int currLevel = 0,
                  void (*click)(LevelTrackBar *, int) = nullptr);

    Rect r;
    int level;//from 0
    int levelCount;
    int id;

    void (*mOnLevelChangedListener)(LevelTrackBar *, int);

    bool onMouseAction(int event, int x, int y, int flags, void *ustc, int *dirty);

    void draw(Mat buf);
};

void DrawTextCenterH(const Mat &canvas, const char *text, int x, int y, const Scalar &color);

void DrawTextLeftCenter(const Mat &canvas, const char *str, int x, int y, const Scalar &color);

void DrawTextLeftCenter(const Mat &canvas, const char *text, int x, int y, const Scalar &color);

void DrawTextLeftCenterAutoColor(const Mat &canvas, const char *text, int x, int y);

}

#endif //OPENCV_LJL0002_WIDGETS_H
