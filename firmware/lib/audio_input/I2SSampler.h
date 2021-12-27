#ifndef __sampler_base_h__
#define __sampler_base_h__
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <algorithm>

#include "RingBuffer.h"

#define AUDIO_BUFFER_COUNT 11

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SSampler
{
private:
    // audio buffers
    AudioBuffer *m_audio_buffers[AUDIO_BUFFER_COUNT];
    RingBufferAccessor *m_write_ring_buffer_accessor;
    // current audio buffer
    int m_current_audio_buffer;
    // I2S reader task
    TaskHandle_t m_reader_task_handle;
    // processor task
    TaskHandle_t m_processor_task_handle;
    // i2s reader queue
    QueueHandle_t m_i2s_queue;
    // i2s port
    i2s_port_t m_i2s_port;

protected:
    void addSample(int16_t sample);
    virtual void configureI2S() = 0;
    virtual void processI2SData(uint8_t *i2sData, size_t bytesRead) = 0;
    i2s_port_t getI2SPort()
    {
        return m_i2s_port;
    }

public:
    I2SSampler();
    void start(i2s_port_t i2s_port, i2s_config_t &i2s_config, TaskHandle_t processor_task_handle);

    RingBufferAccessor *getRingBufferReader();

    int getCurrentWritePosition()
    {
        return m_write_ring_buffer_accessor->getIndex();
    }
    int getRingBufferSize()
    {
        return AUDIO_BUFFER_COUNT * SAMPLE_BUFFER_SIZE;
    }

    friend void i2sReaderTask(void *param);
};

#endif
