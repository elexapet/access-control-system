/**
 *  @file
 *  @brief Configuration of hardware and software.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#include "terminal_config.h"
#include "storage.h"
#include "watchdog.h"
#include "acs_can_protocol.h"

// Network address mask.
#define ACS_ADDR_BIT_MASK ((1 << ACS_ADDR_BITS) - 1)

// Door addresses in ACS.
uint16_t _READER_A_ADDR = ACS_PNL_FIRST_ADDR;     // even
uint16_t _READER_B_ADDR = ACS_PNL_FIRST_ADDR + 1; // odd


inline uint16_t get_reader_a_addr(void)
{
  return _READER_A_ADDR;
}

inline uint16_t get_reader_b_addr(void)
{
 return _READER_B_ADDR;
}

static bool _load_acs_addrs_from_ext_stor(void)
{
  bool ret_val = true;

  uint16_t first_addr = 0;

  ret_val &= storage_read_word_le(PTR_READER_FIRST_ADDR, &first_addr);

  if (ret_val)
  {
    set_reader_addr(first_addr);
  }

  return ret_val;
}

static bool _save_acs_addrs_to_ext_stor(void)
{
  bool ret_val = true;

  ret_val &= storage_write_word_le(PTR_READER_FIRST_ADDR, _READER_A_ADDR);

  return ret_val;
}

void set_reader_addr(uint16_t acs_addr)
{
  acs_addr &= ACS_ADDR_BIT_MASK;

  if (acs_addr & 0x1) // even address
  {
    _READER_B_ADDR = acs_addr;
    _READER_A_ADDR = acs_addr - 1;
  }
  else // odd address
  {
    _READER_A_ADDR = acs_addr;
    _READER_B_ADDR = acs_addr + 1;
  }
}

static inline uint16_t _setup_addr_from_console(void)
{
  uint16_t acs_addr = 0;

  while (Board_UARTGetChar() != EOF); // flush read buffer
  for (volatile uint32_t i = 0; i < SystemCoreClock/4; ++i)
  {
    WDT_Feed();
  }
  if (Board_UARTGetChar() == 's')
  {
    while (Board_UARTGetChar() != EOF); // flush read buffer
    Board_UARTPutSTR("Panel address setup (enter in bin format - 2 bytes, MSB first):\n");

    // read address
    int addr_byte;
    while ((addr_byte = Board_UARTGetChar()) == EOF) WDT_Feed();
    acs_addr = addr_byte << 8;
    while ((addr_byte = Board_UARTGetChar()) == EOF) WDT_Feed();
    acs_addr = addr_byte;

    if (acs_addr >= ACS_PNL_FIRST_ADDR && acs_addr <= ACS_PNL_LAST_ADDR)
    {
      Board_UARTPutSTR("ok\n");
    }
    else
    {
      acs_addr = 0;
      Board_UARTPutSTR("fail\n");
    }
  }
  return acs_addr;
}

bool terminal_config_init(void)
{
  bool ret_val = true;
  uint16_t acs_addr = 0;

  acs_addr = _setup_addr_from_console();

  if (acs_addr != 0)
  {
    set_reader_addr(acs_addr);
#if (ENABLE_LOCAL_ACS_ADDR_WRITE)
    ret_val &= _save_acs_addrs_to_ext_stor();
#endif
  }
  else
  {
  	//Read ACS ID from external storage
  	ret_val &= _load_acs_addrs_from_ext_stor();
  }

  return ret_val;
}
