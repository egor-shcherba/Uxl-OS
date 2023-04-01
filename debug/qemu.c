#include <debug/qemu.h>
#include <sys/io.h>
#include <klib/vsprintf.h>

#define QEMU_PORT 0xE9
#define BUFSIZE   2048

static char buf[BUFSIZE] = { 0x0 };

static void
puts(const char *s)
{
  while (*s != '\0')
    outb(QEMU_PORT, *s++);
}

void
dbg_printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  vsprintf(buf, fmt, ap);
  va_end(ap);

  puts(buf);
}
