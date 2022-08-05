#include "pros/vision.h"
#include "visiondetect/api.h"


void visiondetect::Object::apprx_distance(visiondetect::detected_object_s_t obj) {
    for(int i = 0; i < sample_size; i++) {
        // { area: 267968, distance: 0.0 }
        // { area: 0, distance: 1000.0 }
        int extreeme_dist = 500;
        int extreeme_area = 267968;
        if(obj.area < data[i].area) {
            if(i==0) {
                obj.apprx_distance = ((extreeme_dist - data[i].distance)/(data[i].area - 0))*(obj.area) + data[i].distance;
            } else {
                obj.apprx_distance = ((data[i-1].distance - data[i].distance)/(data[i].area - data[i-1].area))*(obj.area - data[i-1].area) + data[i].distance;
            }
            break;
        }
        if(i+1 == sample_size) {
            obj.apprx_distance = ((data[i-1].distance - 0)/(extreeme_area - data[i-1].area))*(obj.area - data[i-1].area) + 0;
        }
    }
};

visiondetect::Object::Object(pros::vision_signature_s_t sig, float ratio, int samples=5, int brightness=55, int min_area=50, float ratio_range=0.15) {
    this->signature = sig;
    this->ratio = ratio;
    this->sample_size = samples;
    this->best_brightness = brightness;
    this->min_area = min_area;
    this->ratio_range = ratio_range;
}