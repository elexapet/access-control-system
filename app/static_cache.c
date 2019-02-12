/* Similar to Set Associative Cache.
*  Divide into sets. Each set is one line a sorted array.
*
*/

#include <static_cache.h>

static term_cache_item_t _cache_set_0[STATIC_CACHE_CAPACITY / STATIC_CACHE_SETS];
static term_cache_item_t _cache_set_1[STATIC_CACHE_CAPACITY / STATIC_CACHE_SETS];
static term_cache_item_t _cache_set_2[STATIC_CACHE_CAPACITY / STATIC_CACHE_SETS];
static term_cache_item_t _cache_set_3[STATIC_CACHE_CAPACITY / STATIC_CACHE_SETS];
static term_cache_item_t * _cache_sets[STATIC_CACHE_SETS] = {_cache_set_0, _cache_set_1, _cache_set_2, _cache_set_3};

// O(1)
static inline term_cache_item_t * _get_cache_set(uint32_t key)
{
  return _cache_sets[(key & 0xC00000) >> 22];
}

// O(log (STATIC_CACHE_CAPACITY / STATIC_CACHE_SETS))
term_cache_item_t static_cache_get(uint32_t key)
{

}

// O(log (STATIC_CACHE_CAPACITY / 4) + (STATIC_CACHE_CAPACITY / 4))
void static_cache_insert(uint32_t key)
{

}

// O(log (STATIC_CACHE_CAPACITY / 4) + (STATIC_CACHE_CAPACITY / 4))
void static_cache_erase(uint32_t key)
{

}
