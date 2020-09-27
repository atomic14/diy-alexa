#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "I2SSampler.h"
#include "RingBuffer.h"
#include "RecogniseCommandState.h"
#include "../config.h"

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

RecogniseCommandState::RecogniseCommandState(I2SSampler *sample_provider)
{
    m_wifi_client = new WiFiClient();
    // save the sample provider for use later
    m_sample_provider = sample_provider;
}
void RecogniseCommandState::enterState()
{
    m_last_audio_position = -1;
    // stash the start time - we will limit ourselves to 5 seconds of data
    m_start_time = millis();
    m_elapsed_time = 0;

    m_wifi_client->connect(COMMAND_RECOGNITION_HOST, COMMAND_RECOGNITION_PORT);
    char authorization_header[100];
    snprintf(authorization_header, 100, "authorization: Bearer %s", COMMAND_RECOGNITION_ACCESS_KEY);
    m_wifi_client->println("POST /speech?v=20200927 HTTP/1.1");
    m_wifi_client->println(authorization_header);
    m_wifi_client->println("content-type: audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    m_wifi_client->println("transfer-encoding: chunked");
    m_wifi_client->println();
}
bool RecogniseCommandState::run()
{
    if (!m_wifi_client->connected())
    {
        // no http client - something went wrong somewhere move to the next state as there's nothing for us to do
        Serial.println("Error - Attempt to run with no http client");
        return true;
    }
    if (m_last_audio_position == -1)
    {
        m_last_audio_position = m_sample_provider->getCurrentWritePosition();
    }
    // how many samples have been captured since we last ran
    int audio_position = m_sample_provider->getCurrentWritePosition();
    // work out how many samples there are taking into account that we can wrap around
    int sample_count = (audio_position - m_last_audio_position + m_sample_provider->getRingBufferSize()) % m_sample_provider->getRingBufferSize();
    // Serial.printf("Last sample position %d, current position %d, number samples %d\n", m_last_audio_position, audio_position, sample_count);
    if (sample_count > 0)
    {
        // allocate space for the samples
        int16_t *samples = static_cast<int16_t *>(malloc(sizeof(int16_t) * sample_count));
        // copy the samples into our buffer keeping track of where we've got to
        RingBufferAccessor *reader = m_sample_provider->getRingBufferReader();
        reader->setIndex(m_last_audio_position);
        for (int i = 0; i < sample_count; i++)
        {
            samples[i] = reader->getCurrentSample();
            reader->moveToNextSample();
        }
        m_last_audio_position = reader->getIndex();
        delete (reader);
        // send the samples to the server
        char buffer[255];
        snprintf(buffer, 255, "%X\r\n", sample_count * sizeof(int16_t));
        m_wifi_client->print(buffer);
        // send the samples
        m_wifi_client->write((char *)samples, sample_count * sizeof(int16_t));
        m_wifi_client->print("\r\n");
        free(samples);
        // has 5 seconds passed?
        unsigned long current_time = millis();
        m_elapsed_time += current_time - m_start_time;
        m_start_time = current_time;
        if (m_elapsed_time > 4000)
        {
            // all done, move to next state
            Serial.println("4 seconds has elapsed - finishing recognition request");
            // final new line to finish off the request
            m_wifi_client->print("0\r\n");
            m_wifi_client->print("\r\n");
            // get the headers and the content length
            while (m_wifi_client->connected())
            {
                int read = m_wifi_client->readBytesUntil('\n', buffer, 255);
                if (read > 0)
                {
                    buffer[read] = '\0';
                    if (buffer[0] == '\r')
                    {
                        break;
                    }
                    // Serial.println(buffer);
                }
            }
            while (m_wifi_client->connected() && m_wifi_client->available())
            {
                Serial.print((char)m_wifi_client->read());
            }
            Serial.println();
            return true;
        }
    }
    // still work to do, stay in this state
    return false;
}

void RecogniseCommandState::exitState()
{
    // nothing to do
}