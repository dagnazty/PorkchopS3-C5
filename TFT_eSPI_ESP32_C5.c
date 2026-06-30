        ////////////////////////////////////////////////////
        // TFT_eSPI driver functions for ESP32-C5         //
        ////////////////////////////////////////////////////

// Based on TFT_eSPI_ESP32_C3.c - ESP32-C5 is RISC-V single-core
// with the same SPI peripheral structure as C3

////////////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////////////

// ESP32-C5 only has one user SPI port (SPI2), use the default Arduino SPI instance
#if !defined (TFT_PARALLEL_8_BIT)
  #ifdef USE_HSPI_PORT
    SPIClass spi = SPIClass(HSPI);
  #elif defined(USE_FSPI_PORT)
    SPIClass spi = SPIClass(FSPI);
  #else
    SPIClass& spi = SPI;  // Default SPI instance on C5
  #endif
#endif

#ifdef ESP32_DMA
  spi_device_handle_t dmaHAL;
  #define DMA_CHANNEL 1
  spi_host_device_t spi_host = (spi_host_device_t) DMA_CHANNEL;
#endif

#if !defined (TFT_PARALLEL_8_BIT)
  volatile uint32_t* _spi_cmd       = (volatile uint32_t*)(SPI_CMD_REG(SPI_PORT));
  volatile uint32_t* _spi_user      = (volatile uint32_t*)(SPI_USER_REG(SPI_PORT));
  volatile uint32_t* _spi_mosi_dlen = (volatile uint32_t*)(SPI_MOSI_DLEN_REG(SPI_PORT));
  volatile uint32_t* _spi_w         = (volatile uint32_t*)(SPI_W0_REG(SPI_PORT));
#endif

////////////////////////////////////////////////////////////////////////////////////////
#if defined (TFT_SDA_READ) && !defined (TFT_PARALLEL_8_BIT)
////////////////////////////////////////////////////////////////////////////////////////

void TFT_eSPI::begin_SDA_Read(void)
{
  gpio_set_direction((gpio_num_t)TFT_MOSI, GPIO_MODE_INPUT);
  pinMatrixInAttach(TFT_MOSI, FSPIQ_IN_IDX, false);
  SET_BUS_READ_MODE;
}

void TFT_eSPI::end_SDA_Read(void)
{
  gpio_set_direction((gpio_num_t)TFT_MOSI, GPIO_MODE_OUTPUT);
  pinMatrixOutAttach(TFT_MOSI, FSPID_OUT_IDX, false, false);
  SET_BUS_WRITE_MODE;
}

////////////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////////////


/***************************************************************************************
** Function name:           readByte
** Description:             Read a byte from ESP32-C5 8-bit data port
***************************************************************************************/
uint8_t TFT_eSPI::readByte(void)
{
  uint8_t b = 0xAA;

#if defined (TFT_PARALLEL_8_BIT)
  RD_L;
  uint32_t reg;
  reg = gpio_input_get();
  reg = gpio_input_get();
  reg = gpio_input_get();
  RD_H;

  b  = (((reg>>TFT_D0)&1) << 0);
  b |= (((reg>>TFT_D1)&1) << 1);
  b |= (((reg>>TFT_D2)&1) << 2);
  b |= (((reg>>TFT_D3)&1) << 3);
  b |= (((reg>>TFT_D4)&1) << 4);
  b |= (((reg>>TFT_D5)&1) << 5);
  b |= (((reg>>TFT_D6)&1) << 6);
  b |= (((reg>>TFT_D7)&1) << 7);
#endif

  return b;
}

////////////////////////////////////////////////////////////////////////////////////////
#ifdef TFT_PARALLEL_8_BIT
////////////////////////////////////////////////////////////////////////////////////////

void TFT_eSPI::busDir(uint32_t mask, uint8_t mode)
{
  pinMode(TFT_D0, mode);
  pinMode(TFT_D1, mode);
  pinMode(TFT_D2, mode);
  pinMode(TFT_D3, mode);
  pinMode(TFT_D4, mode);
  pinMode(TFT_D5, mode);
  pinMode(TFT_D6, mode);
  pinMode(TFT_D7, mode);
}

