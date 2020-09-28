#ifndef _intent_processor_h_
#define _intent_processor_h_

#include <map>
#include <ArduinoJson.h>

class IntentProcessor
{
private:
    std::map<std::string, int> m_device_to_pin;

public:
    IntentProcessor();
    void addDevice(const std::string &name, int gpio_pin);
    bool processIntent(JsonDocument &intent_json);
};

#endif
