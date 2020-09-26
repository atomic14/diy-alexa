# DIY Alexa with the ESP32

This project consists of three components:

## Model

Jupyter notebooks for creating a TensorFlow Lite model for "wake word" recognition.

A pre-trained model has already been generated and added to the firmware folder.

## Firmware

ESP32 firmware built using Platform.io. This runs the neural network trying to detect the wake word (`Marvin`). When it hears the wake word it starts recording audio. This audio is then transmitted to the server for processing.

## Server

This receives the audio from the ESP32 and sends it onto Microsoft's Language Understanding Cognitive Service (https://www.luis.ai/) to determine what the instruction is.
