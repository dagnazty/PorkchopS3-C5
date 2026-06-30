        ////////////////////////////////////////////////////
        // TFT_eSPI driver functions for ESP32-C5         //
        ////////////////////////////////////////////////////

// Based on TFT_eSPI_ESP32_C3.h - ESP32-C5 is also RISC-V single-core
// with the same SPI peripheral differences from original ESP32

#ifndef _TFT_eSPI_ESP32C5H_
#define _TFT_eSPI_ESP32C5H_

#if !defined(DISABLE_ALL_LIBRARY_WARNINGS)
 #warning >>>>------>> DMA is not supported on the ESP32 C5 (possible future update)
#endif

// Processor ID reported by getSetup()
#define PROCESSOR_ID 0x32C5

// Include processor specific header
#include "soc/spi_reg.h"
#include "driver/spi_master.h"
#include "hal/gpio_ll.h"

// ESP32-C5: Only SPI2 available for user (like C3)
// Map VSPI/HSPI/FSPI all to SPI (the Arduino default SPI instance)
#ifndef VSPI
  #define VSPI FSPI
#endif
#ifndef HSPI
  #define HSPI FSPI
#endif

// Fix for missing REG_SPI_BASE on C5 (same fix as C3)
#ifndef REG_SPI_BASE
  #define REG_SPI_BASE(i) DR_REG_SPI2_BASE
#endif

// Fix for renamed SPI_MOSI_DLEN_REG on C5 (same as C3)
#ifndef SPI_MOSI_DLEN_REG
  #define SPI_MOSI_DLEN_REG(x) SPI_MS_DLEN_REG(x)
#endif

// SUPPORT_TRANSACTIONS is mandatory
#if !defined (SUPPORT_TRANSACTIONS)
  #define SUPPORT_TRANSACTIONS
#endif

// ESP32-C5: Only SPI2_HOST available
#define SPI_PORT SPI2_HOST

#ifdef RPI_DISPLAY_TYPE
  #define CMD_BITS (16-1)
#else
  #define CMD_BITS (8-1)
#endif

// Initialise processor specific SPI functions, used by init()
#define INIT_TFT_DATA_BUS // Not used

// Define a generic flag for 8-bit parallel
#if defined (ESP32_PARALLEL)
  #if !defined (TFT_PARALLEL_8_BIT)
    #define TFT_PARALLEL_8_BIT
  #endif
#endif

#if defined (TFT_PARALLEL_8_BIT)
  #if !defined (ESP32_PARALLEL)
    #define ESP32_PARALLEL
  #endif
#endif

// Processor specific code used by SPI bus transaction startWrite and endWrite functions
#if !defined (ESP32_PARALLEL)
  #if (TFT_SPI_MODE == SPI_MODE1) || (TFT_SPI_MODE == SPI_MODE2)
    #define SET_BUS_WRITE_MODE *_spi_user = SPI_USR_MOSI | SPI_CK_OUT_EDGE
    #define SET_BUS_READ_MODE  *_spi_user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN | SPI_CK_OUT_EDGE
  #else
    #define SET_BUS_WRITE_MODE *_spi_user = SPI_USR_MOSI
    #define SET_BUS_READ_MODE  *_spi_user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN
  #endif
#else
    #define SET_BUS_WRITE_MODE
    #define SET_BUS_READ_MODE
#endif

// DMA not supported on C5 yet
#if !defined(TFT_PARALLEL_8_BIT) && !defined(SPI_18BIT_DRIVER)
  #define ESP32_DMA
  #define DMA_BUSY_CHECK  dmaWait()
#else
  #define DMA_BUSY_CHECK
#endif

#if defined(TFT_PARALLEL_8_BIT)
  #define SPI_BUSY_CHECK
#else
  #define SPI_BUSY_CHECK while (*_spi_cmd&SPI_USR)
#endif

