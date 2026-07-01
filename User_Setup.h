// ============================================================
//  TFT_eSPI User_Setup.h for NM-CYD-C5
//  INSTALL: Replace file at:
//    Documents\Arduino\libraries\TFT_eSPI\User_Setup.h
// ============================================================

#define USER_SETUP_ID 300

// ST7789 driver — NM-CYD-C5 uses ST7789, NOT ILI9341
#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// SPI pins — NM-CYD-C5 confirmed pin map
#define TFT_MOSI  7
#define TFT_SCLK  6
#define TFT_MISO  2   // Shared MISO for display, touch and SD per official NM-CYD-C5 pinout
#define TFT_CS   23
#define TFT_DC   24
#define TFT_RST  -1
#define TFT_BL   25   // Backlight — active HIGH

// Touch — XPT2046, CS only (MISO=9 handled separately by dedicated SPIClass in sketch)
#define TOUCH_CS  1

// Fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI frequency
#define SPI_FREQUENCY        40000000
#define SPI_READ_FREQUENCY    6000000
#define SPI_TOUCH_FREQUENCY   2500000
