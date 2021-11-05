//
// Created by kinit on 2019/12/5.
//

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include "string"
#include "VgaFont.h"
#include "Fixedsys.h"

using namespace std;


RasterGlyph::RasterGlyph() = default;

RasterGlyph::RasterGlyph(int c, int width, int gly[], int valid, int gly_length) {
    memcpy(data, gly, gly_length);
    //data = gly;
    chr = c;
    this->valid = valid;
    this->width = width;
}

float
RasterGlyph::renderGlyph(const cv::Mat &canvas, RasterGlyph glyph, int stx, int sty, cv::Scalar color) {
    int r = 16;//glyph.data.length;
    int pxw = 1;
    int w = glyph.valid > 8 ? 16 : 8;
    int b;
    char *pp;
    for (int a = 0; a </* glyph.data.length*/16; a++) {
        for (b = 0; b < w; b++) {
            if ((1 & glyph.data[a] >> (w - b - 1)) != 0) {
                size_t startAddr = reinterpret_cast<size_t>((void *) &canvas.at<int>(cv::Point(0, 0)));
                size_t endAddr = reinterpret_cast<size_t>((void *) &canvas.at<int>(
                        cv::Point(canvas.cols - 1, canvas.rows - 1)));
                if (size_t(pp) < startAddr || size_t(pp) > endAddr) {
                    //continue;
                }
                pp = (char *) ((int64_t) canvas.data + (int64_t) canvas.step.buf[0] * (sty + a * pxw) +
                               (int64_t) canvas.step.buf[1] * (stx + b * pxw));
                if (((uint64) pp) >= ((uint64_t) canvas.datastart) &&
                    ((uint64) pp + 2) < ((uint64_t) canvas.dataend)) {
                    pp[0] = color.val[0];
                    pp[1] = color.val[1];
                    pp[2] = color.val[2];
                }
            }
        }
    }
    return stx + w * pxw + pxw;
}


float
RasterGlyph::renderGlyphAutoColor(const cv::Mat &canvas, RasterGlyph glyph, int stx, int sty) {
    int r = 16;//glyph.data.length;
    int pxw = 1;
    int w = glyph.valid > 8 ? 16 : 8;
    int b;
    char *pp;
    pp = (char *) ((int64_t) canvas.data + (int64_t) canvas.step.buf[0] * (sty + 6 * pxw) +
                   (int64_t) canvas.step.buf[1] * (stx + 0 * pxw));
    size_t startAddr = reinterpret_cast<size_t>((void *) &canvas.at<int>(cv::Point(0, 0)));
    size_t endAddr = reinterpret_cast<size_t>((void *) &canvas.at<int>(
            cv::Point(canvas.cols - 1, canvas.rows - 1)));
    char c1;
    char c2;
    char c3;
//    if (size_t(pp) < startAddr || size_t(pp) > endAddr) {
//        c1 = 0;
//        c2 = 0;
//        c3 = 255;
//    } else {
    c1 = ((pp[0] & 0xFF) > 128) ? 0 : 255;
    c2 = ((pp[1] & 0xFF) > 128) ? 0 : 255;
    c3 = ((pp[2] & 0xFF) > 128) ? 0 : 255;
//    }
    for (int a = 0; a </* glyph.data.length*/16; a++) {
        for (b = 0; b < w; b++) {
            if ((1 & glyph.data[a] >> (w - b - 1)) != 0) {
                size_t startAddr = reinterpret_cast<size_t>((void *) &canvas.at<int>(cv::Point(0, 0)));
                size_t endAddr = reinterpret_cast<size_t>((void *) &canvas.at<int>(
                        cv::Point(canvas.cols - 1, canvas.rows - 1)));
                if (size_t(pp) < startAddr || size_t(pp) > endAddr) {
//                    continue;
                }
                pp = (char *) ((int64_t) canvas.data + (int64_t) canvas.step.buf[0] * (sty + a * pxw) +
                               (int64_t) canvas.step.buf[1] * (stx + b * pxw));
                if (((uint64) pp) >= ((uint64_t) canvas.datastart) &&
                    ((uint64) pp + 2) < ((uint64_t) canvas.dataend)) {
                    pp[0] = c1;
                    pp[1] = c2;
                    pp[2] = c3;
                }
            }
        }
    }
    return stx + w * pxw + pxw;
}


//RasterFont::RasterFont() {}

RasterGlyph RasterFont::load(int c) {
    if (c < 0 || c > 255)return chars[0];
    if (c < /*chars.length*/256) {
        return chars[c];
    }
    throw exception();
}


class RasterFontLoader {

    char *file;
    int file_length;
public:
    RasterFont fonts;

    RasterFontLoader(const char *file) {
        FILE *fd = fopen(file, "r");
        this->file = static_cast<char *>(malloc(16 * 1024));
        int read = fread(this->file, 1, 10000, fd);
        printf("%d\n", read);
        fclose(fd);
        file_length = read;
        loadStream();
    }

    RasterFontLoader(const unsigned char *data, int length) {
        this->file = (char *) data;
        file_length = length;
        loadStream();
    }


