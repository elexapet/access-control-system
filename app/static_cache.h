#ifndef STATIC_CACHE_H_
#define STATIC_CACHE_H_

#include <stdint.h>
#include <stdbool.h>
#include "terminal_config.h"

#define STATIC_CACHE_SETS 4
#define STATIC_CACHE_SET_CAP 128
#define STATIC_CACHE_CAPACITY (STATIC_CACHE_SET_CAP * STATIC_CACHE_SETS)


// cache item type
#pragma pack(push,1)
typedef union
{
  struct
  {
    uint32_t value : 2;
    uint32_t key : 30;
  };
  uint32_t scalar;
} cache_item_t;
#pragma pack(pop)


bool static_cache_get(cache_item_t * ptr_kv);
void static_cache_insert(const cache_item_t kv);
void static_cache_erase(const cache_item_t kv);
void static_cache_reset(void);
cache_item_t static_cache_convert(uint32_t key, uint32_t value);

#endif /* STATIC_CACHE_H_ */
