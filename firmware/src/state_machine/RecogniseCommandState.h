#ifndef _recognise_command_state_h_
#define _recognise_command_state_h_

#include <SPIFFS.h>
#include <FS.h>

#include "States.h"

class I2SSampler;
class WiFiClient;
class HTTPClient;
class IndicatorLight;
class Speaker;
class IntentProcessor;

class RecogniseCommandState : public State
{
private:
    I2SSampler *m_sample_provider;
    unsigned long m_start_time;
    unsigned long m_elapsed_time;
    int m_last_audio_position;

    IndicatorLight *m_indicator_light;
    Speaker *m_speaker;
    IntentProcessor *m_intent_processor;

    WiFiClient *m_wifi_client;
    File m_audio_buffer;
    TaskHandle_t m_create_ssl_task_handle;

    void createSSLConnection(int contentLength);

public:
    RecogniseCommandState(I2SSampler *sample_provider, IndicatorLight *indicator_light, Speaker *speaker, IntentProcessor *intent_processor);
    void enterState();
    bool run();
    void exitState();
    friend void createSSLConnection(void *param);
};

#endif
