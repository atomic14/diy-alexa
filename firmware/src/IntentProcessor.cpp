#include <Arduino.h>
#include "IntentProcessor.h"

IntentProcessor::IntentProcessor()
{
}

bool IntentProcessor::processIntent(JsonDocument &intent_json)
{
    if (intent_json["text"])
    {
        Serial.printf("I heard \"%s\"\n", intent_json["text"].as<const char *>());
    }
    const char *device_name = intent_json["entities"]["device:device"][0]["value"];
    if (!device_name)
    {
        Serial.println("No device found");
        return false;
    }
    float deviceConfidence = intent_json["entities"]["device:device"][0]["confidence"];
    if (deviceConfidence < 0.8)
    {
        Serial.printf("Only %.f%% certain on device\n", 100 * deviceConfidence);
        return false;
    }
    const char *intent_name = intent_json["intents"][0]["name"];
    if (!intent_name)
    {
        Serial.println("Can't work out what you want to do with the device...");
        return false;
    }
    float intentConfidence = intent_json["intents"][0]["confidence"];
    if (intentConfidence < 0.8)
    {
        Serial.printf("Only %.f%% certain on intent\n", 100 * intentConfidence);
        return false;
    }
    if (strcmp("Turn_on_device", intent_name) != 0)
    {
        Serial.println("Don't understand the intent");
        return false;
    }
    const char *trait_value = intent_json["traits"]["wit$on_off"][0]["value"];
    if (!trait_value)
    {
        Serial.println("Can't work out the intent action");
        return false;
    }
    float trait_confidence = intent_json["traits"]["wit$on_off"][0]["confidence"];
    if (trait_confidence < 0.8)
    {
        Serial.printf("Only %.f%% certain on trait\n", 100 * trait_confidence);
        return false;
    }
    bool is_turn_on = strcmp("on", trait_value) == 0;

    // global device name "lights"
    if (strcmp("lights", device_name) == 0)
    {
        for (const auto &dev_pin : m_device_to_pin)
        {
            digitalWrite(dev_pin.second, is_turn_on);
        }
    }
    else
    {
        // see if the device name is something we know about
        if (m_device_to_pin.find(device_name) == m_device_to_pin.end())
        {
            Serial.printf("Don't recognise the device '%s'\n", device_name);
            return false;
        }
        digitalWrite(m_device_to_pin[device_name], is_turn_on);
    }
    // success!
    return true;
}

void IntentProcessor::addDevice(const std::string &name, int gpio_pin)
{
    m_device_to_pin.insert(std::make_pair(name, gpio_pin));
    pinMode(gpio_pin, OUTPUT);
}
