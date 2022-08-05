#include "main.h"
#include "pros/vision.h"
#include "pros/vision.hpp"
#include <memory>
#include <stdint.h>
#include <sys/types.h>

visiondetect::Vision::Vision(int port, int n_samples, int n_retries, int screen_padding, bool predict_offscreen)
{
    this->screen_padding = screen_padding;
    this->port = port;
    this->n_samples = n_samples;
    this->n_retries = n_retries;
    this->predict_offscreen = predict_offscreen;

    this->sensor = std::make_shared<pros::Vision>(port, pros::E_VISION_ZERO_CENTER);
}


void visiondetect::Vision::insert_sort_samples(uint16_t* arr, uint16_t val, int n) { 
    uint16_t tmp;
    uint16_t current_val= val;
    while(current_val==val) {
        if(current_val<arr[n-1]) {
            arr[n]=arr[n-1];
        } else {
            arr[n]=current_val;
            current_val=0xFFFF;
        }
        n--;
    }
}

bool visiondetect::Vision::validate_object(visiondetect::Object obj, visiondetect::detected_object_s_t detected_object) {

    if(this->predict_offscreen) {
        // if edge of object is >= the edge of the padding
        bool xedge = (abs(detected_object.x_px)+(detected_object.width/2.0) >=(VISION_FOV_WIDTH - screen_padding));
        bool yedge = (abs(detected_object.y_px)+(detected_object.height/2.0) >=(VISION_FOV_HEIGHT - screen_padding));
        // check if object is within corner and screen padding
        if(xedge && yedge) {
            detected_object.x_px = ((detected_object.x_px > 0) - (detected_object.x_px < 0))*VISION_FOV_WIDTH;
            detected_object.y_px = ((detected_object.y_px > 0) - (detected_object.y_px < 0))*VISION_FOV_HEIGHT;
            // if the object is within the corner, there is a possibility it is a valid ratio. attempting to find
            // it's ratio is impossible, as the width and height are both variable.
            return true;
          }
        if(xedge) {
            detected_object.x_px = ((detected_object.x_px > 0) - (detected_object.x_px < 0))*VISION_FOV_WIDTH;
            //if the object is along the x edge, check if it's current visible ratio is less than the maximum ratio.
           return (((double)detected_object.width / (double)detected_object.height) <= (obj.ratio*(1+obj.ratio_range))) && (detected_object.height * detected_object.height * obj.ratio > obj.min_area);
        } else if(yedge) {
            detected_object.y_px = ((detected_object.y_px > 0) - (detected_object.y_px < 0))*VISION_FOV_HEIGHT;
            // read the if statment block above
            return (((double)detected_object.width / (double)detected_object.height) >= (obj.ratio*(1-obj.ratio_range))) && (detected_object.width * detected_object.width / obj.ratio > obj.min_area);
        }
        // do extra stuff to calculate object size and coordinates if offscreen
    }
    //check if the detected object's true ratio is within the range of a valid ratio.
    double detected_ratio = (double)detected_object.width / (double)detected_object.height;
    printf("detected ratio: %lf\n", detected_ratio);
 
    return (detected_ratio > (obj.ratio*(1-obj.ratio_range))) && (detected_ratio < (obj.ratio*(1+obj.ratio_range))) && (detected_object.area > obj.min_area);
}

visiondetect::detected_object_s_t visiondetect::Vision::find_object(visiondetect::Object obj) {
    visiondetect::detected_object_s_t detected;
    int size_search = 0;
    
    detected.valid=false;

    // set our signature and exposure
    printf("OBJECT SIGNATURE: %d\n", obj.signature.u_mean);
    this->sensor->set_signature(1,&obj.signature);
    this->sensor->set_exposure(obj.best_brightness);
    // while we haven't found the object and we haven't exceeded our retry limit
    while(!detected.valid && size_search<this->n_retries) {
        // malloc our sample arrays
        uint16_t* width_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        uint16_t* height_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        uint16_t* x_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        uint16_t* y_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        // get our samples
        for(int i=0; i<this->n_samples; i++) {
            pros::vision_object_s_t c_obj = this->sensor->get_by_sig(0, 1);
            // insertion sort the samples as we get them
            insert_sort_samples(width_samples, c_obj.width, i);
            insert_sort_samples(height_samples, c_obj.height, i);
            insert_sort_samples(x_samples, c_obj.x_middle_coord, i);
            insert_sort_samples(y_samples, c_obj.y_middle_coord, i);
        }

        for(int i=0; i<this->n_samples; i++) {
            //printf("width %d: %d\n", i, width_samples[i]);
        }
        // set detected to our median values
        detected.height=height_samples[this->n_samples/2];
        detected.width=width_samples[this->n_samples/2];
        detected.x_px=x_samples[this->n_samples/2];
        detected.y_px=y_samples[this->n_samples/2];
        detected.area = detected.height*detected.width;

        // free our sample arrays
        free(width_samples);
        free(height_samples);
        free(x_samples);
        free(y_samples);

        // check if detected is valid
        printf("Detected stats: %d, %d, %d, %d, %d\n", detected.height, detected.width, detected.x_px, detected.y_px, detected.area);
        detected.valid = validate_object(obj, detected);
        // increment size_search
        size_search++;
    }
    
    return detected;
    //this->sensor->capture_samples();
}

visiondetect::detected_object_s_t visiondetect::Vision::detect_object(visiondetect::Object obj) {
    printf("finding object\n");
    visiondetect::detected_object_s_t detected = find_object(obj);
    printf("approximating object\n");
    obj.apprx_distance(detected);
    printf("returned\n");
    return detected;
}