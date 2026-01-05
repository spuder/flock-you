#ifndef RGB_LED_H
#define RGB_LED_H

// RGB LED module for T-Dongle S3 only
// This file is only included when BOARD_T_DONGLE_S3 is defined

#include <Arduino.h>

// Initialize the RGB LED
void rgb_init();

// Boot sequence animation
void rgb_boot_sequence();

// Flash red/blue for detection alert (replaces beeper)
void rgb_detection_flash();

// Pulse while device in range (heartbeat)
void rgb_heartbeat_pulse();

// Set specific color
void rgb_set_color(uint8_t r, uint8_t g, uint8_t b);

// Turn off LED
void rgb_off();

// Update LED state (call from loop for animations)
void rgb_update();

#endif // RGB_LED_H
