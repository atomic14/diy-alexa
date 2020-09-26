#ifndef _application_h_
#define _applicaiton_h_

class I2SSampler;
class NeuralNetwork;
class AudioProcessor;

class Application
{
private:
    I2SSampler *m_sample_provider;
    NeuralNetwork *m_nn;
    float *m_input_buffer;
    AudioProcessor *m_audio_processor;

public:
    Application(I2SSampler *sample_provider);
    void run();
};

#endif