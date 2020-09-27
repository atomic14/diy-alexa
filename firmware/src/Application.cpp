#include <Arduino.h>
#include "Application.h"
#include "state_machine/DetectWakeWordState.h"
#include "state_machine/RecogniseCommandState.h"

Application::Application(I2SSampler *sample_provider)
{
    // use the build in LED as an indicator - we'll set it up as a pwm output so we can make it glow nicely
    ledcSetup(0, 10000, 8);
    ledcAttachPin(2, 0);
    ledcWrite(0, 0);
    // create the different application states
    m_detect_wake_word_state = new DetectWakeWordState(sample_provider);
    m_recognise_command_state = new RecogniseCommandState(sample_provider);
    // start off in the detecting wakeword state
    m_current_state = m_detect_wake_word_state;
    m_current_state->enterState();
}

// process the next batch of samples
void Application::run()
{
    bool state_done = m_current_state->run();
    if (state_done)
    {
        m_current_state->exitState();
        // switch to the next state - very simple state machine so we just go to the other state...
        if (m_current_state == m_detect_wake_word_state)
        {
            m_current_state = m_recognise_command_state;
        }
        else
        {
            m_current_state = m_detect_wake_word_state;
        }
        m_current_state->enterState();
    }
    vTaskDelay(10);
}
