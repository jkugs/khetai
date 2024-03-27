#include "image_util.h"

Fl_Image *rotate_image_90_clockwise(Fl_RGB_Image *orig)
{
    int d = orig->d();
    int orig_w = orig->w();
    int orig_h = orig->h();
    int new_w = orig_h;
    int new_h = orig_w;

    const uchar *orig_buf = (const uchar *)orig->data()[0];
    uchar *new_buf = new uchar[new_w * new_h * d];

    for (int y = 0; y < orig_h; ++y)
    {
        for (int x = 0; x < orig_w; ++x)
        {
            int new_x = orig_h - 1 - y;
            int new_y = x;

            for (int c = 0; c < d; ++c)
            {
                new_buf[(new_y * new_w + new_x) * d + c] = orig_buf[(y * orig_w + x) * d + c];
            }
        }
    }

    Fl_RGB_Image *rotated = new Fl_RGB_Image(new_buf, new_w, new_h, d);
    return rotated;
}

Fl_Image *rotate_image_90_counter_clockwise(Fl_RGB_Image *orig)
{
    int d = orig->d();
    int orig_w = orig->w();
    int orig_h = orig->h();
    int new_w = orig_h;
    int new_h = orig_w;

    const uchar *orig_buf = (const uchar *)orig->data()[0];
    uchar *new_buf = new uchar[new_w * new_h * d];

    for (int y = 0; y < orig_h; ++y)
    {
        for (int x = 0; x < orig_w; ++x)
        {
            int new_x = y;
            int new_y = orig_w - 1 - x;

            for (int c = 0; c < d; ++c)
            {
                new_buf[(new_y * new_w + new_x) * d + c] = orig_buf[(y * orig_w + x) * d + c];
            }
        }
    }

    Fl_RGB_Image *rotated = new Fl_RGB_Image(new_buf, new_w, new_h, d);
    return rotated;
}