#include <stdlib.h>
#include <algorithm>
#include "AudioProcessor.h"
#include "HammingWindow.h"
#include "RingBuffer.h"

#define EPSILON 1e-6

AudioProcessor::AudioProcessor(int audio_length, int window_size, int step_size, int pooling_size)
{
    m_audio_length = audio_length;
    m_window_size = window_size;
    m_step_size = step_size;
    m_pooling_size = pooling_size;
    m_fft_size = 1;
    while (m_fft_size < window_size)
    {
        m_fft_size <<= 1;
    }
    m_fft_input = static_cast<float *>(malloc(sizeof(float) * m_fft_size));
    m_energy_size = m_fft_size / 2 + 1;
    m_fft_output = static_cast<kiss_fft_cpx *>(malloc(sizeof(kiss_fft_cpx) * m_energy_size));
    m_energy = static_cast<float *>(malloc(sizeof(float) * m_energy_size));
    // work out the pooled energy size
    m_pooled_energy_size = ceilf((float)m_energy_size / (float)pooling_size);
    printf("m_pooled_energy_size=%d\n", m_pooled_energy_size);
    // initialise kiss fftr
    m_cfg = kiss_fftr_alloc(m_fft_size, false, 0, 0);
    // initialise the hamming window
    m_hamming_window = new HammingWindow(m_window_size);
}

AudioProcessor::~AudioProcessor()
{
    free(m_cfg);
    free(m_fft_input);
    free(m_fft_output);
    free(m_energy);
    delete m_hamming_window;
}

// takes a normalised array of input samples of window_size length
void AudioProcessor::get_spectrogram_segment(float *output)
{
    // apply the hamming window to the samples
    m_hamming_window->applyWindow(m_fft_input);
    // do the fft
    kiss_fftr(
        m_cfg,
        m_fft_input,
        reinterpret_cast<kiss_fft_cpx *>(m_fft_output));
    // pull out the magnitude squared values
    for (int i = 0; i < m_energy_size; i++)
    {
        const float real = m_fft_output[i].r;
        const float imag = m_fft_output[i].i;
        const float mag_squared = (real * real) + (imag * imag);
        m_energy[i] = mag_squared;
    }
    // reduce the size of the output by pooling with average and same padding
    float *output_src = m_energy;
    float *output_dst = output;
    for (int i = 0; i < m_energy_size; i += m_pooling_size)
    {
        float average = 0;
        for (int j = 0; j < m_pooling_size; j++)
        {
            if (i + j < m_energy_size)
            {
                average += *output_src;
                output_src++;
            }
        }
        *output_dst = average / m_pooling_size;
        output_dst++;
    }
    // now take the log to give us reasonable values to feed into the network
    for (int i = 0; i < m_pooled_energy_size; i++)
    {
        output[i] = log10f(output[i] + EPSILON);
    }
}

void AudioProcessor::get_spectrogram(RingBufferAccessor *reader, float *output_spectrogram)
{
    int startIndex = reader->getIndex();
    // get the mean value of the samples
    float mean = 0;
    for (int i = 0; i < m_audio_length; i++)
    {
        mean += reader->getCurrentSample();
        reader->moveToNextSample();
    }
    mean /= m_audio_length;
    // get the absolute max value of the samples taking into account the mean value
    reader->setIndex(startIndex);
    float max = 0;
    for (int i = 0; i < m_audio_length; i++)
    {
        max = std::max(max, fabsf(((float)reader->getCurrentSample()) - mean));
        reader->moveToNextSample();
    }
    // extract windows of samples moving forward by step size each time and compute the spectrum of the window
    for (int window_start = startIndex; window_start < startIndex + 16000 - m_window_size; window_start += m_step_size)
    {
        // move the reader to the start of the window
        reader->setIndex(window_start);
        // read samples from the reader into the fft input normalising them by subtracting the mean and dividing by the absolute max
        for (int i = 0; i < m_window_size; i++)
        {
            m_fft_input[i] = ((float)reader->getCurrentSample() - mean) / max;
            reader->moveToNextSample();
        }
        // zero out whatever else remains in the top part of the input.
        for (int i = m_window_size; i < m_fft_size; i++)
        {
            m_fft_input[i] = 0;
        }
        // compute the spectrum for the window of samples and write it to the output
        get_spectrogram_segment(output_spectrogram);
        // move to the next row of the output spectrogram
        output_spectrogram += m_pooled_energy_size;
    }
}