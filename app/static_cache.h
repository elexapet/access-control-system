#ifndef STATIC_CACHE_H_
#define STATIC_CACHE_H_

#include <stdint.h>

#define STATIC_CACHE_SETS 4
#define STATIC_CACHE_CAPACITY (128 * STATIC_CACHE_SETS)

typedef struct
{
  uint32_t user_id : 24;
  uint32_t is_valid : 1;
  uint32_t location : 2;
  uint32_t : 5; // reserved for future
} term_cache_item_t; // 4B


#endif /* STATIC_CACHE_H_ */
