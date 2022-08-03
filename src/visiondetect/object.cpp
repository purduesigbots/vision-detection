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

class Object {
    public:
        visiondetect::simple_object_data_s_t *data;
        pros::vision_signature_s_t signature;
        int sample_size;
        int best_brightness;
        float ratio;
        float ratio_range;
        bool predict_offscreen;

        Object(int sig);
        ~Object();
        void detect();
        void add_sample(visiondetect::simple_object_data_s_t sample);
        void add_samples(visiondetect::simple_object_data_s_t *samples, int size);
        void clear_samples();
};
