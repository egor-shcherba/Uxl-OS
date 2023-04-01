#ifndef _QEMU_H
#define _QEMU_H

#include <debug/colors.h>
#include <debug/config.h>
#include <stddef.h>

void dbg_printf(const char *fmt, ...);

static inline void
__dbg_unuse(const char *fmt, ...)
{
  (void) fmt;
}

#ifndef __DEBUG_HEADER_COLOR__
#define __DEBUG_HEADER_COLOR__  __DEFAULT_HEADER_COLOR__
#endif /* NOT _DEBUG_HEADER_COLOR */

#ifndef __DEBUG_FONT_COLOR__
#define __DEBUG_FONT_COLOR__  __DEFAULT_FONT_COLOR__
#endif  /* NOT _DEBUG_FONT_COLOR */

#ifndef __DEBUG_SUB_HEADER_COLOR__
#define __DEBUG_SUB_HEADER_COLOR__ __DEFAULT_SUB_HEADER_COLOR__
#endif /* NOT _DEBUG_SUBSYSTEM_COLOR__*/

#define __print_header(HEADER)                                                \
  dbg_printf("[%s %s %s] :: ",                                                \
    __DEBUG_HEADER_COLOR__, HEADER, __FG_RESET__);

#define color_printf(COLOR, ...)                                              \
  dbg_printf(COLOR);                                                          \
  dbg_printf(__VA_ARGS__);                                                    \
  dbg_printf(__FG_RESET__);

#define pr_err(...)                                                           \
  color_printf(__FG_RED__, __VA_ARGS__);

#ifndef __DEBUG_CURRENT_LOG__
#define __DEBUG_CURRENT_LOG__ __LOG_ALL__
#endif /* NOT __CURRENT_LOG__ */

#ifndef __DEBUG_CURRENT_SUB_LOG__
#define __DEBUG_CURRENT_SUB_LOG__ __LOG_ALL__
#endif /* NOT __CURRENT_SUB_LOG__ */

#ifndef __DEBUG_LOG_SUB_SYSTEM__
#define __DEBUG_LOG_SUB_SYSTEM__ __LOG_ALL__
#endif /* NOT __DEBUG_LOG_SUB_SYSTEM__ */

#if ((__DEBUG_LOG_SYSTEM__ & __DEBUG_CURRENT_LOG__)                           \
    && (__DEBUG_LOG_SUB_SYSTEM__ & __DEBUG_CURRENT_SUB_LOG__))

#define dprintf(...)                                                          \
  color_printf(__DEBUG_FONT_COLOR__, __VA_ARGS__);

#define qprintf(...)                                                          \
  color_printf(__DEBUG_FONT_COLOR__, __VA_ARGS__);

#ifdef __DEBUG_HEADER__

#undef dprintf
#define dprintf(...)                                                          \
  __print_header(__DEBUG_HEADER__);                                           \
  color_printf(__DEBUG_FONT_COLOR__, __VA_ARGS__);

#ifdef __DEBUG_SUB_HEADER__

#undef dprintf
#define dprintf(...)                                                          \
  dbg_printf("[%s %s%s::%s%s %s] :: ",                                        \
    __DEBUG_HEADER_COLOR__, __DEBUG_HEADER__, __FG_RESET__,                   \
    __DEBUG_SUB_HEADER_COLOR__, __DEBUG_SUB_HEADER__, __FG_RESET__);          \
  color_printf(__DEBUG_FONT_COLOR__, __VA_ARGS__);

#endif /* __DEBUG_SUBSYSTEM_HEADER */
#endif /* _DEBUG_HEADER */

#else
#define dprintf(...)  { __dbg_unuse(__VA_ARGS__); }
#define qprintf(...)  { __dbg_unuse(__VA_ARGS__); }
#endif /* __DEBUG_LOG_SYSTEM__ */

#ifdef __DEBUG_HEADER__
#undef pr_err
#define pr_err(...)                                                           \
  __print_header(__DEBUG_HEADER__);                                           \
  color_printf(__FG_RED__, __VA_ARGS__);
#endif  /* __DEBUG_HEADER */

#ifdef __DEBUG_SUB_HEADER__
#undef pr_err
#define pr_err(...)                                                           \
  dbg_printf("[%s %s%s::%s%s %s] :: ",                                        \
    __DEBUG_HEADER_COLOR__, __DEBUG_HEADER__, __FG_RESET__,                   \
    __DEBUG_SUB_HEADER_COLOR__, __DEBUG_SUB_HEADER__, __FG_RESET__);          \
  color_printf(__FG_RED__, __VA_ARGS__);
#endif /* __DEBUG_SUB_HEADER */

#define pr_extra(...)                                                         \
  __dbg_unuse(__VA_ARGS__);

#if __LOG_EXTRA__
#undef  pr_extra
#define pr_extra(...)   dprintf(__VA_ARGS__)
#endif /* __LOG_EXTRA__ */

#endif /* NOT _QEMU_H */
