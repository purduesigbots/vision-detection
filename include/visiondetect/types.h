#include "pros/vision.h"
#include "pros/vision.hpp"
#ifndef TYPES_H
#define TYPES_H

namespace visiondetect {

typedef struct simple_object_data {
  float area;
  float distance;
} simple_object_data_s_t;


typedef struct detected_object {
    int height;
    int width;
    int area;
    int x_px;
    int y_px;
    bool valid;

    float apprx_distance;

} detected_object_s_t;

}

#endif // TYPES_H