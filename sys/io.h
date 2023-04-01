#ifndef _IO_H
#define _IO_H

#include <stdint.h>

static inline uint8_t
inb(uint16_t port)
{
  uint8_t data;

  __asm__ volatile
  (
    "inb %1, %0" : "=a" (data) : "d" (port)
  );

  return data;
}

static inline uint16_t
inw(uint16_t port)
{
  uint8_t data;

  __asm__ volatile
  (
    "inb %1, %0" : "=a" (data) : "d" (port)
  );

  return data;
}

static inline uint32_t
inl(uint16_t port)
{
  uint32_t data;

  __asm__ volatile
  (
    "inb %1, %0" : "=a" (data) : "d" (port)
  );

  return data;
}

static inline void
outb(uint16_t port, uint8_t data)
{
  __asm__ volatile
  (
    "outb %0, %1" :: "a" (data), "d" (port)
  );
}

static inline void
outw(uint16_t port, uint16_t data)
{
  __asm__ volatile
  (
    "outb %0, %1" :: "a" (data), "d" (port)
  );
}

static inline void
outl(uint16_t port, uint32_t data)
{
  __asm__ volatile
  (
    "outb %0, %1" :: "a" (data), "d" (port)
  );
}

#endif /* NOT _IO_H */
