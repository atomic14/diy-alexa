// WiFi credentials
#define WIFI_SSID "CMGResearch"
#define WIFI_PSWD "02087552867"

// are you using an I2S microphone - comment this out if you want to use an analog mic and ADC input
#define USE_I2S_MIC_INPUT

// I2S Microphone Settings

// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT
// #define I2S_MIC_CHANNEL RIGHT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_32
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_25
#define I2S_MIC_SERIAL_DATA GPIO_NUM_33

// Analog Microphone Settings

// GPIO35
#define ADC_MIC_CHANNEL ADC1_CHANNEL_7

// command recognition settings
#define COMMAND_RECOGNITION_URL "http://10.0.1.61:1314/speech?v=20200927"
#define COMMAND_RECOGNITION_HOST "10.0.1.61"
#define COMMAND_RECOGNITION_PORT 1314

#define COMMAND_RECOGNITION_ACCESS_KEY "UFFEIMRQL7LH4T2DXRHKER4HPMFB4LNH"
