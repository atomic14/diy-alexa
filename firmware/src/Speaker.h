#ifndef _speaker_h_
#define _speaker_h_

class I2SOutput;
class WAVFileReader;

class Speaker
{
private:
    WAVFileReader *m_ok;
    WAVFileReader *m_cantdo;
    WAVFileReader *m_ready_ping;
    WAVFileReader *m_life;
    WAVFileReader *m_jokes[5];

    I2SOutput *m_i2s_output;

public:
    Speaker(I2SOutput *i2s_output);
    ~Speaker();
    void playOK();
    void playReady();
    void playCantDo();
    void playRandomJoke();
    void playLife();
};

#endif