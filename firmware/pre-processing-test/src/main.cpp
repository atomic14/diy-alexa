#include <Arduino.h>
#include "WavFileReader.h"
#include "AudioProcessor.h"
#include "NeuralNetwork.h"

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting up");
  // create the processing task
}

void testWavFile()
{
  NeuralNetwork *nn = new NeuralNetwork();

  Serial.println("Created Neral Net");

  float *input_buffer = nn->getInputBuffer();

  Serial.println("Got input buffer");

  int16_t *all_samples = (int16_t *)malloc(sizeof(int16_t) * AUDIO_LENGTH);
  if (!all_samples)
  {
    Serial.println("Could not allocate space for samples");
  }

  // open the wave file
  WAVFileReader *wavFileReader = new WAVFileReader("/sample.wav");
  // read in the samples
  wavFileReader->getSamples(all_samples, AUDIO_LENGTH);

  // create the audio processor
  AudioProcessor *processor = new AudioProcessor(WINDOW_SIZE, POOLING_SIZE);

  unsigned long total_audio_prep_time = 0;
  unsigned long total_inference_time = 0;

  unsigned long time1 = millis();

  // get the normalisation parameters
  float max = 0;
  float mean = 0;
  for (int i = 0; i < AUDIO_LENGTH; i++)
  {
    mean += all_samples[i];
  }
  mean /= AUDIO_LENGTH;
  for (int i = 0; i < AUDIO_LENGTH; i++)
  {
    max = std::max(max, ((float)all_samples[i]) - mean);
  }
  int row = 0;
  for (int start = 0; start < AUDIO_LENGTH - WINDOW_SIZE; start += STEP_SIZE)
  {
    // printf("Processing sample starting at %d\n", start);
    // get a window of normalised samples from the input
    float samples[WINDOW_SIZE];
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
      samples[i] = ((float)(all_samples[i + start]) - mean) / max;
    }
    // Process the samples - this is a destructive process - the samples will be changed
    float *results = processor->run(samples);
    float results_size = processor->outputSize();
    for (int i = 0; i < results_size; i++)
    {
      input_buffer[i + row * 43] = results[i];
      //   // Serial.printf("%f,", results[i]);
    }
    // // Serial.printf("Processed row %d\n", row);
    row++;
  }
  unsigned long time2 = millis();
  total_audio_prep_time += time2 - time1;
  float output = nn->predict();
  unsigned long time3 = millis();
  total_inference_time += time3 - time2;
  Serial.printf("Sample Prep Elapsed time %ld\n", total_audio_prep_time);
  Serial.printf("Inference Elapsed time %ld\n", total_inference_time);
  Serial.printf("Output value is %f\n", output);
}

void loop()
{
  // put your main code here, to run repeatedly:
}