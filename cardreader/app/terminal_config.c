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
uint16_t _READER_A_ADDR = 0x4; // even
uint16_t _READER_B_ADDR = 0x5; // odd


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

bool terminal_config_init(void)
{
  bool ret_val = true;
  uint16_t acs_addr = 0;

#if (ENABLE_LOCAL_ACS_ADDR_WRITE)
  Board_UARTGetChar();
	for (volatile uint32_t i = 0; i < 2*SystemCoreClock; ++i)
	{
			WDT_Feed();
	}
  if (Board_UARTGetChar() == 's')
  {
    Board_UARTPutSTR("Panel address setup\n enter in bin format (2 bytes, MSB first)\n");
    acs_addr = Board_UARTGetChar() << 8;
    acs_addr = Board_UARTGetChar();
    if (acs_addr > 0 && acs_addr <= ACS_ADDR_BIT_MASK)
    {
      Board_UARTPutSTR("ok");
    }
	  else
    {
      Board_UARTPutSTR("fail");
    }
  }
#endif

  if (acs_addr != 0)
  {
    set_reader_addr(acs_addr);
		ret_val &= _save_acs_addrs_to_ext_stor();
  }
  else
  {
  	//Read ACS ID from external storage
  	ret_val &= _load_acs_addrs_from_ext_stor();
  }

  return ret_val;
}