void TFT_eSPI::gpioMode(uint8_t gpio, uint8_t mode)
{
  pinMode(gpio, mode);
  digitalWrite(gpio, HIGH);
}

////////////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
#if !defined (SPI_18BIT_DRIVER) && !defined (TFT_PARALLEL_8_BIT)
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len)
{
  uint32_t color32 = (color<<8 | color>>8)<<16 | (color<<8 | color>>8);

  if (len > 1)
  {
    *_spi_mosi_dlen = 32-1;
    *_spi_w = color32;
    *_spi_cmd = SPI_UPDATE;
    while (*_spi_cmd & SPI_UPDATE);
    *_spi_cmd = SPI_USR;
    len -= 2;

    while(len)
    {
      while (*_spi_cmd & SPI_USR);
      *_spi_cmd = SPI_UPDATE;
      while (*_spi_cmd & SPI_UPDATE);
      *_spi_cmd = SPI_USR;
      len--;
    }
    while (*_spi_cmd & SPI_USR);
  }
  else if (len)
  {
    *_spi_mosi_dlen = 16-1;
    *_spi_w = (color<<8 | color>>8);
    *_spi_cmd = SPI_UPDATE;
    while (*_spi_cmd & SPI_UPDATE);
    *_spi_cmd = SPI_USR;
    while (*_spi_cmd & SPI_USR);
  }
}

/***************************************************************************************
** Function name:           pushSwapBytePixels
** Description:             Write a sequence of pixels with swapped bytes
***************************************************************************************/
void TFT_eSPI::pushSwapBytePixels(const void* data_in, uint32_t len)
{
  uint16_t *data = (uint16_t*)data_in;

  while (len--)
  {
    uint16_t color = *data++;
    *_spi_mosi_dlen = 16-1;
    *_spi_w = color;
    *_spi_cmd = SPI_UPDATE;
    while (*_spi_cmd & SPI_UPDATE);
    *_spi_cmd = SPI_USR;
    while (*_spi_cmd & SPI_USR);
  }
}

/***************************************************************************************
** Function name:           pushPixels
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len)
{
  if (_swapBytes) {
    pushSwapBytePixels(data_in, len);
    return;
  }

  uint16_t *data = (uint16_t*)data_in;

  while (len--)
  {
    uint16_t color = *data++;
    tft_Write_16(color);
  }
}

////////////////////////////////////////////////////////////////////////////////////////
#elif defined (SPI_18BIT_DRIVER)
////////////////////////////////////////////////////////////////////////////////////////

void TFT_eSPI::pushBlock(uint16_t color, uint32_t len)
{
  while (len--) { tft_Write_16(color); }
}

void TFT_eSPI::pushSwapBytePixels(const void* data_in, uint32_t len)
{
  uint16_t *data = (uint16_t*)data_in;
  while (len--) { uint16_t color = *data++; tft_Write_16(color>>8 | color<<8); }
}

void TFT_eSPI::pushPixels(const void* data_in, uint32_t len)
{
  uint16_t *data = (uint16_t*)data_in;
  if (_swapBytes) { while (len--) { uint16_t color = *data++; tft_Write_16(color>>8|color<<8); } }
  else { while (len--) { tft_Write_16(*data++); } }
}

////////////////////////////////////////////////////////////////////////////////////////
#else // TFT_PARALLEL_8_BIT
////////////////////////////////////////////////////////////////////////////////////////

void TFT_eSPI::pushBlock(uint16_t color, uint32_t len)
{
  while (len--) { tft_Write_16(color); }
}

void TFT_eSPI::pushSwapBytePixels(const void* data_in, uint32_t len)
{
  uint16_t *data = (uint16_t*)data_in;
  while (len--) { uint16_t color = *data++; tft_Write_16(color>>8|color<<8); }
}

void TFT_eSPI::pushPixels(const void* data_in, uint32_t len)
{
  uint16_t *data = (uint16_t*)data_in;
  if (_swapBytes) { while (len--) { uint16_t color = *data++; tft_Write_16(color>>8|color<<8); } }
  else { while (len--) { tft_Write_16(*data++); } }
}

////////////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
// DMA FUNCTIONS - stub implementations (DMA not supported on C5 yet)
////////////////////////////////////////////////////////////////////////////////////////

#ifdef ESP32_DMA

/***************************************************************************************
** Function name:           dc_callback
** Description:             Toggles DC line during DMA transaction
***************************************************************************************/
extern "C" void dc_callback();

