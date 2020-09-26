#include <Arduino.h>
#include "Application.h"
#include "I2SSampler.h"
#include "AudioProcessor.h"
#include "NeuralNetwork.h"

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

Application::Application(I2SSampler *sample_provider)
{
    // Create our neural network
    m_nn = new NeuralNetwork();
    Serial.println("Created Neral Net");
    // get hold of the input buffer for the neural network so we can feed it data
    m_input_buffer = m_nn->getInputBuffer();
    Serial.println("Got input buffer");
    // create our audio processor
    m_audio_processor = new AudioProcessor(AUDIO_LENGTH, WINDOW_SIZE, STEP_SIZE, POOLING_SIZE);
    Serial.println("Created audio processor");
    // save the sample provider for use later
    m_sample_provider = sample_provider;
}

// process the next batch of samples
void Application::run()
{
    // time how long this takes for stats
    long start = millis();
    // get access to the samples that have been read in
    RingBufferAccessor *reader = m_sample_provider->getRingBufferReader();
    // process the samples to get the spectrogram
    m_audio_processor->get_spectrogram(reader, m_input_buffer);
    // feed the watchdog so we don't get killed
    vTaskDelay(10);
    // get the prediction for the spectrogram
    float output = m_nn->predict();
    long end = millis();
    if (output > 0.95)
    {
        Serial.printf("Here I am, brain the size of a planet: Output=%.2f  - %ld\n", output, end - start);
    }
    ledcWrite(0, output * 255);
    delete reader;
}
