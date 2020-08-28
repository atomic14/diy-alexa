
#include <Arduino.h>
#include "driver/i2s.h"
#include <math.h>
#include <SPIFFS.h>
#include <FS.h>

#include "SampleSource.h"
#include "I2SOutput.h"

// number of frames to try and send at once (a frame is a left and right sample)
#define NUM_FRAMES_TO_SEND 128

void i2sWriterTask(void *param)
{
    I2SOutput *output = (I2SOutput *)param;
    int availableBytes = 0;
    int buffer_position = 0;
    Frame_t frames[NUM_FRAMES_TO_SEND];
    while (true)
    {
        // wait for some data to be requested
        i2s_event_t evt;
        if (xQueueReceive(output->m_i2sQueue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_TX_DONE)
            {
                size_t bytesWritten = 0;
                do
                {
                    if (availableBytes == 0)
                    {
                        // get some frames from the wave file - a frame consists of a 16 bit left and right sample
                        if (output->m_sample_generator && output->m_sample_generator->available())
                        {
                            int frames_available = output->m_sample_generator->getFrames(frames, NUM_FRAMES_TO_SEND);
                            // how maby bytes do we now have to send
                            availableBytes = frames_available * sizeof(uint32_t);
                        }
                        else
                        {
                            // no sample generator available - just send silence
                            for (int i = 0; i < NUM_FRAMES_TO_SEND; i++)
                            {
                                frames[i].left = frames[i].right = 0;
                            }
                            availableBytes = NUM_FRAMES_TO_SEND * sizeof(uint32_t);
                        }
                        // reset the buffer position back to the start
                        buffer_position = 0;
                    }
                    // do we have something to write?
                    if (availableBytes > 0)
                    {
                        // write data to the i2s peripheral
                        i2s_write(output->m_i2sPort, buffer_position + (uint8_t *)frames,
                                  availableBytes, &bytesWritten, portMAX_DELAY);
                        availableBytes -= bytesWritten;
                        buffer_position += bytesWritten;
                    }
                } while (bytesWritten > 0);
            }
        }
    }
}

void I2SOutput::start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins)
{
    // i2s config for writing both channels of I2S
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0};
    m_i2sPort = i2sPort;
    //install and start i2s driver
    i2s_driver_install(m_i2sPort, &i2sConfig, 4, &m_i2sQueue);
    // set up the i2s pins
    i2s_set_pin(m_i2sPort, &i2sPins);
    // clear the DMA buffers
    i2s_zero_dma_buffer(m_i2sPort);
    // start a task to write samples to the i2s peripheral
    TaskHandle_t writerTaskHandle;
    xTaskCreate(i2sWriterTask, "i2s Writer Task", 4096, this, 1, &writerTaskHandle);
}

void I2SOutput::setSampleGenerator(SampleSource *sample_generator)
{
    m_sample_generator = sample_generator;
}
