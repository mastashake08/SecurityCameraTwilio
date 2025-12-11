#ifndef SENSE_AI_H
#define SENSE_AI_H

#include "esp_camera.h"

class SenseAI {
private:
    camera_fb_t* previousFrame;
    unsigned long lastInferenceTime;
    bool motionDetected;
    
    bool detectMotion(camera_fb_t* currentFrame);
    float runInference(camera_fb_t* frame);

public:
    SenseAI();
    ~SenseAI();
    
    void begin();
    bool processFrame();
    bool hasMotion();
    void sendAlert();
};

#endif
