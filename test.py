import pyaudio
import time
import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.python.ops import gen_audio_ops as audio_ops
from datetime import datetime

model =  keras.models.load_model("checkpoint.model")

FORMAT = pyaudio.paFloat32
RATE = 16000
CHANNELS = 1
NOFFRAMES = 8000

audio = pyaudio.PyAudio()

info = audio.get_host_api_info_by_index(0)
numdevices = info.get('deviceCount')
for i in range(0, numdevices):
    if (audio.get_device_info_by_host_api_device_index(0, i).get('maxInputChannels')) > 0:
        print("Input Device id ", i, " - ", audio.get_device_info_by_host_api_device_index(0, i).get('name'))


samples = np.zeros((8000))


def callback(input_data, frame_count, time_info, flags):
    global samples
    # print("Got audio " + str(frame_count))
    new_samples = np.frombuffer(input_data, np.float32)
    samples = np.concatenate((samples, new_samples))
    samples = samples[-16000:]
    
    if len(samples) == 16000:
        start = time.perf_counter()
        # normalise the samples
        normalised = samples - np.mean(samples)
        max = np.max(normalised)
        if max > 0:
            normalised = normalised / max

        # create the spectrogram
        spectrogram = audio_ops.audio_spectrogram(
            np.reshape(normalised, (16000, 1)),
            window_size=320,
            stride=160,
            magnitude_squared=True)
        # reduce the number of frequency bins in our spectrogram to a more sensible level
        spectrogram = tf.nn.pool(
            input=tf.expand_dims(spectrogram, -1),
            window_shape=[1, 6],
            strides=[1, 6],
            pooling_type='AVG',
            padding='SAME')
        # remove the first 1 index
        spectrogram = tf.squeeze(spectrogram, axis=0)
        spectrogram = np.log10(spectrogram + 1e-6)
        prediction = model.predict(np.reshape(spectrogram, (1, 99, 43, 1)))
        if prediction[0][0] > 0.9:
            print(f"{datetime.now().time()} - Here I am, brain the size of a planet.... {prediction[0][0]}")
        end = time.perf_counter()
        # print((end-start)*1000)

    return input_data, pyaudio.paContinue


stream = audio.open(
    input_device_index=0,
    format=FORMAT,
    channels=CHANNELS,
    rate=RATE,
    input=True,
    stream_callback=callback,
    frames_per_buffer=NOFFRAMES)

stream.start_stream()

# wait for stream to finish (5)
while stream.is_active():
    time.sleep(0.1)

# stream.stop_stream()
# stream.close()
# p.terminate()
# print('done')
# plt.plot(decoded)
# plt.show()
