#include <Arduino.h>
#include "I2SSampler.h"
#include "AudioProcessor.h"
#include "NeuralNetwork.h"
#include "RingBuffer.h"
#include "DetectWakeWordState.h"

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

DetectWakeWordState::DetectWakeWordState(I2SSampler *sample_provider)
{
    // Create our neural network
    m_nn = new NeuralNetwork();
    Serial.println("Created Neral Net");
    // create our audio processor
    m_audio_processor = new AudioProcessor(AUDIO_LENGTH, WINDOW_SIZE, STEP_SIZE, POOLING_SIZE);
    Serial.println("Created audio processor");
    // save the sample provider for use later
    m_sample_provider = sample_provider;
    // some stats on performance
    m_average_detect_time = 0;
}
void DetectWakeWordState::enterState()
{
    // nothing to do
}
bool DetectWakeWordState::run()
{
    // time how long this takes for stats
    long start = millis();
    // get access to the samples that have been read in
    RingBufferAccessor *reader = m_sample_provider->getRingBufferReader();
    // get hold of the input buffer for the neural network so we can feed it data
    float *input_buffer = m_nn->getInputBuffer();
    // process the samples to get the spectrogram
    m_audio_processor->get_spectrogram(reader, input_buffer);
    // feed the watch dog
    vTaskDelay(10);
    // get the prediction for the spectrogram
    float output = m_nn->predict();
    long end = millis();
    m_average_detect_time = (end - start) * 0.1 + m_average_detect_time * 0.9;
    // set the LED to glow depending on how strong the input is
    ledcWrite(0, output * 255);
    delete reader;
    // use quite a high threshold to prevent false positives
    if (output > 0.95)
    {
        Serial.printf("%.2f: Here I am, brain the size of a planet\n", output);
        // detected the wake word, move to the next state
        return true;
    }
    // nothing detected stay in the current state
    return false;
}
void DetectWakeWordState::exitState()
{
    // nothing to do
}