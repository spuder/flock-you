// Only compile for T-Dongle S3
#ifdef BOARD_T_DONGLE_S3

#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define DEVICE_NAME "T-Dongle S3"

// T-Dongle S3 Display pins
#define TFT_CS     4
#define TFT_RST    1
#define TFT_DC     2
#define TFT_MOSI   3
#define TFT_SCLK   5
#define TFT_BL     38

// Use HSPI
static SPIClass hspi(HSPI);
static Adafruit_ST7735 tft = Adafruit_ST7735(&hspi, TFT_CS, TFT_DC, TFT_RST);

// State tracking
static DisplayState current_state = DISPLAY_BOOT;
static unsigned long last_update = 0;
static bool flash_state = false;

// Detection info storage
static char det_type[32] = {0};
static char det_name[32] = {0};
static char det_mac[24] = {0};
static int det_threat = 0;

// Cool color scheme
#define COLOR_BG        ST77XX_BLACK
#define COLOR_TITLE_BG  0x0018  // Deep blue-purple
#define COLOR_TITLE_FG  ST77XX_CYAN
#define COLOR_TEXT      ST77XX_WHITE
#define COLOR_SCANNING  ST77XX_CYAN
#define COLOR_ALERT_BG  0xF800  // Bright red
#define COLOR_ALERT_FG  ST77XX_WHITE
#define COLOR_HEARTBEAT ST77XX_MAGENTA
#define COLOR_ACCENT    0x07FF  // Cyan
#define COLOR_WARN      0xFBE0  // Orange-yellow

void display_init() {
    // Turn on backlight first (active LOW on T-Dongle S3)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);  // LOW = ON

    // Initialize SPI for display
    hspi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

    // Initialize ST7735 display with correct variant
    tft.initR(INITR_MINI160x80_PLUGIN);
    tft.setRotation(1);  // Landscape (160x80)
    tft.fillScreen(COLOR_BG);
}

void display_boot_screen() {
    current_state = DISPLAY_BOOT;
    tft.fillScreen(COLOR_BG);

    // Title bar with gradient effect (just solid for now)
    tft.fillRect(0, 0, 160, 16, COLOR_TITLE_BG);
    tft.setTextColor(COLOR_TITLE_FG);
    tft.setTextSize(1);
    tft.setCursor(4, 4);
    tft.print("FLOCK YOU");

    // Version badge
    tft.setCursor(100, 4);
    tft.setTextColor(ST77XX_MAGENTA);
    tft.print("v1.0");

    // Device info
    tft.setTextColor(COLOR_ACCENT);
    tft.setCursor(4, 24);
    tft.print(DEVICE_NAME);

    tft.setCursor(4, 38);
    tft.setTextColor(COLOR_WARN);
    tft.print("Initializing...");

    tft.setCursor(4, 56);
    tft.setTextColor(ST77XX_CYAN);
    tft.print("WiFi + BLE Scanner");

    last_update = millis();
}

void display_scanning(uint8_t wifi_channel, bool ble_active) {
    if (current_state == DISPLAY_DETECTION) {
        return;  // Don't overwrite active detection
    }

    current_state = DISPLAY_SCANNING;

    // Clear main area (keep title)
    tft.fillRect(0, 16, 160, 64, COLOR_BG);

    // Title bar
    tft.fillRect(0, 0, 160, 16, COLOR_TITLE_BG);
    tft.setTextColor(COLOR_TITLE_FG);
    tft.setTextSize(1);
    tft.setCursor(4, 4);
    tft.print("FLOCK YOU");

    // Scanning indicator with cool styling
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.setCursor(4, 20);
    tft.print("SCANNING");

    // Animated dots area (static for now)
    tft.setTextColor(ST77XX_MAGENTA);
    tft.print("...");

    // Status line with icons
    tft.setTextSize(1);
    tft.setCursor(4, 46);
    tft.setTextColor(COLOR_WARN);
    tft.print("WiFi:");
    tft.setTextColor(ST77XX_WHITE);
    tft.print(" Ch ");
    tft.setTextColor(ST77XX_CYAN);
    tft.print(wifi_channel);

    tft.setCursor(80, 46);
    tft.setTextColor(COLOR_WARN);
    tft.print("BLE:");
    tft.setTextColor(ble_active ? ST77XX_GREEN : ST77XX_RED);
    tft.print(ble_active ? " ON" : " --");

    // Bottom status
    tft.setCursor(4, 64);
    tft.setTextColor(0x4208);  // Dark gray
    tft.print("Hunting Flock cameras...");

    last_update = millis();
}

void display_detection(const char* type, const char* name, const char* mac, int threat_score) {
    current_state = DISPLAY_DETECTION;
    flash_state = true;

    // Store detection info
    strncpy(det_type, type, sizeof(det_type) - 1);
    strncpy(det_name, name, sizeof(det_name) - 1);
    strncpy(det_mac, mac, sizeof(det_mac) - 1);
    det_threat = threat_score;

    // Alert screen
    tft.fillScreen(COLOR_ALERT_BG);

    // Title bar (red alert)
    tft.setTextColor(COLOR_ALERT_FG);
    tft.setTextSize(1);
    tft.setCursor(4, 2);
    tft.print("!! DETECTED !!");

    // Detection type
    tft.setCursor(4, 16);
    tft.print(type);

    // Device name (truncate if needed)
    tft.setCursor(4, 30);
    if (strlen(name) > 20) {
        char truncated[21];
        strncpy(truncated, name, 20);
        truncated[20] = '\0';
        tft.print(truncated);
    } else {
        tft.print(name);
    }

    // MAC address
    tft.setCursor(4, 44);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print(mac);

    // Threat score
    tft.setCursor(4, 62);
    tft.setTextColor(COLOR_ALERT_FG);
    tft.print("Threat: ");
    tft.print(threat_score);
    tft.print("%");

    last_update = millis();
}

void display_heartbeat() {
    if (current_state != DISPLAY_DETECTION) {
        return;
    }

    // Flash the title to indicate device still in range
    flash_state = !flash_state;

    if (flash_state) {
        tft.fillRect(0, 0, 160, 14, COLOR_HEARTBEAT);
        tft.setTextColor(COLOR_BG);
    } else {
        tft.fillRect(0, 0, 160, 14, COLOR_ALERT_BG);
        tft.setTextColor(COLOR_ALERT_FG);
    }

    tft.setTextSize(1);
    tft.setCursor(4, 2);
    tft.print("!! IN RANGE !!");
}

void display_clear_detection() {
    current_state = DISPLAY_SCANNING;
    det_type[0] = '\0';
    det_name[0] = '\0';
    det_mac[0] = '\0';
    det_threat = 0;
}

void display_update() {
    // Periodic update for animations/refresh
    unsigned long now = millis();

    if (current_state == DISPLAY_DETECTION) {
        // Flash effect during detection
        if (now - last_update >= 500) {
            flash_state = !flash_state;
            if (flash_state) {
                tft.fillRect(0, 0, 160, 14, COLOR_ALERT_BG);
            } else {
                tft.fillRect(0, 0, 160, 14, COLOR_HEARTBEAT);
            }
            tft.setTextColor(COLOR_ALERT_FG);
            tft.setTextSize(1);
            tft.setCursor(4, 2);
            tft.print("!! DETECTED !!");
            last_update = now;
        }
    }
}

#endif // BOARD_T_DONGLE_S3
