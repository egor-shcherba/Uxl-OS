#ifndef _STDINT_H
#define _STDINT_H

typedef char                int8_t;
typedef short               int16_t;
typedef long                int32_t;
typedef long long           int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned long long  uint64_t;

_Static_assert(sizeof(uint8_t)  == 1, "uin8_t must be 1 byte on target platform.");
_Static_assert(sizeof(uint16_t) == 2, "uin16_t must be 2 byte on target platform.");
_Static_assert(sizeof(uint32_t) == 4, "uin32_t must be 4 byte on target platform.");
_Static_assert(sizeof(uint64_t) == 8, "uin64_t must be 8 byte on target platform.");

#endif /* NOT _STDINT_H */