  /*********/
 /* Blink */
/*********/
#include "mcp23s17.h"

const int LED_PIN = 7;
mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_0); // All addressing pins set to GND

void setup (void) {
    gpio_x.pinMode(LED_PIN, mcp23s17::PinMode::OUTPUT);
}

void loop (void) {
    gpio_x.digitalWrite(LED_PIN, mcp23s17::PinLatchValue::HIGH);
    delay(500);
    gpio_x.digitalWrite(LED_PIN, mcp23s17::PinLatchValue::LOW);
    delay(500);
}

