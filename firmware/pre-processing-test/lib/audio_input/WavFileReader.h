#ifndef __wav_file_reader_h__
#define __wav_file_reader_h__

#include <SPIFFS.h>
#include <FS.h>

class File;

class WAVFileReader
{
private:
    int m_num_channels;
    int m_sample_rate;
    File m_file;

public:
    WAVFileReader(const char *file_name);
    ~WAVFileReader();
    int sampleRate() { return m_sample_rate; };
    void getSamples(int16_t *samples, int number_samples);
};

#endif