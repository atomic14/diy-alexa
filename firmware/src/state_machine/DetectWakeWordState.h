#ifndef _detect_wake_word_state_h_
#define _detect_wake_word_state_h_

#include "States.h"

class I2SSampler;
class NeuralNetwork;
class AudioProcessor;

class DetectWakeWordState : public State
{
private:
    I2SSampler *m_sample_provider;
    NeuralNetwork *m_nn;
    AudioProcessor *m_audio_processor;
    float m_average_detect_time;

public:
    DetectWakeWordState(I2SSampler *sample_provider);
    void enterState();
    bool run();
    void exitState();
};

#endif
