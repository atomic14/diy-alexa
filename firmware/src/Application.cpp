#include <Arduino.h>
#include "Application.h"
#include "state_machine/DetectWakeWordState.h"
#include "state_machine/RecogniseCommandState.h"
#include "IndicatorLight.h"
#include "Speaker.h"
#include "IntentProcessor.h"

Application::Application(I2SSampler *sample_provider, I2SOutput *i2s_output, IntentProcessor *intent_processor)
{
    // detect wake word state - waits for the wake word to be detected
    m_detect_wake_word_state = new DetectWakeWordState(sample_provider);
    // indicator light to show when we are listening
    m_indicator_light = new IndicatorLight();
    // speaker to play sounds in response to the user
    m_speaker = new Speaker(i2s_output);
    // command recongiser - streams audio to the server for recognition
    m_recognise_command_state = new RecogniseCommandState(sample_provider, m_indicator_light, m_speaker, intent_processor);
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
