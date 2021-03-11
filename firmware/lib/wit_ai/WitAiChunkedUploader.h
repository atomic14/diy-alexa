#ifndef _witai_chunked_uploaded_h_
#define _witai_chunked_uploaded_h_

#include <stdint.h>
#include <string>

class WiFiClientSecure;

typedef struct
{
    std::string text;
    std::string intent_name;
    float intent_confidence;
    std::string device_name;
    float device_confidence;
    std::string trait_value;
    float trait_confidence;
} Intent;

class WitAiChunkedUploader
{
private:
    WiFiClientSecure *m_wifi_client;

public:
    WitAiChunkedUploader(const char *access_key);
    ~WitAiChunkedUploader();
    bool connected();
    void startChunk(int size_in_bytes);
    void sendChunkData(const uint8_t *data, int size_in_bytes);
    void finishChunk();
    Intent getResults();
};

#endif