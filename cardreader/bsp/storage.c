/**
 *  @file
 *  @brief Storage implementation.
 *
 *         Supports I2C EEPROMs and I/O expanders.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#include "storage.h"
#include "FreeRTOS.h"
#include "task.h"

static inline void _init_I2C_pins(void)
{
#ifdef BOARD_NXP_XPRESSO_11C24
  Chip_SYSCTL_DeassertPeriphReset(RESET_I2C0);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[0][4], IOCON_SFI2C_EN, IOCON_FUNC1);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[0][5], IOCON_SFI2C_EN, IOCON_FUNC1);
#else
  #error "Unsupported board for I2C storage."
#endif
}

static inline void _wait_for_dev_ready(void)
{
  uint8_t tmp;
  for (int i = 0; i < STORE_DEV_BUSY_FOR; ++i)
  {
    if (Chip_I2C_MasterRead(STORE_I2C_DEV, STORE_I2C_SLAVE_ADDR, &tmp, 0))
    {
      break;
    }
  }
}

void storage_init(void)
{
  uint32_t freq = STORE_I2C_BUS_FREQ;

  configASSERT(freq > 1000 && freq <= 400000);

  _init_I2C_pins();

  /* Initialize I2C */
  Chip_I2C_Init(STORE_I2C_DEV);
  Chip_I2C_SetClockRate(STORE_I2C_DEV, freq);

  /* Set mode to interrupt */
  Chip_I2C_SetMasterEventHandler(STORE_I2C_DEV, Chip_I2C_EventHandler);
  NVIC_EnableIRQ(I2C0_IRQn);
}

bool storage_read_word_le(const uint8_t addr, uint16_t * data)
{
  const uint8_t len = sizeof(*data);

  uint8_t n_got = Chip_I2C_MasterCmdRead(STORE_I2C_DEV, STORE_I2C_SLAVE_ADDR, addr, (uint8_t *)data, len);

  return (n_got == len);
}

bool storage_write_word_le(const uint8_t addr, const uint16_t data)
{
  // in order of sending
  const uint8_t tx_buff[] = {addr, (uint8_t)data, (uint8_t)(data >> 8)};

  uint8_t n_sent = Chip_I2C_MasterSend(STORE_I2C_DEV, STORE_I2C_SLAVE_ADDR, tx_buff, ARRAY_SIZE(tx_buff));

  _wait_for_dev_ready();

  return (n_sent == ARRAY_SIZE(tx_buff));
}

bool storage_read_byte(const uint8_t addr, uint8_t * data)
{
  const uint8_t len = sizeof(*data);

  uint8_t n_got = Chip_I2C_MasterCmdRead(STORE_I2C_DEV, STORE_I2C_SLAVE_ADDR, addr, data, len);

  return (n_got == len);
}

bool storage_write_byte(const uint8_t addr, const uint8_t data)
{
  const uint8_t tx_buff[] = {addr, data};

  uint8_t n_sent = Chip_I2C_MasterSend(STORE_I2C_DEV, STORE_I2C_SLAVE_ADDR, tx_buff, ARRAY_SIZE(tx_buff));

  _wait_for_dev_ready();

  return (n_sent == ARRAY_SIZE(tx_buff));
}

/**
 * @brief I2C Interrupt Handler
 * @return  None
 */
void I2C_IRQHandler(void)
{
  if (Chip_I2C_IsMasterActive(STORE_I2C_DEV))
  {
    Chip_I2C_MasterStateHandler(STORE_I2C_DEV);
  }
  else
  {
    Chip_I2C_SlaveStateHandler(STORE_I2C_DEV);
  }
}
