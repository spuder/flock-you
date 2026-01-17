#ifndef DISPLAY_H
#define DISPLAY_H

// Display module for T-Dongle S3 only
// This file is only included when BOARD_T_DONGLE_S3 is defined

#include <Arduino.h>

// Display states
enum DisplayState {
    DISPLAY_BOOT,
    DISPLAY_SCANNING,
    DISPLAY_DETECTION,
    DISPLAY_HEARTBEAT
};

// Initialize the TFT display
void display_init();

// Show boot screen with device info
void display_boot_screen();

// Show scanning status with channel info
void display_scanning(uint8_t wifi_channel, bool ble_active);

// Show detection alert
void display_detection(const char* type, const char* name, const char* mac, int threat_score);

// Show heartbeat indicator (device still in range)
void display_heartbeat();

// Clear detection and return to scanning
void display_clear_detection();

// Update display (call periodically from loop)
void display_update();

#endif // DISPLAY_H
