#ifndef _BITMAP_H
#define _BITMAP_H

#include <stdint.h>

static inline int32_t
bitmap_get_free(uint8_t *map, uint32_t size)
{
  uint32_t bit = 0;

  for (uint32_t i = 0; i < size; i++)
    {
      bit = map[i / 8] & (1 << (i % 8));

      if (bit == 0)
        return i;
    }

  return -1;
}

static inline void
bitmap_set(uint8_t *map, uint32_t bit)
{
  map[bit / 8] |= 1 << (bit % 8);
}

static inline void
bitmap_unset(uint8_t *map, uint32_t bit)
{
  map[bit / 8] &= ~(1 << (bit % 8));
}

#endif /* NOT _BITMAP_H */