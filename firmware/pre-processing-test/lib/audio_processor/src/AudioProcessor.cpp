#include <stdlib.h>
#include "AudioProcessor.h"
#include "HammingWindow.h"

#define EPSILON 1e-6

AudioProcessor::AudioProcessor(int window_size, int pooling_size)
{
    m_pooling_size = pooling_size;
    m_window_size = window_size;
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
float *AudioProcessor::run(float *input)
{
    size_t i;
    for (i = 0; i < m_window_size; i++)
    {
        m_fft_input[i] = input[i];
    }
    // Zero out whatever else remains in the top part of the input.
    for (; i < m_fft_size; i++)
    {
        m_fft_input[i] = 0;
    }
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
    float *output_dst = m_energy;
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
        m_energy[i] = log10f(m_energy[i] + EPSILON);
    }
    return m_energy;
}
