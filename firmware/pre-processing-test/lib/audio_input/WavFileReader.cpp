#include <stdio.h>
#include <stdlib.h>
#include "WavFileReader.h"

typedef struct
{
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate;          // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth;        // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes;      // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header_t;

WAVFileReader::WAVFileReader(const char *file_name)
{
    SPIFFS.begin();
    Serial.println("Opening file");
    m_file = SPIFFS.open(file_name);
    Serial.println("Reading Header");
    // read the WAV header
    wav_header_t wav_header;
    m_file.read((uint8_t *)&wav_header, sizeof(wav_header_t));
    Serial.println("Read header");
    // sanity check the bit depth
    if (wav_header.bit_depth != 16)
    {
        Serial.printf("ERROR: bit depth %d is not supported\n", wav_header.bit_depth);
    }

    Serial.printf("fmt_chunk_size=%d, audio_format=%d, num_channels=%d, sample_rate=%d, sample_alignment=%d, bit_depth=%d, data_bytes=%d\n",
                  wav_header.fmt_chunk_size, wav_header.audio_format, wav_header.num_channels, wav_header.sample_rate, wav_header.sample_alignment, wav_header.bit_depth, wav_header.data_bytes);

    m_num_channels = wav_header.num_channels;
    m_sample_rate = wav_header.sample_rate;
}

WAVFileReader::~WAVFileReader()
{
    m_file.close();
}

void WAVFileReader::getSamples(int16_t *samples, int number_samples)
{
    m_file.read((uint8_t *)samples, sizeof(int16_t) * number_samples);
}