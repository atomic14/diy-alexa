#ifndef _application_h_
#define _applicaiton_h_

#include "state_machine/States.h"

class I2SSampler;
class I2SOutput;
class State;
class IndicatorLight;
class Speaker;

class Application
{
private:
    State *m_detect_wake_word_state;
    State *m_recognise_command_state;
    State *m_current_state;
    IndicatorLight *m_indicator_light;
    Speaker *m_speaker;

public:
    Application(I2SSampler *sample_provider, I2SOutput *i2s_output);
    ~Application();
    void run();
};

#endif