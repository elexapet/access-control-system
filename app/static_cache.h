/**
 *  @file
 *  @brief Statically allocated cache for user IDs.
 *
 *  It is similar to Set Associative Cache.
 *  Divide key and value into sets by key hash. Each set is a sorted array.
 *
 *  The key in cache can be up to 30 bits in size.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef STATIC_CACHE_H_
#define STATIC_CACHE_H_

#include <stdint.h>
#include <stdbool.h>
#include "terminal_config.h"

/** Configuration of the static cache. */
#define STATIC_CACHE_SETS     4
#define STATIC_CACHE_SET_CAP  128
#define STATIC_CACHE_CAPACITY (STATIC_CACHE_SET_CAP * STATIC_CACHE_SETS)


/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

#pragma pack(push,1)

typedef union
{
  struct
  {
    uint32_t value : 2;
    uint32_t key : 30;
  };
  uint32_t scalar;
} cache_item_t;  ///< Generic type of the item used in cache.

#pragma pack(pop)


/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
* @brief Retrieve item from the cache.
*
*        Complexity is O(log(STATIC_CACHE_SET_CAP)).
*
* @param ptr_kv ... Pointer to cache_item_t containing key. The value will be filled in
*                   if key is found.
*
* @return true if key is found.
*/
bool static_cache_get(cache_item_t * ptr_kv);

/**
* @brief Insert item to the cache.
*
*        Will overwrite items already in cache if the cache is full. Will also update item
*        with same key.
*
*        Complexity is O(log(STATIC_CACHE_SET_CAP) + 2*(STATIC_CACHE_SET_CAP)).
*
* @param kv ... Key and value to be inserted.
*/
void static_cache_insert(const cache_item_t kv);

/**
* @brief Erase item from the cache.
*
*        Complexity is O(log(STATIC_CACHE_SET_CAP) + 2*(STATIC_CACHE_SET_CAP)).
*
* @param kv ... Key containing key to be erased. The item will erased
*                   if key is found.
*/
void static_cache_erase(const cache_item_t kv);

/**
* @brief Clear all data in the cache.
*
*        Resets all data in cache to 0s.
*        Complexity is O(STATIC_CACHE_CAPACITY).
*
*/
void static_cache_reset(void);

/**
* @brief Create cache item from key and value parameters.
*
*
* @param key ... Key for the value.
* @param value ... Value for key to use.
*
* @return Cache item structure.
*/
cache_item_t static_cache_convert(uint32_t key, uint32_t value);


#endif /* STATIC_CACHE_H_ */