// If smooth font is used then SPIFFS may be needed
#ifdef SMOOTH_FONT
  #define FS_NO_GLOBALS
  #include <FS.h>
  #include "SPIFFS.h"
  #define FONT_FS_AVAILABLE
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the DC (TFT Data/Command) pin drive code
// ESP32-C5 GPIO struct uses .val member (same as C3)
////////////////////////////////////////////////////////////////////////////////////////
#ifndef TFT_DC
  #define DC_C
  #define DC_D
#else
  #if defined (TFT_PARALLEL_8_BIT)
    #if (TFT_DC >= 0) && (TFT_DC < 32)
      #define DC_C GPIO.out_w1tc.val = (1 << TFT_DC)
      #define DC_D GPIO.out_w1ts.val = (1 << TFT_DC)
    #elif (TFT_DC >= 32)
      #define DC_C GPIO.out_w1tc.val = (1 << (TFT_DC - 32))
      #define DC_D GPIO.out_w1ts.val = (1 << (TFT_DC - 32))
    #else
      #define DC_C
      #define DC_D
    #endif
  #else
    #if (TFT_DC >= 32)
      #define DC_C GPIO.out_w1tc.val = (1 << (TFT_DC - 32))
      #define DC_D GPIO.out_w1ts.val = (1 << (TFT_DC - 32))
    #elif (TFT_DC >= 0)
      #define DC_C GPIO.out_w1tc.val = (1 << TFT_DC)
      #define DC_D GPIO.out_w1ts.val = (1 << TFT_DC)
    #else
      #define DC_C
      #define DC_D
    #endif
  #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the CS (TFT chip select) pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#ifndef TFT_CS
  #define TFT_CS -1
  #define CS_L
  #define CS_H
#else
  #if defined (TFT_PARALLEL_8_BIT)
    #if TFT_CS >= 32
      #define CS_L GPIO.out_w1tc.val = (1 << (TFT_CS - 32))
      #define CS_H GPIO.out_w1ts.val = (1 << (TFT_CS - 32))
    #elif TFT_CS >= 0
      #define CS_L GPIO.out_w1tc.val = (1 << TFT_CS)
      #define CS_H GPIO.out_w1ts.val = (1 << TFT_CS)
    #else
      #define CS_L
      #define CS_H
    #endif
  #else
    #if (TFT_CS >= 32)
      #define CS_L GPIO.out_w1tc.val = (1 << (TFT_CS - 32)); GPIO.out_w1tc.val = (1 << (TFT_CS - 32))
      #define CS_H GPIO.out_w1ts.val = (1 << (TFT_CS - 32))
    #elif (TFT_CS >= 0)
      #define CS_L GPIO.out_w1tc.val = (1 << TFT_CS); GPIO.out_w1tc.val = (1 << TFT_CS)
      #define CS_H GPIO.out_w1ts.val = (1 << TFT_CS)
    #else
      #define CS_L
      #define CS_H
    #endif
  #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the WR (TFT write strobe) pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#ifdef TFT_WR
  #if (TFT_WR >= 32)
    #define WR_L GPIO.out_w1tc.val = (1 << (TFT_WR - 32))
    #define WR_H GPIO.out_w1ts.val = (1 << (TFT_WR - 32))
  #elif (TFT_WR >= 0)
    #define WR_L GPIO.out_w1tc.val = (1 << TFT_WR)
    #define WR_H GPIO.out_w1ts.val = (1 << TFT_WR)
  #else
    #define WR_L
    #define WR_H
  #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Macros to write commands/pixel colour data to SPI displays
