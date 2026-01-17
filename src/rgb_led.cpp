// Only compile for T-Dongle S3
#ifdef BOARD_T_DONGLE_S3

#include "rgb_led.h"

#include <FastLED.h>

// T-Dongle S3 APA102 LED configuration
#define DATA_PIN    40
#define CLOCK_PIN   39
#define NUM_LEDS    1
#define LED_TYPE    APA102
#define COLOR_ORDER BGR

// LED array
static CRGB leds[NUM_LEDS];

// Animation state
static bool detection_active = false;
static bool heartbeat_active = false;
static unsigned long last_update = 0;
static uint8_t flash_count = 0;
static bool flash_red = true;

void rgb_init() {
    FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(50);
    rgb_off();
}

void rgb_boot_sequence() {
    // Quick RGB cycle to show it's working
    FastLED.setBrightness(100);

    // Red
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(100);

    // Blue
    leds[0] = CRGB::Blue;
    FastLED.show();
    delay(100);

    // Cyan
    leds[0] = CRGB::Cyan;
    FastLED.show();
    delay(100);

    // Off
    FastLED.setBrightness(50);
    rgb_off();
}

void rgb_detection_flash() {
    // Start rapid red/blue strobe (replaces beeper)
    detection_active = true;
    heartbeat_active = false;
    flash_count = 0;
    flash_red = true;
    last_update = millis();

    // Immediate red flash
    FastLED.setBrightness(255);
    leds[0] = CRGB::Red;
    FastLED.show();
}

void rgb_heartbeat_pulse() {
    if (!detection_active) {
        heartbeat_active = true;
    }
}

void rgb_set_color(uint8_t r, uint8_t g, uint8_t b) {
    detection_active = false;
    heartbeat_active = false;
    leds[0] = CRGB(r, g, b);
    FastLED.show();
}

void rgb_off() {
    detection_active = false;
    heartbeat_active = false;
    leds[0] = CRGB::Black;
    FastLED.show();
}

void rgb_update() {
    unsigned long now = millis();

    if (detection_active) {
        // Fast red/blue strobe - like police lights (replaces beeper)
        if (now - last_update >= 50) {  // 50ms = very fast strobe
            flash_red = !flash_red;

            if (flash_red) {
                leds[0] = CRGB::Red;
            } else {
                leds[0] = CRGB::Blue;
            }
            FastLED.setBrightness(255);
            FastLED.show();
            last_update = now;
            flash_count++;

            // Stop strobing after ~3 seconds (60 flashes), go to heartbeat
            if (flash_count > 60) {
                detection_active = false;
                heartbeat_active = true;
                FastLED.setBrightness(50);
            }
        }
    } else if (heartbeat_active) {
        // Slow orange pulse for heartbeat (device still in range)
        if (now - last_update >= 500) {
            static bool pulse_on = false;
            pulse_on = !pulse_on;

            if (pulse_on) {
                leds[0] = CRGB(255, 100, 0);  // Orange
                FastLED.setBrightness(80);
            } else {
                leds[0] = CRGB(255, 50, 0);  // Dim orange
                FastLED.setBrightness(30);
            }
            FastLED.show();
            last_update = now;
        }
    }
}

#endif // BOARD_T_DONGLE_S3
