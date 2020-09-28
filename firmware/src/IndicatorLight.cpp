#include <Arduino.h>
#include "IndicatorLight.h"

// This task does all the heavy lifting for our application
void indicatorLedTask(void *param)
{
    IndicatorLight *indicator_light = static_cast<IndicatorLight *>(param);
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
    while (true)
    {
        // wait for someone to trigger us
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        if (ulNotificationValue > 0)
        {
            switch (indicator_light->getState())
            {
            case OFF:
            {
                ledcWrite(0, 0);
                break;
            }
            case ON:
            {
                ledcWrite(0, 255);
                break;
            }
            case PULSING:
            {
                // do a nice pulsing effect
                float angle = 0;
                while (indicator_light->getState() == PULSING)
                {
                    ledcWrite(0, 255 * (0.5 * cos(angle) + 0.5));
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    angle += 0.4 * M_PI;
                }
            }
            }
        }
    }
}

IndicatorLight::IndicatorLight()
{
    // use the build in LED as an indicator - we'll set it up as a pwm output so we can make it glow nicely
    ledcSetup(0, 10000, 8);
    ledcAttachPin(2, 0);
    ledcWrite(0, 0);
    // start off with the light off
    m_state = OFF;
    // set up the task for controlling the light
    xTaskCreate(indicatorLedTask, "Indicator LED Task", 4096, this, 1, &m_taskHandle);
}

void IndicatorLight::setState(IndicatorState state)
{
    m_state = state;
    xTaskNotify(m_taskHandle, 1, eSetBits);
}

IndicatorState IndicatorLight::getState()
{
    return m_state;
}