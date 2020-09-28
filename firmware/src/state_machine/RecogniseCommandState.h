#ifndef _recognise_command_state_h_
#define _recognise_command_state_h_

#include <esp_http_client.h>

#include "States.h"

class I2SSampler;
class WiFiClient;
class HTTPClient;
class IndicatorLight;
class Speaker;

class RecogniseCommandState : public State
{
private:
    WiFiClient *m_wifi_client;

    I2SSampler *m_sample_provider;
    esp_http_client_handle_t m_http_client;
    unsigned long m_start_time;
    unsigned long m_elapsed_time;
    int m_last_audio_position;

    IndicatorLight *m_indicator_light;
    Speaker *m_speaker;

public:
    RecogniseCommandState(I2SSampler *sample_provider, IndicatorLight *indicator_light, Speaker *speaker);
    void enterState();
    bool run();
    void exitState();
};

#endif
