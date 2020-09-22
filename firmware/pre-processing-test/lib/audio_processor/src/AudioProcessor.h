#ifndef AUDIO_PROCESSOR
#define AUDIO_PROCESSOR

#include <stdlib.h>
#include <stdint.h>
// #define FIXED_POINT (16)
#include "./kissfft/tools/kiss_fftr.h"

class HammingWindow;

class AudioProcessor
{
private:
    int m_window_size;
    size_t m_fft_size;
    float *m_fft_input;
    int m_energy_size;
    int m_pooled_energy_size;
    int m_pooling_size;
    float *m_energy;
    kiss_fft_cpx *m_fft_output;
    kiss_fftr_cfg m_cfg;

    HammingWindow *m_hamming_window;

public:
    AudioProcessor(int window_size, int pooling_size);
    ~AudioProcessor();
    float *run(float *input);
    int outputSize() { return m_pooled_energy_size; }
};

#endif