    bool loadStream() {

        char ___MZ[] = {'M', 'Z'};
        char ___NE[] = {'N', 'E'};
        char ___PE00[] = {'P', 'E', 0, 0};

        char **fin;
        try {
            if (!bytesEqu(file, 0, 2, ___MZ)) {
                return false;
            }
            int off = dword(file, 0x3C);
            if (bytesEqu(file, off, 2, ___NE)) {
                return parseNe(off);
            }
            if (bytesEqu(file, off, 4, ___PE00)) {
                return false;//Not supported yet
            }
        } catch (exception &e) {

        }
        return true;
    }
/*
       RasterFont get(string name, int size, int ascent) {
        if (fonts == nullptr || fonts.size() == 0)throw exception();
           iterator<RasterFont> it = fonts.iterator();
           while (it.hasNext()) {
               RasterFont fnt = (RasterFont) it.next();
               if (name.equals(fnt.facename))
                   if (size < 1 || size == fnt.height)
                       if (ascent < 1 || ascent == fnt.ascent)return fnt;
           }
        return fonts[0];
    }
    */
private:
    RasterFont parseFont(char fnt[]) {
        RasterFont font;
        font.version = word(fnt, 0);
        memcpy(font.copyright, fnt + 6, 60);
        //font.copyright = string(fnt, 6, 60);
        int ftype = word(fnt, 0x42);
        if ((ftype & 1) == 1) {
            // This font is a vector font
            throw "vector font";
        }
        // face name offset
        int off_facename = dword(fnt, 0x69);
        if (off_facename < 0 || off_facename > file_length /*fnt.length*/) {
            // Face name not contained within font data
            throw "no facename";
        }
        memset(font.facename, 0, sizeof(font.facename));
        readStringFull(font.facename, fnt, off_facename, file_length);
        font.pointsize = word(fnt, 0x44);
        font.ascent = word(fnt, 0x4a);
        font.width = 0; // max width
        font.height = word(fnt, 0x58);
        font.italic = 0xFF & fnt[0x50];
        font.underline = 0xFF & fnt[0x51];
        font.strikeout = 0xFF & fnt[0x52];
        font.weight = word(fnt, 0x53);
        font.charset = 0xFF & fnt[0x55];
        // Read the char table.
        int ctstart, ctsize;
        if (font.version == 0x200) {
            ctstart = 0x76;
            ctsize = 4;
        } else {
            ctstart = 0x94;
            ctsize = 6;
        }
        //int maxwidth = 0;
        //font.chars = new RasterGlyph[256];
        /* for (int i = 0; i < 256; i++) {
             //font.chars[i] = RasterGlyph((char) i);
             //font.
             font.chars[i].data = new int[font.height];
         }*/
        int firstchar = 0xFF & fnt[0x5f];
        int lastchar = 0xFF & fnt[0x60];
        //printf(firstchar + "-" + lastchar + "\n");
        int off;
        for (int i = firstchar; i <= lastchar; i++) {
            int entry = ctstart + ctsize * (i - firstchar);
            int w = word(fnt, entry);
            font.chars[i].valid = w;
            font.width = w > font.width ? w : font.width;
            if (ctsize == 4) {
                off = word(fnt, entry + 2);
            } else {
                off = dword(fnt, entry + 2);
            }
            int widthbytes = (int) floor((w + 7) / 8.0);
            //echo widthbytes . " ";
            for (int j = 0; j < font.height; j++) {
                for (int k = 0; k < widthbytes; k++) {
                    int bytepos = off + k * font.height + j;
                    font.chars[i].data[j] = font.chars[i].data[j] << 8;
                    font.chars[i].data[j] = font.chars[i].data[j] | 0xFF & fnt[bytepos];
                }
                font.chars[i].data[j] = font.chars[i].data[j] >> (8 * widthbytes - w);
                //echo font.chars[i].data[j] . " ";
            }
        }
        //print_r(font);
        return font;
    }

    bool parseNe(int offset) {
        // Find the resource table.
        int restable = offset + word(file, offset + 0x24);
        // 32h: A shift count that is used to align the logical sector. This
        // count is log2 of the segment sector size. It is typically 4,
        // although the default count is 9.
        int shift = word(file, restable);
        // Now loop over the rest of the resource table.
        int p = restable + 2;
        int ct = 0;
        while (true) {
            int rtype = word(file, p);
            // end of resource table
            if (rtype == 0) {
                break;
            }
            int count = word(file, p + 2);
            // type, count, 4 bytes reserved
            p += 8;
            int start, size;
            for (int i = 0; i < count; i++) {
                start = word(file, p) << shift;
                size = word(file, p + 2) << shift;
                if (start < 0 || size < 0 || start + size > file_length) {
                    throw "2003:Resource overruns file boundaries";
                }
                // this is an actual font
                if (rtype == 0x8008) {
                    RasterFont ft = parseFont(subBuf(file, start, size));
                    //fonts.add(ft);
                    fonts = ft;
                    return 1;
                    //System.out.println(ft);
                    //System.out.printf("font start at %d, size:%d\n",start,size);
                    ct++;
                }
                // start, size, flags, name/id, 4 bytes reserved
                p += 12;
            }
        }
        return ct > 0;
    }


private:

    bool bytesEqu(char data[], int start, int len, char std[]) {
        for (int i = 0; i < len; i++) {
            if (data[start + i] != std[i])return false;
        }
        return true;
    }

    void readStringFull(char *out, char buf[], int pos, int buf_length) {
        int stop = -1;
        //String t=new String(buf,pos,100);
        for (int i = pos; i < buf_length; i++) {
            if (buf[i] == 0) {
                stop = i;
                break;
            }
        }
        int len;
        if (stop != -1) {
            len = stop - pos;
        } else len = buf_length - pos;
        //return string(buf, pos, len);
        memcpy(out, buf + pos, len);
    }

    int word(char buf[], int pos) {
        int i = (0xff & buf[pos]) | ((0xff & buf[pos + 1]) << 8);
        return i;
    }

    int dword(char buf[], int pos) {
        int a = word(buf, pos);
        int b = (word(buf, pos + 2) << 16);
        return a | b;
    }

    char *subBuf(char *org, int start, int len) {
        return org + start;
    }

};

RasterFont FIXEDSYS = RasterFontLoader(_RESOURCES_Fixedsys_data, _RESOURCES_Fixedsys_length).fonts;
