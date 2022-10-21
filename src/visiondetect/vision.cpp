#include "main.h"
#include "pros/vision.h"
#include "pros/vision.hpp"
#include <memory>
#include <stdint.h>
#include <sys/types.h>
#include "visiondetect/classes.hpp"

visiondetect::Vision::Vision(int port, int n_samples, int n_retries, int screen_padding, bool predict_offscreen)
{
    this->screen_padding = screen_padding;
    this->port = port;
    this->n_samples = n_samples;
    this->n_retries = n_retries;
    this->predict_offscreen = predict_offscreen;
    this->attempt_combine_close_objects = 0;

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

bool visiondetect::Vision::validate_object(visiondetect::Object obj, visiondetect::detected_object_s_t* detected_object) {

    if(this->predict_offscreen) {
        // if edge of object is >= the edge of the padding
        bool xedge = (abs(detected_object->x_px)+(detected_object->width/2.0) >=(VISION_FOV_WIDTH - screen_padding));
        bool yedge = (abs(detected_object->y_px)+(detected_object->height/2.0) >=(VISION_FOV_HEIGHT - screen_padding));
        // check if object is within corner and screen padding
        if(xedge && yedge) {
            detected_object->x_px = ((detected_object->x_px > 0) - (detected_object->x_px < 0))*VISION_FOV_WIDTH;
            detected_object->y_px = ((detected_object->y_px > 0) - (detected_object->y_px < 0))*VISION_FOV_HEIGHT;
            // if the object is within the corner, there is a possibility it is a valid ratio. attempting to find
            // it's ratio is impossible, as the width and height are both variable.
            return true;
          }
        if(xedge) {
            detected_object->x_px = ((detected_object->x_px > 0) - (detected_object->x_px < 0))*VISION_FOV_WIDTH;
            //if the object is along the x edge, check if it's current visible ratio is less than the maximum ratio.
           return (((double)detected_object->width / (double)detected_object->height) <= (obj.ratio*(1+obj.ratio_range))) && (detected_object->height * detected_object->height * obj.ratio > obj.min_area);
        } else if(yedge) {
            detected_object->y_px = ((detected_object->y_px > 0) - (detected_object->y_px < 0))*VISION_FOV_HEIGHT;
            // read the if statment block above
            return (((double)detected_object->width / (double)detected_object->height) >= (obj.ratio*(1-obj.ratio_range))) && (detected_object->width * detected_object->width / obj.ratio > obj.min_area);
        }
        // do extra stuff to calculate object size and coordinates if offscreen
    }
    //check if the detected object's true ratio is within the range of a valid ratio.
    double detected_ratio = (double)detected_object->width / (double)detected_object->height;
    //printf("detected ratio: %lf\n", detected_ratio);
 
    return (detected_ratio > (obj.ratio*(1-obj.ratio_range))) && (detected_ratio < (obj.ratio*(1+obj.ratio_range))) && (detected_object->area > obj.min_area);
}

bool visiondetect::Vision::get_largest_object(visiondetect::Object obj, pros::vision_object_s_t *detected) {

    if(this->attempt_combine_close_objects) {

        printf("attempting to combine objects\n");
        // attempt to combine close objects
        
        // get all objects
        std::vector<pros::vision_object_s_t> objects; // our vector of objects
        pros::vision_object_s_t current_object = this->sensor->get_by_sig(0, 1); // get the first object
        // cursed for loop to get objects while checking if they are valid
        for(int i=0; current_object.width!=PROS_ERR_2_BYTE; (current_object=this->sensor->get_by_sig(++i, 1))) {
            objects.push_back(this->sensor->get_by_sig(i, 1));
        }

        std::vector<pros::vision_object_s_t> combined_objects;
        
        // combine objects
        for(int i=0; i < objects.size(); i++) {
            for(int j=i+1; j < objects.size(); j++) {
                if(abs(objects[i].x_middle_coord - objects[j].x_middle_coord) < this->attempt_combine_close_objects && abs(objects[i].y_middle_coord - objects[j].y_middle_coord) < this->attempt_combine_close_objects) {
                    // combine objects
                    objects[i].x_middle_coord = (objects[i].x_middle_coord + objects[j].x_middle_coord) / 2;
                    objects[i].y_middle_coord = (objects[i].y_middle_coord + objects[j].y_middle_coord) / 2;
                    objects[i].width = (objects[i].width + objects[j].width) / 2;
                    objects[i].height = (objects[i].height + objects[j].height) / 2;
                    objects[i].signature = objects[i].signature;
                    objects.erase(objects.begin() + j);
                    j--;
                }
            }
        }

        // find largest object
        for(int i=0; i < objects.size(); i++) {
            if(objects[i].width * objects[i].height > detected->width * detected->height) {
                *detected = objects[i];
            }
        }

    } else {
        // get largest object
        printf("attempting to get largest object\n");
        *detected = this->sensor->get_by_size(0);
    }
    printf("Detected width: %d, height: %d\n", detected->width, detected->height);
    return detected->width > 0&& detected->width <= 640;


}

bool visiondetect::Vision::find_object(visiondetect::Object obj, visiondetect::detected_object_s_t* detected) {

    int size_search = 0;
    
    detected->valid=false;

    // set our signature and exposure
    printf("OBJECT SIGNATURE: %d\n", obj.signature.u_mean);
    this->sensor->set_signature(1,&obj.signature);
    this->sensor->set_exposure(obj.best_brightness);
    //printf("signature set\n");
    // while we haven't found the object and we haven't exceeded our retry limit
    while(!detected->valid && size_search<this->n_retries) {
        //printf("searching for object\n");
        // malloc our sample arrays
        uint16_t* width_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        uint16_t* height_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        uint16_t* x_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        uint16_t* y_samples = (uint16_t*)malloc(sizeof(uint16_t)*this->n_samples);
        // get our samples

        //printf("getting samples\n");
        pros::vision_object_s_t detected_obj;
        for(int i=0; i<this->n_samples; i++) {
            

            bool found = this->get_largest_object(obj, &detected_obj);
            printf("found: %d\n", found);
            if(found) {
                printf("got object\n");
                // insertion sort the samples as we get them
                insert_sort_samples(width_samples, detected_obj.width, i);
                insert_sort_samples(height_samples, detected_obj.height, i);
                insert_sort_samples(x_samples, detected_obj.x_middle_coord, i);
                insert_sort_samples(y_samples, detected_obj.y_middle_coord, i);
                //printf("sorted samples\n");
 
                //printf("eee\n");
            } else {
                insert_sort_samples(width_samples, 0, i);
                insert_sort_samples(height_samples, 0, i);
                insert_sort_samples(x_samples, 0, i);
                insert_sort_samples(y_samples, 0, i);

            }
            pros::lcd::set_text(1, std::to_string(i));
            printf("millis: %d\n", pros::millis());
            pros::delay(20);
            printf("millis: %d\n", pros::millis());
        }


        int start_not_bad_index_x = 0;
        int start_not_bad_index_y = 0;
        int start_not_bad_index_w = 0;
        int start_not_bad_index_h = 0;



        for(int i=0; i<this->n_samples; i++) {
            // find the first index that is not bad (0)
            if(width_samples[i] != 0 && !start_not_bad_index_w) {
                start_not_bad_index_w = i;
            }
            if(height_samples[i] != 0 && !start_not_bad_index_h) {
                start_not_bad_index_h = i;
            }
            if(x_samples[i] != 0 && !start_not_bad_index_x) {
                start_not_bad_index_x = i;
            }
            if(y_samples[i] != 0 && !start_not_bad_index_y) {
                start_not_bad_index_y = i;
            }
        }
        // set detected to our median values

        printf("setting detected\n");


        detected->height=height_samples[start_not_bad_index_h + (this->n_samples - start_not_bad_index_h)/2];
        detected->width=width_samples[start_not_bad_index_w + (this->n_samples - start_not_bad_index_w)/2];
        detected->x_px=x_samples[start_not_bad_index_x + (this->n_samples - start_not_bad_index_x)/2];
        detected->y_px=y_samples[start_not_bad_index_y + (this->n_samples - start_not_bad_index_y)/2];
        detected->area = detected->height*detected->width;

        // free our sample arrays
        printf("freeing samples\n");
        free(width_samples);
        free(height_samples);
        free(x_samples);
        free(y_samples);

        // check if detected is valid
        printf("Detected stats: %d, %d, %d, %d, %d\n", detected->height, detected->width, detected->x_px, detected->y_px, detected->area);
        detected->valid = validate_object(obj, detected);
        // increment size_search
        size_search++;
    }
    
    return detected->valid;
    //this->sensor->capture_samples();
}

bool visiondetect::Vision::detect_object(visiondetect::Object obj, visiondetect::detected_object_s_t* output_obj) {
    printf("finding object\n");

    bool detected = find_object(obj, output_obj);

    if(false && detected) {
        printf("object detected\n");
        obj.apprx_distance(output_obj);
    }
    printf("returned\n");
    return detected;
}