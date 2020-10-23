# DIY Alexa with the ESP32

Want to build your own Alexa? All you will need is an ESP32 and Microphone board.

You can find a detailed tutorial in the [README.md](../master/README.md) file and there's also a detailed tutorial video [here on YouTube](https://youtu.be/re-dSV_a0tM)

[![Demo Video](https://img.youtube.com/vi/re-dSV_a0tM/0.jpg)](https://www.youtube.com/watch?v=re-dSV_a0tM)

I'm using a microphone breakout board that I've built myself based around the ICS-43434 - but any microphone board will work. The code has been written so that you can either use an I2S microphone or an analogue microphone using the built-in ADC.

I would recommend using an I2S microphone if you have one as they have a lot better noise characteristics.

My board is available on [eBay](https://www.ebay.co.uk/itm/154115095985) and [Tindie](https://www.tindie.com/products/atomic14/ics-43434-i2s-mems-microphone-breakout-board/)

Other I2S microphones are equally suitable. Boards based around the INMP441 work very well.

Wake word detection is carried out using a model trained with TensorFlow and runs on the ESP32 using TensorFlow Lite. A pre-trained model is included in the Firmware folder so you can get up and running straight away.

Command recognition is carried using wit.ai - a free service from Facebook - I've included the access token for this in the code but will disable it in the next few weeks.

There are two folders in this repo `model` and `firmware` check the `README.md` file in each one for complete details.

## Model

Jupyter notebooks for creating a TensorFlow Lite model for "wake word" recognition.

A pre-trained model has already been generated and added to the firmware folder.

## Firmware

ESP32 firmware built using Platform.io. This runs the neural network trying to detect the wake word (`Marvin`). When it hears the wake word it starts recording audio. This audio is then transmitted to the wit.ai server for processing.
