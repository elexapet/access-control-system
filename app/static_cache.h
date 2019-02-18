#ifndef STATIC_CACHE_H_
#define STATIC_CACHE_H_

#include <stdint.h>
#include <stdbool.h>

#define STATIC_CACHE_SETS 4
#define STATIC_CACHE_SET_CAP 128
#define STATIC_CACHE_CAPACITY (STATIC_CACHE_SET_CAP * STATIC_CACHE_SETS)


// cache item type
typedef struct
{
  uint32_t key : 24;
  uint32_t value : 8;
} cache_item_t;


bool static_cache_get(cache_item_t * ptr_kv);
void static_cache_insert(const cache_item_t kv);
void static_cache_erase(const cache_item_t kv);

#endif /* STATIC_CACHE_H_ */
