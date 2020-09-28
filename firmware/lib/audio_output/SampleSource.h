#ifndef __sample_source_h__
#define __sample_source_h__

#include <Arduino.h>

typedef struct
{
    int16_t left;
    int16_t right;
} Frame_t;

/**
 * Base class for our sample generators
 **/
class SampleSource
{
public:
    // This should fill the samples buffer with the specified number of frames
    // A frame contains a LEFT and a RIGHT sample. Each sample should be signed 16 bits
    virtual int getFrames(Frame_t *frames, int number_frames) = 0;
    virtual bool available() = 0;
    virtual ~SampleSource(){};
};

#endif