void IRAM_ATTR dc_callback(spi_transaction_t *spi_tx)
{
  if ((bool)spi_tx->user) {DC_D;}
  else {DC_C;}
}

/***************************************************************************************
** Function name:           dmaWait
** Description:             Wait for DMA to complete
***************************************************************************************/
void TFT_eSPI::dmaWait(void)
{
  if (!DMA_Enabled) return;
  spi_transaction_t *rtrans;
  esp_err_t ret;
  for (int i = 0; i < spiBusyCheck; ++i) {
    ret = spi_device_get_trans_result(dmaHAL, &rtrans, portMAX_DELAY);
    assert(ret == ESP_OK);
  }
  spiBusyCheck = 0;
}

/***************************************************************************************
** Function name:           dmaBusy
** Description:             Check if DMA is in progress
***************************************************************************************/
bool TFT_eSPI::dmaBusy(void)
{
  if (!DMA_Enabled || !spiBusyCheck) return false;
  spi_transaction_t *rtrans;
  esp_err_t ret = spi_device_get_trans_result(dmaHAL, &rtrans, 0);
  if (ret == ESP_OK) { spiBusyCheck--; return false; }
  return true;
}

/***************************************************************************************
** Function name:           pushPixelsDMA
** Description:             Push pixels to TFT via DMA
***************************************************************************************/
void TFT_eSPI::pushPixelsDMA(uint16_t* image, uint32_t len)
{
  if ((len == 0) || (!DMA_Enabled)) return;
  dmaWait();
  if (_swapBytes) {
    for (uint32_t i = 0; i < len; i++) image[i] = image[i] << 8 | image[i] >> 8;
  }
  esp_err_t ret;
  static spi_transaction_t trans;
  memset(&trans, 0, sizeof(spi_transaction_t));
  trans.user = (void *)1;
  trans.tx_buffer = image;
  trans.length = len * 16;
  trans.flags = 0;
  ret = spi_device_queue_trans(dmaHAL, &trans, portMAX_DELAY);
  assert(ret == ESP_OK);
  spiBusyCheck++;
}

/***************************************************************************************
** Function name:           pushImageDMA
** Description:             Push const image to window via DMA
***************************************************************************************/
void TFT_eSPI::pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t const* image)
{
  if ((w == 0) || (h == 0) || (!DMA_Enabled)) return;
  uint32_t len = w * h;
  dmaWait();
  setAddrWindow(x, y, w, h);
  esp_err_t ret;
  static spi_transaction_t trans;
  memset(&trans, 0, sizeof(spi_transaction_t));
  trans.user = (void *)1;
  trans.tx_buffer = image;
  trans.length = len * 16;
  trans.flags = 0;
  ret = spi_device_queue_trans(dmaHAL, &trans, portMAX_DELAY);
  assert(ret == ESP_OK);
  spiBusyCheck++;
}

