# DIY Alexa ESP32 Firmware

This folder contains the firmware for running DIY Alexa on the ESP32.

We are using Platform.io to build the firmware.

You will also need to upload the SPIFFs filesystem as there are some WAV files that are played.

If you're having problems detecting the wakeword `Marvin` then you can try lowering the detection threshold:

`/src/state_machine/DetectWakeWordState.cpp` line 59.

```
if (output > 0.95)
```

Change 0.95 to something lower and you will increase the sensitivity. It's also worth logging this value out if you are having problems so you can see how well the detector is picking up the work.

To understand the code the best place to start is `src/Application.cpp`. This is a very simple state machine that switched between either waiting for a wake word or trying to interpret the user's command.

From there you can look at `src/state_machine/DetectWakeWordState.cpp' and `src/state_machine/RecogniseCommandState.cpp`.

If you've watched the video the code should be fairly easy to follow.

## Config options

To set things up for yourself, edit the `config.h` file and fill in your WiFi details.

There are a number of options in this file that you can modify to suit your own setup.

If you want to use an analog microphone instead of I2S then you need to comment out this line:

```
// are you using an I2S microphone - comment this out if you want to use an analog mic and ADC input
#define USE_I2S_MIC_INPUT
```

And you will need to select the appropriate ADC channel to read data from:

```
// Analog Microphone Settings - ADC1_CHANNEL_7 is GPIO35
#define ADC_MIC_CHANNEL ADC1_CHANNEL_7
```

If you are using an I2S Microphone then you need to tell the system which channel you have configure the microphone on (left or right - generally these devices default to left).

```
// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
// #define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT
```

And you will need to tell it which pins you have connected to the microphone:

```
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_33
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_26
#define I2S_MIC_SERIAL_DATA GPIO_NUM_25
```

If you want to have speaker output then you will need to connect or change the following pins to your I2S amplifier:

```
// speaker settings
#define I2S_SPEAKER_SERIAL_CLOCK GPIO_NUM_14
#define I2S_SPEAKER_LEFT_RIGHT_CLOCK GPIO_NUM_12
#define I2S_SPEAKER_SERIAL_DATA GPIO_NUM_27
```

Finally, we have the access key for wit.ai - I will leave my key active for as long as possible.

```
// command recognition settings
#define COMMAND_RECOGNITION_ACCESS_KEY "UFFEIMRQL7LH4T2DXRHKER4HPMFB4LNH"
```
