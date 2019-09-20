/*
 * storage.c
 *
 *  Created on: 20. 9. 2019
 *      Author: Petr
 */
#include "storage.h"

static void Init_I2C_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
  Chip_SYSCTL_PeriphReset(RESET_I2C0);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
#elif (defined(BOARD_NXP_XPRESSO_11C24) || defined(BOARD_MCORE48_1125))
  Chip_SYSCTL_PeriphReset(RESET_I2C0);
  Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
  Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
#else
  #error "Unsupported board for I2C operation."
#endif
}

void storage_init(I2C_ID_T dev_id, uint32_t freq)
{
  configASSERT(freq > 1000);
  configASSERT(freq <= 400000);

  Init_I2C_PinMux();

  /* Initialize I2C */
  Chip_I2C_Init(dev_id);
  Chip_I2C_SetClockRate(dev_id, freq);

  /* Set mode to interrupt */
  Chip_I2C_SetMasterEventHandler(dev_id, Chip_I2C_EventHandler);
  NVIC_EnableIRQ(I2C0_IRQn);
}

bool storage_read_word_le(const uint8_t addr, uint16_t * data)
{
  const uint8_t len = sizeof(*data);

  uint8_t n_got = Chip_I2C_MasterCmdRead(STORE_I2C_DEV, STORE_I2C_SLAVE_ADDR, addr, data, len);

  return (n_got == len);
}

bool storage_write_word_le(const uint8_t addr, const uint16_t data)
{
  const uint8_t tx_buff[] = {addr, (uint8_t)data, (uint8_t)(data >> 8)};

  uint8_t n_sent = Chip_I2C_MasterSend(STORE_I2C_DEV, STORE_I2C_SLAVE_ADDR, tx_buff, ARRAY_SIZE(tx_buff));

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
