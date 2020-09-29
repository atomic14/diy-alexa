#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "I2SSampler.h"
#include "RingBuffer.h"
#include "RecogniseCommandState.h"
#include "IndicatorLight.h"
#include "Speaker.h"
#include "IntentProcessor.h"
#include "../config.h"
#include <string.h>
#include <SPIFFS.h>
#include <FS.h>

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

void RecogniseCommandState::createSSLConnection(int contentLength)
{
    uint32_t free_ram = esp_get_free_heap_size();
    Serial.printf("Free ram before connection %d\n", free_ram);
    m_wifi_client = new WiFiClientSecure();
    m_wifi_client->connect("api.wit.ai", 443);
    Serial.println("Connected");
    m_wifi_client->println("POST /speech?v=20200927 HTTP/1.1");
    m_wifi_client->printf("host: api.wit.ai\n");
    m_wifi_client->printf("authorization: Bearer %s\n", COMMAND_RECOGNITION_ACCESS_KEY);
    m_wifi_client->println("content-type: audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    m_wifi_client->printf("content-length: %d\n", m_audio_buffer.available());
    m_wifi_client->println();
    Serial.println("Sent headers");
    // we've taken the connection as far as we can - someone else will fill on the content-length and finalise the headers

    free_ram = esp_get_free_heap_size();
    Serial.printf("Free ram after connection %d\n", free_ram);
}

RecogniseCommandState::RecogniseCommandState(I2SSampler *sample_provider, IndicatorLight *indicator_light, Speaker *speaker, IntentProcessor *intent_processor)
{
    // save the sample provider for use later
    m_sample_provider = sample_provider;
    m_indicator_light = indicator_light;
    m_speaker = speaker;
    m_intent_processor = intent_processor;
    m_wifi_client = NULL;
}
void RecogniseCommandState::enterState()
{
    // Open a temporary file to buffer our audio
    // Ideally we would just stream the audio straight to the server but creating
    // an SSL connection can take almost 1.5 seconds so we either have to delay
    // the user from speaking or open the connection after we've gather the audio

    m_audio_buffer = SPIFFS.open("/audio.raw", "w");
    // start the recording from the current write position
    m_last_audio_position = m_sample_provider->getCurrentWritePosition();

    // stash the start time - we will limit ourselves to 3 seconds of audio data
    m_start_time = millis();
    m_elapsed_time = 0;

    // indicate that we are now recording audio
    m_indicator_light->setState(ON);
    m_speaker->playReady();

    Serial.println("I'm listening...");
}

bool RecogniseCommandState::run()
{
    unsigned long current_time = millis();
    m_elapsed_time += current_time - m_start_time;
    m_start_time = current_time;

    // how many samples have been captured since we last ran
    int audio_position = m_sample_provider->getCurrentWritePosition();
    // work out how many samples there are taking into account that we can wrap around
    int sample_count = (audio_position - m_last_audio_position + m_sample_provider->getRingBufferSize()) % m_sample_provider->getRingBufferSize();
    Serial.printf("%ld Last sample position %d, current position %d, number samples %d\n", m_elapsed_time, m_last_audio_position, audio_position, sample_count);
    if (sample_count > 0)
    {
        // write the samples to our audio buffer
        RingBufferAccessor *reader = m_sample_provider->getRingBufferReader();
        reader->setIndex(m_last_audio_position);
        for (int i = 0; i < sample_count; i++)
        {
            int16_t sample = reader->getCurrentSample();
            // m_audio_buffer.write((uint8_t *)&sample, 2);
            reader->moveToNextSample();
        }
        m_last_audio_position = reader->getIndex();
        delete (reader);
        // allocate space for the samples
        // int16_t *samples = static_cast<int16_t *>(malloc(sizeof(int16_t) * sample_count));
        // // copy the samples into our buffer keeping track of where we've got to
        // RingBufferAccessor *reader = m_sample_provider->getRingBufferReader();
        // reader->setIndex(m_last_audio_position);
        // for (int i = 0; i < sample_count; i++)
        // {
        //     samples[i] = reader->getCurrentSample();
        //     reader->moveToNextSample();
        // }
        // m_last_audio_position = reader->getIndex();
        // delete (reader);
        // m_audio_buffer.write((const uint8_t *)samples, sample_count * 2);
        // free(samples);
    }
    // has 3 seconds passed?
    if (m_elapsed_time < 3000)
    {
        // still want to record more audio so stay in this state
        return false;
    }
    // close the file for writing
    m_audio_buffer.close();
    // now in the recognition phase
    m_indicator_light->setState(PULSING);
    // Open audio buffer for reading
    m_audio_buffer = SPIFFS.open("/audio.raw", "r");
    // connect to the server
    createSSLConnection(m_audio_buffer.available());
    // send the file
    m_wifi_client->write(m_audio_buffer);
    m_audio_buffer.close();

    // get the headers and the content length
    int status = -1;
    int content_length = 0;
    char buffer[255];
    while (m_wifi_client->connected())
    {
        int read = m_wifi_client->readBytesUntil('\n', buffer, 255);
        if (read > 0)
        {
            buffer[read] = '\0';
            Serial.println(buffer);
            // blank line indicates the end of the headers
            if (buffer[0] == '\r')
            {
                break;
            }
            if (strncmp("HTTP", buffer, 4) == 0)
            {
                sscanf(buffer, "HTTP/1.1 %d", &status);
            }
            else if (strncmp("Content-Length:", buffer, 15) == 0)
            {
                sscanf(buffer, "Content-Length: %d", &content_length);
            }
        }
    }
    Serial.printf("Http status is %d with content length of %d\n", status, content_length);
    if (status == 200)
    {
        StaticJsonDocument<500> filter;
        filter["entities"]["device:device"][0]["value"] = true;
        filter["entities"]["device:device"][0]["confidence"] = true;
        filter["text"] = true;
        filter["intents"][0]["name"] = true;
        filter["intents"][0]["confidence"] = true;
        filter["traits"]["wit$on_off"][0]["value"] = true;
        filter["traits"]["wit$on_off"][0]["confidence"] = true;
        StaticJsonDocument<500> doc;
        deserializeJson(doc, *m_wifi_client, DeserializationOption::Filter(filter));
        if (m_intent_processor->processIntent(doc))
        {
            m_speaker->playOK();
        }
        else
        {
            // m_speaker->playCantDo();
        }
    }
    else
    {
        // m_speaker->playCantDo();
    }
    // clean up the wifi client as it takes up a lot of RAM
    delete m_wifi_client;
    m_wifi_client = NULL;
    uint32_t free_ram = esp_get_free_heap_size();
    Serial.printf("Free ram after request %d\n", free_ram);

    // indicate that we are done
    m_indicator_light->setState(OFF);
    // move to the next state
    return true;
}

void RecogniseCommandState::exitState()
{
    // nothing to do
}