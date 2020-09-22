
#include <Arduino.h>
#include "I2SSampler.h"
#include "driver/i2s.h"

// 10 buffers - 1 second + an extra 300 ms so we don't overwrite data while processing
#define AUDIO_BUFFER_COUNT 13

void I2SSampler::addSample(int16_t sample)
{
    // store the sample
    AudioBuffer *audio_buffer = m_audio_buffers[m_current_audio_buffer];
    audio_buffer->samples[m_audio_buffer_pos] = sample;
    audio_buffer->max = std::max(audio_buffer->max, sample);
    audio_buffer->total += sample;
    // move to the next sample in the buffer
    m_audio_buffer_pos++;
    // have we filled the current buffer?
    if (m_audio_buffer_pos == 160)
    {
        // move to the next buffer wrapping around to the start of the buffers
        m_audio_buffer_pos = 0;
        m_current_audio_buffer = (m_current_audio_buffer + 1) % AUDIO_BUFFER_COUNT;
        // reset the max and total ready to receive new samples
        m_audio_buffers[m_current_audio_buffer]->max = 0;
        m_audio_buffers[m_current_audio_buffer]->total = 0;
        // trigger the processor task
        xTaskNotify(m_processor_task_handle, 1, eSetBits);
    }
}

void i2sReaderTask(void *param)
{
    I2SSampler *sampler = (I2SSampler *)param;
    while (true)
    {
        // wait for some data to arrive on the queue
        i2s_event_t evt;
        if (xQueueReceive(sampler->m_i2sQueue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_RX_DONE)
            {
                size_t bytesRead = 0;
                do
                {
                    // read data from the I2S peripheral
                    uint8_t i2sData[1024];
                    // read from i2s
                    i2s_read(sampler->getI2SPort(), i2sData, 1024, &bytesRead, 10);
                    // process the raw data
                    sampler->processI2SData(i2sData, bytesRead);
                } while (bytesRead > 0);
            }
        }
    }
}

void I2SSampler::start(i2s_port_t i2s_port, i2s_config_t &i2s_config, TaskHandle_t processor_task_handle)
{
    m_i2s_port = i2s_port;
    m_processor_task_handle = processor_task_handle;
    // allocate the audio buffers
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++)
    {
        m_audio_buffers[i] = new AudioBuffer();
    }
    m_audio_buffer_pos = 0;
    m_current_audio_buffer = 0;
    //install and start i2s driver
    i2s_driver_install(m_i2s_port, &i2s_config, 4, &m_i2s_queue);
    // set up the I2S configuration from the subclass
    configureI2S();
    // start a task to read samples from the ADC
    xTaskCreatePinnedToCore(i2sReaderTask, "i2s Reader Task", 4096, this, 1, &m_reader_task_handle, 0);
}
