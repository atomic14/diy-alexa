# DIY Alexa Wake Word Detection

This folder contains the Jupyter notebooks for creating the training data, training the model, and exporting the model to TensorFlow Lite.

## Setup

You will need python3 installed - follow the instructions for your platform to get this set up and then create a virtual environment.

```
python3 -m venv venv
. ./venv/bin/activate
pip install -r requirements.txt
```

## Running the notebooks

```
. ./venv/bin/activate
jupyter notebook .
```

# The notebooks

## Generate Training Data.pynb

We make use of the speech commands dataset available from here:

[https://storage.cloud.google.com/download.tensorflow.org/data/speech_commands_v0.02.tar.gz](https://storage.cloud.google.com/download.tensorflow.org/data/speech_commands_v0.02.tar.gz)

Download and then expand the data using:

```
mkdir speech_data
tar -xzf data_speech_commands_v0.02.tar.gz -C speech_data
```

For my training, I add a lot more data to the `_background_noise_` folder. I also created a `_problem_noise_` folder and recorded sounds that seemed to confuse the model - low frequency humming noises around 100Hz seem to cause problems.

I also tried to eliminate confusion between other words that start with `Mar` and created a folder called `_mar_sounds_`. To generate the `mar` sounds I created a script called `generate_m_words.sh` which uses the built-in `say` command on the Mac computer to generate speech.

You can remove this from the training data if you don't have this available.

The notebook will run through all these samples and output files for the training step. You will need about 4GB of free space to save these files.

## Tain Model.ipynb

This will train a model against the training data. This will train on a CPU in 1-2 hours. If you have a suitable GPU this training will be considerably faster.

The training will output a file called `checkpoint.model` every time is sees an improvement in the validation performance and a file called `trained.model` on training completion.

You can optionally take these and train them on the complete dataset.

## Convert Trained Model To TFLite.ipynb

This will take the TensorFlow model and convert it for use in TensorFlow lite.

Copy the output of this workbook into `firmware/lib/nerual_network/model.cc`.

A pre-trained model has already been generated and placed in that location.
