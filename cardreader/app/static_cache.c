/**
 *  @file
 *  @brief Statically allocated cache for user IDs.
 *
 *  It is similar to Set Associative Cache.
 *  Place key and value into sets by key hash. Each set is a sorted array.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#include "static_cache.h"
#include <string.h>

#if CACHING_ENABLED

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

typedef struct
{
  cache_item_t * const ptr_items;
  int length;
} cache_set_t;

// Allocate cache in memory.
static cache_item_t _cache_set_0[STATIC_CACHE_SET_CAP];
static cache_item_t _cache_set_1[STATIC_CACHE_SET_CAP];
static cache_item_t _cache_set_2[STATIC_CACHE_SET_CAP];
static cache_item_t _cache_set_3[STATIC_CACHE_SET_CAP];

static cache_set_t _cache_sets[STATIC_CACHE_SETS]
                                       = {
                                           {_cache_set_0, 0},
                                           {_cache_set_1, 0},
                                           {_cache_set_2, 0},
                                           {_cache_set_3, 0}
                                       };

/*****************************************************************************
 * Private functions
 ****************************************************************************/

// Standard implementation of binary search.
static bool _binary_search(const cache_set_t * ptr_set, const cache_item_t kv, int * ptr_idx)
{
  int down = 0;
  int top = ptr_set->length - 1;
  int mid;

  while (down <= top)
  {
    mid = (down + top) / 2;
    if (ptr_set->ptr_items[mid].key > kv.key) top = mid - 1;
    else if (ptr_set->ptr_items[mid].key < kv.key) down = mid + 1;
    else
    {
      *ptr_idx = mid;
      return true; // Found.
    }
  }
  *ptr_idx = down;
  return false; // Not found.
}

// Retrieve cache set the key should be in. Decide from first two bits.
// O(1)
static inline cache_set_t * _get_cache_set(const cache_item_t kv)
{
  return &_cache_sets[(kv.key & 0x3)];
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

bool static_cache_get(cache_item_t * ptr_kv)
{
  const cache_set_t * ptr_set = _get_cache_set(*ptr_kv);

  int idx = 0;
  if (_binary_search(ptr_set, *ptr_kv, &idx))
  {
    // Found.
    *ptr_kv = ptr_set->ptr_items[idx];
    return true;
  }
  else
  {
    // Not found.
    return false;
  }
}

void static_cache_insert(const cache_item_t kv)
{
  cache_set_t * ptr_set = _get_cache_set(kv);

  int idx = 0;
  if (_binary_search(ptr_set, kv, &idx))
  {
    ptr_set->ptr_items[idx] = kv; // Update existing item.
  }
  else
  {
    if ( 1 + ptr_set->length >= STATIC_CACHE_SET_CAP)
    {
      // Cache set full - replace existing.
      ptr_set->ptr_items[idx] = kv;
    }
    else
    {
      // Move items and place new one.
      for (int i = ptr_set->length - 1; i >= idx; --i)
      {
        ptr_set->ptr_items[i + 1] = ptr_set->ptr_items[i];
      }
      ptr_set->ptr_items[idx] = kv;
      ++ptr_set->length;
    }
  }
}

void static_cache_erase(const cache_item_t kv)
{
  cache_set_t * ptr_set = _get_cache_set(kv);

  int idx;
  if (_binary_search(ptr_set, kv, &idx))
  {
    // Key was found - delete and fill the empty position.
    for (int i = idx; i < ptr_set->length - 1; ++i)
    {
      ptr_set->ptr_items[i] = ptr_set->ptr_items[i + 1];
    }
    --ptr_set->length;
  }
}

void static_cache_reset(void)
{
  for (int i = 0; i < STATIC_CACHE_SETS; ++i)
  {
    memset(_cache_sets[i].ptr_items, 0, STATIC_CACHE_SET_CAP);
  }
}

cache_item_t static_cache_convert(uint32_t key, uint32_t value)
{
  cache_item_t kv = {.key = key, .value = value};
  return kv;
}

#endif