/***************************************************************************************
** Function name:           pushImageDMA
** Description:             Push image to window via DMA with optional clipping/swap
***************************************************************************************/
void TFT_eSPI::pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* image, uint16_t* buffer)
{
  if ((x >= _vpW) || (y >= _vpH) || (!DMA_Enabled)) return;

  int32_t dx = 0, dy = 0, dw = w, dh = h;

  if (x < _vpX) { dx = _vpX - x; dw -= dx; x = _vpX; }
  if (y < _vpY) { dy = _vpY - y; dh -= dy; y = _vpY; }
  if ((x + dw) > _vpW) dw = _vpW - x;
  if ((y + dh) > _vpH) dh = _vpH - y;
  if (dw < 1 || dh < 1) return;

  uint32_t len = dw * dh;

  if (buffer == nullptr) { buffer = image; dmaWait(); }

  if ((dw != w) || (dh != h)) {
    if (_swapBytes) {
      for (int32_t yb = 0; yb < dh; yb++)
        for (int32_t xb = 0; xb < dw; xb++) {
          uint32_t src = xb + dx + w * (yb + dy);
          buffer[xb + yb * dw] = image[src] << 8 | image[src] >> 8;
        }
    } else {
      for (int32_t yb = 0; yb < dh; yb++)
        memcpy((uint8_t*)(buffer + yb * dw), (uint8_t*)(image + dx + w * (yb + dy)), dw << 1);
    }
  } else if (buffer != image || _swapBytes) {
    if (_swapBytes) {
      for (uint32_t i = 0; i < len; i++) buffer[i] = image[i] << 8 | image[i] >> 8;
    } else {
      memcpy(buffer, image, len * 2);
    }
  }

  if (spiBusyCheck) dmaWait();

  setAddrWindow(x, y, dw, dh);

  esp_err_t ret;
  static spi_transaction_t trans;
  memset(&trans, 0, sizeof(spi_transaction_t));
  trans.user = (void *)1;
  trans.tx_buffer = buffer;
  trans.length = len * 16;
  trans.flags = 0;
  ret = spi_device_queue_trans(dmaHAL, &trans, portMAX_DELAY);
  assert(ret == ESP_OK);
  spiBusyCheck++;
}

/***************************************************************************************
** Function name:           initDMA
** Description:             Initialise the DMA engine
***************************************************************************************/
bool TFT_eSPI::initDMA(bool ctrl_cs)
{
  if (DMA_Enabled) return false;

  esp_err_t ret;
  spi_bus_config_t buscfg = {
    .mosi_io_num = TFT_MOSI,
    .miso_io_num = TFT_MISO,
    .sclk_io_num = TFT_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .data4_io_num = -1,
    .data5_io_num = -1,
    .data6_io_num = -1,
    .data7_io_num = -1,
    .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2 + 8,
    .flags = 0,
    .intr_flags = 0
  };

  int8_t pin = -1;
  if (ctrl_cs) pin = TFT_CS;

  spi_device_interface_config_t devcfg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = TFT_SPI_MODE,
    .duty_cycle_pos = 0,
    .cs_ena_pretrans = 0,
    .cs_ena_posttrans = 0,
    .clock_speed_hz = SPI_FREQUENCY,
    .input_delay_ns = 0,
    .spics_io_num = pin,
    .flags = SPI_DEVICE_NO_DUMMY,
    .queue_size = 1,
    .pre_cb = 0,
    .post_cb = 0
  };

  ret = spi_bus_initialize(spi_host, &buscfg, DMA_CHANNEL);
  ESP_ERROR_CHECK(ret);
  ret = spi_bus_add_device(spi_host, &devcfg, &dmaHAL);
  ESP_ERROR_CHECK(ret);

  DMA_Enabled = true;
  spiBusyCheck = 0;
  return true;
}

/***************************************************************************************
** Function name:           deInitDMA
** Description:             Disconnect the DMA engine from SPI
***************************************************************************************/
void TFT_eSPI::deInitDMA(void)
{
  if (!DMA_Enabled) return;
  spi_bus_remove_device(dmaHAL);
  spi_bus_free(spi_host);
  DMA_Enabled = false;
}

////////////////////////////////////////////////////////////////////////////////////////
#endif // ESP32_DMA
////////////////////////////////////////////////////////////////////////////////////////
