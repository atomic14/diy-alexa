#ifndef AUDIO_PROCESSOR
#define AUDIO_PROCESSOR

#include <stdlib.h>
#include <stdint.h>
// #define FIXED_POINT (16)
#include "./kissfft/tools/kiss_fftr.h"

class HammingWindow;

class RingBufferAccessor;

class AudioProcessor
{
private:
    int m_audio_length;
    int m_window_size;
    int m_step_size;
    int m_pooling_size;
    size_t m_fft_size;
    float *m_fft_input;
    int m_energy_size;
    int m_pooled_energy_size;
    float *m_energy;
    kiss_fft_cpx *m_fft_output;
    kiss_fftr_cfg m_cfg;

    HammingWindow *m_hamming_window;

    void get_spectrogram_segment(float *output_spectrogram_row);

public:
    AudioProcessor(int audio_length, int window_size, int step_size, int pooling_size);
    ~AudioProcessor();
    void get_spectrogram(RingBufferAccessor *reader, float *output_spectrogram);
};

#endif