////////////////////////////////////////////////////////////////////////////////////////
#if !defined (TFT_PARALLEL_8_BIT) && !defined (SPI_18BIT_DRIVER) && !defined (RPI_DISPLAY_TYPE)

  // ESP32-C5 (like C3) needs SPI_UPDATE before SPI_USR
  #define TFT_WRITE_BITS(D, B) *_spi_mosi_dlen = B-1;  \
                             *_spi_w = D;              \
                             *_spi_cmd = SPI_UPDATE;   \
                      while (*_spi_cmd & SPI_UPDATE);  \
                             *_spi_cmd = SPI_USR;      \
                      while (*_spi_cmd & SPI_USR);

  // Write 8 bits
  #define tft_Write_8(C)   TFT_WRITE_BITS(C, 8)

  // Write 16 bits with corrected endianness for 16-bit colours
  #define tft_Write_16(C)  TFT_WRITE_BITS((C)<<8 | (C)>>8, 16)

  // Write 16 bits without waiting (C5 needs UPDATE step like C3)
  #define tft_Write_16N(C) *_spi_mosi_dlen = 16-1;    \
                         *_spi_w = ((C)<<8 | (C)>>8); \
                         *_spi_cmd = SPI_UPDATE;      \
                  while (*_spi_cmd & SPI_UPDATE);     \
                         *_spi_cmd = SPI_USR;

  // Write 16 bits swapped
  #define tft_Write_16S(C) TFT_WRITE_BITS(C, 16)

  // Write 32 bits
  #define tft_Write_32(C)  TFT_WRITE_BITS(C, 32)

  // Write two address coordinates
  #define tft_Write_32C(C,D) TFT_WRITE_BITS((uint16_t)((D)<<8 | (D)>>8)<<16 | (uint16_t)((C)<<8 | (C)>>8), 32)

  // Write same value twice
  #define tft_Write_32D(C) TFT_WRITE_BITS((uint16_t)((C)<<8 | (C)>>8)<<16 | (uint16_t)((C)<<8 | (C)>>8), 32)

#elif defined (SPI_18BIT_DRIVER)

  #define tft_Write_8(C)   spi.transfer(C)
  #define tft_Write_16(C)  spi.transfer(((C) & 0xF800)>>8); spi.transfer(((C) & 0x07E0)>>3); spi.transfer(((C) & 0x001F)<<3)
  #define tft_Write_16N(C) tft_Write_16(C)
  #define tft_Write_16S(C) spi.transfer((C) & 0xF8); spi.transfer(((C) & 0xE000)>>11 | ((C) & 0x07)<<5); spi.transfer(((C) & 0x1F00)>>5)
  #define tft_Write_32(C)  spi.write32(C)
  #define tft_Write_32C(C,D) spi.write32((C)<<16 | (D))
  #define tft_Write_32D(C)   spi.write32((C)<<16 | (C))

#endif

#ifndef tft_Write_16N
  #define tft_Write_16N tft_Write_16
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Macros to read from display using SPI
////////////////////////////////////////////////////////////////////////////////////////
#if !defined (TFT_PARALLEL_8_BIT)
  #define tft_Read_8() spi.transfer(0)
#endif

#define DAT8TO32(P) ( (uint32_t)P[0]<<8 | P[1] | P[2]<<24 | P[3]<<16 )

////////////////////////////////////////////////////////////////////////////////////////
// Define the touch screen chip select pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#if !defined TOUCH_CS || (TOUCH_CS < 0)
  #define T_CS_L // No macro allocated so it generates no code
  #define T_CS_H // No macro allocated so it generates no code
#else
  #if (TOUCH_CS >= 32)
    #define T_CS_L GPIO.out_w1tc.val = (1 << (TOUCH_CS - 32))
    #define T_CS_H GPIO.out_w1ts.val = (1 << (TOUCH_CS - 32))
  #elif (TOUCH_CS >= 0)
    #define T_CS_L GPIO.out_w1tc.val = (1 << TOUCH_CS)
    #define T_CS_H GPIO.out_w1ts.val = (1 << TOUCH_CS)
  #else
    #define T_CS_L
    #define T_CS_H
  #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Make sure TFT_MISO is defined if not used to avoid an error message
////////////////////////////////////////////////////////////////////////////////////////
#ifndef TFT_MISO
  #define TFT_MISO -1
#endif

#endif // Header end
