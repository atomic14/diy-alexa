from flask import Flask, request
import azure.cognitiveservices.speech as speechsdk

intent_key="fe7b4822a97f4d048fc49a01415223eb"
intent_service_region="westus"
language_understanding_app_id="41325105-7aea-4825-b494-4af81805d9ba"

app = Flask(__name__)

model = speechsdk.intent.LanguageUnderstandingModel(intent_key, intent_service_region, language_understanding_app_id)

@app.route('/interpret_speech', methods=['POST'])
def interpret_speech():
        data = request.data
        intent_config = speechsdk.SpeechConfig(subscription=intent_key, region=intent_service_region)

        audio_format = speechsdk.audio.AudioStreamFormat(samples_per_second=16000, bits_per_sample=16, channels=1)
        push_stream = speechsdk.audio.PushAudioInputStream(stream_format=audio_format)
        audio_config = speechsdk.audio.AudioConfig(stream=push_stream)

        push_stream.write(data)
        push_stream.close()

        # audio_config = speechsdk.audio.AudioConfig(filename="off_sample.wav")
        # Set up the intent recognizer
        intent_recognizer = speechsdk.intent.IntentRecognizer(speech_config=intent_config, audio_config=audio_config)
        intent_recognizer.add_all_intents(model)
        intent_result = intent_recognizer.recognize_once()

        # Check the results
        if intent_result.reason == speechsdk.ResultReason.RecognizedIntent:
            print("Recognized: \"{}\" with intent id `{}`".format(intent_result.text, intent_result.intent_id))
            print(intent_result)
        elif intent_result.reason == speechsdk.ResultReason.RecognizedSpeech:
            print("Recognized: {}".format(intent_result.text))
        elif intent_result.reason == speechsdk.ResultReason.NoMatch:
            print("No speech could be recognized: {}".format(intent_result.no_match_details))
        elif intent_result.reason == speechsdk.ResultReason.Canceled:
            print("Intent recognition canceled: {}".format(intent_result.cancellation_details.reason))
            if intent_result.cancellation_details.reason == speechsdk.CancellationReason.Error:
                print("Error details: {}".format(intent_result.cancellation_details.error_details))

        return "OK"

