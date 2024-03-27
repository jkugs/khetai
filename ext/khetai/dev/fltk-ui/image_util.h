#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H

#include <FL/Fl_RGB_Image.H>

Fl_Image* rotate_image_90_clockwise(Fl_RGB_Image* orig);
Fl_Image* rotate_image_90_counter_clockwise(Fl_RGB_Image* orig);

#endif