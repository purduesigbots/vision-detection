#include "pros/vision.h"
#include "pros/vision.hpp"
#include "types.h"
#include <memory>

#ifndef OBJECT_HPP
#define OBJECT_HPP

namespace visiondetect {
    class Object {
    public:
        visiondetect::simple_object_data_s_t *data;
        pros::vision_signature_s_t signature;
        int sample_size;
        int best_brightness;
        int min_area;
        float ratio; /// width / height
        float ratio_range; /// percent error to remain valid

        bool predict_offscreen;

        Object(pros::vision_signature_s_t sig, float ratio, int sample_size, int best_brightness, int min_area, float ratio_range);
        //~Object();
        void apprx_distance(visiondetect::detected_object_s_t);
        void add_sample(visiondetect::simple_object_data_s_t sample);
        void add_samples(visiondetect::simple_object_data_s_t *samples, int size);
        void clear_samples();
};
}
#endif // VISION_HPP

#ifndef VISION_HPP
#define VISION_HPP

namespace visiondetect {
    class Vision {
    public:
        std::shared_ptr<pros::Vision> sensor;
        pros::vision_signature_s_t signatures[7];
        int n_samples;
        int n_retries;
        int port;
        int screen_padding;

        bool predict_offscreen;

        Vision(int port, int n_samples = 5, int n_retries = 10, int screen_padding = 4, bool predict_offscreen = false);
        //~Vision();
        void calibrate_brightness(visiondetect::Object);
        visiondetect::detected_object_s_t find_object(visiondetect::Object);
        visiondetect::detected_object_s_t detect_object(visiondetect::Object);
        bool validate_object(visiondetect::Object, visiondetect::detected_object_s_t);
       
    private:
        void insert_sort_samples(uint16_t*, uint16_t, int);
    };
}
#endif // VISION_HPP