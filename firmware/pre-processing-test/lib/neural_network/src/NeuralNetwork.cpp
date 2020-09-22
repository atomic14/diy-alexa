#include "NeuralNetwork.h"
#include "converted_model.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

NeuralNetwork::NeuralNetwork()
{
    error_reporter = new tflite::MicroErrorReporter();

    const int kModelArenaSize = 85152;
    const int kExtraArenaSize = 2000;
    const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;

    tensor_arena = (uint8_t *)malloc(kTensorArenaSize);
    if (!tensor_arena)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Could not allocate arena");
        return;
    }
    TF_LITE_REPORT_ERROR(error_reporter, "Loading model");

    model = tflite::GetModel(converted_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Model provided is schema version %d not equal to supported version %d.",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }
    // This pulls in all the operation implementations we need.
    resolver = new tflite::MicroMutableOpResolver<10>();
    resolver->AddConv2D();
    resolver->AddMaxPool2D();
    resolver->AddFullyConnected();
    resolver->AddMul();
    resolver->AddAdd();
    resolver->AddLogistic();
    resolver->AddReshape();
    resolver->AddQuantize();
    resolver->AddDequantize();

    // Build an interpreter to run the model with.
    interpreter = new tflite::MicroInterpreter(
        model, *resolver, tensor_arena, kTensorArenaSize, error_reporter);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    size_t used_bytes = interpreter->arena_used_bytes();
    TF_LITE_REPORT_ERROR(error_reporter, "Used bytes %d\n", used_bytes);

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);
    // Get information about the memory area to use for the model's input.
    if (input->dims->size != 4)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Wrong number of input dimensions?");
    }
    // Serial.printf("Model inputs have %d dimensions of (%d, %d, %d, %d) type is %d\n",
    //               input->dims->size,
    //               input->dims->data[0],
    //               input->dims->data[1],
    //               input->dims->data[2],
    //               input->dims->data[3],
    //               input->type);
}

float *NeuralNetwork::getInputBuffer()
{
    return input->data.f;
}

float NeuralNetwork::predict()
{
    interpreter->Invoke();
    return output->data.f[0];
}
