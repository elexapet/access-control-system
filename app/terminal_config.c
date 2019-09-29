/*
 * terminal_config.c
 *
 *      Author: Petr
 */

#include "terminal_config.h"
#include "storage.h"
#include "can/can_term_protocol.h"

#define ACS_ADDR_BIT_MASK ((1 << ACS_ADDR_BITS) - 1)

// door addresses in ACS
uint16_t _ACS_PANEL_A_ADDR = 0x4; // even
uint16_t _ACS_PANEL_B_ADDR = 0x5; // odd

inline uint16_t get_acs_panel_a_addr(void)
{
  return _ACS_PANEL_A_ADDR;
}

inline uint16_t get_acs_panel_b_addr(void)
{
 return _ACS_PANEL_B_ADDR;
}

static bool _load_acs_addrs_from_ext_stor(void)
{
  bool ret_val = true;

  uint16_t first_addr = 0;

  ret_val &= storage_read_word_le(PTR_ACS_PANEL_FIRST_ADDR, &first_addr);

  if (ret_val)
  {
    set_acs_panel_addr(first_addr);
  }

  return ret_val;
}

static bool _save_acs_addrs_from_ext_stor(void)
{
  bool ret_val = true;

  ret_val &= storage_write_word_le(PTR_ACS_PANEL_FIRST_ADDR, _ACS_PANEL_A_ADDR);

  return ret_val;
}

void set_acs_panel_addr(uint16_t first_acs_addr)
{
  first_acs_addr &= ACS_ADDR_BIT_MASK;

  if (first_acs_addr & 0x1) // even address
  {
    _ACS_PANEL_B_ADDR = first_acs_addr;
    _ACS_PANEL_A_ADDR = first_acs_addr - 1;
  }
  else // odd address
  {
    _ACS_PANEL_A_ADDR = first_acs_addr;
    _ACS_PANEL_B_ADDR = first_acs_addr + 1;
  }
}

bool terminal_config_init(void)
{
  bool ret_val = true;

#if (defined(DEVEL_BOARD) && ENABLE_LOCAL_ACS_ADDR_WRITE)
  ret_val &= _save_acs_addrs_from_ext_stor();
#endif

  //Read ACS ID from external storage
  ret_val &= _load_acs_addrs_from_ext_stor();

  return ret_val;
}
